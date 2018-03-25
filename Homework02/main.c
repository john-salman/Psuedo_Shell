#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

void handle_input(char *input, char* cmd[], int size);
void handle_output(char *output, char* cmd[], int size);
void handle_pipe(char* command[], int size, int pfd[], int side);
void run_exec(char* command[], int size, int pfd[], int side);
int pipeFlagFlipper(int flag);

void handle_error_file(const char *message, char *file); // incase the error happens openin\
g/closing a file
void handle_error(const char *message);

int main() {
  // Buffer for reading one line of input
  char line[MAX_LINE_CHARS];
  char* line_words[MAX_LINE_WORDS + 1];

  // Loop until user hits Ctrl-D (end of input)
  // or some other input error occurs
  while( fgets(line, MAX_LINE_CHARS, stdin) ) {
    int redirect = 0;
    int comm_count = 0; // tracks how many commands (always +1 greater)
    int pipe_flag = 0;
    int pfd[2];
    int num_words = split_cmd_line(line, line_words);
    char *command[MAX_LINE_WORDS + 1];
    // Just for demonstration purposes
    for (int i=0; i < num_words; i++) {
      //printf("%s\n", line_words[i]);
      // still need to test input redirect
      if (strcmp(line_words[i], "<") == 0) {
        redirect = 1;
        handle_input(line_words[i + 1], command, comm_count);
        i+=2;
      }
      if (strcmp(line_words[i], ">") == 0) {
        redirect = 1;
        handle_output(line_words[i + 1], command, comm_count);
        i+=2;
      }

      // short circuit, so redirects nested in pipes don't seg fault.
      if (strcmp(line_words[i], "|") == 0) {
        //printf("%s\n", "I've seen a pipe");
        // check to make sure there is a command after the pipe.
        if (pipe_flag) {
          //redirect output of current commands
          handle_pipe(command, comm_count, pfd, 0); // middle pipe
        }
        else {
          if (pipe(pfd) == -1) {
            handle_error("Could not create a pipe");
          }
          run_exec(command, comm_count, pfd, pipeFlagFlipper(pipe_flag)); // 1
          pipe_flag = 1; // i've seen a pipe
        }

        comm_count = 0;
        i+=1; // skip the pipe character
        //wait(0);
      }

      if (i < num_words) {
        //printf("%d, %d num_words\n", i, num_words);
        //printf("storing: %s\n", line_words[i]);
        command[comm_count] = line_words[i];
        comm_count += 1;
      }

      if (((i == num_words-1) && pipe_flag == 1)) {
        //printf("inside last pipe if\n");
        //printf("%s\n", command[0]);

        run_exec(command, comm_count, pfd, 0);
        wait(0);
        if (close(pfd[0]) == -1)
            handle_error( "Parent could not close stdin" );
        if (close(pfd[1]) == -1)
            handle_error( "Parent could not close stdout" );
      }

    }

    if ((pipe_flag == 0) && (redirect == 0)) {
      run_exec(command, comm_count, NULL, -1);
    }
    while (wait(NULL) != -1);
  }

  return 0;
}

void run_exec(char* command[], int size, int pfd[], int side) {
  // redirects only one input. Only used for first and last
  pid_t child = fork();
  //printf("%s, %d child id\n", command[0], child);
  if (child == 0) {
    // left side of the pipe redirects output -- side == 1
    // right side of the pipe redirects input -- side == 0
    if (pfd != NULL) { // redirect the input
      //printf("doing pipe redirect\n");
      if (close(side) == -1) {
        handle_error("Could not close file redirect from pipe");
      }
      dup(pfd[side]);
      if (close(pfd[0]) == -1 || close(pfd[1]) == -1) {
        handle_error("Could not close pfd from pipe");
      }
    }
    //printf("running command: %s\n", command[0]);
    command[size] = (char*)NULL; // set the last value to NULL
    execvp(command[0], command);
  }
  return;
}

void handle_pipe(char* command[], int size, int pfd[], int side) {
  // redirects both inputs. used for middle pipe.
  pid_t child = fork();
  //printf("%s, %d child id from handle_pipe\n", command[0], child);
  if (child == 0) {
    // left side of the pipe redirects output -- side == 1
    // right side of the pipe redirects input -- side == 0
    if (pfd != NULL) { // redirect the input
      //printf("doing pipe redirect from handle_pipe\n");
      if (close(0) == -1) {
        handle_error("Could not close file redirect from pipe");
      }
      dup(pfd[0]);
      if (close(pfd[0]) == -1 || close(pfd[1]) == -1) {
        handle_error("Could not close pfd from pipe");
      }

      //CREATE NEW pipe
      if (pipe(pfd) == -1) {
          handle_error("Could not create a pipe");
      }

      if (close(1) == -1) {
        handle_error("Could not close file redirect from pipe");
      }
      dup(pfd[1]);
      if (close(pfd[0]) == -1 || close(pfd[1]) == -1) {
        handle_error("Could not close pfd from pipe");
      }

    }
    //printf("running command: %s\n", command[0]);
    command[size] = (char*)NULL; // set the last value to NULL
    execvp(command[0], command);
  }
  return;
}

void handle_input(char *input, char* cmd[], int size) {
  //printf("%s\n", input); // for testing
  int stdincpy = dup(0);
  int infile = open(input, O_RDONLY);
  // error checking and i/o redirect below
  if (infile == -1) {
    handle_error_file("Could not open input file: ", input);
  }
  if (dup2(infile, 0) < 0) { // i found this < notation in online documentation, will errors return anything less than -1?
    handle_error_file("Error redirecting input with file: ", input);
  }

  run_exec(cmd, size, NULL, -1);

  if (dup2(stdincpy, 0) < 0) {
    handle_error_file("Error redirecting output with file: ", input);
  }
  if (close(infile) == -1) {
    handle_error_file("Could not close input file: ", input);
  }
  return;
}

void handle_output(char *output, char* cmd[], int size) {
  //printf("%s\n", output);
  int stdoutcpy = dup(1);
  int outfile = open(output, O_WRONLY | O_TRUNC, 0777); // idk if we need to set permission bits
  //printf("opened file, %d\n", outfile);
  if (outfile == -1) {
    handle_error_file("Could not open output file: ", output);
  }

  if (dup2(outfile, 1) < 0) {
    handle_error_file("Error redirecting output with file: ", output);
  }

  run_exec(cmd, size, NULL, -1);

  if (dup2(stdoutcpy, 1) < 0) {
    handle_error_file("Error redirecting output with file: ", output);
  }
  if (close(outfile) == -1) {
    handle_error_file("Could not close output file: ", output);
  }
  return;
}

// The 2 functions below are modified from the error function in pipe_demo.c
void handle_error_file(const char *message, char *file) {
  extern int errno;

  printf( "%s%s\n", message, file );
  printf( " (%s)\n", strerror(errno) );
  exit( 1 );
}

void handle_error(const char *message) {
  extern int errno;

  printf( "%s\n", message);
  printf( " (%s)\n", strerror(errno) );
  exit( 1 );
}

int pipeFlagFlipper(int flag){
  return 1 - flag;
}
