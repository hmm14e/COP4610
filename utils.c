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


char *str_tok(char * str, const char * delim)
{
    static char *saves;
    register int ch;

    if (s == 0)
        s = saves;
    do {
        if ((ch = *s++) == '\0')
            return 0;
    } while (strchr(delim, ch));
    --s;
    saves = s + strcspn(s, delim);
    if (*saves != 0)
        *saves++ = 0;
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


char *str_replace(char *orig, char *rep, char *with)
{
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep || !with)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count)
        ins = tmp + len_rep;

    // malloc space with the proper replaced str size
    tmp = result = calloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
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
