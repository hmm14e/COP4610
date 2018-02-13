#include <sys/types.h>

/**
 ************************************************************************************
 **************************** Interface for Command *********************************
 ************************************************************************************
 */

/*
 * The logical representaiton of a single command on the shell, without pipes and redirects.
 * e.g. ls - al
 */
typedef struct {
    size_t capacity;
    size_t num_args;
    char** args;
} Command;


Command *command_create();


void command_append_arg(Command *cmd, char *arg);


void command_print(Command *cmd);


void command_free();



/**
 ************************************************************************************
 **************************** Interface for CommandGroup ****************************
 ************************************************************************************
 */

/*
 * The structure to hold an entire command entered on the shell. This strucutre accounts for
   pipes and redirects.
 * fin, fout will be set depending on the presence of redirections
 * background will be set whether or not the '&' appears
 * e.g. ls -al | grep foo > outfile < infile &
 */
typedef struct {
    size_t capacity;
    size_t num_commands;
    Command** commands;
    size_t num_unreaped_pids;
    pid_t* unreaped_pids; /* for tracking background processes */
    char* fin;
    char* fout;
    char* ferr;
    bool background;
} CommandGroup;

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


/**
 * command_group_reap_pid - remove a pid from the `unpead_pids` array
 */
void command_group_reap_pid(CommandGroup *cmd_grp, pid_t pid);
