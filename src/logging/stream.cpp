/** @file */
#include "logging/stream.h"

/**
 * Closes a StreamLog. Note that this doesn't close the underlying output
 * stream - it just prevents anything else from being logged.
 */
void StreamLog::stop()
{
    m_closed = true;
}

/**
 * Prepares to write a log message (normally, this sets up the priority of the
 * next log, but this doesn't record that information).
 */
Log &StreamLog::log(int priority)
{
    return *this;
}

/**
 * Writes the given string to the output stream.
 */
void StreamLog::write(std::string &str)
{
    if (!m_closed)
        m_stream << str;
}

/**
 * Writes a newline to the output stream.
 */
void StreamLog::flush()
{
    if (!m_closed)
        m_stream << std::endl;
}
