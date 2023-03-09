#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "linuxCommands.h"
#include <signal.h>

#define MAX_LINE 80 /* The maximum length command */
//array to store strings
char *commands[11] = {"ls", "cd", "pwd", "exit", "grep", "cat",
                      "&", ">", "<", "|", "help"};

int valid_command(char *string);
void sigint_handler(int sig);


int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    int line = 0;

    while (should_run) {
        signal(SIGINT, sigint_handler); //used to catch and ignore ctrl c
        //print the prompt with the line number
        printf("teenysh%d> ", line++);
        fflush(stdout);

        char input[MAX_LINE];

        //checks to see if the user entered a command
        //if the command is ctrl d, exit the program
        if (fgets(input, MAX_LINE, stdin) == NULL) {
            if (feof(stdin)) {
                // EOF detected, exit the program
                printf("\n");
                exit(0);
            }
            // Some other error occurred, print error message and continue
            continue;
        }

        input[strlen(input) - 1] = '\0'; // remove the trailing newline character

        char *token = strtok(input, " ");
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // set the last element to null pointer

        //if the user enters nothing, continue
        if (strlen(input) == 0) {
            continue;
        }
        else if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
        }
        else if (strcmp(args[0], "help") == 0) {
            printf("Commands:\n");
            for (int j = 0; j < 11; j++) {
                printf("%s\n", commands[j]);
            }
        }
        else if (strcmp(input, "pwd") == 0) {
            char cwd[100];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            }
        }
        else if (strncmp(input, "cd", 3) == 0) {
            char *path = input + 3; // skip the "cd " prefix
            if (chdir(path) == 0) {
                printf("Changed directory to: %s\n", path);
            } else {
                perror("chdir() error");
            }
        }
        else if (strcmp(input, "ls") == 0) {
            my_ls(".");
        }
        else if (strcmp(input, "cat") == 0) {
            //read in next argument as filename
            char *filename = args[1];
            //print_file(filename);
            my_cat(filename);
        }
        else {
            pid_t pid = fork();
            if (pid < 0) { /* error occurred */
                fprintf(stderr, "Fork failed");
                return 1;
            } else if (pid == 0) { /* child process */
                execvp(args[0], args);
                if (valid_command(args[0]) == 0) {
                    fprintf(stderr, "Unknown command: %s\n", args[0]);
                }
                return 0;
            } else { /* parent process */
                wait(NULL);
            }
        }
        //***** NOT SURE IF NECESSARY, KEEP FOR NOW *****
        //clear input buffer for next command
        memset(input, 0, sizeof(input));
    }
    return 0;
}

int valid_command(char *string) {
    for (int i = 0; i < 11; i++) {
        if (strcmp(string, commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void sigint_handler(int sig) {
    //Do nothing; ignore the signal
    printf("\n");
}
