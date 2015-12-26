/** @file */
#include "changes.h"

/**
 * Returns true if there are changes to be processed, or  false otherwise.
 */
bool ChangeStream::has_more()
{
    return !m_changes.empty();
}

/**
 * Gets the next change, or NULL if no changes remain.
 *
 * Note that this change should be deleted after the caller is finished with
 * it.
 */
ChangeStream::change_ptr ChangeStream::get_next()
{
    if (has_more())
    {
        change_ptr change = m_changes.front();
        m_changes.pop();
        return change;
    }
    else
        return 0;
}

/**
 * Pushes a change into the change buffer.
 */
void ChangeStream::push(change_ptr change)
{
    m_changes.push(change);
}

/**
 * Removes all changes which are still stored.
 */
void ChangeStream::flush()
{
    change_ptr change;
    while ((change = get_next()) != 0)
        delete change;
}
