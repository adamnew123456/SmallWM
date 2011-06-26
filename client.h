#ifndef __CLIENT__
#define __CLIENT__

#include "global.h"

typedef enum {
	Visible,
	Hidden,
	MoveResz
} WState;

typedef struct {
	Window win;
	GC gc;
	int x,y;
} icon_t;

typedef struct client_s client_t;
struct client_s {
	Window win, pholder;
	icon_t *icon;
	char *title;
	int x,y;
	unsigned int w, h;
	WState state;
	int class;
	client_t *next;
};

extern client_t *head;
extern client_t *focused;

client_t *tail();
client_t *fromevent(XEvent ev);
client_t *fromicon(Window icon);
client_t *fromwin(Window win);

client_t *create(Window w);
void destroy(client_t *, int);
void hide(client_t *);
void unhide(client_t*, int);

void raise_(client_t *);
void lower(client_t *);

void beginmvrsz(client_t *);
void endmvrsz(client_t *);

void paint(client_t *);
void updicons();

void chfocus(client_t *);
#endif
