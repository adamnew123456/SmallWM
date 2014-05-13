/** @file */
#include "logging.h"

/**
 * Adds an option to the options flag given to syslog.
 * @param syslog_option An option, provided by <syslog.h>
 */
void SysLog::add_option(int syslog_option)
{
    m_options |= syslog_option;
};

/**
 * Removes an option from the options flag given to syslog.
 * @param syslog_option An option, provided by <syslog.h>
 */
void SysLog::remove_option(int syslog_option)
{
    m_options &= ~syslog_option;
};

/**
 * Sets the name to appear before all of the program's logs.
 * @param syslog_ident The program's name.
 */
void SysLog::set_identity(std::string syslog_ident)
{
    m_identity = syslog_ident;
};

/**
 * The minimum priority to log messages at.
 * @param syslog_priority The minimum priority level.
 */
SysLog &SysLog::set_priority(int syslog_priority)
{
    m_priority = syslog_priority;
    return *this;
};

/**
 * Sets the facility to use when logging messages.
 * @param syslog_facility The output facility to use.
 */
void SysLog::set_facility(int syslog_facility)
{
    m_facility = syslog_facility;
};

/**
 * Sets which logging messages are handled by syslog.
 * @note Use the LOG_MASK and LOG_UPTO macros to construct this flag.
 * @param mask The logging mask to let syslog handle.
 */
void SysLog::set_log_mask(int syslog_logmask)
{
    setlogmask(syslog_logmask);
}

/**
 * Begins accepting log messages.
 * @note After this, calling:
 *  - set_identity
 *  - add_options / remove_option
 *  - set_facility
 * Will do nothing.
 */
void SysLog::start()
{
    if (!m_started)
    {
        openlog(m_identity.c_str(), m_options, m_facility);
        m_started = true;
    }
};

/**
 * Stops accepting log messages.
 */
void SysLog::stop()
{
    if (m_started)
    {
        closelog();

        // Although this logger is intended to be started once, when the
        // program launches, be consistent and do this anyway.
        m_started = false;
    }
};


/**
 * Does special processing for SysLog::endl.
 * @param manipulator This should only ever by SysLog::endl.
 */
SysLog &SysLog::operator<<(SysLogManipulator manipulator)
{
    return manipulator(*this);
}

/**
 * Flushes the current messages to syslog, and then becomes ready for a new
 * message.
 * @param stream The stream to operate upon.
 */
SysLog& SysLog::endl(SysLog &stream)
{
    if (stream.m_started)
    {
        syslog(stream.m_priority, stream.m_formatter.str().c_str());
    }
        
    stream.m_formatter.str("");
    return stream;
};
