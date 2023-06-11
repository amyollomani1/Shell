shell: shell.c helpers.c builtinCommands.c execProgram.c execProgramHelper.c pipe.c
	gcc -o shell shell.c helpers.c builtinCommands.c execProgram.c execProgramHelper.c pipe.c -Wall -Werror

clean:
	rm -rf *.o main
