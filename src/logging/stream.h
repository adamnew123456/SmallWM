/** @file */
#ifndef __SMALLWM_LOGGING_SYSLOG__
#define __SMALLWM_LOGGING_SYSLOG__

#include "logging/logging.h"

/**
 * A Log which writes to a C++ ostream.
 */
class StreamLog : public Log
{
public:
    StreamLog(std::ostream &stream) :
        m_stream(stream), m_closed(false)
    {}

    void stop();

    Log &log(int);
    void write(std::string&);
    void flush();

private:
    // The stream to write log messages to
    std::ostream &m_stream;

    // Whether or not the log is closed for future writes
    bool m_closed;
};

#endif
