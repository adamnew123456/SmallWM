#include "event.h"
#include "struct.h"
#include "wm.h"
#include "x11.h"

int main()
{
    smallwm_t *wm = init_wm();
    events_t *events = event_init(wm);

    add_handler_event(events, wm->xrandr_event_offset + RRScreenChangeNotifyMask, 
                      (event_callback_t)set_size_wm);
    run_loop_event(events);
}
