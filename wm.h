#ifndef __SMALLWM_WM__
#define __SMALLWM_WM__

#include <stdlib.h>
#include <unistd.h>

#include "x11.h"
#include "struct.h"
#include "table.h"
#include "util.h"
#include "inih/ini.h"

smallwm_t *init_wm();
void set_size_wm(smallwm_t *state, XEvent *event);
void shell_launch_wm(smallwm_t *state);
void update_icons_wm(smallwm_t *state);
void refocus_wm(smallwm_t *state, Window window);
void update_desktop_wm(smallwm_t *state);
void restack_wm(smallwm_t *state);
void ignore_window_wm(smallwm_t *state, Window window);
Bool should_ignore_window_wm(smallwm_t *state, Window window);

void add_client_wm(smallwm_t *wm, Window window);
#endif
