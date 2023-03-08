//
// Created by megan on 3/8/23.
//

#ifndef SHELL_LINUXCOMMANDS_H
#define SHELL_LINUXCOMMANDS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#endif //SHELL_LINUXCOMMANDS_H

pid_t my_fork();

void my_exec(char *program, char *args[]);

void my_ls(char *dir);

void my_cat(char *filename);