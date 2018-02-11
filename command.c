#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
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
 * fin, fout will be set depending on the presence of redirections
 * background will be set whether or not the '&' appears
 * e.g. ls -al | grep foo > outfile < infile &
 */
struct _CommandGroup {
    size_t capacity;
    size_t num_commands;
    Command** commands;
    char* fin;
    char* fout;
    char* ferr;
    bool background;
};

/*
 * Default constructor for Command, Will allocate the args array to an upperbound of 255 args
 * Due to the project specifications, we do not handle dynamic resizing of the array
 */
Command* command_create()
{
    Command *cmd = calloc(1, sizeof(Command));
    /* Initalize array size, upperbounded at 255 for now, +1 for terminator */
    cmd->args = calloc(255 + 1, sizeof(char*));
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
void command_append_arg(Command *cmd, char *arg)
{
    if (cmd->num_args >= cmd->capacity)
        return;
    char *copy = calloc(strlen(arg) + 1, sizeof(char));
    cmd->args[cmd->num_args] = strcpy(copy, arg);
    cmd->num_args++;
}


void command_print(Command *cmd)
{
    for (int i = 0; i < cmd->num_args; i++)
        printf("%s ", cmd->args[i]);
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
    CommandGroup *cmd_grp = calloc(1, sizeof(CommandGroup));
    cmd_grp->commands = calloc(255 + 1, sizeof(Command*));
    cmd_grp->capacity = 255;
    cmd_grp->num_commands = 0;
    return cmd_grp;
}

/* args should have gone through the parsing pipeline before reaching this stage */
/* parses through args, appending discrete Commands and detecting redirects and background ps indicator */
CommandGroup *command_group_from_args(char **args)
{
    CommandGroup *cmd_grp = command_group_create();
    Command * cur_cmd = command_create();

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            command_group_append_command(cmd_grp, cur_cmd);
            cur_cmd = command_create();
        }
        else if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0)
            continue;
        else if (i > 0 && strcmp(args[i], "&") == 0)
            /* '&'s only occur at beginning and end */
            cmd_grp->background = true;
        else if (i > 0 && strcmp(args[i - 1], "<") == 0)
            cmd_grp->fin = strdup(args[i]);
        else if (i > 0 && strcmp(args[i - 1], ">") == 0)
            cmd_grp->fout = strdup(args[i]);
        else
            command_append_arg(cur_cmd, args[i]);
    }
    /* append last cur_cmd */
    command_group_append_command(cmd_grp, cur_cmd);
    return cmd_grp;
}


void command_group_append_command(CommandGroup *cmd_grp, Command *cmd)
{
    if (cmd_grp->num_commands >= cmd_grp->capacity)
        return;
    /* should `cmd` be copied? */
    cmd_grp->commands[cmd_grp->num_commands++] = cmd;
}


/*
 * Executes the CommandGroup, accounting for pipes, redirections, and background
 */
void command_group_execute(CommandGroup *cmd_grp)
{
    /* save stdin and stdout for later restoration */
    int tmp_stdin = dup(0), tmp_stdout = dup(1);
    int fdin;
    pid_t pid;

    /* set initial input, handling input redireciton if present */
    if (cmd_grp->fin)
        fdin = open(cmd_grp->fin, O_RDONLY);
    else
        fdin = dup(tmp_stdin);

    int ret, fdout;
    /* execute the commands in the pipeline */
    for (int i = 0; i < cmd_grp->num_commands; i++) {
        /* redirect input */
        dup2(fdin, 0);
        close(fdin);

        /* if last command, handle output redirection if present */
        if (i == cmd_grp->num_commands - 1){
            if (cmd_grp->fout)
                fdout = open(cmd_grp->fout,  O_WRONLY|O_CREAT|O_TRUNC, 0666);
            else
                fdout = dup(tmp_stdout);
        }
        /* pipe otherwise */
        else {
            int fd[2];
            pipe(fd);
            fdout = fd[1];
            fdin = fd[0];
        }

        /* redirect output */
        dup2(fdout, 1);
        close(fdout);

        /* create child ps */
        pid = fork();
        if (pid == -1){
            perror("failed to fork");
            exit(1);
        }
        else if (pid == 0) {
            /* exec never returns if successful */
            setpgid(0, 0); /* set to new process group? */
            execv(cmd_grp->commands[i]->args[0], cmd_grp->commands[i]->args);
            perror("failed to execute child");
            exit(1);
        }
    }

    /* restore stdin, stdout */
    dup2(tmp_stdin, 0);
    dup2(tmp_stdout, 1);
    close(tmp_stdin);
    close(tmp_stdout);
    int status;
    if (!cmd_grp->background)
        waitpid(pid, &status, 0);
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
    for (int i = 0; i < cmd_grp->num_commands; i++) {
        command_print(cmd_grp->commands[i]);
        if (i + 1 < cmd_grp->num_commands)
            printf(" | ");
    }
    if (cmd_grp->fin)
        printf(" < %s", cmd_grp->fin);
    if (cmd_grp->fout)
        printf(" > %s", cmd_grp->fout);
    if (cmd_grp->background)
        printf(" &");
}

