/** @file */
#ifndef __SMALLWM_LOGGING_FILE__
#define __SMALLWM_LOGGING_FILE__

#include "logging/logging.h"

#include <ctime>
#include <fstream>

class FileLog : public Log
{
public:
    FileLog(std::string filename, int level) :
        m_level(level),
        m_filename(filename)
    {}

    void stop();

    Log &log(int);
    void write(std::string &message);
    void flush();

private:
    // Whether or not the user actually started to log something
    bool m_stopped;

    // The name of the file to log messages to
    std::string m_filename;

    // What level to log messages at
    int m_level;

    // The log level of the current message
    int m_priority;

    // A string stream kept around to punt the operator<< formatting duties to
    std::ostringstream m_formatter;
};

#endif
