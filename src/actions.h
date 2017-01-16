/** @file */
#ifndef __SMALLWM_ACTIONS__
#define __SMALLWM_ACTIONS__

#include "common.h"

/// Indicates that an action will stick a window.
const unsigned int ACT_STICK = 1 << 0,
    /// Indicates that an action will maximize a window.
    ACT_MAXIMIZE = 1 << 1,
    /// Indicates that an action will set the layer of a window.
    ACT_SETLAYER = 1 << 2,
    /// Indicates that an action will snap the window to the screen's edge.
    ACT_SNAP = 1 << 3,
    /// Moves a client in the X direction
    ACT_MOVE_X = 1 << 4,
    /// Moves a client in the Y direction
    ACT_MOVE_Y = 1 << 5;

/**
 * A grouping of class actions which are applied to all clients of a particular class.
 *
 * ("Class" here refers to class in the X11 sense)
 */
struct ClassActions
{
    /// Initialize a blank ClassAction, with a few defaults.
    ClassActions() : actions(0), snap(DIR_TOP), layer(0),
        relative_x(0), relative_y(0)
    {}

    /// All the actions which are applied; the flags are the values of ACT_*.
    unsigned int actions;

    /// The direction to snap a window.
    Direction snap;

    /// The layer to place the client on.
    Layer layer;

    /** The relative location (0% is the left/top, 100% is the bottom/right) of
     * the window.
     */
    double relative_x, relative_y;
};

#endif
