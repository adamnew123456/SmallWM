/** @file */
#ifndef __SMALLWM_LOGGING__
#define __SMALLWM_LOGGING__

#include <ostream>
#include <sstream>
#include <string>

#include <syslog.h>
#include <unistd.h>

/**
 * A basic logging API, which can be used to define the various kinds of
 * loggers.
 */
class Log
{
public:
    virtual void stop() = 0;

    virtual Log &log(int) = 0;
    virtual void write(std::string&) = 0;
    virtual void flush() = 0;

    template<typename T>
    Log &operator<<(T value)
    {
        m_formatter.str("");
        m_formatter << value;

        std::string format_out = m_formatter.str();
        this->write(format_out);
        return *this;
    }

    Log &operator<<(Log& (*manipulator)(Log&));

    static Log &endl(Log &stream);

protected:
    // A utility object, used to input things via <<
    std::ostringstream m_formatter;
};

/// The type of Log::endl
typedef Log& (*LogManipulator)(Log&);

#endif
