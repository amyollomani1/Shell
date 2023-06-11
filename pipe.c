#include "pipe.h"
#include "helpers.h"
#include "execProgramHelper.h"
#include "execProgram.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>


/*
pipeMain is called from parsingCommandLine in shell.c and takes inputs argn, argArr, and back_round
First pipeMain calls pipeCount, which returns the number of pipes. If only one pipe is found, singlePipe() is called
Otherwise it calls multipipe();
returns 1 on default to signal an error since there must be >1 pipes
*/
int pipeMain(int argn, char **argArr, int back_round){
   // printf("In pipe Main\n");


    int pipenum= pipeCount(argn, argArr);
    //printf("PipeCount: %d\n", pipenum);

    if(pipenum == -1){
        puts("Error: incorrect pipe placement");
        return 1;
    }
    if(pipenum>=3){
        return 1; 
    }
    if(pipenum == 1){
        return(singlePipe(argn, argArr, back_round));
    }
    else{
        return(multiPipe(argn, argArr, pipenum, back_round));
    }
    return 1;
}

//Iterates through argArr to get count of '|' in user input. returns count
int pipeCount(int argn, char **argArr){
    
   // int arrInd[100];
    //int j = 0;
    int count = 0;
    for(int i =0; argArr[i]!=NULL; i++)
	{
		if(strcmp(argArr[i],"|") ==0){
			//arrInd[j++]=i;
            count++;
            if(i == argn-1){
                count  = -1;
                break;
            }
        }
	}
    

    return count;
}




    //p1 arg1 > p2 arg2
    //0    1  2  3  4
    //argn = 5;
    //indSpecial = 2
    //vec1[3] = index+1 

/*
Single pipe calls isCommand(argArr, 0, pIndex, pIndex+1); and isCommand(argArr, pIndex+1, size-1, size) twice
isCommand can be found in executeProgramHelper.c This function returns a char **argVec array, which hold the arguments from startindex to number of arguments
In this case. argVec1 holds command aguments from before | and argVec2 holds them from after the pipe.
Then, fd[2] is initilized and pipe(fd) is called to create the pipe
Then parent calls fork()
The child closes the read end of pipe, calls dup2(fd[1], STDOUT_FILENO) on write end, and then closes fd[1] and calls execv(argVec1)

Then parent calls fork() again
The second child closes fd[1] - write end of pipe. and calls dup2(fd[0], STDIN_FILENO); Then closes fd[0] and calls execv on argVec
    exits if execv fails
parent closes both ends of pipe, and calls waitpid on both (note if back_round = 1, then & was inputed and WNOHANG flag will be used)


*/
    
int singlePipe(int argn, char **argArr, int back_round){
    int res = 0;
    int pIndex =  find_special(argArr, "|");
    int size = argn - pIndex;
    char **argVec1 = isCommand(argArr, 0, pIndex, pIndex+1);
    char **argVec2 = isCommand(argArr, pIndex+1, size-1, size);
    if(argVec1==NULL){
        return 1;
    }if(argVec2==NULL){
        return 1;
    }
    int fd[2];
    int pid1, pid2;

    if(pipe(fd)<0){
        perror("pipe error");
        return 1;
    }else if((pid1 = fork())<0){
        perror("fork error");
        return 1;
    }
    if (pid1 ==0){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execv(argVec1[0], argVec1);
        perror("Command Failed: ");
        exit(2);
    }
    else if((pid2 = fork())< 0){
        //perror("second child fork error");
        exit(3);
    }else if(pid2 == 0){
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        execv(argVec2[0], argVec2);
        //perror("exec P2 Error");
        exit(1);
    }
    else{
        close(fd[0]);
        close(fd[1]);
        int status1;
        int status2;
        //int child1;
       // int child2;
        if(back_round == 0){
            waitpid(pid1, &status1, 0);
            waitpid(pid2, &status2, 0);
        }else{
            waitpid(pid1, &status1, WNOHANG); 
            waitpid(pid2, &status2, WNOHANG);
        }
        
       // if(status1 != 0 || status2 !=0){
            //printf("Comand not found\n");
         //   res = 1;
      //  }
        //printf("ls: pid = %d, corpse = %d, exit status = 0x%.4X\n", pid1, child1, status1);
        //printf("ls: pid = %d, corpse = %d, exit status = 0x%.4X\n", pid2, child2, status2);

    }
    fflush(stdout);

    free(argVec1);
    free(argVec2);
    return res;
}

