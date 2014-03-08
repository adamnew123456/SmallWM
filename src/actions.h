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
      ACT_SNAP = 1 << 3;

/** 
 * The side of the screen to snap a window to. 
 */
enum SnapDir
{
    SNAP_TOP = 1, 
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
    /// Initialize a blank ClassAction, with a few defaults.
    ClassActions() : actions(0), snap(SNAP_TOP), layer(0) {}
    
    /// All the actions which are applied; the flags are the values of ACT_*.
    unsigned int actions;

    /// The direction to snap a window.
    SnapDir snap;

    /// The layer to place the client on.
    Layer layer;
};

#endif