#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "linuxCommands.h"

#define READ_END 0
#define WRITE_END 1

char *my_ls(char *dir) {
    int pipefd[2];
    pid_t pid;
    char buffer[1024];
    char* output = malloc(1); // initialize output string with null terminator

    // create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // fork a child process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // child process: redirect stdout to the write end of the pipe
        close(pipefd[READ_END]);
        dup2(pipefd[WRITE_END], STDOUT_FILENO);
        close(pipefd[WRITE_END]);

        execvp("ls", NULL);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // parent process: read from the read end of the pipe and store output in a string
        close(pipefd[WRITE_END]);
        while (read(pipefd[READ_END], buffer, sizeof(buffer)) != 0) {
            output = realloc(output, strlen(output) + strlen(buffer) + 1);
            strcat(output, buffer);
        }
        close(pipefd[READ_END]);
    }

    return output; // return the output string
}

char *my_cat(char *filename) {
    FILE *fp;
    char *output = NULL;
    char buffer[1024];
    int size = 0, n;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        return NULL;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        output = realloc(output, size + n + 1);
        if (output == NULL) {
            fclose(fp);
            fprintf(stderr, "Error: Memory allocation failed\n");
            return NULL;
        }
        memcpy(output + size, buffer, n);
        size += n;
    }

    fclose(fp);
    if (size > 0) {
        output[size] = '\0';
    }
    return output;
}