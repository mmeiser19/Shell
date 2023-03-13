#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

//Gusty, the code for my shell is so cool that I want you to read it!

#define MAX_LINE 80 /* The maximum length command */

int valid_command(char *string);
void sigint_handler();
int getLength(char **string);
int checkRedirectOutput(char **args);
void redirectOutput(char **args, char *command);

//array to store strings
char *commands[7] = {"ls", "cd", "pwd", "exit", "grep", "cat","help"};

int main(void) {
    char *args[MAX_LINE / 2 + 1]; /* command line arguments */
    int argc = 0; //counter for number of arguments
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
            argc++;
        }
        args[i] = NULL; // set the last element to null pointer

        //split args check modifier
        int haspipe = 0;
        for (int i = 0; i < argc; i++) {
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
            for (int i = 0; i < argc; i++) {
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
                for (int i = 0; i < (MAX_LINE / 2); i++) {
                    args[i] = NULL;
                }
                for (int i = 0; i < repargc; i++) {
                    args[i] = repargs[i];
                }
                argc = repargc;
            }
                //erase arguments after pipe in parent process from args array
            else {
                int erase = 0; //false by default
                for (int i = 0; i < argc; i++) {
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
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                if (checkRedirectOutput(args) == 1) {
                    //open file
                    /*FILE *output;
                    output = fopen(args[i-1], "w");
                    fprintf(output, "%s", cwd);
                    fclose(output);*/
                    redirectOutput(args, cwd);
                } else {
                    printf("%s\n", cwd);
                }
            }
        } else if (strncmp(input, "cd", 2) == 0) {
            char *path = input + 3; // skip the "cd " prefix
            if (chdir(path) == 0) {
                printf("Changed directory to: %s\n", path);
            } else {
                perror("chdir() error");
            }
        } else if (strcmp(input, "grep") == 0) {
            //use the grep command
        }
            //base case for commands such as ls, cat
        else {
            if (fork() == 0) {
                if (checkRedirectOutput(args) == 1) {
                    //redirect output with execvp
                    i = 0;
                    while (args[i] != NULL) {
                        if (strcmp(args[i], ">") == 0) {
                            //open file
                            FILE *output;
                            output = fopen(args[i + 1], "w");
                            //print to file
                            fprintf(output, "%s", (char) execvp(args[0], args));
                            fclose(output);
                        }
                        i++;
                    }
                }
                    //if last arg is &, run in background
                else {
                    execvp(args[0], args);
                }
                //execvp(args[0], args);
                if (valid_command(args[0]) == 0) {
                    printf("Unknown command: %s\n", args[0]);
                }
            } else {
                //run process in background
                if (strcmp(args[argc - 1], "&") == 0) {
                    continue;
                } else {
                    wait(NULL);
                }
                // Add a newline at the end of the output
                putchar('\n');
            }
        }

        if (haspipe == 1) {
            wait(NULL);
        }
        //***** NOT SURE IF NECESSARY, KEEP FOR NOW *****
        //clear input buffer for next command
        memset(input, 0, sizeof(input));


        if (fint == 0 && haspipe == 1) {
            exit(0);
        }

        haspipe = 0;
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

//check if > command is in args
int checkRedirectOutput(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

//check if < command is in args
int checkRedirectInput(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

//function that redirects output to a file
void redirectOutput(char **args, char *command) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            //open file
            FILE *output;
            output = fopen(args[i+1], "w");
            //print to file
            fprintf(output, "%s", command);
            fclose(output);
        }
        i++;
    }
}

//function that redirects input from a file