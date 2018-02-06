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

CommandGroup *command_group_create();

CommandGroup *command_group_create_from_tokens(char** tokens);

void command_group_insert_command(CommandGroup* cmds);

void command_group_execute(CommandGroup* cmds);

void command_group_print(CommandGroup* cmds);
