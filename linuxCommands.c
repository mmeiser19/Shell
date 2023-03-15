#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "linuxCommands.h"

char *my_ls(char *path) {
    DIR* directory;
    struct dirent* entry; // Directory entry
    char* result = NULL;
    int result_size = 0; // The size of the result array
    int current_size = 0; // The current size of the result array

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
            result = realloc(result, result_size); // Resize the result array
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
        output = realloc(output, size + n + 1); // Resize the output buffer
        if (output == NULL) {
            fclose(fp);
            fprintf(stderr, "Error: Memory allocation failed\n");
            return NULL;
        }
        memcpy(output + size, buffer, n); // Copy the buffer to the output
        size += n;
    }

    fclose(fp);
    if (size > 0) {
        output[size] = '\0'; // Add a terminating null character
    }
    return output;
}