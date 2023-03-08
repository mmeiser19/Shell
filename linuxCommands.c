#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t my_fork() {
    // create a child process that is a copy of the current process
    pid_t pid = fork();
    if (pid == -1) {
        perror("clone() failed");
        exit(1);
    }

    // return the process ID of the child process in the parent process,
    // and 0 in the child process
    return pid;
}

void my_exec(char *program, char *args[]) {
    // replace the current process image with a new process image
    execvp(program, args);

    // if execvp() returns, it means the new process image could not be loaded
    perror("execvp() failed");
    exit(1);
}

void my_ls(char *dir) {
    // create a child process
    pid_t pid = my_fork();
    if (pid == 0) {
        // child process
        char *args[] = {"ls",dir, NULL};
        //execvp(args[0], args);
        my_exec(args[0], args);

        // if execvp() returns, it means the new process image could not be loaded
        perror("my_exec() failed");
        exit(1);
    } else {
        // parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

void my_cat(char *filename) {
    // create a child process
    pid_t pid = my_fork();
    if (pid == 0) {
        // child process
        char *args[] = {"cat", filename, NULL};
        my_exec(args[0], args);
    } else {
        // parent process
        int status;
        waitpid(pid, &status, 0);

        // Add a newline after the last line of the file
        putchar('\n');
    }
}