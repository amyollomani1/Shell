

#include "execProgramHelper.h"
#include "execProgram.h"
#include "helpers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>


/*
RedirectMain is called by parsingCommandLine in shell.c
It is called when redirection is found in arguments.
First, redirectMain uses find_special()- which is inside helpers.c to check if both < and > is found in argArr.
    If so, char input[] and char output[] is created to store the input file and output file names. Then argVec is created, and is filled
    by calling argVecHelper, located in execProgramHelper.c
    if argVec[0] (which stores the command) is not an openable file, file = parsingPATH is called, which returns the correct file (or NULL on failure)
    if file = NULL, then redirectMain returns 1 on failure. Othersise, argVec[0] =file
    Then leftandRightRedirect(argVec, input, indexSpecial, output, back_round) is called to exec argVec, and to call dup2 on both files

Then redirectMain checks if < is inputed by user
    Similarily, input[] stores the input file name. argVec is created to store arguments from argArr. parsingPATH is called if argVec[0] is not a valid command
    if parsingPATH doesn't return NULL, leftREdirect(indexSpecial, argArr, input, back_round, argVec) is called.
    Otherwise redirectMain returns 1 since command is invalid

Then redirectMain checks if > is inputed by user
    Similarily, output[] stores the output file name. argVec is created to store arguments from argArr. parsingPATH is called if argVec[0] is not a valid command
    if parsingPATH doesn't return NULL, leftREdirect(indexSpecial, argArr, output, back_round, argVec) is called.
    Otherwise redirectMain returns 1 since command is invalid

Note: redirectMain returns 1 if execv fails and commandline arguments are invalid/ input file doesn't exist.
    redicretMain also erturns immediatly after calling leftRedirect, rightRedirect, and leftAndRightRedirect



*/
int RedirectMain(char **argArr, int argn, int back_round){
    //printf("PATH: %s", PATH);
   // puts("redirect main");
    int result = 1;
    int secondInt = -1; 
    int firstInt = -1;
    int indexSpecial = -1;
    //char *file =  malloc(sizeof(char) * 2048);


    if(((firstInt = find_special(argArr, "<"))!= -1) && ((secondInt = find_special(argArr, ">"))!= -1)){
       // puts("On inside");
        char input[strlen(argArr[firstInt+1])+1]; 
        strcpy(input, argArr[firstInt+1]);
        char output[strlen(argArr[secondInt+1])+1]; 
        strcpy(output, argArr[secondInt+1]);

       // printf("input: %s and Output %s\n", input, output);
        if(firstInt < secondInt){
            indexSpecial = firstInt;
        }else{
            indexSpecial = secondInt;
        }
        char *argVec[indexSpecial+1];
        argVecHelper(argVec, argArr, 0, indexSpecial);

       if(is_openable_file(argVec[0])==0){
            char *file = parsingPATH(argVec);
            if(file != NULL){
                argVec[0] = file;
              //  printf("FilePath after exec: %s\n", argArr[0]);
                return leftAndRightRedirect(input, output, back_round, argVec);          
            }else{
              //  puts("Inside second");
                 return 1;
            }
            //free(file);
        }else{
            return leftAndRightRedirect(input, output, back_round, argVec);             
        }
    
    }

    else if((indexSpecial = find_special(argArr, "<"))!= -1){
        char input[strlen(argArr[indexSpecial+1])+1]; 
        strcpy(input, argArr[indexSpecial+1]);
    
        char *argVec[indexSpecial+1];
        argVecHelper(argVec, argArr, 0, indexSpecial);

        if(is_openable_file(argVec[0])==0){
            char *file = parsingPATH(argVec);
            if(file != NULL){
                argVec[0] = file;
               // printf("FilePath after exec: %s\n", argArr[0]);
                return leftRedirect(input, back_round, argVec);                
            }else{
               // puts("Inside second");
                 return 1;
            }
            //free(file);
        }else{
            return leftRedirect(input, back_round, argVec);                
        }
    }
    //attempts to see if it works without using PATH, if not it then ahs to use PATH variable
    else if((indexSpecial = find_special(argArr, ">"))!= -1){
        char output[strlen(argArr[indexSpecial+1])+1]; 
        strcpy(output, argArr[indexSpecial+1]);

        char *argVec[indexSpecial+1];
        argVecHelper(argVec, argArr, 0, indexSpecial);

         if(is_openable_file(argVec[0])==0){
           // puts("in if");
            char *file = parsingPATH(argVec);
            if(file != NULL){
                argVec[0] = file;
             //   printf("FilePath after exec: %s\n", argVec[0]);

                return rightRedirect(output, back_round, argVec);                
            }else{
                 return 1;
            }
            //free(file);
        }else{
           // puts("in else");
            return rightRedirect( output, back_round, argVec);                
        }
       
     }
     //free(file);
    return result;

}

