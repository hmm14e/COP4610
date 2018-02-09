/** args[0] is always 'ch' and args[1] is the path
 * if there is more than one path, signal an error
 */
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
    return 0.0;
}


/**
 *
 */
void sh_exit(char **args)
{
	fprintf(stderr, "Exiting Shell....\n");
	exit(0);
}


/**
 *
 */
void sh_echo(char **args)
{

}


/**
 * don't know the return type for this one
 */
void sh_io(char **args)
{

}
