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


/**
 * str_split - split a string into by any of the `delims` into an array of tokens
 * @return: the array of tokens, NULL if failed to allocate
 * NOTE: the original string is left unmodified
 */
char **str_split(char* str, const char* delims);


/*
 * strstr_copy - very unsafe function to copy the args (char**) into a copy (char**)
 * @return: the copy, NULL if allocation failed
 */
char** strstr_copy(char** src);

/**
 * str_replace - replaces all occurences of `rep` with `with`
 * @orig: the original string to which replacing should be applied
 * @rep: the substring to replace
 * @with: the replacement substring
 * @return: the result of the replacement(s), will be NULL if invalid call, no room, etc.
 */
char *str_replace(char *orig, char *rep, char *with);


/**
 * str_combine - combines str1 and str2 in a copy
 * @return: COPY of the two strings combined, NULL if failed to allocate
 */
char *str_combine(char *str1, char *str2);
