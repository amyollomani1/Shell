
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>


#include "execProgramHelper.h"
#include "helpers.h"

//BUG:::::
//  /bin/l
//  exit
//output: success with executing programhelper

//BUG FOUND: 
//executeProgram is not returning what its supposed to return
//IDEA:::::
//path func returns the correct filename that should be in argvec[0]
//argvecHelper uses this to create the exectuable array
//then mainExec calls exec.


/*

mainExec is called by  parseCommands() in shell.c
First it creates a char **array called argVec. It then calls argVecHelper(argVec, argArr, 0, argnNum),
which copies relevant indecies from argArr, and adds NULL to the end - thus making argVec executable with execv
Then mainExec calls is_openable_file, which tests to see if argVec[0]- the command, is an openable file
If not, it calls parsingPATH(argVec), which takes argVec as input and returns a char* file with the command and PATH input.
if no PATH is found to corrleate with the command being inputed, then parsingPATH returns NULL, and mainExec returns 1

argVec[0], is updated to hold the new PATH if one is found, and then mainExec calls executeProgram(argVec, back_round)


Note: mainExec returns 1 on failure, caused when the command inputed can't be found in PATH, otherwise it returns 0.

*/

int mainExec(char **argArr, int processArgsNum, int back_round){
    //puts("mainExec");
    //int result = 1;

    char *argVec[processArgsNum+1];

    //puts("here");
    //puts null at end of argument list
    argVecHelper(argVec, argArr, 0, processArgsNum);

    //puts("HEREERERER");
    if(is_openable_file(argVec[0])==0){
       // puts("On te inside");
        char *file = parsingPATH(argVec);
        if(file != NULL){
            argVec[0] = file;
            //printf("FilePath after exec: %s\n", argVec[0]);

            executeProgram(argVec, back_round);
             //put("After exec");  
        }else{
            return 1;
        }    
        //free(file);
    }else{
        executeProgram(argVec, back_round);
    }
   
    return 0;
}


/*
argVecHelper copies the strings in argArr to argVec, adding a NULL to the end of argVec
Note that this function can copy from argArr at any index, say starting at 0 and ending at 2, even if there are 5 arguments from user input
    This features was made to make parsing with redirection and pipes easier
*/
void argVecHelper(char *argVec[], char**argArr, int start, int argn){
    for(int i = 0; i<argn+1; i++){
        if(i== argn){
            argVec[argn] = NULL;
        }else {
            argVec[i] = argArr[i+start];
        }
      // printf("string %s\n", argVec[i]);
    }
}

//change parsingPATH to include forloop thats above to change 0 index

/*

parsingPATH takes argVec as input and returns a file holding the command with its PATH.
For example, if argVec[0] = ls, then this function returns /bin/ls

parsingPATH first allocates memory to envPath, which holds getenv("PATH");. It then copies envPATH to another variable called PATH.
strtok is used on PATH to seperate the directories between :

Then, strtok is called while (strtok(NULL, ": ")!=NULL);
During each iteration of the while loop, strcat is used to combine PATH and argVec[0] (which holds the command). This is stored in char *file
Then is_openable_file(file) is called, which returns 1 if file is openable. In this case, parsingPATH returns file
Otherwise, if is_openable_file never returns 1 (indicating that command does not belong to anything in PATH), char *parsingPATH returns NULL

*/
 char* parsingPATH(char **argVec){ 

   // puts("in parsingPATH");
    char *envPath = malloc(sizeof(char) * 2048);
    char *file=malloc(sizeof(char)*20);
    //file = NULL;
    if(envPath== NULL ){
      //printf("was't Couldn't able to allocate requested memory");
    }
    envPath = getenv("PATH");
    char *PATH = malloc(sizeof(char) * 2048);
    strncpy(PATH, envPath, 2047);
    //char *cmd = malloc(sizeof(char) * 2048);
    //strncpy(cmd, PATH, 2047);

    //int res = 1;
   // printf("PATH: %s", PATH);
    strtok(PATH, ":");
    int count = 0;
   // printf("\n\nafter original\n");
    strcpy(file, PATH);
    strcat(file, "/");
    strcat(file,argVec[0]);
    if(is_openable_file(file)==1){
            return file;
    }
 
    while((PATH = strtok(NULL,":"))!=NULL){
        //char *finalPath = malloc(sizeof(char) * 2048);
        count++;
        //printf("PATH: %s\n", PATH);
        strcpy(file, PATH);
        strcat(file, "/");
        //puts("after");
        if(file==NULL){
            free(file);
            file = NULL;
            return file;
        }
        strcat(file,argVec[0]);
       // printf("newPath: %s\n", file);
        //argVec[0] = cmd;
       if(is_openable_file(file)==1){
            return file;
       }
          
    }

    //free(envPath);
    //free(PATH);

    return NULL;
}


//returns 0 when file is not openable, it returns 1 when true
//This function is mainly used as helper to parsingPATH, and to check if command is valid before calling execv.
int is_openable_file(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}


/*
executeProgram takes input char *argVec[] and int back_round.
execute program first calls fork(). The child calls execv(argVec[0], argVec). If execv fails, child exits.
The parent calls waitpid to wait for process.
Note that when int back_round = 1, then & is at end of command. Thus, waitpid is called with 
flag WNOHANG to allow parent process to continue

*/
int executeProgram(char *argVec[], int back_round){

   // puts("execProgramin Helper");
    int pid = fork();
    if(pid==-1){
        puts("fork() failed");
        return 1;
    }
    if(pid == 0){
        execv(argVec[0], argVec);
        //printf("This shouldn't print since exec replaces everything in child process\n");
        exit(3);

    }else{
       // int wstatus;
       // wait(&wstatus); //this code checks return of child process to see if exec ran correctly
       int wstatus;
        if(back_round == 0){
            waitpid(pid, &wstatus, 0);

        }else{
            waitpid(pid, &wstatus, WNOHANG);  
        }
        //fflush(STDOUT_FILENO);
        
    }

    fflush(stdout);

    return 0;
   
}

/*
This function is used by pipe.c. It is passed argArr, which holds all arguments inputed, and the start, end index and size.
This function creates an argVec array that can be used with execv. It uses parameters start,end and size to create the size of the array and
to copy the relevant argument indecies.
This function also tests to ensure that the command, which is found in array_ptr, is executable. If not, it calls parsingPATH to return the full filepath.

*/
char** isCommand(char**argArr, int startInd, int endInd, int size){
    //argVecHelper(argVecP2, argArr, pIndex+1, size-1);
    char **array_ptr = (char**) malloc(size * sizeof(char**));
    for(int i = 0; i < size; i++) {
        array_ptr[i] = (char*) malloc(250 * sizeof(char));
    }
    argVecHelper(array_ptr, argArr, startInd, endInd);

    if (is_openable_file(array_ptr[0]) == 0){
       //puts("not obpenable");
        char* file = parsingPATH(array_ptr);    

        //int res = parsingPATH(argArr, *file2);
        if(file != NULL){
            array_ptr[0] = file;
            //printf("Now execuablte, argVec[0]: %s", array_ptr[0]);
        }else{
            array_ptr[0] = NULL;

        }
        //free(file);
    }else{
        //puts("openable");
    }
    return array_ptr;
}


