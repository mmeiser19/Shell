#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "linuxCommands.h"

// Gusty, the code for my shell is so cool that I want you to read it!

#define MAX_INPUT 80 /* The maximum length command */

int valid_command(char *string);
void sigint_handler();
int getLength(char **string);
void redirectOutToFile(char* filename, char* text);

// List of valid commands
char *commands[7] = {"ls", "cd", "pwd", "exit", "grep", "cat","help"};

int main(void) {
    char *args[MAX_INPUT / 2 + 1]; // Command line arguments
    int argc = 0; // Counter for number of arguments
    int line = 0; // Line number printed for every command prompt
    int numCommands = getLength(commands);
    int background = 0; // Flag for background processes
    int haspipe = 0; // Flag for pipe processes

    while (1) {
        signal(SIGINT, sigint_handler); // Used to catch and ignore ctrl c

        // Print the prompt with the line number
        printf("teenysh%d> ", line++);
        fflush(stdout);

        char input[MAX_INPUT]; // Input buffer

        // Checks to see if the user entered a command
        // If the command is ctrl d, exit the program
        // Ignores cases of no input from the user
        if (fgets(input, MAX_INPUT, stdin) == NULL  && feof(stdin)) {
            printf("\n");
            exit(0);
        }

        input[strlen(input) - 1] = '\0'; // Remove the trailing newline character

        char *token = strtok(input, " ");
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
            argc++;
        }
        args[i] = NULL; // Set the last element to null pointer

        // Check to see if args contains a > symbol
        int redirectOutput = 0;
        i = 0;
        while (args[i] != NULL) {
            if (strcmp(args[i], ">") == 0) {
                redirectOutput = 1;
                break;
            }
            i++;
        }

        // Check to see if args contains a | symbol
        for (i = 0; i < argc; i++) {
            if (strcmp(args[i], "|") == 0) {
                haspipe = 1;
            }
        }

        // Check for multiple args, replacement values
        char *repargs[MAX_INPUT / 2 + 1];
        int repargc = 0;
        int fint; // Forked int
        if (haspipe == 1) {
            // Read and fork if pipe symbol
            for (i = 0; i < argc; i++) {
                // Check if arg is pipe
                if (strcmp(args[i], "|") == 0) {
                    // Create new process
                    fint = fork();
                    // Erase replacement values
                    for (int j = 0; j < repargc; j++) {
                        repargs[j] = NULL;
                    }
                    repargc = 0;
                }
                else {
                    repargs[repargc] = args[i];
                    repargc++;
                }
            }

            // Initialize forked pipe process
            if (fint == 0) {
                for (i = 0; i < (MAX_INPUT / 2); i++) {
                    args[i] = NULL;
                }
                for (i = 0; i < repargc; i++) {
                    args[i] = repargs[i];
                }
                argc = repargc;
            }

            // Erase arguments after pipe in parent process from args array
            else {
                int erase = 0; // False by default
                for (i = 0; i < argc; i++) {
                    if (erase == 1) {
                        args[i] = NULL;
                        break;
                    } else if (strcmp(args[i], "|") == 0) {
                        args[i] = NULL;
                        argc = i;// - 1;
                        erase = 1;
                    } else {
                        continue;
                    }
                }
            }
        }

        char *outfile = args[i - 1]; // Get filename of output file if specified

        if (strlen(input) == 0) { // If the user entered no input, continue
            continue;
        }
        else if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }
        else if (strcmp(args[0], "help") == 0) { // Print the list of valid commands
            printf("Commands:\n");
            for (int j = 0; j < numCommands; j++) {
                printf("%s\n", commands[j]);
            }
        } else if (strcmp(input, "pwd") == 0) { // Print the current working directory
            char cwd[100];
            char *path = getcwd(cwd, sizeof(cwd));
            if (redirectOutput == 1) {
                redirectOutToFile(outfile, path);
            }
            else if (path != NULL) {
                printf("%s\n", path);
            } else {
                perror("getcwd() error");
            }
        } else if (strncmp(input, "cd", 2) == 0) { // Change the current working directory
            char *path = input + 3; // Skip the "cd " prefix
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
                if (redirectOutput == 1) { // Redirect output to a file
                    char *result;
                    if (strcmp(args[0], "ls") == 0) { // Print the contents of the current directory
                        char cwd[100];
                        char *path = getcwd(cwd, sizeof(cwd));
                        result = my_ls(path);
                        redirectOutToFile(outfile, result);
                    }
                    else if (strcmp(args[0], "cat") == 0){ // Print the contents of a file
                        char *filename = args[1]; // File to be printed
                        result = my_cat(filename);
                        redirectOutToFile(outfile, result);
                    }
                    exit(1);
                }
                if (strcmp(args[0], "ls") == 0) { // Print the contents of the current directory
                    char cwd[100];
                    char *path = getcwd(cwd, sizeof(cwd));
                    printf("%s\n", my_ls(path));
                }
                else if (strcmp(args[0], "cat") == 0){ // Print the contents of a file
                    char *filename = args[1]; // File that is to be printed to the screen
                    char *output = my_cat(filename);
                    printf("%s\n", output);
                }
                if (valid_command(args[0]) == 0) { // Check if the command is valid
                    printf("Unknown command: %s\n", args[0]);
                    printf("Enter 'help' to see a list of valid commands.\n");
                }
            } else {
                // Run process in background
                if (strcmp(args[argc - 1], "&") == 0 || background == 1) {
                    continue;
                }
                else {
                    wait(NULL);
                }
            }
            if (background != 1) {
                exit(0); // Exit child process
            }
        }

        // Wait for child process to finish if there is a pipe
        if (haspipe == 1) {
            wait(NULL);
        }
        // Clear input buffer for next command
        memset(input, 0, sizeof(input));

        // If there is a pipe, exit the child process
        if (fint == 0 && haspipe == 1) {
            exit(0);
        }

        haspipe = 0; // Reset haspipe
        argc = 0; // Reset argc

    }

}

// Returns an int representing if the command is valid
int valid_command(char *string) {
    int numCommands = getLength(commands);
    for (int i = 0; i < numCommands; i++) {
        if (strcmp(string, commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Returns the length of the provided string
int getLength(char **string) {
    int length = 0;
    while (string[length] != NULL) {
        length++;
    }
    return length;
}

// Handle SIGINT signal by ignoring it and printing a new line
void sigint_handler() {
    //Do nothing; ignore the signal
    printf("\n");
}

// Redirect output to a file
void redirectOutToFile(char *filename, char *text) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "%s", text);
    fclose(f);
}