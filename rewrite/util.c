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
