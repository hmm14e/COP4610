/**
 ************************************************************************************
 **************************** Interface for Command *********************************
 ************************************************************************************
 */

typedef struct _Command Command;


Command *command_create();


void command_append_arg(Command *cmd, char *arg);


void command_print(Command *cmd);


void command_free();



/**
 ************************************************************************************
 **************************** Interface for CommandGroup ****************************
 ************************************************************************************
 */

typedef struct _CommandGroup CommandGroup;

/**
 * command_group_create - Default constructor for CommandGroup
 */
CommandGroup *command_group_create();


/**
 * command_group_from_args - specialized constructor for CommandGroup, will create CommandGroup from sequence of tokens
 * IMPORTANT: args must have gone through the parsing pipeline and error checks before reaching this stage
 * e.g. ["ls", "-al", "|", "grep", "foo", ">", "outfile", "<", "infile"]
 */
CommandGroup *command_group_from_args(char **args);


/**
 * command_group_append_command - appends a command to CommandGroup class, for piping purposes
 */
void command_group_append_command(CommandGroup *cmd_grp, Command *cmd);


/**
 * command_group_execute - executes the entire CommandGroup, accounting for pipes, redirection, and background ps
 */
void command_group_execute(CommandGroup *cmd_grp);


/**
 * command_group_free - free the enitre CommandGroup
 */
void command_group_free(CommandGroup *cmd_grp);


/**
 * command_group_print - pretty print the CommandGroup
 */
void command_group_print(CommandGroup *cmd_grp);
