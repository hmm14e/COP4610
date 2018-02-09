/*
 * Interface for the `Command` class
 */

typedef struct _Command Command;

Command *command_create();

void command_insert_arg(Command *cmd, char *arg);

void command_free();

/*
 * Interface for the `CommandGroup` class
 */

typedef struct _CommandGroup CommandGroup;

/*
 * command_group_create - Default constructor for CommandGroup
 * Will initalize an empty commands array to
 */
CommandGroup *command_group_create();

CommandGroup *command_group_from_tokens(char** tokens);

void command_group_insert_command(CommandGroup* cmd_grp);

void command_group_execute(CommandGroup* cmd_grp);

void command_group_print(CommandGroup* cmd_grp);
