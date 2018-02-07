#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "shell.h"
#include "command.h"
#include "utils.h"

#define SH_LINE_BUFFSIZE 255
#define SH_TOKEN_BUFFSIZE 255
#define  SH_PATH_BUFFSIZE 255
#define SH_TOKEN_DELIMS " \t\n\r<>|&"



void _print_args(char** args)
{
    for (char *s = *args; s != NULL; s=*++args)
        printf("%s ", s);
    printf("\n");
}


char *sh_read_line()
{
    char *buffer = malloc(sizeof(char) * SH_LINE_BUFFSIZE);
    if (!buffer) {
        perror("Failed to allocate line buffer");
        return 0;
    }
    size_t ix = 0;
    char c;
    while(1) {
        c = getchar();
        if (c == '\n') {
            buffer[ix] = '\0';
            return buffer;
        }
        else
            buffer[ix] = c;
        ix++;
    }
}


char **sh_parse_line(char *line)
{
    /* User input is restricted to 255 chars, so a simple upperbound on tokens is 255 */
    char **tokens = malloc((SH_TOKEN_BUFFSIZE) * sizeof(char*));
    if (!tokens) {
        perror("Failed to allocate tokens buffer");
        return NULL; /* EXIT? */
    }
    size_t ix = 0;
    char *tok = str_tok(line, SH_TOKEN_DELIMS);
    while (tok != NULL) {
        tokens[ix] = tok;
        tok = str_tok(NULL, SH_TOKEN_DELIMS);
        ix++;
    }
    tokens[ix] = NULL;
    return tokens;
}


bool _is_env_variable(char* tok)
{
    return tok && tok[0] == '$';
}


bool _is_path_variable(char* tok)
{
    return tok && (tok[0] == '/' || tok[0] == '.' || tok[0] == '~');
}


char **sh_expand_args(char** args)
{
    int i = 0;
    char **expanded_args = strstr_copy(args);
    char **expanded_args_begin = expanded_args;
    for (char *tok = *expanded_args; tok != NULL; tok=*++expanded_args) {
        if (_is_env_variable(tok)) {
            printf("envtok: <%s>\n", tok);
            char* env_val = getenv(tok + 1);
            if (!env_val){
                /* DO SOMETHING */
            }
            free(expanded_args[i]);
            expanded_args[i] = env_val;
        }
    }
    return expanded_args_begin;
}


char** sh_resolve_args(char** args)
{
    int i = 0;
    char **resolved_args = strstr_copy(args);
    char **resolved_args_begin = resolved_args;
    for (char *tok = *resolved_args; tok != NULL; tok=*++resolved_args) {
        if (_is_path_variable(tok)){
            printf("pathtok: <%s>\n", tok);
            char* resolved_path = malloc((SH_PATH_BUFFSIZE) * sizeof(char));
            printf("realpathtok: <%s>\n", realpath(tok, resolved_path));
        }
    }
    return resolved_args_begin;
}


void sh_prompt()
{
    printf("%s@%s :: %s => ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));
}


void sh_loop()
{
    char *line;
    char **args, **expanded_args;
    do {
        sh_prompt();
        line = sh_read_line();
        args = sh_parse_line(line);
        _print_args(args);
        /* expand env variables, resolve paths */
        expanded_args = sh_expand_args(args);
        _print_args(expanded_args);
        /* cleanup */
        free(line);
        free(args);
        free(expanded_args);
    } while(1);
}



int main(int argc, char **argv)
{
    sh_loop();

    return 0;
}






