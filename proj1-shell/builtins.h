/* the names of the builtin funcs we have implemented */
extern char *builtin_func_names[];

/* the table referring to the builtin funcs */
extern int (*builtin_funcs[]) (char **);


/**
 *
 */
int sh_cd(char ** args);


/**
 *
 */
int sh_etime(char ** args);


/**
 *
 */
int sh_exit(char ** args);


/**
 *
 */
int sh_echo(char ** args);


/**
 * don't know the return type for this one
 */
int sh_io(char ** args);


/**
 * is_builtin_cmd - return whether `arg` is a builtin we have defined
 */
int is_builtin_cmd(char *arg);

/**
 * sh_execute_builtin - looks up command in `builtin_funcs` table and calls it
 * @args: the args of the command
 */
int sh_execute_builtin(char **args);
