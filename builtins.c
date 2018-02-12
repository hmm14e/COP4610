#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "utils.h"


char *builtin_func_names[] = {"cd", "echo", "etime", "exit", "io"};

/** args[0] is always 'cd' and args[1] is the path
 * if there is more than one path, signal an error
 */
int sh_cd(char ** args)
{
    if (args[1] == NULL)
        chdir(getenv("HOME"));
    /* more than 1 arg */
    else if (args[2] != NULL)
        fprintf(stderr, "sh: more than one arg supplied to cd\n");
    else
        chdir(args[1]);
    return 1;
}


/* given input ["etime", "cmd", "arg1", ..., NULL], execute <cmd> and print time elapsed */
int sh_etime(char **args)
{
    /* clip etime off command, args now holds just the cmd */
    int i = 0;
    char ** args_cpy = strstr_copy(args + 1);
    struct timeval start, end, difference;

    gettimeofday(&start, NULL);

    pid_t pid = fork();
    if (pid == -1) {
        perror("failed to fork");
        exit(1);
    }
    else if (pid == 0) {
        execv(args_cpy[0], args_cpy);
        perror("sh: etime exec failure");
        exit(1);
    }
    else {
        /* block waiting for child to execute */
        waitpid(pid, 0, 0);
        gettimeofday(&end, NULL);
    }

    /* calculate difference */

    difference.tv_sec = end.tv_sec - start.tv_sec;
    difference.tv_usec = end.tv_usec - start.tv_usec;

    if(difference.tv_usec < 0){
        difference.tv_sec--;
        difference.tv_usec += 1000000;
    }

    _free2d(args_cpy);
    printf("Elapsed time: %f\n", ((double)difference.tv_sec + ((double)difference.tv_usec/1000000.0)));
    return 1;
}


/**
 *
 */
int sh_exit(char **args)
{
    fprintf(stderr, "Exiting Shell....\n");
    return 0;
}

int sh_echo(char **args)
{
    int i = 1;
    while( args[i] != NULL ){
        /* environment variables are already expanded in sh_loop*/
        fprintf(stdout, "%s ", args[i]);
        i++;
    }
    printf("\n");
    return 1;
}


int sh_io(char **args)
{
	char * start = "/proc/";
	char * end = "/io";
	char * filename;
    pid_t pid = fork();
    if (pid == 0){
        /*execute command */
    }

    else{
        /*wait for child process*/
        /*need to get data in /proc/pid/io*/
        sprintf(filename, "%s%d%s", pre, pid, post);
        char line[256];
 		FILE * infile;
  		infile = fopen(filename, "r");
  		char ** lines;

    	while (fgets(line, sizeof(line), infile)) {
         	lines = str_split(line, ":"); 
         	printf("%s:", lines[0]);
         	int MAX = 35;
         	int len = strlen(lines[1]) + strlen(lines[0]);
         	for (int i = 1; i < MAX-len; i++){
          	printf(" ");
         }

         printf("%s", lines[1]);
    	}	

    	printf("\n");
        waitpid(pid, NULL, 0);
    }
    return 1;
}


/* lookup table of builtin funcs, see `sh_execute_builtin for usage */
int (*builtin_funcs[]) (char**) = {
    &sh_cd,
    &sh_echo,
    &sh_etime,
    &sh_exit,
    &sh_io
};


int is_builtin_cmd(char *arg) {
    size_t num_builtins = sizeof(builtin_func_names) / sizeof(builtin_func_names[0]);
    for (int i = 0; i < num_builtins; i++) {
        if (strcmp(arg, builtin_func_names[i]) == 0)
            return 1;
    }
    return 0;
}





int sh_execute_builtin(char **args)
{
    size_t num_builtins = sizeof(builtin_func_names) / sizeof(builtin_func_names[0]);
    for (int i = 0; i < num_builtins; i++) {
        if (strcmp(args[0], builtin_func_names[i]) == 0){
            return (*builtin_funcs[i])(args);
        }
    }
    return 0;
}
