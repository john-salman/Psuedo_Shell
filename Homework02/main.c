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

void handle_input(char *input);
void handle_output(char *output);
void handle_pipe(char *command);

void handle_error_file(const char *message, char *file); // incase the error happens openin\
g/closing a file
void handle_error(const char *message);

int pfd[2];

int main() {
  // Buffer for reading one line of input
  char line[MAX_LINE_CHARS];
  char* line_words[MAX_LINE_WORDS + 1];

  // Loop until user hits Ctrl-D (end of input)
  // or some other input error occurs
  while( fgets(line, MAX_LINE_CHARS, stdin) ) {
    int redirect = 0; // flag to use when we've reached the end of the command that can be passed to exec
    int comm_count = 0; // tracks how many commands (always +1 greater)
    int num_words = split_cmd_line(line, line_words);
    char *command[MAX_LINE_WORDS + 1];
    // Just for demonstration purposes
    for (int i=0; i < num_words; i++) {
      printf("%s\n", line_words[i]);
      if (strcmp(line_words[i], "<") == 0) {
        redirect = 1; // we dont want to keep anymore of the input in command[]
        handle_input(line_words[i + 1]);
      }
      if (strcmp(line_words[i], ">") == 0) {
        redirect = 1;
        handle_output(line_words[i + 1]);
      }
      if (strcmp(line_words[i], "|") == 0) {
        redirect = 1; // this might not be neccessary / it might be contradictory to what we want to do
                      // maybe this variable should be global? idk how yall wanna do pipes
        handle_pipe(line_words[i + 1]);
      }
      if (redirect == 0) {
        command[i] = line_words[i];
        comm_count += 1;
      }

    }
    // read in the input to line_words, run command using fork & exec
    pid_t chile = fork();
    if (chile == 0) {
      command[comm_count] = (char*)NULL; // set the last value to NULL
      execvp(command[0], command);
    }
    wait(0);

  }



  return 0;
}

void handle_input(char *input) {
  printf("%s\n", input); // for testing
  int infile = open(input, O_RDONLY);
  // error checking and i/o redirect below
  if (infile == -1) {
    handle_error_file("Could not open input file: ", input);
  }
  if (dup2(infile, 0) < 0) { // i found this < notation in online documentation, will errors return anything less than -1?
    handle_error_file("Error redirecting input with file: ", input);
  }
  if (close(infile) == -1) {
    handle_error_file("Could not close input file: ", input);
  }
  return;
}

void handle_output(char *output) {
  printf("%s\n", output);
  int outfile = open(output, O_WRONLY | O_TRUNC, 0777); // idk if we need to set permission bits
  if (outfile == -1) {
    handle_error_file("Could not open output file: ", output);
  }
  if (dup2(outfile, 1) < 0) {
    handle_error_file("Error redirecting output with file: ", output);
  }
  if (close(outfile) == -1) {
    handle_error_file("Could not close output file: ", output);
  }
  return;
}

void handle_pipe(char *command){
  return;
}

// The 2 functions below are modified from the error function in pipe_demo.c
void handle_error_file(const char *message, char *file) {
  extern int errno;

  fprintf( stderr, "%s%s\n", message, file );
  fprintf( stderr, " (%s)\n", strerror(errno) );
  exit( 1 );
}

void handle_error(const char *message) {
  extern int errno;

  fprintf( stderr, "%s\n", message);
  fprintf( stderr, " (%s)\n", strerror(errno) );
  exit( 1 );
}

