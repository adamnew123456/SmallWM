/** @file */
#ifndef __SMALLWM_COMMON__
#define __SMALLWM_COMMON__

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <tuple>
#include <unistd.h>

/// A size of some kind.
typedef unsigned int Dimension;
/// The extend of some window or other surface.
typedef std::tuple<Dimension,Dimension> Dimension2D;

// Some convienence wrappers for accessing width-height pairs
#define DIM2D_WIDTH(dim2d) (std::get<0>((dim2d)))
#define DIM2D_HEIGHT(dim2d) (std::get<1>((dim2d)))
// More convienence wrappers for accessing x-y pairs
#define DIM2D_X(dim2d) (std::get<0>((dim2d)))
#define DIM2D_Y(dim2d) (std::get<1>((dim2d)))

/// The desktop number.
typedef unsigned long long Desktop;
/// The z-layer of a window.
typedef unsigned char Layer;

/// The maximum layer of any non-dialog window.
const Layer MAX_LAYER = 9,
      /// The lowest layer for any window.
      MIN_LAYER = 1,
      /// The layer of dialogs, which are on top of everything else.
      DIALOG_LAYER = 10,
      /// The default layer assigned to all windows.
      DEF_LAYER = 5;
    
/// A value indicating that the current desktop value should be used.
const Desktop THIS_DESKTOP = -1;

/// The button to click to launch a terminal
const int LAUNCH_BUTTON = 1,
      /// The button to click to move a client
      MOVE_BUTTON = 1,
      /// The button to click to resize a client
      RESIZE_BUTTON = 3,
      /// The key to hold to activate window manager functions
      BUTTON_MASK = Mod4Mask;

#endif
