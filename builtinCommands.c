#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "builtinCommands.h"

/*
//flush(stdout) after child prints something
You should also add a new built-in command wait, which waits until all background jobs have terminated before returning to the prompt.
You can assume that there will always be spaces around the & character. 
You can assume that, if there is a & character, it will be the last token on that line. 

wait builtin
- repeatedly call wait untill therse no more children. wait throws an error when this happens

*/

int builtInCommands(char** argArr, int argn, char dir[]){
    
    if (strcmp(argArr[0], "exit") == 0) {
        return(exitfunc(argArr, argn));
    }
    else if (strcmp(argArr[0], "cd") == 0) {
        return(cdCommand(argArr, argn));
    }else if(strcmp(argArr[0], "pwd")==0){
        return(printpwd(argArr, argn));
    }else if (strcmp(argArr[0], "help") ==0){
        return(help(argn, dir));
    }
    else if (strcmp(argArr[0], "wait") ==0){
        return(wait1(argArr, argn));
    }
    
    
    return 2; //this program assumes that command is builtin, this returns 2 if it isn't. Then program exec will run
}

int wait1(char **argArr, int argn){
    if(argn >1){
        printf("Too many arguments for wait command\n");
        return 1;
        //while ((wpid = wait(&status)) > 0);
    }
    while (wait(NULL) > 0){
        ;
    }
    return 0;

}

int cdCommand(char** argArr, int argn){
    if(argn >2){
        printf("Too many arguments for cd command\n");
        return 1;
    }if(argn ==1){
        printf("Too little arguments for cd command\n");
        return 1;
    }

    if(chdir(argArr[1])==-1){
        printf("Error, directory not found\n");
        return 1;
    }return 0;
}
int exitfunc(char** argArr, int argn){
    if(argn>1){
        printf("Too many arguments for exit command\n");
        return 1;
    }
    exit(0);

}
int printpwd(char** argArr, int argn){
     if(argn >1){
        printf("Too many arguments for pwd command\n");
        return 1;
    }
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 0;
}

int help(int argn, char dir[]){
    if(argn >1){
        printf("Too many arguments for help command\n");
        return 1;
    }

    FILE *fp;

    fp = fopen(dir, "r");

    
    if(fp == NULL){
        printf("Error file not found\n");
        return 1;
    }
    char c;
    while((c = fgetc(fp))!= EOF){
        putchar(c);
    }
    puts(" ");
    return 0;

}