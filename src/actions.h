#ifndef __SMALLWM_ACTIONS__
#define __SMALLWM_ACTIONS__

#include "common.h"

/// All the types of class actions, for use as bit flags
const unsigned int ACT_STICK = 1 << 0,
      ACT_MAXIMIZE = 1 << 1,
      ACT_SETLAYER = 1 << 2,
      ACT_SNAP = 1 << 3;

/// The side of the screen to snap a window to. 
enum SnapDir
{
    SNAP_NONE,
    SNAP_TOP, 
    SNAP_BOTTOM,
    SNAP_LEFT,
    SNAP_RIGHT,
};

/**
 * A grouping of class actions which are applied to all clients of a particular class.
 *
 * ("Class" here refers to class in the X11 sense)
 */
struct ClassActions
{
    ClassActions() : actions(0), snap(SNAP_NONE), layer(0) {}
    
    /// All the actions which are applied; the flags are the values of ACT_*
    unsigned int actions;

    /// The direction to snap a window
    SnapDir snap;

    /// The layer to place the client on
    Layer layer;
};

#endif
