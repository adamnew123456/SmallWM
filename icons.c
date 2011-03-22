/* Iconification - Hides/unhides windows to/from a window menu */
#include "icons.h"

// I don't much appreciate the semantics of the
// `case` statement.
#define NOOP(x) (x)

wlist_t *head;

void initList(){
	head = malloc(sizeof(wlist_t));
	head->title = NULL;
	head->win = None;
	head->next = NULL;
}

wlist_t* tailList(){
	wlist_t *tmp = head;
	while (tmp->next) tmp = tmp->next;
	return tmp;
}

int lenList(){
	wlist_t *tmp = head->next;
	int i = 0;

	while (tmp){
		i++;
		tmp = tmp->next;
	}
	return i;
}

wlist_t* indList(int index){
	wlist_t *tmp = head;
	while (index--) tmp = tmp->next;
	return tmp;
}

void hideWindow(Display *dpy, Window win){
	char *title = malloc(50);
	XFetchName(dpy, win, &title);
	
	wlist_t *node = malloc(sizeof(wlist_t));
	node->title = title;
	node->win = win;
	node->next = NULL;

	wlist_t *prev = tailList();
	prev->next = node;
	XUnmapWindow(dpy, win);
}

void unHideWindow(Display *dpy, Window win){
	wlist_t *tmp, *prev;

	tmp = head;
	prev = NULL;
	while (tmp){
		if (tmp->win == win) break;
		wlist_t *x = tmp;
		tmp = tmp->next;
		prev = x;
	}

	if (!tmp) return;
	
	prev->next = tmp->next;
	XMapWindow(dpy, tmp->win);
	free(tmp);
}

int find(Window *array, int len, Window data){
	int i;
	for (i = 0; i < len; i++){
		if (array[i] == data) return i;
	}
	return -1;
}

void showMenu(Display *dpy){
	if (!lenList()) return;

	Window dump, crap;
	int x,y;
	int rx,ry;
	unsigned int mask;
	XQueryPointer(dpy, DefaultRootWindow(dpy), &dump, &crap, &rx, &ry, &x, &y, &mask);

	Window menu = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), MAX(MWIDTH, x), MAX(MHEIGHT, y), MWIDTH, MHEIGHT * lenList(), 2, 
			BlackPixel(dpy, DefaultScreen(dpy)), WhitePixel(dpy, DefaultScreen(dpy)));

	GC gc = XCreateGC(dpy, menu, 0, NULL);
	XSetForeground(dpy, gc, BlackPixel(dpy, DefaultScreen(dpy)));
	XSetBackground(dpy, gc, WhitePixel(dpy, DefaultScreen(dpy)));

	Window buttons[25]; // Rough max, based upon nothing in particular
	memset(buttons, None, 25 * sizeof(Window));
	wlist_t *tmp = head->next;

	int i = 0;
	while (tmp){
		buttons[i++] = XCreateSimpleWindow(dpy, menu, 0, MWIDTH * i, MWIDTH, MHEIGHT, 2, BlackPixel(dpy, DefaultScreen(dpy)), WhitePixel(dpy, DefaultScreen(dpy)));
		XSelectInput(dpy, buttons[i-1], ButtonPressMask | ButtonReleaseMask | ExposureMask);
		tmp = tmp->next;
	}

	XMapSubwindows(dpy, menu);
	XMapWindow(dpy, menu);

	XGrabPointer(dpy, menu, True, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, menu, None, CurrentTime);

	tmp = head->next;
	i = 0;
	while (tmp){
		XDrawString(dpy, buttons[i++], gc, 0, MHEIGHT,
				tmp->title, strlen(tmp->title));
		tmp = tmp->next;
	}

	Window w;
	XEvent ev;
	while (1){
		XNextEvent(dpy, &ev);

		switch (ev.type){
			case ButtonPress:
				NOOP(1);
				int ind = find(buttons, 25, ev.xbutton.subwindow);
				wlist_t *ent = indList(ind);
	
				if (ev.xbutton.button == 1){
					w = ent->win;
					goto unhide;
				}
				else goto cleanup;
				break;
		}
	}

unhide:
	unHideWindow(dpy, w);

cleanup:
	XUngrabPointer(dpy, CurrentTime);
	XDestroySubwindows(dpy, menu);
	XDestroyWindow(dpy, menu);
}
