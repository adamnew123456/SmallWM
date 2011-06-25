#ifndef __GLOBAL__
#define __GLOBAL__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <string.h>
#include <signal.h>

static Display *dpy;
static Window root;

// Icon dimensions
#define ICON_WIDTH 75
#define ICON_HEIGHT 20

// Screen dimensions
#define SCREEN_WIDTH DisplayWidth(dpy, DefaultScreen(dpy))
#define SCREEN_HEIGHT DisplayHeight(dpy, DefaultScreen(dpy))

// Basic colors
#define BLACK BlackPixel(dpy, DefaultScreen(dpy))
#define WHITE WhitePixel(dpy, DefaultScreen(dpy))

// Utility definitions
#define MASK Mod1Mask
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define SHELL "xterm"		// The universal X shell
// #define SHELL "mrxvt" // My favorite X shell

#endif
