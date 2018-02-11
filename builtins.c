#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "utils.h"


/** args[0] is always 'cd' and args[1] is the path
 * if there is more than one path, signal an error
 */
int sh_cd(char ** args)
{
    if (args[1] == NULL)
        chdir(getenv("HOME"));

    /* more than 1 arg */
    else if (args[2] != NULL)
        fprintf(stderr, "cd: error\n");
    else
        chdir(args[1]);
    return 0;
}


/**
 *
 */
double sh_etime(char **args)
{
    /* clip etime off command
        args now holds just the cmd
     */
    int i = 0;
    char ** args_cpy = strstr_copy(args + 1);
    struct timeval start, end, difference;

    gettimeofday(&start, NULL);
    /* exec command */
    gettimeofday(&end, NULL);

    /* calculate difference */

    difference.tv_sec = end.tv_sec - start.tv_sec;
    difference.tv_usec = end.tv_usec - start.tv_usec;    

    if(differecne.tv_usec < 0){
        difference.tv_sec--;
        difference.tv_usec += 1000000;
    }

    _free2d(args_cpy);

    return ((double)difference.tv_sec + ((double)difference.tv_usec/1000000.0));
}


/**
 *
 */
void sh_exit(char **args)
{
    fprintf(stderr, "Exiting Shell....\n");
    exit(0);
}

void sh_echo(char **args)
{
    int i = 1;
    while( args[i] != NULL ){
        /* environment variables are already expanded in sh_loop*/
        fprintf("%s ", args[i]);
        i++;
    }
    fprintf("\n");
}


void sh_io(char **args)
{
    pid_t pid = fork();

    if (pid == 0){
        /*execute command */ 
    }

    else{
        /*wait for child process*/
        /*need to get data in /proc/pid/io*/
        waitpid(pid, NULL, 0);
    }
}
