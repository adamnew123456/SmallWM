#ifndef __SMALLWM_COMMON__
#define __SMALLWM_COMMON__

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <tuple>

typedef unsigned int Dimension;
typedef std::tuple<Dimension,Dimension> Dimension2D;
typedef unsigned long long Desktop;
typedef unsigned char Layer;

#endif
