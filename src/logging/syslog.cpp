/** @file */
#include "logging/syslog.h"

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
 * This is the main entry point to SysLog, which sets the priority and returns
 * itself for use with the << operator and Log::endl.
 * @param syslog_priority The minimum priority level.
 */
Log &SysLog::log(int syslog_priority)
{
    m_priority = syslog_priority;
    return *this;
};


/**
 * Buffers writes from the Log, so we can flush them later.
 */
void SysLog::write(std::string &str)
{
    m_formatter << str;
}

/**
 * Flushes the content of a SysLog by writing it to syslog().
 */
void SysLog::flush()
{
    if (m_started)
        syslog(m_priority, "%s", m_formatter.str().c_str());
        
    m_formatter.str("");
}

