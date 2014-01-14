#ifndef __SMALLWM_UTIL__
#define __SMALLWM_UTIL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    SUCCESS, FAIL
} status_t;

unsigned long long string_to_long(const char *, status_t *);
void die(const char *message);
unsigned int string_hash(const char *text);
unsigned int count_occurences(const char *text, char c);
char *strip_string(const char *text, const char *remove);
#endif
