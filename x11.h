/* All the headers required from X11 */
#ifndef __SMALLWM_X11__
#define __SMALLWM_X11__

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

// Mouse buttons for performing specific commands
#define MOVE 1 // Moves a window
#define RESZ 3 // Resizes a window
#define LAUNCH 1 // Launches a shell

// The modifier key which must be pressed
#define MASK Mod4Mask
#endif
