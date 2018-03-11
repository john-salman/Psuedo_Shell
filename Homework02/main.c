#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    char* command[MAX_LINE_WORDS + 1];
    char command_list[100][MAX_LINE_WORDS][MAX_LINE_CHARS];
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs

    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);


        int n = 0;
        int num_commands = 0;
        // Just for demonstration purposes
        for (int i=0; i < num_words; i++) {
            // printf("%s\n", line_words[i]);
            // look for a pipe '|'
            if (!strncmp(line_words[i], "|", 1)) {

              for(int j = 0; j < n; j++) {
                  strcpy(command_list[num_commands][j], line_words[j]);
              }

              int length = strlen(command_list); // data filled correctly
              // strlen doesn't work correctly
              printf("length of command: %d\n", length);
              printf("length of n: %d\n", n);
              num_commands += 1;
              n = 0;
            }
            else {
              n += 1; // size of command
            }
        }
        printf("steiner\n");
        for(int k = 0; k < num_commands; k++) {
          int length = strlen(*command_list[k]);
          printf("length of command: %d\n", length);
          for (int l = 0; l < 4; l++) {
            printf("command read: %s\n", command_list[k][l]);
          }
        }

/*
        // read in the input to line_words, run command using fork & exec
        pid_t child = fork();
        if (child == 0) {
          line_words[num_words] = (char*)NULL;
          execvp(line_words[0], line_words);
        }
        wait(0);
*/

    }




    return 0;
}
