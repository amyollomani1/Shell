

# Desgin Plan

## Files:

- shell.c
- builtinCommands.c
- builtinCommands.h
- makefile
- helpers.c 
- helpers.h
- execProgram.c
- execProgram.h
- execProgramHelper.c
- execProgramHelper.h
- pipe.c
- pipe.h

# shell.c
Contains functions: 
- int main()
- int readFromLine(char* str, int back_round);
- void displayIntroMessage();
- int parseCommandLine(char **argArr, int argn, int back_round);
- int is_valid(char **argArr, int argn);

### main()
First it calls displayIntroMessage(), which prints "Welcome to my shell" onto screen.
Then theres a while loop. During each iteration of this loop, main calls readFromLine.
readFromLine accepts str as a parameter and modifies it so it contains the input. Then main calls parse(str,delim)
which uses this string to create argArr, which is an array of all arguments, seperated by " ". 
Then main calls is_valid(argArr, argn) to ensures that special characters ( | and >) are not at the end or beginning of the line
It also checks that & is at the end of the line. After the input is validated, main calls builtInCommands(argArr,argn).
If the command isn't a built in, it then calls parseCommandLine(argArr, argn, back_round).
While loop continues until exit is called.

### int parseCommandLine(argArr, argn, back_round)
First, this function calls find_special, which takes argArr and special character as input and returns index of character.
If redirection is found ( < or >) then RedirectMain(argArr, argn, back_round) is called.
If pipe is found ( | ), then pipeMain (argArr, argn, back_round) is called.
Otherwise, it calls mainExec(argArr, argn, back_round) which is found in execProgramHelper.c
Note: returns 1 on errors, 0 on success

### int is_valid(argArr, argn)
Checks indecies of special characters using find_special() from helpers.c to ensure the input is valid.
Returns 1 when not valid. 0 When it is valid

void readFromLine(char* str);
Uses fgets to put user input into char input[]. Then uses strcpy to copy input into parameter str.

# execProgramHelper.c
- void argVecHelper(char *argVec[], char**argArr, int start, int argn);
- int mainExec(char **argArr, int processArgsNum, int back_round);
- int executeProgram(char *argVec[], int back_round);
- char *parsingPATH(char **argVec);
- void concrenateHelper(char *PATH,char** argVec);
- int is_openable_file(char *path);
- char** isCommand(char**argArr, int startInd, int endInd, int size);

mainExec is called by  parseCommands() in shell.c
### mainExec():
First it creates a char \**array called argVec. It then calls argVecHelper(argVec, argArr, 0, argnNum),
which copies relevant indecies from argArr, and adds NULL to the end, thus making argVec executable with execv
Then mainExec calls is_openable_file, which tests to see if argVec[0] (the command), is an openable file
If not, it calls parsingPATH(argVec), which takes argVec as input and returns a char* file with the command and PATH input.
if no PATH is found to corrleate with the command being inputed, then parsingPATH returns NULL, and mainExec returns 1 for failure.
argVec[0], is updated to hold the new PATH if one is found, and then mainExec calls executeProgram(argVec, back_round)
Note: mainExec returns 1 on failure, caused when the command inputed can't be found in PATH, otherwise it returns 0.

### void argVecHelper(char *argVec[], char**\argArr, int start, int argn){
This function iterates through argArr and copies strings from argArr indexes start to argn into argVec. Then it adds a NULL to end of argVec to ensure its executable.

 ### char* parsingPATH(char \**argVec)
 Uses getenv("PATH") and puts it into string called envPath, which is then copied to string PATH using strncpy.
 strtok(PATH ":") is called while (strok(NULL, ":")!=NULL); 
 During each iteration through the while loop, the directory from the strtok'd path is concacterated with char *file. Then strcat is used to combine file with argVec[0]. (for example, if argVec[0] = ls, then eventually in while loop it file become bin/ls) Then parsingPath calls is_openable_file(file), which returns 1 if it is an openable file. If it is, then parsingPATH returns the *file. Othewise, if no openable file is found (not valid command) it returns NULL
 
 int is_openable_file(char *path);
 This function is mainly used as helper to parsingPATH, and to check if command is valid before calling execv.

### int executeProgram(char *argVec[], int back_round)
executeProgram takes input char *argVec[] and int back_round.
execute program first calls fork(). The child calls execv(argVec[0], argVec). If execv fails, child exits.
The parent calls waitpid to wait for process.
Note that when int back_round = 1, then & is at end of command. Thus, waitpid is called with 
flag WNOHANG to allow parent process to continue

### char** isCommand(char\**argArr, int startInd, int endInd, int size){
This function is used by pipe.c. It is passed argArr, which holds all arguments inputed, and the start, end index and size.
This function creates an argVec array that can be used with execv. It uses parameters start,end and size to create the size of the array and
to copy the relevant argument indecies.
This function also tests to ensure that the command, which is found in array_ptr, is executable. If not, it calls parsingPATH to return the full filepath.

# execProgram.c
- int RedirectMain(char **argArr, int argn, int back_round);
- int rightRedirect(char *output, int back_round, char ** argVec);
- int leftRedirect( char *inputfile, int back_round, char **argVec);
- int leftAndRightRedirect(char* input, char* output,  int back_round, char **argVec);

