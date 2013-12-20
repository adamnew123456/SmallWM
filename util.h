#ifndef __SMALLWM_UTIL__
#define __SMALLWM_UTIL__

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    SUCCESS, FAIL
} status_t;

unsigned long long string_to_long(const char *, status_t *);
void die(const char *message);
#endif
