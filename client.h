#ifndef __CLIENT__
#define __CLIENT__

#include "global.h"

// All the states a client can be in
typedef enum {
    Visible,
    Hidden,
    MoveResz
} WState;

// Information needed to manage/draw icons
typedef struct {
    Window win;
    GC gc;
    int x, y;
} icon_t;

// All info about a client
typedef struct client_s {
    Display *dpy;
    Window win, pholder;
    icon_t *icon;
    int x, y;
    unsigned int w, h;
    WState state;
    int desktop;
    struct client_s *next;
} client_t;

extern client_t *head;
extern Window focused;

extern int current_desktop;
#define MAX_DESKTOP 5

client_t *tail();
client_t *fromicon(Window);
client_t *fromwin(Window);

client_t *create(Display *, Window);
void destroy(client_t *, int);
void hide(client_t *);
void unhide(client_t *, int);

void set_desktop();

void raise_(client_t *);
void lower(client_t *);

void beginmvrsz(client_t *);
void endmvrsz(client_t *);

void paint(client_t *);
void updicons();

void chfocus(Display *, Window);
#endif