### int RedirectMain(char \**argArr, int argn, int back_round)
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
    if parsingPATH doesn't return NULL, leftRedirect(indexSpecial, argArr, input, back_round, argVec) is called.
    Otherwise redirectMain returns 1 since command is invalid

Then redirectMain checks if > is inputed by user
    Similarily, output[] stores the output file name. argVec is created to store arguments from argArr. parsingPATH is called if argVec[0] is not a valid command
    if parsingPATH doesn't return NULL, leftREdirect(indexSpecial, argArr, output, back_round, argVec) is called.
    Otherwise redirectMain returns 1 since command is invalid

Note: redirectMain returns 1 if execv fails and commandline arguments are invalid/ input file doesn't exist.
    redicretMain also returns immediatly after calling leftRedirect, rightRedirect, and leftAndRightRedirect

### int leftAndRightRedirect(char* input, char* output, int back_round, char \**argVec)
leftAndRightRedirect takes input filename, output, argVec and back_round.
First it checks if input is openable, and if not it returns 1;
Then it calls fork();
child opens input and output files.. and then calls dup2(fd, STDIN_FILENO); and dup2(fd2, STDOUT_FILENO); (fd and fd2 are input and ouput files respectivly)
Then child calls execv(argVec[0], argVec); Child exits if execv fails
Parent calls wait. if back_round = 1 (indicating & is present), then it calls waitpid with WNOHANG flag
Note: this function returns 1 on error and 0 otherwise

### int leftRedirect(char *file, int back_round, char \**argVec)
leftRedirect takes input filename, argVec and back_round.
First it checks if input is openable, and if not it returns 1;
Then it calls fork();
child opens input file. and then calls dup2(fd, STDIN_FILENO); (fd is input file )
Then child calls execv(argVec[0], argVec); Child exits if execv fails
Parent calls wait. if back_round = 1 (indicating & is present), then it calls waitpid with WNOHANG flag

Note: this function returns 1 on error and 0 otherwise

### int rightRedirect(char* output, int back_round, char \**argVec);
rightRedirect takes output filename, argVec and back_round.
It calls fork
child opens ouput file (creates a new one if needed). and then calls dup2(fd, STDOUT_FILENO); (fd is output file )
Then child calls execv(argVec[0], argVec); Child exits if execv fails
Parent calls wait. if back_round = 1 (indicating & is present), then it calls waitpid with WNOHANG flag

Note: this function returns 1 on error and 0 otherwise

# pipe.c
- int pipeMain(int argn, char **argArr, int back_round);
- int pipeCount(int argn, char **argArr);
- int singlePipe(int argn, char **argArr, int back_round);
- int multiPipe(int argn, char \**argArr, int count, int back_round);

int pipeMain(int argn, char \**argArr, int back_round)
This function is called by shell.c when there is a pipe takes inputs argn, argArr, and back_round
First pipeMain calls pipeCount, which returns the number of pipes. If only one pipe is found, singlePipe() is called
Otherwise it calls multipipe();
returns 1 on default to signal an error since there must be >1 pipes

int pipeCount(int argn, char \**argArr);
Iterates through argArr to get count of '|' in user input. returns count

### int singlePipe(int argn, char \**argArr, int back_round)
Single pipe calls isCommand(argArr, 0, pIndex, pIndex+1); and isCommand(argArr, pIndex+1, size-1, size) twice
isCommand can be found in executeProgramHelper.c This function returns a char \**argVec array, which hold the arguments from startindex to number of arguments
In this case. argVec1 holds command aguments from before | and argVec2 holds them from after the pipe.
Then, fd[2] is initilized and pipe(fd) is called to create the pipe
Then parent calls fork()
The child closes the read end of pipe, calls dup2(fd[1], STDOUT_FILENO) on write end, and then closes fd[1] and calls execv(argVec1)

Then parent calls fork() again
The second child closes fd[1] - write end of pipe. and calls dup2(fd[0], STDIN_FILENO); Then closes fd[0] and calls execv on argVec
    exits if execv fails
parent closes both ends of pipe, and calls waitpid on both (note if back_round = 1, then & was inputed and WNOHANG flag will be used)

### int multiPipe(int argn, char \**argArr, int pipeCount, int back_round)
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

# builtinCommands.c
- int builtInCommands(char** argArr, int argn);
- int printpwd(char** argArr, int argn);
- int cdCommand(char** argArr, int argn);
- int exitfunc(char** argArr, int argn);
- int help(int argn);
- int wait1(char **argArr, int argn);


Note: all builtin's return 1 on error, which is then returned by builinCommands() - this function acts as the main hub.

### int builtInCommands(char** argArr, int argn)
Called in main() while loop. Returns 2 if input is not a builtin
This function uses strcmp on argArr[0]. If "exit" is found, then it calls exitfunc(). if cd is found in argArr[0], the it calls cdCommand(), and etc for each built in. (builtins are exit, cd, pwd, help, and wait)

### int wait1(char \**argArr, int argn)
When called, it calls wait(NULL) > 0) to wait for all child process to finish

### int cdCommand(char** argArr, int argn)
uses chrdir to change directory to argArr[1]. 

### int exitfunc(char** argArr, int argn)
calls exit(0)

### int printpwd(char** argArr, int argn)
calls getcwd(cwd,sizeof(cwd). then prints string cwd

### int help(int argn)
calls fopen() to open help file
Prints help file using fgetc(fp) and putchar(c) in a while loop








