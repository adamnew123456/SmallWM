/** @file */
#ifndef __SMALLWM_COMMON__
#define __SMALLWM_COMMON__

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>
#include <unistd.h>

#include "logging.h"

/// A size of some kind.
typedef int Dimension;
/// The extend of some window or other surface.
typedef std::pair<Dimension,Dimension> Dimension2D;

// Some convenience wrappers for accessing width-height pairs
#define DIM2D_WIDTH(dim2d) ((dim2d).first)
#define DIM2D_HEIGHT(dim2d) ((dim2d).second)
// More convenience wrappers for accessing x-y pairs
#define DIM2D_X(dim2d) ((dim2d).first)
#define DIM2D_Y(dim2d) ((dim2d).second)

// A box is exactly what is sounds like - a rectangular region
struct Box {
    Box(): x(0), y(0), width(-1), height(-1)
    {}

    Box(int _x, int _y, Dimension _width, Dimension _height) :
        x(_x), y(_y), width(_width), height(_height)
    {}

    bool operator ==(const Box &other) const
    {
        return other.x == x && other.y == y && 
                other.width == width && other.height == height;
    }

    int x, y;
    Dimension width, height;
};

static std::ostream &operator<<(std::ostream &out, const Box &box)
{
    out << "Box(" << box.x << "," << box.y << " " <<
                     box.width << "x" << box.height << ")";
    return out;
}

/**
 * Directions are used both for snapping in the configuration loader, as well
 * as for moving windows to different relative screens.
 */
enum Direction
{
    DIR_TOP = 1,
    DIR_BOTTOM,
    DIR_LEFT,
    DIR_RIGHT
};

// Note '(value) < (max)' - this is mostly used for finding screen boundaries,
// and a window on the left edge of a screen should not be considered to be on
// a different monitor
#define IN_BOUNDS(value, min, max) ((value) >= (min) && (value) < (max))

/// The z-layer of a window.
typedef unsigned char Layer;
/// A difference between two layers.
typedef char LayerDiff;

/// The maximum layer of any non-dialog window.
const Layer MAX_LAYER = 10,
      /// The default value for the layer type, representing an invalid
      /// layer
      INVALID_LAYER = 0,
      /// The lowest layer for any window.
      MIN_LAYER = 1,
      /// The layer of dialogs, which are on top of everything else.
      DIALOG_LAYER = 11,
      /// The default layer assigned to all windows.
      DEF_LAYER = 5;
    
/// The button to click to launch a terminal
const int LAUNCH_BUTTON = 1,
      /// The button to click to move a client
      MOVE_BUTTON = 1,
      /// The button to click to resize a client
      RESIZE_BUTTON = 3,
      /// The key to hold to activate window manager functions
      ACTION_MASK = Mod4Mask;

// This is useful for tests, since they insist on printing things that they
// have no knowledge of.
template <typename T, typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &out, const T &value)
{
    out << "[Unknown Value]";
    return out;
}

// Note that this would otherwise be a circular dependency between 
// model/changes.h and model/client-model.h

/**
 * How a client is positioned and scaled - all but CPS_FLOATING are managed 
 * by SmallWM.
 */
enum ClientPosScale
{
    CPS_FLOATING, //< The user has manually moved/resized the window
    CPS_SPLIT_LEFT, //< A split on the left half of the screen
    CPS_SPLIT_RIGHT,
    CPS_SPLIT_TOP,
    CPS_SPLIT_BOTTOM,
    CPS_MAX, //< The window takes up the entire viewable area
};
#endif
