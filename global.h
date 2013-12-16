#ifndef __GLOBAL__
#define __GLOBAL__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include <string.h>
#include <signal.h>

static Display *dpy;
static Window root;

void die(const char *, int);

// Icon dimensions
#define ICON_WIDTH 75
#define ICON_HEIGHT 20

// Screen dimensions
#define SCREEN_WIDTH(dpy) DisplayWidth(dpy, DefaultScreen(dpy))
#define SCREEN_HEIGHT(dpy) DisplayHeight(dpy, DefaultScreen(dpy))

// Basic colors
#define BLACK(dpy) BlackPixel(dpy, DefaultScreen(dpy))
#define WHITE(dpy) WhitePixel(dpy, DefaultScreen(dpy))

// Utility definitions
#define MOVE 1
#define RESZ 3
#define MASK Mod4Mask
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
extern char *SHELL;

#endif
