#ifndef __SMALLWM_EVENT__
#define __SMALLWM_EVENT__

#include <signal.h>
#include <stdlib.h>

#include "client.h"
#include "struct.h"
#include "wm.h"
#include "x11.h"

// A generic event callback
typedef void(*event_callback_t)(smallwm_t *wm, XEvent *event);

events_t *event_init(smallwm_t *smallwm);
void add_keyboard_handler_event(events_t *events, KeySym key, event_callback_t *callback);
void add_handler_event(events_t *events, KeySym key, event_callback_t *callback);
void run_loop_event(events_t *events);

void on_keypress_event(smallwm_t *wm, XEvent *event);
void on_buttonpress_event(smallwm_t *wm, XEvent *event);
void on_buttonrelease_event(smallwm_t *wm, XEvent *event);
void on_motionnotify_event(smallwm_t *wm, XEvent *event);
void on_mapnotify_event(smallwm_t *wm, XEvent *event);

int event_types[] = { KeyPress, ButtonPress, ButtonRelease, MotionNotify, MapNotify };
event_callback_t event_callbacks[] = { on_keypress_event, on_buttonpress_event,
                                       on_buttonrelease_event, on_motionnotify_event,
                                       on_mapnotify_event , NULL };

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

int keysym_types[] = { XK_Page_Up, XK_Page_Down, XK_m, XK_c, XK_x, XK_h,
                        XK_bracketright, XK_bracketleft, XK_backslash, XK_Left,
                        XK_Right, XK_Up, XK_Down , XK_Escape, XK_comma, XK_period };
event_callback_t key_callbacks[] = { do_raise_event, do_lower_event, do_close_event,
                                     do_hide_event, do_movetodesktopnext_event, do_movetodesktopprev_event,
                                     do_stick_event, do_snapleft_event, do_snapright_event,
                                     do_snapup_event, do_snapdown_event, do_endwm_event, 
                                     do_desktopnext_event, do_destkopprev_event, NULL };
#endif
