/** @file */
#include "logging/logging.h"

/**
 * Does special processing for Log::endl.
 * @param manipulator This should only ever by Log::endl.
 */
Log &Log::operator<<(LogManipulator manipulator)
{
    return manipulator(*this);
}

/**
 * Flushes the current messages to syslog, and then becomes ready for a new
 * message.
 * @param stream The stream to operate upon.
 */
Log &Log::endl(Log &stream)
{
    stream.flush();
    return stream;
};
