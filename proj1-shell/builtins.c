#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.h"


char *builtin_func_names[] = {"cd", "echo", "etime", "exit", "io"};

/** args[0] is always 'cd' and args[1] is the path
 * if there is more than one path, signal an error
 */
int sh_cd(char ** args)
{
    int ret;

    if (args[1] == NULL)
        ret = chdir(getenv("HOME"));
    else
        ret = chdir(args[1]);
    if (ret < 0)
        perror("chdir error: ");
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        setenv("PWD", cwd, 1);
    else
        perror("getcwd() error");
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
        perror("sh: etime failed to fork");
        return 1;
    }
    else if (pid == 0) {
        execv(args_cpy[0], args_cpy);
        perror("sh: etime exec failure");
        return 1;
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
    printf("Exiting Shell....\n");
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
	char * filename;
    pid_t pid = fork();
    /* ret will be 0 or 1, indicating whether we want the shell to keep running or not */
    int ret;
    /* create copy of args without the 'io' command */
    char **args_cpy = strstr_copy(args + 1);
    if (pid == -1) {
        perror("sh: io failed to fork");
        _free2d(args_cpy);
        ret = 1;
    }
    else if (pid == 0) {
       execv(args_cpy[0], args_cpy);
       perror("sh: io exec failure");
       ret = 1;
    }
    else{
        /* need to get data in /proc/pid/io */
        sprintf(filename, "/proc/%d/io", pid);
        char line[256];
        FILE * infile;
  		infile = fopen(filename, "r");
        if (!infile) {
            perror("sh_io failed to open proc file");
            _free2d(args_cpy);
            return 1;
        }
  		char *prop_colon, ** lines;
        /* grab file line by line */
    	while (fgets(line, sizeof(line), infile)) {
            if (!isalpha(line[0]))
                continue;
            lines = str_split(line, ":");
            if (!lines) {
                fprintf(stderr, "sh_io error");
                return 1;
            }
            /* add ':'' after the property */
            prop_colon = str_combine(lines[0], ":");
         	printf("%-25s: %s", prop_colon, lines[1]);
            free(prop_colon);
            _free2d(lines);
    	}
    	printf("\n");
        waitpid(pid, NULL, 0);
        ret = 1;
    }
    _free2d(args_cpy);
    return ret;
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
