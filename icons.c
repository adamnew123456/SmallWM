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

void dumpList(){
	wlist_t *tmp = head;
	while (tmp){
		printf("Window title: %s, Window ID: %x, Next Node: %x\n", tmp->title, tmp->win, tmp->next);
		tmp = tmp->next;
	}
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

	dumpList();
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

	dumpList();
}

int find(Window *array, int len, Window data){
	int i;
	for (i = 0; i < len; i++){
		if (array[i] == data) return i;
	}
	return -1;
}

void showMenu(Display *dpy){
}
