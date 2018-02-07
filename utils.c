#include <string.h>
#include <stdlib.h>
#include "utils.h"


char* str_tok(char* str, const char* delims)
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


char** strstr_copy(char** src)
{
    /* 255 is a simple hard upperbound (see specification) */
    char** cpy = malloc((256) * sizeof(char*));
    int i = 0;
    char* s = *src;
    for (;s != NULL && i < 256; s=*++src, i++)
        cpy[i] = strdup(s);
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
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

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
