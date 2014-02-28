/* Various utilities which can be tested apart from the SmallWM. */
#include "util.h"

// A simpler version of strtoul which has a clear status flag
unsigned long long string_to_long(const char *string, status_t *status)
{
    char *str_error;
    unsigned long long x = strtoul(string, &str_error, 0);
    *status = (*str_error == '\0' ? SUCCESS : FAIL);
    return x;
}

// Exits the process immediately
void die(const char *message)
{
    fprintf(stderr, "SmallWM Terminated: %s\n", message);
    exit(1);
}

// The classic `ndbm` string hash - copied from here: http://www.cse.yorku.ca/~oz/hash.html
unsigned int string_hash(const char *text)
{
    unsigned int hash = 0;

    int idx;
    for (idx = 0; text[idx] != '\0'; idx++)
    {
        hash = text[idx] + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

// Counts the occurences of a character in a string
unsigned int count_occurences(const char *text, char c)
{
    unsigned int found = 0;

    int idx;
    for (idx = 0; text[idx] != '\0'; idx++)
    {
        if (text[idx] == c) 
            found++;
    }

    return found;
}

// Strips characters from a string
char *strip_string(const char *text, const char *remove)
{
    char *buffer = malloc(sizeof(char) * strlen(text));
    char *buffer_iter = buffer;

    int idx;
    for (idx = 0; text[idx] != '\0'; idx++)
    {
        if (strchr(remove, text[idx]) == NULL)
        {
            *buffer_iter = text[idx];
            buffer_iter++;
        }
    }

    *buffer_iter = '\0';

    return buffer;
}
