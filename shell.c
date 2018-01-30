#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SH_LINE_BUFFSIZE 255
#define SH_TOKEN_BUFFSIZE 255
#define SH_TOKEN_DELIMS " \t\n\r"


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

/**
 * sh_start - loop getting commands and executing them
 */
void sh_start()
{
    char *line;
    char **args;
    do {
        printf("> ");
        line = sh_read_line();
        args = sh_parse_line(line);
        free(line);
        free(args);
    } while(1);
}

int main(int argc, char **argv)
{
    sh_start();
    return 0;
}




