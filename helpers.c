#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"

char ** parse(char*line,char*delim){

        char**array=malloc(sizeof(char*));
        *array=NULL;
        int n = 0;

        char*buf = strtok(line,delim);

        if (buf == NULL){
                free(array);
                array=NULL;
                return array;
        }

        while(buf!=NULL ){
                char**temp = realloc(array,(n+2)*sizeof(char*));

                if(temp==NULL){
                        free(array);
                        array=NULL;
                        return array;
                }

                array=temp;
                temp[n++]=buf;
                temp[n]=NULL;

                buf = strtok(NULL,delim);

        }
        return array;
}

int find_special (char*args[], char * special){
	int i = 0;
	while(args[i]!=NULL){
		if(strcmp(args[i],special)==0){
			return i;
		}
		i++;
	}
	return -1;
}
/*
int main(){

        char _line[1000] = "a line of text lest see if it works for a huge intput like this\n";
        char * line = strdup(_line);
        char ** array = parse(line," \n");

        if (array==NULL)
                exit(1);

        int count = 0;
        while (array[count]!=NULL){
                printf("%s\n",array[count]);
                count++;
        }
        free(array);
        free(line);

}
*/