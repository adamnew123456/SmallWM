/* Recieve and handle X events */
#include "event.h"

// Creates a new empty event table
events_t *event_init(smallwm_t *smallwm)
{
    events_t *events = malloc(sizeof(events_t));
    events->event_callbacks = new_table();
    events->key_callbacks = new_table();
    return events;
}

// Adds a keyboard event handler
void add_keyboard_handler_event(events_t *events, KeySym key, event_callback_t *callback)
{
    add_table(events->key_callbacks, key, callback);
}

// Adds a general event handler
void add_handler_event(events_t *events, int event, event_callback_t *callback)
{
    add_table(events->event_callbacks, event, callback);
}
