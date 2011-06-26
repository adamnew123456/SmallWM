#include "client.h"

client_t *head, *focused;

client_t*
tail()
{
	client_t *c = head;
	while (c->next) c = c->next;
	return c;
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
create(Display *dpy, Window win)
{
	client_t *cli = malloc(sizeof(client_t));
	XWindowAttributes attr;
	char *title = malloc(sizeof(32));

	XGetWindowAttributes(dpy, win, &attr);
	XFetchName(dpy, win, &title);

	XSetWindowBorderWidth(dpy, win, 3);

	cli->dpy = dpy;
	cli->win = win;
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

	chfocus(cli);

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
		XDestroyWindow(client->dpy, client->win);

	free(client->title);
	free(client);

	updicons();
}

void
hide(client_t *client)
{
	if (client->state != Visible) return;

	client->icon = malloc(sizeof(icon_t));
	client->icon->win = XCreateSimpleWindow(client->dpy, 
				RootWindow(dpy, DefaultScreen(dpy)),
				-200, -200, ICON_WIDTH, ICON_HEIGHT, 1, BLACK, WHITE);
	XSelectInput(client->dpy, client->icon->win, ButtonPressMask | 
				ButtonReleaseMask | ExposureMask);
	XMapWindow(client->dpy, client->icon->win);

	client->icon->gc = XCreateGC(client->dpy, client->icon->win, 0, NULL);

	XUnmapWindow(client->dpy, client->win);

	client->state = Hidden; 

	updicons();
}

void
unhide(client_t *client, int danger)
{
	if (client->state != Hidden) return;

	if (!danger)
	{
		XMapWindow(client->dpy, client->win);
		XRaiseWindow(client->dpy, client->win);
	}

	XDestroyWindow(client->dpy, client->icon->win);
	XFreeGC(client->dpy, client->icon->gc);
	free(client->icon);

	client->state = Visible;

	updicons();
}

void
beginmvrsz(client_t *client)
{	
	if (client->state != MoveResz) return;
	client->state = MoveResz;

	XUnmapWindow(client->dpy, client->win);

	client->pholder = 
		XCreateSimpleWindow(client->dpy, 
			RootWindow(dpy, DefaultScreen(dpy)),
			client->x, client->y, client->w, client->h,
			1, BLACK, WHITE);
	
	XMapWindow(client->dpy, client->pholder);

	XGrabPointer(client->dpy, client->pholder, True,
		PointerMotionMask | ButtonReleaseMask,
		GrabModeAsync, GrabModeAsync, None,
		None, CurrentTime);
	
	XRaiseWindow(client->dpy, client->pholder);
}

void
endmoversz(client_t *client)
{
	if (client->state != MoveResz) return;
	client->state = Visible;

	XUngrabPointer(client->dpy, CurrentTime);

	XWindowAttributes attr;
	XGetWindowAttributes(client->dpy, client->pholder, &attr);
	XDestroyWindow(client->dpy, client->pholder);

	client->x = attr.x;
	client->y = attr.y;
	client->w = attr.width;
	client->h = attr.height;

	XMapWindow(client->dpy, client->win);
	XMoveResizeWindow(client->dpy, client->win, client->x, client->y,
		client->w, client->h);

	raise_(client);
}

void
raise_(client_t *client)
{
	if (client->state != Visible) return;
	XRaiseWindow(client->dpy, client->win);
}

void
lower(client_t *client)
{
	if (client->state != Visible) return;
	XLowerWindow(client->dpy, client->win);
}

void
maximize(client_t *client)
{
	client->x = 0;
	client->y = ICON_HEIGHT;
	client->w = SCREEN_WIDTH;
	client->h = SCREEN_HEIGHT - ICON_HEIGHT;

	XMoveResizeWindow(client->dpy, client->win, client->x, client->y,
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
		
		XMoveWindow(curr->dpy, curr->icon->win, curr->icon->x, curr->icon->y);

		paint(curr);

		curr = curr->next;
		x += ICON_WIDTH;
	}
}

void
paint(client_t *client)
{
	XClearWindow(client->dpy, client->icon->win);
	XDrawString(client->dpy, client->win, client->icon->gc, 0, ICON_HEIGHT,
		(client->title ? client->title : 0),
		MIN((client->title ? strlen(client->title) : 0), 10));
}

void
chfocus(client_t *client)
{
	if (focused)
	{
		XGrabButton(focused->dpy, AnyButton, AnyModifier, client->win, 
			False, ButtonPressMask | ButtonReleaseMask,
			GrabModeAsync, GrabModeAsync, None, None);
	}
	
	if (client)
	{
		XUngrabButton(client->dpy, AnyButton, AnyModifier, client->win);
		XSetInputFocus(client->dpy, client->win, RevertToPointerRoot, CurrentTime);

		Window focusr;
		int revert;
		XGetInputFocus(client->dpy, &focusr, &revert);
		if (focusr != client->win)
		{
			XGrabButton(client->dpy, AnyButton, AnyModifier, client->win,
				False, ButtonPressMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None);
		}
		else
		{
			raise_(client);
			focused = client;
		}
	}
}
