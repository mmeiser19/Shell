#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include "linuxCommands.h"

#define READ_END 0
#define WRITE_END 1

char *my_ls(char *path) {
    /*int pipefd[2];
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
            printf("buffer size:%lu\n", sizeof(buffer));
            output = realloc(output, strlen(output) + strlen(buffer) + 1);
            strcat(output, buffer);
        }
        close(pipefd[READ_END]);
    }

    return output; // return the output string*/

    DIR* directory;
    struct dirent* entry;
    char* result = NULL;
    int result_size = 0;
    int current_size = 0;

    // Open the directory
    directory = opendir(path);
    if (directory == NULL) {
        perror("Unable to open directory");
        return NULL;
    }

    // Loop through the directory entries
    while ((entry = readdir(directory)) != NULL) {
        // Skip the current directory (".") and parent directory ("..")
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Resize the result array if necessary
        int name_len = strlen(entry->d_name);
        if (current_size + name_len + 2 > result_size) {
            result_size += 1024;
            result = realloc(result, result_size);
            if (result == NULL) {
                perror("Unable to allocate memory");
                closedir(directory);
                return NULL;
            }
        }

        // Add the entry name to the result array
        snprintf(result + current_size, name_len + 2, "%s\n", entry->d_name);
        current_size += name_len + 1;
    }

    // Close the directory
    closedir(directory);

    // Add a terminating null character to the result array
    if (result != NULL) {
        result[current_size] = '\0';
    }

    return result;
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