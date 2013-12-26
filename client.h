#ifndef __SMALLWM_CLIENT__
#define __SMALLWM_CLIENT__

#include <stdlib.h>

#include "struct.h"
#include "table.h"
#include "wm.h"
#include "x11.h"

void raise_client(client_t *client);
void lower_client(client_t *client);
void set_layer_client(client_t *client, int layer);
void begin_moveresize_client(client_t *client);
void end_moveresize_client(client_t *client);
void request_close_client(client_t *client);
void destroy_client(client_t *client);

#endif
