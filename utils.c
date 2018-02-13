#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"


void _free2d(char **arr)
{
    for (int i = 0; arr[i] != NULL; i++)
        free(arr[i]);
    free(arr);
}


char *str_tok(char * s, const char * delim)
{
    static char *saves;
    register int ch;

    if (!s)
        s = saves;
    do {
      ch = *s;
      s++;
        if (ch == '\0'){
         return 0;
        }
    } while (strchr(delim, ch));
    s--;
    saves = s + strcspn(s, delim);
    if (*saves != 0){
       *saves = 0;
       saves++;
    }
    return s;
}


/* split the str by delims into tokens */
char **str_split(char *str, const char *delims)
{
    /* first create copy of str since sh_strtok modifies in place */
    char *cpy = calloc(strlen(str) + 1, sizeof(char));
    strcpy(cpy, str);

    int n_delims = 0;
    for (int i = 0; i < strlen(cpy); i++)
        if (strchr(delims, cpy[i]))
            n_delims++;

    /* n_delims splits a str into (n_delims + 1) tokens */
    char **tokens = calloc(n_delims + 2, sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "failed to allocate\n");
        free(cpy);
        return NULL;
    }
    /* use strtok to break up cpy into tokens */
    char *tok = cpy;
    int i = 0;
    while ((tok = str_tok(tok, delims)) != NULL) {
        tokens[i++] = strdup(tok);
        tok = NULL; /* see sh_strtok implementation */
    }
    free(cpy);
    return tokens;
}


char** strstr_copy(char** src)
{
    /* unsafe way to find the length of `src` */
    int i;
    for (i = 0; src[i] != NULL; i++);

    /* assign duplicate of each string in `src` to `cpy` */
    char** cpy = calloc((i + 1), sizeof(char*));
    if (!cpy)
        return NULL;
    for (i = 0; src[i] != NULL; i++)
        cpy[i] = strdup(src[i]);
    cpy[i] = NULL;
    return cpy;
}


char *str_replace(char *first, char *new, char *with)
{
    char *finish;
    char *here;
    char *test;
    int distance;
    int replace;
    int new_and_last;
    int number;

    if (!with || !first || !new)
        return NULL;
    distance = strlen(new);
    if (distance == 0)
        return NULL;
    replace = strlen(with);

    here = first;
    for (number = 0; (test = strstr(here, new)); ++number)
        here = test + distance;

    test = finish = calloc(strlen(first) + (replace - distance) * number + 1, sizeof(char));

    if (!finish)
        return NULL;

    while (number--) {
        here = strstr(first, new);
        new_and_last = here - first;
        test = strncpy(test, first, new_and_last) + new_and_last;
        test = strcpy(test, with) + replace;
        first += new_and_last + distance;
    }
    strcpy(test, first);
    return finish;
}


/* combine str1 and str2 into a copy */
char *str_combine(char *str1, char *str2)
{
    size_t len1 = strlen(str1), len2 = strlen(str2);
    char *combined = calloc(len1 + len2 + 1, sizeof(char));
    for (int i = 0; i < len1; i++)
        combined[i] = str1[i];
    for (int i = 0; i < len2; i++)
        /* account for offset */
        combined[len1 + i] = str2[i];
    combined[len1 + len2] = '\0';
    return combined;
}
