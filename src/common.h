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
#include <utility>
#include <unistd.h>

/// A size of some kind.
typedef unsigned int Dimension;
/// The extend of some window or other surface.
typedef std::pair<Dimension,Dimension> Dimension2D;

// Some convenience wrappers for accessing width-height pairs
#define DIM2D_WIDTH(dim2d) ((dim2d).first)
#define DIM2D_HEIGHT(dim2d) ((dim2d).second)
// More convenience wrappers for accessing x-y pairs
#define DIM2D_X(dim2d) ((dim2d).first)
#define DIM2D_Y(dim2d) ((dim2d).second)

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
    
/// The button to click to launch a terminal
const int LAUNCH_BUTTON = 1,
      /// The button to click to move a client
      MOVE_BUTTON = 1,
      /// The button to click to resize a client
      RESIZE_BUTTON = 3,
      /// The key to hold to activate window manager functions
      BUTTON_MASK = Mod4Mask;

#endif
