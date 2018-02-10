/** args[0] is always 'ch' and args[1] is the path
 * if there is more than one path, signal an error
 */
#include <time.h>
#include <sys/time.h>
int sh_cd(char ** args)
{
	if (args[1] == NULL)
		chdir(getenv("HOME"));
	if (args[2] != NULL)
		fprintf(stderr, "ch: command not found\n");
	else
		chdir(args[1]);
	return 0;
}


/**
 *
 */
double sh_etime(char **args)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    //do command
    gettimeofday(&end, NULL);

    int t = end.tv_usec - start.tv_usec;

    return ((double)t / 100000);
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


/**
 * don't know the return type for this one
 */
void sh_io(char **args)
{

}
