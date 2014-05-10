/** @file */
#ifndef __SMALLWM_LOGGING__
#define __SMALLWM_LOGGING__

#include <ostream>
#include <sstream>
#include <stack>
#include <string>

#include <syslog.h>
#include <unistd.h>

/**
 * A C++ friendly wrapper around the standard Unix syslog API, but adapted to the
 * ostreams syntax.
 */
class SysLog
{
public:
    SysLog() :
        m_identity("my-program"),
        m_options(0),
        m_facility(0),
        m_priority(LOG_INFO)
    {};

    /**
     * Terminates the connection to syslog when the object is destroyed.
     */
    ~SysLog()
    {
        stop();
    };

    void add_option(int);
    void remove_option(int);

    void set_identity(std::string);
    SysLog &set_priority(int);
    void set_facility(int);
    void set_log_mask(int);

    void start();
    void stop();

    /// The type of SysLog::endl
    typedef SysLog& (*SysLogManipulator)(SysLog&);

    /**
     * The general output routine for sending data to syslog.
     * @note That the message will not be sent to syslog until SysLog::endl is
     *  provided to this method (`me << ... << SysLog::endl;`).
     * @param value The value to append to the current message.
     */
    template<typename T>
    SysLog &operator<<(T value)
    {
        m_formatter << value;
        return *this;
    };

    SysLog &operator<<(SysLogManipulator manipulator);
    static SysLog &endl(SysLog &stream);

private:
    // Whether or not the user actually started to log something
    bool m_started;
    // What identity to use for this logger
    std::string m_identity;
    // What options to send to syslog
    int m_options;
    // What facility to use for syslog
    int m_facility;
    // How sever the message is
    int m_priority;
    // A string stream kept around to punt the operator<< formatting duties to
    std::ostringstream m_formatter;
};

#endif
