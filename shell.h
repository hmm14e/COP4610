/**
 * Functions responsible for controlling the event loop of the shell, involving prompting, parsing, and executing
 */


#define SH_LINE_BUFFSIZE 255
#define SH_TOKEN_BUFFSIZE 255
#define  SH_PATH_BUFFSIZE 255
#define SH_TOKEN_DELIMS " \t\n\r"

/**
 ************************************************************************************
 ********************************* Helper Functions *********************************
 ************************************************************************************
 */


/**
 * _is_evn_variable - returns whether a token is referencing an environment variable
 * NOTE: this function only deals with environment variables at the start of a token
 * e.g. ls $HOME or ls $PWD/<somedir>
 */
bool _is_env_variable(char* tok);


/**
 * _get_env_var_len - return the length of the env var including the '$'
 */
int _get_env_var_len(char* tok);


/**
 * _is_path_variable - returns whether a token is a path to some file
 */
bool _is_path_variable(char* tok);


/**
 ************************************************************************************
 ********************************* Shell Functions **********************************
 ************************************************************************************
 */


/**
 * sh_read_line - read shell user input from stdin
 * @return: char * to beginning of line
 * Assumption: The user input will be no more than 255 characters
 */
char *sh_read_line();


/**
 * sh_add_whitespace - adds whitespace around chars, so it can later be tokenized
 * @line: command line input
 * @char: the chars to add whitespace around, (<, >, |, &) usually
 * @return: copy of the line with added whitespace
 * e.g. 'ls -al|grep me>outfile <infile' --> 'ls -al | grep me > outfile  <infile'
 */
char *sh_add_whitespace(char *line, char *chars);


/**
 * sh_parse_line - parse a line into tokens for future execution
 * @line: Line to be parsed
 * @return: array of tokens
 */
char **sh_parse_line(char *line);


/**
 * sh_expand_env_vars - expand environment variables in args
 * @args: array of char* denoting the arguments
 * @returns: copy of args with environment variables expanded
 */
char **sh_expand_env_vars(char** args);


/**
 * sh_resolve_paths - resolve pathnames
 * @args: array of char* denoting the arguments
 * @returns: copy of args with all the paths expanded to absolute paths
 */
char **sh_resolve_paths(char** args);



/**
 * sh_prompt - prompt user with '$USER@$MACHINE :: $PWD =>'
 */
void sh_prompt();


/**
 * sh_loop - loop grabbing commands from the user and executing them
 */
void sh_loop();





