#include "client.h"

client_t *head;
client_t *focused;

client_t*
tail()
{
	client_t *c = head;
	while (c->next) c = c->next;
	return c->next;
}

client_t*
fromevent(XEvent ev)
{
	client_t *c = head;
	while (c)
	{
		if (c->win == ev.xany.window)
			return c;
		c = c->next;
	}
	return NULL;
}

client_t*
fromicon(Window icon)
{
	client_t *c = head;
	while (c)
	{
		if (c->state == Hidden && c->icon->win == icon)
			return c;
		c = c->next;
	}
	return NULL;
}

client_t*
fromwin(Window win)
{
	client_t *c = head;
	while (c)
	{
		if (c->state == Visible && c->win == win)
			return c;
		c = c->next;
	}
	return NULL;
}

client_t*
create(Window w)
{
	client_t *cli = malloc(sizeof(client_t));
	XWindowAttributes attr;
	char *title = malloc(sizeof(32));

	XGetWindowAttributes(dpy, w, &attr);
	XFetchName(dpy, w, &title);

	XSetWindowBorderWidth(dpy, w, 3);

	cli->win = w;
	cli->pholder = None;
	cli->icon = NULL;
	cli->title = title;
	cli->x = attr.x;
	cli->y = attr.y;
	cli->w = attr.width;
	cli->h = attr.height;
	cli->state = Visible;
	cli->class = attr.class;
	cli->next = NULL;

	if (!head)
		head = cli;
	else
	{
		client_t *tl = tail();
		tl->next = cli;
	}

	focus(cli);

	return cli;
}

void
destroy(client_t *client, int danger)
{
	client_t *prec = head;
	client_t *succ = client->next;
	while (prec && prec->next != client)
		prec = prec->next;
	if (!prec) return;

	prec->next = succ;
	if (client->icon)
		unhide(client, 1);

	if (!danger)
		XDestroyWindow(dpy, client->win);

	free(client->title);
	free(client);

	updicons();
}

void
hide(client_t *client)
{
	if (client->state != Visible) return;

	client->icon = malloc(sizeof(icon_t));
	client->icon->win = XCreateSimpleWindow(dpy, 
				RootWindow(dpy, DefaultScreen(dpy)),
				-200, -200, ICON_WIDTH, ICON_HEIGHT, 1, BLACK, WHITE);
	XSelectInput(dpy, client->icon->win, ButtonPressMask | 
				ButtonReleaseMask | ExposureMask);
	XMapWindow(dpy, client->icon->win);

	client->icon->gc = XCreateGC(dpy, client->icon->win, 0, NULL);

	XUnmapWindow(dpy, client->win);

	client->state = Hidden; 

	updicons();
}

void
unhide(client_t *client, int danger)
{
	if (client->state != Hidden) return;

	if (!danger)
	{
		XMapWindow(client->win);
		XRaiseWindow(client->win);
	}

	XDestroyWindow(dpy, client->icon->win);
	XFreeGC(dpy, client->icon->gc);
	free(client->icon);

	client->state = Visible;

	updicons();
}

void
beginmvrsz(client_t *client)
{	
	if (client->state != MoveResz) return;
	client->state = MoveResz;

	XUnmapWindow(dpy, client->win);

	client->pholder = 
		XCreateSimpleWindow(dpy, 
			RootWindow(dpy, DefaultScreen(dpy)),
			client->x, client->y, client->w, client->h,
			1, BLACK, WHITE);
	
	XMapWindow(dpy, client->pholder);

	XGrabPointer(dpy, client->pholder, True
		PointerMotionMask | ButtonReleaseMask,
		GrabModeAsync, GrabModeAsync, None,
		None, CurrentTime);
	
	XRaiseWindow(dpy, client->pholder);
}

void
endmoversz(client_t *client)
{
	if (client->state != MoveRsz) return;
	client->state = Visible;

	XUngrabPointer(dpy, CurrentTime);

	XWindowAttributes attr;
	XGetWindowAttributes(dpy, client->pholder, &attr);
	XDestroyWindow(dpy, client->pholder);

	client->x = attr.x;
	client->y = attr.y;
	client->w = attr.w;
	client->h = attr.h;

	XMapWindow(dpy, client->win);
	XMoveResizeWindow(dpy, client->win, client->x, client->y,
		client->width, client->height);

	raise(client);
}

void
raise(client_t *client)
{
	if (cli->state != Visible) return;
	XRaiseWindow(dpy, cli->win);
}

void
lower(client_t *client)
{
	if (cli->state != Visible) return;
	XLowerWindow(dpy, cli->win);
}

void
maximize(client_t *client)
{
	client->x = 0;
	client->y = ICON_HEIGHT;
	client->w = SCREEN_WIDTH;
	client->h = SCREEN_HEIGHT - ICON_HEIGHT;

	XMoveResizeWindow(dpy, cli->win, client->x, client->y,
		client->w, client->h);
}

void
updicons()
{
	client_t *curr = head;
	int x = 0;
	int y = 0;

	while (curr)
	{
		if (curr->state != Hidden) continue;

		if (x + ICON_WIDTH > SCREEN_WIDTH)
		{
			x = 0;
			y += ICON_HEIGHT;
		}

		curr->icon->x = x;
		curr->icon->y = y;
		
		XMoveWindow(dpy, curr->icon->win, curr->icon->x, curr->icon->y);

		paint(curr);

		curr = curr->next;
		x += ICON_WIDTH;
	}
}

void
paint(client_t *client)
{
	XClearWindow(dpy, curr->icon->win);
	XDrawString(dpy, curr->ic on->win, curr->icon->gc, 0, ICON_HEIGHT,
		(curr->title ? curr->title : 0),
		MIN((curr->title ? strlen(curr->title) : 0), 10));
}

void
chfocus(client_t *client)
{
	if (focused)
	{
		XGrabButton(dpy, AnyButton, AnyModifier, client->win, 
			False, ButtonPressMask | ButtonReleaseMask,
			GrabModeAsync, GrabModeAsync, None, None);
	}
	
	if (client)
	{
		XUngrabButton(dpy, AnyButton, AnyModifier, client->win);
		XSetInputFocus(dpy, client->win, RevertToPointerRoot, CurrentTime);

		Window focusr;
		int revert;
		XGetInputFocus(dpy, &focusr, &revert);
		if (focusr != client->win)
		{
			XGrabButton(dpy, AnyButton, AnyModifier, client->win,
				False, ButtonPressMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None);
		}
		else
			raise(client);
	}
}

void
updfocus()
{
	Window w_, chld;
	int i_;
	unsigned int u_;

	XQueryPointer(dpy, root, &w_, &chld, &i_, &i_, &i_, &i_, &u_);
	client_t *cli = fromwin(chld);

	if (chld == root)
	{
		chfocus(NULL);
	}
	else if (cli && cli->class != InputOnly)
	{
		chfocus(cli);
	}
}
