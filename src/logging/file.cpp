/** @file */
#include "logging/file.h"

/**
 * Stops accepting log messages.
 */
void FileLog::stop()
{
    m_stopped = true;
}

/**
 * Sets the current log message priority, and starts building up a log message.
 */
Log &FileLog::log(int priority)
{
    m_priority = priority;
    return *this;
}

/**
 * Adds a new fragment to the current log message.
 */
void FileLog::write(std::string &str)
{
    m_formatter << str;
}

/**
 * Completes the current log message and writes it to the log file.
 */
void FileLog::flush()
{
    if (!m_stopped && m_priority < m_level)
    {
        std::fstream output(m_filename.c_str(),
                            std::fstream::out | std::fstream::app);

        output << std::time(NULL) 
            << " "
            << m_priority
            << ": "
            << m_formatter.str()
            << "\n";

        output.close();
    }

    m_formatter.str("");
}
