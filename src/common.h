/** @file */
#ifndef __SMALLWM_COMMON__
#define __SMALLWM_COMMON__

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <tuple>

/// A size of some kind.
typedef unsigned int Dimension;
/// The extend of some window or other surface.
typedef std::tuple<Dimension,Dimension> Dimension2D;
/// The desktop number.
typedef unsigned long long Desktop;
/// The z-layer of a window (1 to 9).
typedef unsigned char Layer;

#endif
