#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "linuxCommands.h"

//Gusty, the code for my shell is so cool that I want you to read it!

#define MAX_LINE 80 /* The maximum length command */

int valid_command(char *string);
void sigint_handler();
int getLength(char **string);
void redirectOutToFile(char* filename, char* text);

//array to store strings
char *commands[7] = {"ls", "cd", "pwd", "exit", "grep", "cat","help"};

int main(void) {
    char *args[MAX_LINE / 2 + 1]; /* command line arguments */
    int argc = 0; //counter for number of arguments
    int line = 0; //line number printed for every command prompt
    int numCommands = getLength(commands);
    int background = 0; //flag for background processes
    int haspipe = 0; //flag for pipe processes

    while (1) {
        signal(SIGINT, sigint_handler); //used to catch and ignore ctrl c
        //print the prompt with the line number
        printf("teenysh%d> ", line++);
        fflush(stdout);

        char input[MAX_LINE];

        //checks to see if the user entered a command
        //if the command is ctrl d, exit the program
        if (fgets(input, MAX_LINE, stdin) == NULL  && feof(stdin)) {
            printf("\n");
            exit(0);
        }

        input[strlen(input) - 1] = '\0'; // remove the trailing newline character

        char *token = strtok(input, " ");
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
            argc++;
        }
        args[i] = NULL; // set the last element to null pointer

        //check to see if args contains a > symbol
        int redirectOutput = 0;
        i = 0;
        while (args[i] != NULL) {
            if (strcmp(args[i], ">") == 0) {
                redirectOutput = 1;
                break;
            }
            i++;
        }

        //split args check modifier
        for (i = 0; i < argc; i++) {
            if (strcmp(args[i], "|") == 0) {
                haspipe = 1;
            }
        }

        //check for multiple args, replacement values
        char *repargs[MAX_LINE / 2 + 1];
        int repargc = 0;
        int fint;
        if (haspipe == 1) {

            //printf("%d", argc);
            //read and fork if pipe symbol
            for (i = 0; i < argc; i++) {
                //check if arg is pipe
                if (strcmp(args[i], "|") == 0) {
                    //create new process
                    fint = fork();
                    //erase replacement values
                    for (int j = 0; j < repargc; j++) {
                        repargs[j] = NULL;
                    }
                    repargc = 0;
                } else {
                    repargs[repargc] = args[i];
                    repargc++;
                }
            }

            //initialize forked pipe process
            if (fint == 0) {
                for (i = 0; i < (MAX_LINE / 2); i++) {
                    args[i] = NULL;
                }
                for (i = 0; i < repargc; i++) {
                    args[i] = repargs[i];
                }
                argc = repargc;
            }
                //erase arguments after pipe in parent process from args array
            else {
                int erase = 0; //false by default
                for (i = 0; i < argc; i++) {
                    if (erase == 1) {
                        args[i] = NULL;
                    } else if (strcmp(args[i], "|") == 0) {
                        args[i] = NULL;
                        argc = i;// - 1;
                        erase = 1;
                        //break;
                    } else {
                        //this spot intentionally left blank
                    }
                }
            }
        }

        char *outfile = args[i - 1]; //get filename of output file if specified

        //if the user enters nothing, continue
        if (strlen(input) == 0) {
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(args[0], "help") == 0) {
            printf("Commands:\n");
            for (int j = 0; j < numCommands; j++) {
                printf("%s\n", commands[j]);
            }
        } else if (strcmp(input, "pwd") == 0) {
            char cwd[100];
            char *path = getcwd(cwd, sizeof(cwd));
            if (redirectOutput == 1) {
                //get filename
                redirectOutToFile(outfile, path);
            }
            else if (path != NULL) {
                printf("%s\n", path);
            } else {
                perror("getcwd() error");
            }
        } else if (strncmp(input, "cd", 2) == 0) {
            char *path = input + 3; // skip the "cd " prefix
            if (chdir(path) == 0) {
                if (redirectOutput == 1) {
                    redirectOutToFile(outfile, path);
                }
                else {
                    printf("Changed directory to: %s\n", path);
                }
            }
            else {
                perror("chdir() error");
            }
        }
        //base case for commands such as ls, cat
        else {
            pid_t pid = fork();
            if (pid == 0) {
                //check for & symbol in args
                if (strcmp(args[argc - 1], "&") == 0) {
                    background = 1;
                    //remove & from args
                    args[argc - 1] = NULL;
                }
                if (redirectOutput == 1) {
                    char *result;
                    if (strcmp(args[0], "ls") == 0) {
                        char cwd[100];
                        char *path = getcwd(cwd, sizeof(cwd));
                        result = my_ls(path);
                        //printf("%s\n", result);
                        redirectOutToFile(outfile, result);
                    } else if (strcmp(args[0], "cat") == 0){
                        char *filename = args[1]; //file to be printed
                        result = my_cat(filename);
                        redirectOutToFile(outfile, result);
                    }
                    exit(1);
                }
                if (strcmp(args[0], "ls") == 0) {
                    char cwd[100];
                    char *path = getcwd(cwd, sizeof(cwd));
                    printf("%s\n", my_ls(path));
                }
                else if (strcmp(args[0], "cat") == 0){
                    char *filename = args[1]; //file that is to be printed
                    char *output = my_cat(filename);
                    printf("%s\n", output);
                }
                if (valid_command(args[0]) == 0) {
                    printf("Unknown command: %s\n", args[0]);
                }
            } else {
                //run process in background
                if (strcmp(args[argc - 1], "&") == 0 || background == 1) {
                    continue;
                }
                else {
                    wait(NULL);
                }
            }
            if (background != 1) {
                exit(0); //exit child process
            }
        }

        //wait for child process to finish
        if (haspipe == 1) {
            wait(NULL);
        }
        //clear input buffer for next command
        memset(input, 0, sizeof(input));


        if (fint == 0 && haspipe == 1) {
            exit(0);
        }

        haspipe = 0; //reset haspipe
        argc = 0; //reset argc

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

//function that redirects output to a file
void redirectOutToFile(char* filename, char* text) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "%s", text);
    fclose(f);
}