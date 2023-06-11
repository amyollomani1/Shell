#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h>
#include <stdbool.h>
#include <time.h>


#include "helpers.h"
#include "builtinCommands.h"
#include "execProgram.h"
#include "execProgramHelper.h"
#include "pipe.h"

int readFromLine(char* str, int back_round);
void displayIntroMessage();
int parseCommandLine(char **argArr, int argn, int back_round);
int is_valid(char **argArr, int argn);

//int parseCommandLineTwice(char **argArr, int argn, int startCommmandIndex, int endCommandIndex );

//[process] > [file] tells your shell to redirect the process’s standard output to a file. 
//Similarly, the syntax [process] < [file]

//Error: ls | |

//NOTES:
// /usr/bin/wc is weird infinite loop
// cd < input.txt returns "too many arguments error"
//help file needs to be found anywhere
//should ls < input.txt work if input.txt doesn't exit
//for ls < input.txt, last character in the file needs to be space or enter. if its -a, then only - will be read 
        //(doesn't read last char)
//Multy redirection in both directions
//Leaking memory
//Make sure main process functions close all file descriptors too
//will wait & be called
//weird error saying command not found when doing sleep for first time
//clear is weird sometimes
//wc < shell.c is weird
//wc < input.txt is weird
//redirection needs a space after



/*
Main contains a while loop which calls func readFromLine(inputString), which recieves input from user
Then inside while loop main parse(line, delim) from helpers.c which puts the input into a char** array called arrArgs
To get the argn, it iterates through argArr untill NULL character is found

Then main tests to ensure the input is valid (that & is at the end if present, and that < or > is not at beginning)

Once input is realtivly valid. it assumes input is builtin and calls builtInCommands(argArr, argn) which is in builtInCommands.c
This function returns 2 if its not a builtin, in which case main calls parseCommandLine

While loop contintues until user calls exit built in.
*/

int main(){


    displayIntroMessage();
    char*delim=" "; 

    char helpFile[800];
    getcwd(helpFile, sizeof(helpFile));
    strcat(helpFile, "/");

    strcat(helpFile, "help");

    //printf("helpfile: %s\n", helpFile);

    while(1){
        int back_round = 0;
        char inputString[1000];
        readFromLine(inputString, back_round);
        char *line = strdup(inputString);
        char** argArr = parse(line, delim);
        int argn = 0;
        while (argArr[argn]!=NULL){
                //printf("mainArg: %s\n",argArr[argn]);
                argn++;
        }

       int index = find_special(argArr,"&");
       // printf("index: %d\n", index);
        
        if((index != -1) &&(index == argn-1)){
            back_round = 1;
            argArr[index] = NULL;
            argn--;
        }

        if(is_valid(argArr, argn)==1){
            continue;
        }
       
        
        
     //   for(int i = 0; i<argn; i++){
         //   printf("mainArg: %s\n", argArr[i]);
     //   }
        //char *PATH = getenv("PATH");
        if(index == 0 || ((index != argn) && (index !=-1))){
            puts("Invalid Command, incorrect & location");
        }else{
            if(builtInCommands(argArr,argn, helpFile) == 2){
            //printf("Not a built-in\n");
                if(parseCommandLine(argArr, argn, back_round) ==1){
                    printf("Invalid Command/File not found\n");
                }
            } 
        }

        

        //free(argArr);      
    }

    return 0;
}

/*
argVecHelper can have parameters start, end: to signal which part of the arrArg to execute
this func will call execMain();
also send file input as parameter
ls - l -a > output.txt
ws < input.txt > output.txt
ws shell.c > output.txt
*/



/*
This function first checks if there are special characters inside argArr (array of string inputs)
If redirection (> or <) located. Then it calls and returns RedirectMain(argArr, argn, back_round)
Then it checks if there is a |, if so it calls pipeMain(argArr, argn, back_round);
If it contains no special characters, it calls mainExec(argArr,argn,back_round), 
which wll call exec with no dups or additional alterations made
*/

int parseCommandLine(char **argArr, int argn, int back_round){
    //int processArgsNum = argn;
   // puts("in parseCommandLine");
     //printf("PATH in shell.c: %s\n", PATH);
    int pipe = find_special(argArr,"|");
    int rightRed = find_special(argArr,">");
    int leftRed = find_special(argArr,"<");

   //int indexSpecial = -1;
    if((rightRed!= -1)||((leftRed)!= -1)){
        //wc < input.txt //indexSpecial = 1. argn = 3 real argn = 1
       // printf("containsredirect\n");
        return(RedirectMain(argArr, argn, back_round));
    }else if(pipe!= -1){
        //printf("containspipe\n");
        return(pipeMain(argn, argArr, back_round));
    }
    else{
       // puts("mainExec handling it");
        return(mainExec(argArr, argn, back_round));
        return 0;
    }
    return 1;

}

/*
This program ensures that special characters are in correct places
If a special char is beginning or ending the line, or if both pipe and redirections are being used, it returns 1
returns 0 on sucess - when theres no issues with input format
*/
int is_valid(char **argArr, int argn){
     int pipe = find_special(argArr,"|");
    int rightRed = find_special(argArr,">");
    int leftRed = find_special(argArr,"<");
    if(((rightRed != -1) || (leftRed!= -1)) && (pipe!=-1)){
       puts("Invalid Command");
        return 1;
    }

    if((rightRed == 0) || (leftRed==0) || (pipe==0)){
       puts("Invalid Command: Special character in incorrect location");
        return 1;
    }
    if((rightRed == argn-1) || (leftRed==argn-1) || (pipe==argn-1)){   
       puts("Invalid Command: Special character in incorrect location");
        return 1;
    }
    return 0;
}


//Displays welcome message
void displayIntroMessage(){
    printf("\n\n");
    puts("**************************************");
    puts("");
    printf("\tWelcome to my shell\n");
    puts("");
    puts("**************************************");
    puts("");
}

//uses fgets to take input from stdin and to store it in input[]. Then input[] is copied into parameter str.
//returns 0 on sucess
int readFromLine(char* str, int back_round){
    //fflush(stdout);

//    fflush(stdout);
    char input[1000];
    printf(">>> ");
    fgets(input, 1000, stdin);
    while(strlen(input)==1){
        printf(">>> ");
        fgets(input, 1000, stdin);
    }
    input[strlen(input)-1] = '\0';
    strcpy(str,input);
    return 0;
   
}

// Function to execute builtin commands
//comands are cd, exit, pwd, help
// cd that takes one agrumetn- directory path- and changes current directory to that directory
//pwd that prints current working directory to standard output

//returns 1 if not built in, 0 if it is built in, then executes built in

//Too many arguments doesn't work, since when input line has too many arguments, helper.c acts weird
//Even when helper.c acts wierd, the special charcters function remains true
//does directions say you can assume cd will only ever take one argument
//make this work for directories accross file system

   /*


    char *line = strdup(argv[0]);
    char *arg = NULL;
    for(int a = 1; a<argc; a++){
        arg = strdup(argv[a]);
        strcat(line," ");
        strcat(line, arg);
    }
    char*delim=" "; 
    
    int argn = parse(line, &argArr,delim);
    printf("argn = %d\n", argn);
    printf("&:%d,>:%d,<:%d,>>:%d,|:%d\n",find_special(argArr,"&"),find_special(argArr,">"),find_special(argArr,"<"),find_special(argArr,">>"),find_special(argArr,"|"));

    int i=0;
    while(argArr[i]!=NULL){
        printf("Arr %s\n",argArr[i++]); //prints P[p??]  of text
    }


plan:
1. read
2. parse
3. execute


*/

