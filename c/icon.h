#ifndef __SMALLWM_ICON__
#define __SMALLWM_ICON__

#include <stdlib.h>

#include "struct.h"
#include "table.h"
#include "wm.h"
#include "x11.h"

void to_icon(client_t *client);
void place_icon(icon_t *icon, unsigned int x, unsigned int y);
void paint_icon(icon_t *icon);
void to_client(icon_t *icon);

#endif