//
//number of pipes: n-1 (n = num processes)
//ls | grep <arg> | wc
//example: ps -ef | grep root | wc -l
//ex: cat /etc/passwd | grep /bin/bash$ | wc -l

/*
This function 
Creates arrInd[], which stores index of all pipes.
Then it creates ***args, which is an array of argVecs for each process
isCommand(argArr, start, argNums, argNums+1) is called, to copy argArr arguments into each argVec, with the corespponding
start and argNum indecies.
Then theres a forloop which iterates over ***args and calls is_openable_file on each args[i][0] to ensure each process
contains an exectuable command
At this point, int fd[pipeCount][2] is created to store all file descrpitors of pipes.
Then in for loop i = 0, i < pipeCount. pipe(fd[i]) is called to create n-1 pipes.
Then to execute infinite pipes, I used a for loop from (int i = 0; i < pipeCount + 1; i ++).
During each iteration, int pid = fork() is called to create n children.

    if pid == 0 and i ==0, then all other fd's are closed using for loop, expect for fd[0][1]
    dup2(fd[0][1], STDOUT_FILENO); is called and execv is called on args[0]

    else if pid == 0 and i ==pipeCount (last process), then all other fd's are closed using for loop, expect for fd[pipeCount][0]
    dup2(fd[pipeCount-1][0], STDIN_FILENO); is called and execv is called on args[pipeCount] (last command)

    else if pid == 0 (middle processes)
        ll other fd's are closed using for loop, expect for fd[i-1][0]
    dup2(fd[pipeCount-1][0], STDIN_FILENO);  and fd[i][1] dup2(fd[0][1], STDOUT_FILENO);
    is called and execv is called on args[i]
    }

After this, parent process closes all fd's using for loop. Also calls waitpid(-1, NULL, WNOHANG) on all process except for last
if back_round == 1 (indicating that & is at end of line), the waitpid with flag WNOHANG is called for all children
Otherwise waitpid uses no flags for last process
   

*/
int multiPipe(int argn, char **argArr, int pipeCount, int back_round){

    ////////////CREATE array which stores indexes of pipes
    int arrInd[100];
    int j = 0;
    for(int i =0; argArr[i]!=NULL; i++)
	{
		if(strcmp(argArr[i],"|") ==0){
			arrInd[j++]=i;
        }
	}
  //  for(int i=0; i<j; i++){
		//printf("%d \t",arrInd[i]);
  //  }puts(" ");
    //char *args[pipeCount+1][1000];

        //Create all argVec arrays
    /////////////////////////////
    char ***args = malloc(sizeof(char **) * (pipeCount + 1));
    for(int k = 0; k < pipeCount + 1; k++) {
      args[k] = malloc(sizeof(char *) * 1024);
    }
   
    int start = 0;
    int argNums = arrInd[0];
    //int size = arrInd[0] + 1;
    //puts("Next");
    **(args) = *isCommand(argArr, start, argNums, argNums+1);
   // for(int j = 0; j < argNums+1; j++){
        //printf("args+0 : %s\n", args[0][j]);
    // }
    for(int i = 1; i< pipeCount+1; i++){
       // puts("Next");

        start = arrInd[i-1]+1;
        if(i !=pipeCount){
            argNums = arrInd[i] - arrInd[i-1] - 1;
        }else{
            argNums = argn - arrInd[i-1]-1;
        }
        *(args+i) = isCommand(argArr, start, argNums, argNums+1);
       // for(int j = 0; j < argNums+1; j++){
           // printf("args+%d : %s\n", i, args[i][j]);
        //}
    }
    
    /////////////
    //Check to make sure that each process input is correct
    for(int i = 0; i < pipeCount+1; i ++){
        if(is_openable_file(args[i][0])==0){
            printf("One command is not a process\n");
            return 1;
        }
    }
////////////////////////////
//Actual pipe and fork time


    int fd[pipeCount][2];
    for(int i = 0; i < pipeCount; i++){
        pipe(fd[i]);
    }
    
    
   // puts("hERE");
    for(int i = 0; i < pipeCount + 1; i ++){
        int pid = fork();
        if((pid == 0) && (i == 0)){
         //   puts("first");
            close(fd[0][0]);
            for(int k = 1; k < pipeCount; k++){
                for(int j = 0; j < 2; j++){
                    close(fd[k][j]);
                }
            }
            //close(fd[0][0]);
           // close(fd[1][0]);
           // close(fd[1][1]);
         //   puts("Here");

            dup2(fd[0][1], STDOUT_FILENO);
            close(fd[0][1]);
            execv(args[0][0], args[0]);
            perror("Command Failed: ");
            exit(2);
            
        }
        else if((pid == 0) && (i == pipeCount)){
           // puts("third");
            for(int a = 0; a<pipeCount-1; a++){
                for(int b = 0; b <2; b++){
                    close(fd[a][b]);
                }
            }
           // close(fd[0][1]);
            //close(fd[1][1]);
           // close(fd[0][0]);
            close(fd[pipeCount-1][1]);//[1][1]
            dup2(fd[pipeCount-1][0], STDIN_FILENO);
            close(fd[pipeCount-1][0]); //[1][0]
            execv(args[pipeCount][0], args[pipeCount]);
            //perror("exec P2 Error");
            exit(1);
        }else if (pid ==0){
            //puts("second");
            for(int f = 0; f< pipeCount; f++){
                for(int p = 0; p <2; p++){
                    if(f == 0 && p ==0){
                        continue;
                    }if(f == 1 && p == 1){
                        continue;
                    }else{
                        close(fd[f][p]);
                    }
                }
            }
            dup2(fd[i-1][0], STDIN_FILENO); //[0][0]
            dup2(fd[i][1], STDOUT_FILENO);//[1][1]
            close(fd[i-1][0]);
            close(fd[i][1]);
            execv(args[i][0], args[i]);
            exit(1);
        }
       // fflush(stdin);
        //fflush(stdout);
        
        //wait(NULL);
    }
    //ps -ef | grep root | wc -l
        for(int e = 0; e < pipeCount; e++){
            for(int l = 0; l <2; l++){
                close(fd[e][l]);
            }
        }
        int status;
        //int wpid;
        //int status2;
        //int child1;
       // int child2;
       for(int w = 0; w < pipeCount; w++){
            waitpid(-1, &status, WNOHANG); 
            //printf("Comand not found\n");
        }

        if(back_round == 0){
            waitpid(-1, &status, 0);
            
        }else{
            waitpid(-1, &status, WNOHANG);
        }

        while (wait(NULL) > 0){
                ;
        }

        
         
            //while (wait(NULL) > 0){
           //     ;
           // }
            //waitpid(-1, &status, 0);
            //waitpid(pid2, &status2, 0);
            //while ((wpid = wait(&status)) > 0);
        

        fflush(stdout);
        return 0;
}

/*



PIPES:

Start with making one pipe:

see how many pipes there are
make an array to place file descritpors. Make it so descritpors are next to each other
 cmd1 | cmdd2 | cmd3
 fd[0,1,2,3]
 0 = input to first, 1 = output to first
 2 = input2
 3 = ouptut2

 process 1 writes to output, 
 process 2 reads from original output, and writes to output2
 process 3 reads from output2 and writes to output

 Background processing
    -check character
    -don't use wait, cause you want user to keep doing commands, but you also don't want to have zombies
    -look at wait and its flags to accomplish this
    -have a spot where you can routinely call it, like right after user types something

wait builtin
- repeatedly call wait untill therse no more children. wait throws an error when this happens




*/
