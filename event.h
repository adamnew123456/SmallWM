#ifndef __SMALLWM_EVENT__
#define __SMALLWM_EVENT__

#include <signal.h>
#include <stdlib.h>

#include "client.h"
#include "icon.h"
#include "struct.h"
#include "wm.h"
#include "x11.h"

// A generic event callback
typedef void(*event_callback_t)(smallwm_t*, XEvent*);

events_t *event_init(smallwm_t *wm);
void add_keyboard_handler_event(events_t *events, KeySym key, event_callback_t callback);
void add_handler_event(events_t *events, int event, event_callback_t callback);
void run_loop_event(events_t *events);

void on_keypress_event(smallwm_t *wm, XEvent *event);
void on_buttonpress_event(smallwm_t *wm, XEvent *event);
void on_buttonrelease_event(smallwm_t *wm, XEvent *event);
void on_motionnotify_event(smallwm_t *wm, XEvent *event);
void on_mapnotify_event(smallwm_t *wm, XEvent *event);
void on_expose_event(smallwm_t *wm, XEvent *event);
void on_destroynotify_event(smallwm_t *wm, XEvent *event);

// A better way to organize default event callbacks than array pairs
typedef struct {
    int type;
    event_callback_t callback;
} event_pair_t;

static event_pair_t event_callbacks[] = {
    {KeyPress, on_keypress_event}, {ButtonPress, on_buttonpress_event}, 
    {ButtonRelease, on_buttonrelease_event}, {MotionNotify, on_motionnotify_event},
    {MapNotify, on_mapnotify_event}, {Expose, on_expose_event}, {DestroyNotify, on_destroynotify_event},
    {-1, NULL}};

void do_raise_event(smallwm_t *wm, XEvent *event);
void do_lower_event(smallwm_t *wm, XEvent *event);
void do_maximize_event(smallwm_t *wm, XEvent *event);
void do_close_event(smallwm_t *wm, XEvent *event);
void do_kill_event(smallwm_t *wm, XEvent *event);
void do_hide_event(smallwm_t *wm, XEvent *event);
void do_desktopnext_event(smallwm_t *wm, XEvent *event);
void do_desktopprev_event(smallwm_t *wm, XEvent *event);
void do_stick_event(smallwm_t *wm, XEvent *event);
void do_snapleft_event(smallwm_t *wm, XEvent *event);
void do_snapright_event(smallwm_t *wm, XEvent *event);
void do_snapup_event(smallwm_t *wm, XEvent *event);
void do_snapdown_event(smallwm_t *wm, XEvent *event);
void do_movetodesktopnext_event(smallwm_t *wm, XEvent *event);
void do_movetodesktopprev_event(smallwm_t *wm, XEvent *event);
void do_endwm_event(smallwm_t *wm, XEvent *event);

static event_pair_t keysym_callbacks[] = {
    {XK_Page_Up, do_raise_event}, {XK_Page_Down, do_lower_event}, {XK_m, do_maximize_event},
    {XK_c, do_close_event}, {XK_x, do_kill_event}, {XK_h, do_hide_event},
    {XK_bracketright, do_movetodesktopnext_event}, {XK_bracketleft, do_movetodesktopprev_event},
    {XK_backslash, do_stick_event}, {XK_Left, do_snapleft_event}, {XK_Right, do_snapright_event},
    {XK_Up, do_snapup_event}, {XK_Down, do_snapdown_event}, {XK_Escape, do_endwm_event},
    {XK_period, do_desktopnext_event}, {XK_comma, do_desktopprev_event}, {-1, NULL}};
#endif
