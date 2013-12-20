#include "event.h"
#include "struct.h"
#include "wm.h"
#include "x11.h"

// Prints out X errors, but doesn't crash the program like the normal handler
int x_error_handler(Display *display, XErrorEvent *event)
{
    char err_desc[500];
    XGetErrorText(display, event->error_code, err_desc, 500);

    printf("X Error: { display = %s, serial = %lu, error = '%s', request = %d, minor = %d }\n",
            XDisplayName(NULL), event->serial, err_desc, event->request_code, event->minor_code);
}

int main()
{
    // Don't crash when an error is generated
    XSetErrorHandler(x_error_handler);

    smallwm_t *wm = init_wm();
    events_t *events = event_init(wm);

    add_handler_event(events, wm->xrandr_event_offset + RRNotify, (event_callback_t)set_size_wm);
    run_loop_event(events);
}
