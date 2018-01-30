#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SH_LINE_BUFFSIZE 255
#define SH_TOKEN_BUFFSIZE 255
#define SH_TOKEN_DELIMS " \t\n\r"


void _print_args(char** args) {
    for (char *s = *args; s != NULL; s=*++args)
        printf("%s ", s);
    printf("\n");
}

/**
 * sh_read_line - read shell user input from stdin
 * @return: char * to beginning of line
 * Assumption: The user input will be no more than 255 characters
*/
char *sh_read_line()
{
    size_t ix = 0;
    char c;
    char *buffer = malloc(sizeof(char) * SH_LINE_BUFFSIZE);
    if (!buffer) {
        perror("Failed to allocate line buffer");
        return 0;
    }
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

/**
 * sh_strtok - sequence of calls to this function split str into tokens, which are sequences of contiguous characters
 *             separated by any of the characters that are part of delimiters.
 * @str: on first call, str is start of line to be tokenized, on subsequent calls  the function expects a null pointer
         and uses the position right after the end of the last token as the new starting location for scanning.
 * @return: token
 */
char *sh_strtok(char* str, const char* delims)
{
    static char* lasts;
    int ch;
    if (str == 0)
        str = lasts;
    /* move str up until not at delim or end of string */
    do {
        if ((ch = *str++) == '\0')
            return NULL;
    } while (strchr(delims, ch));
    --str;
    lasts = str + strcspn(str, delims);
    if (*lasts != 0)
        *lasts++ = 0;
    return str;
}

/**
 * sh_parse_line - parse a line into tokens for future execution
 * @line: Line to be parsed
 * @return: array of tokens
 */
char **sh_parse_line(char *line)
{
    /* User input is restricted to 255 chars, so a simple upperbound on tokens is 255 */
    char **tokens = malloc((SH_TOKEN_BUFFSIZE) * sizeof(char*));
    if (!tokens) {
        perror("Failed to allocate tokens buffer");
        return NULL; /* EXIT? */
    }
    size_t ix = 0;
    char *tok = sh_strtok(line, SH_TOKEN_DELIMS);
    while (tok != NULL) {
        tokens[ix] = tok;
        tok = sh_strtok(NULL, " ");
        ix++;
    }
    tokens[ix] = NULL;
    return tokens;
}


bool _is_env_variable(char* tok)
{
    return tok && tok[0] == '$';
}

/**
 * strstrcpy - very unsafe function to copy the args (char**) into a copy (char**)
*/
char **strstrcpy(char** src) {
    char** cpy = malloc((SH_TOKEN_BUFFSIZE) * sizeof(char*));
    int i = 0;
    char* s = *src;
    for (;s != NULL && i < SH_TOKEN_BUFFSIZE; s=*++src, i++)
        cpy[i] = strdup(s);
    cpy[i] = NULL;
    return cpy;
}

/*
 * sh_expand_args - expand environment variables in args
 * @args: array of char* denoting the arguments
 * @returns: copy of args with environment variables expanded
 */
char **sh_expand_args(char** args)
{
    int i = 0;
    char **cpy = strstrcpy(args);
    char **cpy_begin = cpy;
    for (char *s = *cpy; s != NULL; s=*++cpy) {
        if (_is_env_variable(s)) {
            char* env_val = getenv(s + 1);
            if (!env_val){
                /* DO SOMETHING */
            }
            free(cpy[i]);
            cpy[i] = env_val;
        }
    }
    return cpy_begin;
}

/**
 * sh_start - loop getting commands and executing them
 */
void sh_start()
{
    char *line;
    char **args, **expanded_args;
    do {
        printf("> ");
        line = sh_read_line();
        args = sh_parse_line(line);
        _print_args(args);
        expanded_args = sh_expand_args(args);
        _print_args(expanded_args);
        free(line);
        free(args);
    } while(1);
}

int main(int argc, char **argv)
{
    sh_start();
    return 0;
}




