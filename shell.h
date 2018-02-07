/**
 * Functions responsible for controlling the event loop of the shell, involving prompting, parsing, and executing
 */


#define SH_LINE_BUFFSIZE 255
#define SH_TOKEN_BUFFSIZE 255
#define  SH_PATH_BUFFSIZE 255
#define SH_TOKEN_DELIMS " \t\n\r"


/**
 * _is_evn_variable - returns whether a token is referencing an environment variable
 */
bool _is_env_variable(char* tok);


/**
 * _is_path_variable - returns whether a token is a path to some file
 */
bool _is_path_variable(char* tok);


/**
 * sh_read_line - read shell user input from stdin
 * @return: char * to beginning of line
 * Assumption: The user input will be no more than 255 characters
 */
char *sh_read_line();


/**
 * sh_parse_line - parse a line into tokens for future execution
 * @line: Line to be parsed
 * @return: array of tokens
 */
char **sh_parse_line(char *line);


/**
 * sh_expand_args - expand environment variables in args
 * @args: array of char* denoting the arguments
 * @returns: copy of args with environment variables expanded
 */
char **sh_expand_args(char** args);


/**
 * sh_resolve_args - resolve pathnames
 * @args: array of char* denoting the arguments
 * @returns: copy of args with all the paths expanded to absolute paths
 */
char **sh_resolve_args(char** args);


/**
 * sh_prompt - prompt user with '$USER@$MACHINE :: $PWD =>'
 */
void sh_prompt();


/**
 * sh_loop - loop grabbing commands from the user and executing them
 */
void sh_loop();





