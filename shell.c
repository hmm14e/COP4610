#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "shell.h"
#include "command.h"
#include "utils.h"

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


/* for each char, replace all instances of it with " <char> " */
char *sh_add_whitespace(char *line, char *chars)
{
    char old[2], new[4];
    old[1] = '\0';
    /* tmp will hold intermediary stages */
    char *ret, *tmp = calloc(512, sizeof(char));
    strcpy(tmp, line);
    for (int i = 0; i < strlen(chars); i++) {
        old[0] = chars[i];
        sprintf(new, " %s ", old);
        ret = str_replace(tmp, old, new);
        if (!ret)
            continue;
        free(tmp);
        tmp = ret;
    }
    return ret;
}


char **sh_parse_line(char *line)
{
    /* User input is restricted to 255 chars, so a simple upperbound on tokens is 255 */
    char **tokens = calloc(SH_TOKEN_BUFFSIZE, sizeof(char*));
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
    if (strlen(tok) <= 1)
        return false;
    /* env variable is of form $[A-z][A-z0-9]* */
    return tok && tok[0] == '$' && isalpha(tok[1]);
}


/* returns length of environment variable, $[A-z][A-z0-9]* */
int _get_env_var_len(char *tok)
{
    for (int i = 0; i < strlen(tok); i++) {
        if (tok[i] != '$' && !isalnum(tok[i])) {
            return i;
        }
    }
    return strlen(tok);
}


bool _is_path_variable(char* tok)
{
    return tok && (strchr(tok, '/') || tok[0] == '/' || tok[0] == '.' || tok[0] == '~');
}


/*
 * expands all environment variables found in the args
 * assumes that environment variables only start at the beginning of a token
 */
char **sh_expand_env_vars(char** args)
{
    /* create copy instead of modifying in place */
    char **expanded_args = strstr_copy(args);
    for(int i = 0; expanded_args[i] != NULL; i++){
        char *arg = expanded_args[i];
        if (_is_env_variable(arg)) {
            /* copy the env_var name into separate variable */
            int env_var_len = _get_env_var_len(arg); /* including $ */
            char *env_var = calloc(env_var_len + 1, sizeof(char));
            if (!env_var)
                continue;
            strncpy(env_var, arg, env_var_len);

            /* lookup the value, and replace the variable with the actual value */
            char *env_var_val = getenv(env_var + 1); /* +1 excludes $ */
            if (!env_var_val){
                fprintf(stderr, "environment variable not found\n");
                continue;
            }

            char *expanded = str_replace(arg, env_var, env_var_val);
            if (!expanded)
                continue;

            /* update to expanded argen */
            free(expanded_args[i]);
            expanded_args[i] = expanded;
        }
    }
    return expanded_args;
}


/*
 * resolve paths (any token containing a '/')
 * also for commands search the $PATH for the relevant path
 * e.g. python prog.py -> /usr/bin/python prog.py
 */
char** sh_resolve_paths(char** args)
{
    int i = 0;
    /* create copy instead of modifying in place */
    char **rargs = strstr_copy(args);
    char **it = rargs;
    while (*it) {
        /* realpath doesn't handle tilde, so expand if found before*/
        if (rargs[i][0] == '~') {
            char* new_tok = str_replace(rargs[i], "~", getenv("HOME"));
            free(rargs[i]);
            rargs[i] = new_tok;
        }

        /* now handle path expansion with realpath() */
        if (_is_path_variable(rargs[i])) {
            char* resolved_path = calloc(PATH_MAX, sizeof(char));
            char* ret = realpath(rargs[i], resolved_path);
            if (!ret) {
                perror("couldn't resolve path");
                free(resolved_path);
                exit(1);
            }
            free(rargs[i]);
            rargs[i] = resolved_path;
        }
        /* if not builtin and not filepath search $PATH*/
        else if (i == 0 || strcmp(rargs[i-1], "|") == 0) {

        }
        it++;
        i++;
    }

    return rargs;
}


void sh_prompt()
{
    printf("%s@%s :: %s => ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));
}


void sh_loop()
{
    char *line, *whitespaced_line;
    char **args, **expanded_args, **resolved_args;
    do {
        sh_prompt();
        line = sh_read_line();

        whitespaced_line = sh_add_whitespace(line, "<>|&");
        printf("after adding whitespace: %s\n", whitespaced_line);

        args = sh_parse_line(whitespaced_line);
        printf("after parsing line: ");
        _print_args(args);


        /* expand env variables, resolve paths */
        expanded_args = sh_expand_env_vars(args);
        printf("after expanding env vars: ");
        _print_args(expanded_args);

        resolved_args = sh_resolve_paths(expanded_args);
        printf("after resolving paths: ");
        _print_args(resolved_args);

        /* cleanup */
        free(line);
        free(whitespaced_line);
        free(args);
        free(expanded_args);
        free(resolved_args);
    } while(1);
}



int main(int argc, char **argv)
{
    sh_loop();

    return 0;
}






