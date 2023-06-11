void argVecHelper(char *argVec[], char**argArr, int start, int argn);
int mainExec(char **argArr, int processArgsNum, int back_round);
int executeProgram(char *argVec[], int back_round);
char *parsingPATH(char **argVec);
void concrenateHelper(char *PATH,char** argVec);
int is_openable_file(char *path);
char** isCommand(char**argArr, int startInd, int endInd, int size);

