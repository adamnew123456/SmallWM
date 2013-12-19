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

void on_pageup_event(smallwm_t *wm, XEvent *event);
void on_pagedown_event(smallwm_t *wm, XEvent *event);
void on_m_event(smallwm_t *wm, XEvent *event);
void on_c_event(smallwm_t *wm, XEvent *event);
void on_x_event(smallwm_t *wm, XEvent *event);
void on_h_event(smallwm_t *wm, XEvent *event);
void on_r_event(smallwm_t *wm, XEvent *event);
void on_bracketright_event(smallwm_t *wm, XEvent *event);
void on_bracketleft_event(smallwm_t *wm, XEvent *event);
void on_backslash_event(smallwm_t *wm, XEvent *event);
void on_left_event(smallwm_t *wm, XEvent *event);
void on_right_event(smallwm_t *wm, XEvent *event);
void on_up_event(smallwm_t *wm, XEvent *event);
void on_down_event(smallwm_t *wm, XEvent *event);

int keysym_types[] = { XK_Page_Up, XK_Page_Down, XK_m, XK_c, XK_x, XK_h, XK_r,
                        XK_bracketright, XK_bracketleft, XK_backslash, XK_Left,
                        XK_Right, XK_Up, XK_Down , XK_Escape, XK_comma, XK_period };
event_callback_t key_callbacks[] = { on_pageup_event, on_pagedown_event,
                                     on_m_event, on_c_event, on_x_event, on_h_event,
                                     on_r_event, on_bracketright_event, on_bracketleft_event,
                                     on_backslash_event, on_left_event, on_right_event,
                                     on_up_event, on_down_event, on_escape_event, on_comma_event,
                                     on_period_event, NULL};
#endif
