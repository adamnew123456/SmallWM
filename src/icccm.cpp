/** @file */
#include "icccm.h"

/**
 * Ensures that the result of a movement or resize obeys the client's requests.
 * The caller must already have an idea of how it wants to change the client's
 * size - this just "fixes" the caller's idea of the size.
 *
 * Returns the new width/height of the window.
 */
Dimension2D icccm_move_resize_check(WMShared &shared, Window window, Dimension2D current_size)
{
    Dimension width = DIM2D_WIDTH(current_size);
    Dimension height = DIM2D_HEIGHT(current_size);

    XSizeHints size_prefs;
    long flags = 0;
    XGetWMNormalHints(shared.display, window, &size_prefs, &flags);

    if (flags & PMinSize)
    {
        if (size_prefs.min_width > 0 && width < size_prefs.min_width)
            width = size_prefs.min_width;

        if (size_prefs.min_height > 0 && height < size_prefs.min_height)
            height = size_prefs.min_height;
    }

    if (flags & PMaxSize)
    {
        if (size_prefs.max_width > 0 && width > size_prefs.max_width)
            width = size_prefs.max_width;

        if (size_prefs.max_height > 0 && height > size_prefs.max_height)
            height = size_prefs.max_height;
    }

    if (flags & PResizeInc)
    {
        if (size_prefs.width_inc > 0 && width % size_prefs.width_inc != 0)
            width -= width % size_prefs.width_inc;

        if (size_prefs.height_inc > 0 && height % size_prefs.height_inc != 0)
            height -= height % size_prefs.height_inc;
    }

    return Dimension2D(width, height);
}
