#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define COMMAND_LINE_SIZE 1024 // max size command line


int main(){
    char cwd[COMMAND_LINE_SIZE];
    char newdir[COMMAND_LINE_SIZE];
    getcwd(cwd, COMMAND_LINE_SIZE);

    fprintf(stdout, "%s\n" , cwd);
    //char *dir = "test dir";
    char dir[COMMAND_LINE_SIZE];
    fgets(dir, COMMAND_LINE_SIZE, stdin);
    fprintf(stdout, "%s\n", dir);
    strcpy(newdir, cwd);
    strcat(newdir, "/");
    strcat(newdir, dir);
    fprintf(stdout, "%s\n", newdir);
    int n = chdir(newdir);
    fprintf(stdout, "chdir %i\n", n);
    getcwd(cwd, COMMAND_LINE_SIZE);
    fprintf(stdout, "%s\n" , cwd);

}