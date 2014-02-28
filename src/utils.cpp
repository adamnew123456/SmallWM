#include "utils.h"

/**
 * Removes leading and trailing characters from a string.
 *
 * @param text The text to remove from
 * @param remove A list of all characters to remove from the text
 * @param buffer The loctaion to put the output (must be at least as long as the input)
 */
void strip_string(const char *text, const char *remove, char *buffer)
{
    char *buffer_iter = buffer;

    int idx;
    for (idx = 0; text[idx] != '\0'; idx++)
    {
        if (std::strchr(remove, text[idx]) == NULL)
        {
            *buffer_iter = text[idx];
            buffer_iter++;
        }
    }

    // Ensure that the NUL terminator comes after all the content
    *buffer_iter = '\0';
}
