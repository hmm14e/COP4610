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
 * args[0] is echo
 */
void sh_echo(char **args)
{
	int i = 1;
	char* evar;

	while( args[i] != NULL ){
		/* for non environment variables*/
		if( args[i][0] != '$' )
			fprintf("%s ", args[i]);
		else{
			/*for environment variable*/	
			char nextarg[strlen(args[i])];
			char* x = &(args[i][1]);	
			char* y = nextarg;

			while(*x != '\0'){
				*y = *x;
				x++;
				y++;
			}
			*y = *x;
			if(evar = getenv(nextarg))
				fprintf("%s ", evar);	
			else
				fprintf(stderr, "(%s: Variable undefined) ", nextarg);
		}
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
