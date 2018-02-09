#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "command.h"

/*
 * The logical representaiton of a single command on the shell, without pipes and redirects.
 * e.g. ls - al
 */
struct _Command {
    size_t capacity;
    size_t num_args;
    char** args;
};

/*
 * The structure to hold an entire command entered on the shell. This strucutre accounts for
   pipes and redirects.
 * in_file, out_file will be set depending on the presence of redirections
 * background will be set whether or not the '&' appears
 * e.g. ls -al | grep foo > outfile < infile &
 */
struct _CommandGroup {
    size_t capacity;
    size_t num_commands;
    Command* commands;
    char* in_file;
    char* out_file;
    char* err_file;
    bool background;
};

/*
 * Default constructor for Command, Will allocate the args array to an upperbound of 255 args
 * Due to the project specifications, we do not handle dynamic resizing of the array
 */
Command* command_create()
{
    Command *cmd = (Command *) malloc(sizeof(Command));
    /* Initalize array size, upperbounded at 255 for now, +1 for terminator */
    cmd->args = (char**) calloc(255 + 1, sizeof(char*));
    if (!cmd->args) {
        perror("failed to allocate command args");
        exit(EXIT_FAILURE);
    }
    cmd->capacity = 255;
    cmd->num_args = 0;
    return cmd;
}

/*
 * Insert a new command into the args array
 * Will not doing anything if the args array is full (shouldn't ever happen)
 */
void command_insert_arg(Command* cmd, char* arg)
{
    if (cmd->num_args >= cmd->capacity)
        return;
    char *copy = calloc(strlen(arg), sizeof(char));
    cmd->args[cmd->num_args] = strcpy(copy, arg);
    cmd->num_args++;
}

/*
 * Deallocate all memory the Command had allocated
 */
void command_free(Command* cmd)
{
    for (int i = 0; i < cmd->num_args; i++)
        free(cmd->args[i]);
    free(cmd->args);
    free(cmd);
}

/*
 * Default constructor for CommandGroup, will allocate the commands array to an upperbound of 255 commands
 */
CommandGroup *command_group_create()
{
    return NULL;
}

/*
 * Specialized constructor for CommandGroup, will create a CommandGroup from a sequence of tokens
 * e.g. ["ls", "-al", "|", "grep", "foo", ">", "outfile", "<", "infile"]
 */
CommandGroup *command_group_from_tokens(char** tokens)
{
    return NULL;
}

/*
 * Executes the CommandGroup, accounting for pipes, redirections, and background
 */
void command_group_execute(CommandGroup *cmd_grp)
{

}



/*
 * Deallocate all memory the CommandGroup had allocated
 */
void command_group_free(CommandGroup *cmd_grp)
{

}


/*
 *
 */
void command_group_print(CommandGroup* cmd_grp)
{

}

