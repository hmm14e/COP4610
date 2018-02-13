#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "command.h"
#include "builtins.h"


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
    char *copy = strdup(arg);
    cmd->args[cmd->num_args++] = copy;
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
    cmd_grp->unreaped_pids = calloc(255 + 1, sizeof(pid_t*));
    cmd_grp->num_unreaped_pids = 0;
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
    int ret, fdout, fdin, tmp_stdin = dup(0), tmp_stdout = dup(1);
    pid_t pid;

    /* set initial input, handling input redireciton if present */
    if (cmd_grp->fin)
        fdin = open(cmd_grp->fin, O_RDONLY);
    else
        fdin = dup(tmp_stdin);

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

        if (is_builtin_cmd(cmd_grp->commands[i]->args[0])) {
            /* call builtin, no forking */
            int ret = sh_execute_builtin(cmd_grp->commands[i]->args);
            if (!ret)
                exit(0);
        }
        else {
            /* create child ps */
            pid = fork();
            if (pid == -1){
                perror("failed to fork");
                exit(1);
            }
            else if (pid == 0) {
                /* put child in new ps group, to detatch its stdin from the fg shell */
                if (i == 0 && cmd_grp->background)
                    setpgid(0, 0);
                execv(cmd_grp->commands[i]->args[0], cmd_grp->commands[i]->args);
                /* exec never returns if successful */
                perror("failed to execute child");
                exit(1);
            }
            else {
                /* for bg processing, add pid to array for later printing out */
                cmd_grp->unreaped_pids[cmd_grp->num_unreaped_pids++] = pid;
            }
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
    for (int i = 0; i < cmd_grp->num_commands; i++)
        command_free(cmd_grp->commands[i]);
    free(cmd_grp->commands);
    free(cmd_grp->unreaped_pids);
    free(cmd_grp);
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


void command_group_reap_pid(CommandGroup* cmd_grp, pid_t pid)
{
    for (int i = 0; i < cmd_grp->num_unreaped_pids; i++) {
        if (cmd_grp->unreaped_pids[i] == pid) {
            cmd_grp->unreaped_pids[i] = 0;
            /* shift everything after left to fill vacant spot */
            for (int j = i + 1; j < cmd_grp->num_unreaped_pids; j++) {
                cmd_grp->unreaped_pids[j - 1] = cmd_grp->unreaped_pids[j];
                cmd_grp->unreaped_pids[j] = 0;
            }
            cmd_grp->num_unreaped_pids--;
            break;
        }
    }

}
