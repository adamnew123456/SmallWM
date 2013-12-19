#ifndef __SMALLWM_EVENT__
#define __SMALLWM_EVENT__

#include <stdlib.h>

#include "struct.h"
#include "x11.h"

// A generic event callback
typedef void(*event_callback_t)(smallwm_t *wm, XEvent *event);

events_t *event_init(smallwm_t *smallwm);
void add_keyboard_handler_event(events_t *events, KeySym key, event_callback_t *callback);
void add_handler_event(events_t *events, KeySym key, event_callback_t *callback);
void run_loop_event(events_t *events);
#endif
