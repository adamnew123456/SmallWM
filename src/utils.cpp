/** @file */
#include "utils.h"

/**
 * Removes characters from a string, irrespective of position.
 *
 * @param text The text to remove from.
 * @param remove A list of all characters to remove from the text.
 * @param buffer The location to put the output (must be at least as long as the input).
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

/**
 * Tries to convert a string to an unsigned integer.
 *
 * @param[in] string The text to convert.
 * @param default_ The default value to return if there is an error.
 * @return The parsed value, or the default.
 */
unsigned long try_parse_ulong(const char *string, unsigned long default_)
{
    // strtoul is a little weird about negatives, and thus we have to filter
    // them out beforehand.
    if (strchr(string, '-'))
        return default_;

    // Since we have to make sure that the ending value indicates the end of the
    // value (instead of a non-numeric character), we have to strip the spaces
    // from the string.
    char *buffer = new char[strlen(string)];
    strip_string(string, " \f\t\r\n\v", buffer);

    char *end_of_string;
    unsigned long result = strtoul(buffer, &end_of_string, 0);


    if (*end_of_string != '\0')
        result = default_;

    // This has to happen, since GCC causes the buffer to be cleared to zeroes.
    //
    // Since we use a comparison to zero when figuring out whether or not our
    // conversion succeeded, we have to wait until here to get rid of the
    // buffer.
    delete[] buffer;
    return result;
}

/**
 * Tries to convert a string to an unsigned integer, returning the default if
 * the input is equal to 0.
 *
 * @param[in] string The text to convert.
 * @param default_ The default value to return if there is an error.
 * @return The parsed value, or the default.
 */
unsigned long try_parse_ulong_nonzero(const char *string, unsigned long default_)
{
    unsigned long result = try_parse_ulong(string, default_);
    if (result == 0)
        return default_;
    else
        return result;
}
