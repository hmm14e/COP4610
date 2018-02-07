/*
 * Utility functions to be used throughout the shell
 */


/**
 * strtok - sequence of calls to this function split str into tokens, which are sequences of contiguous characters
 *             separated by any of the characters that are part of delimiters.
 * @str: on first call, str is start of line to be tokenized, on subsequent calls  the function expects a null pointer
         and uses the position right after the end of the last token as the new starting location for scanning.
 * @return: token
 */
char* str_tok(char* str, const char* delims);

/*
 * strstr_copy - very unsafe function to copy the args (char**) into a copy (char**)
 * @return: the copy, NULL if allocation failed
 */
char** strstr_copy(char** src);

/*
 * str_replace - replaces all occurences of `rep` with `with`
 * @orig: the original string to which replacing should be applied
 * @rep: the substring to replace
 * @with: the replacement substring
 * @return: the result of the replacement(s), will be NULL if invalid call, no room, etc.
 */
char *str_replace(char *orig, char *rep, char *with);
