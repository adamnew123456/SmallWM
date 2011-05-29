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

#define IWIDTH 75
#define IHEIGHT 20

#define SWIDTH DisplayWidth(dpy, DefaultScreen(dpy))
#define SHEIGHT DisplayHeight(dpy, DefaultScreen(dpy))

#define BLACK BlackPixel(dpy, DefaultScreen(dpy))
#define WHITE WhitePixel(dpy, DefaultScreen(dpy))

#define MASK Mod1Mask
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define SHELL "xterm"		// The universal X shell
//#define SHELL "mrxvt" // My favorite X shell

#endif
