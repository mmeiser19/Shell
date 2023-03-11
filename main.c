#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define MAX_LINE 80 /* The maximum length command */

int valid_command(char *string);
void sigint_handler();
int getLength(char **string);

//array to store strings
char *commands[7] = {"ls", "cd", "pwd", "exit", "grep", "cat","help"};

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int line = 0; //line number printed for every command prompt
    int numCommands = getLength(commands);

    while (1) {
        signal(SIGINT, sigint_handler); //used to catch and ignore ctrl c
        //print the prompt with the line number
        printf("teenysh%d> ", line++);
        fflush(stdout);

        char input[MAX_LINE];

        //checks to see if the user entered a command
        //if the command is ctrl d, exit the program
        if (fgets(input, MAX_LINE, stdin) == NULL && feof(stdin)) {
            printf("\n");
            exit(0);
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
            exit(0);
        }
        else if (strcmp(args[0], "help") == 0) {
            printf("Commands:\n");
            for (int j = 0; j < numCommands; j++) {
                printf("%s\n", commands[j]);
            }
        }
        else if (strcmp(input, "pwd") == 0) {
            char cwd[100];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            }
        }
        else if (strncmp(input, "cd", 2) == 0) {
            char *path = input + 3; // skip the "cd " prefix
            if (chdir(path) == 0) {
                printf("Changed directory to: %s\n", path);
            } else {
                perror("chdir() error");
            }
        }
        //base case for commands such as ls, grep, and cat
        else if (fork() == 0) {
            execvp(args[0], args);
            if (valid_command(args[0]) == 0) {
                printf("Unknown command: %s\n", args[0]);
            }
        }
        else {
            wait(NULL);
            // Add a newline at the end of the output
            putchar('\n');
        }
        //***** NOT SURE IF NECESSARY, KEEP FOR NOW *****
        //clear input buffer for next command
        memset(input, 0, sizeof(input));
    }
}

int valid_command(char *string) {
    int numCommands = getLength(commands);
    for (int i = 0; i < numCommands; i++) {
        if (strcmp(string, commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int getLength(char **string) {
    int length = 0;
    while (string[length] != NULL) {
        length++;
    }
    return length;
}

void sigint_handler() {
    //Do nothing; ignore the signal
    printf("\n");
}
