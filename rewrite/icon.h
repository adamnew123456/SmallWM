#ifndef __SMALLWM_ICON__
#define __SMALLWM_ICON__

#include <stdlib.h>

#include "struct.h"
#include "wm.h"
#include "x11.h"

void place_icon(icon_t *icon, unsigned int x, unsigned int y);
void paint_icon(icon_t *icon);

#endif