/*
leftAndRightRedirect takes input filename, output, argVec and back_round.
First it checks if input is openable, and if not it returns 1;
Then it calls fork();
child opens input and output files.. and then calls dup2(fd, STDIN_FILENO); and dup2(fd2, STDOUT_FILENO); (fd and fd2 are input and ouput files respectivly)
Then child calls execv(argVec[0], argVec); Child exits if execv fails
Parent calls wait. if back_round = 1 (indicating & is present), then it calls waitpid with WNOHANG flag


Note: this function returns 1 on error and 0 otherwise
*/

int leftAndRightRedirect(char* input, char* output, int back_round, char **argVec){
    //puts("leftAndRightRedirect");
    if(is_openable_file(input) ==0){
    //puts("File Not Found");
        return 1;
   }
    int pid = fork();
    if(pid==0){
        int fd = open(input, O_RDONLY);
        int fd2 = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0777); //returns file descriptor
        if(fd == -1){
            perror("input file Doesn't Open");
            exit(2);
        }
        if(fd2== -1){
            perror("output file doesn't open");
            exit(2);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);

        //printf("argVec[0]: %s, argVec: %s", argVec[0], *argVec);

        dup2(fd2, STDOUT_FILENO);
        close(fd2);

        execv(argVec[0],argVec);
        exit(1);

    }
     else{
        int wstatus;
        if(back_round == 0){
            wait(&wstatus);
        }else{
            waitpid(-1, &wstatus, WNOHANG);  
        }
        
    }
    fflush(stdout);

    return 0;

}
/*
leftRedirect takes input filename, argVec and back_round.
First it checks if input is openable, and if not it returns 1;
Then it calls fork();
child opens input file. and then calls dup2(fd, STDIN_FILENO); (fd is input file )
Then child calls execv(argVec[0], argVec); Child exits if execv fails
Parent calls wait. if back_round = 1 (indicating & is present), then it calls waitpid with WNOHANG flag

Note: this function returns 1 on error and 0 otherwise

*/
int leftRedirect(char *file, int back_round, char **argVec){
   if(is_openable_file(file) ==0){
    //puts("File Not Found");
    return 1;
   }
    int pid = fork();
    if(pid==0){
        int fd = open(file, O_RDONLY);
        if(fd == -1){
            exit(4);
        }
        //char *argVec[indexSpecial];

        dup2(fd, STDIN_FILENO);

        execv(argVec[0],argVec);
        exit(1);// fail
    }
    else{
        int wstatus;
        if(back_round == 0){
            wait(&wstatus);
        }else{
            waitpid(-1, &wstatus, WNOHANG);  
        }
    }
    fflush(stdout);

    return 0;

}

/*

rightRedirect takes output filename, argVec and back_round.
It calls fork
child opens ouput file (creates a new one if needed). and then calls dup2(fd, STDOUT_FILENO); (fd is output file )
Then child calls execv(argVec[0], argVec); Child exits if execv fails
Parent calls wait. if back_round = 1 (indicating & is present), then it calls waitpid with WNOHANG flag

Note: this function returns 1 on error and 0 otherwise
*/
//NOTE: wstatus part doesn't work on server???
int rightRedirect(char* output, int back_round, char **argVec){
 
    int pid = fork();
    if(pid==-1){
        return 1;
    }
    if(pid ==0){
        int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0777); //returns file descriptor
        if(fd == -1){
            perror("file does not open\n");
            exit(2);
        }
        dup2(fd, STDOUT_FILENO);  //STDOUT_FILENO is 1
        close(fd); 
      
        execv(argVec[0], argVec);
    
        exit(5);      
    }else{
        int wstatus;
        if(back_round == 0){
            wait(&wstatus);
        }else{
            waitpid(-1, &wstatus, WNOHANG);  
        }
    }
    fflush(stdout);

    return 0;
}


/*
You do need to worry about flushing standard I/O streams (fflush(stdout))
 at appropriate times as you mess around with this. That means 'before you switch stdout over'.*/