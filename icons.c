/* Iconification - Hides/unhides windows to/from a window menu */
#include "icons.h"

// I don't much appreciate the semantics of the
// `case` statement.
#define NOOP(x) (x)

wlist_t *head;

void
initList ()
{
  head = malloc (sizeof (wlist_t));
  head->title = NULL;
  head->win = None;
  head->icon = None;
  head->next = NULL;
}

wlist_t *
tailList ()
{
  wlist_t *tmp = head;
  while (tmp->next)
    tmp = tmp->next;
  return tmp;
}

// Used for painting an icon window - the event
// loop knows nothing about the iconification system,
// so there has to be a way to map icon window to structure
wlist_t *
findList (Window icon)
{
  wlist_t *tmp = head;
  while (tmp)
    {
      if (tmp->icon == icon)
	return tmp;
      else
	tmp = tmp->next;
    }
  return NULL;
}

// Same as above, but searching by hidden window
wlist_t *
revList (Window win)
{
  wlist_t *tmp = head;
  while (tmp)
    {
      if (tmp->win == win)
	return tmp;
      else
	tmp = tmp->next;
    }
  return NULL;
}

void
hideWindow (Display * dpy, Window win)
{
  char *title = malloc (50);
  if (XFetchName (dpy, win, &title) == 0){
    free(title);
	title = NULL;
  }

  wlist_t *node = malloc (sizeof (wlist_t));
  node->title = title;
  node->win = win;

  // Don't do much with the window - It'll be taken care of later
  node->icon = XCreateSimpleWindow (dpy,
				    RootWindow (dpy, DefaultScreen (dpy)),
				    -200, -200,
				    IWIDTH, IHEIGHT, 1, BLACK, WHITE);

  XSelectInput (dpy, node->icon, ButtonPressMask |
		ButtonReleaseMask | ExposureMask);
  XMapWindow (dpy, node->icon);

  // For drawing the title text
  node->gc = XCreateGC (dpy, node->icon, 0, NULL);

  node->next = NULL;

  wlist_t *prev = tailList ();
  prev->next = node;
  XUnmapWindow (dpy, win);

  paintIcons (dpy);
}

void
unHideWindow (Display * dpy, Window icon, int careful)
{
  wlist_t *tmp, *prev;
  tmp = head;
  prev = NULL;

  // Does not use finder loop because this requires the
  // previous node as well
  while (tmp)
    {
      if (tmp->icon == icon)
	break;

      wlist_t *x = tmp;
      tmp = tmp->next;
      prev = x;
    }

  if (!tmp)
    return;

  prev->next = tmp->next;

  // Careful flag is set if the window has been destroyed
  if (!careful)
    {
      XMapWindow (dpy, tmp->win);
      XRaiseWindow (dpy, tmp->win);
    }

  XDestroyWindow (dpy, tmp->icon);
  XFreeGC (dpy, tmp->gc);
  free (tmp);

  paintIcons (dpy);
}

void
paintIcons (Display * dpy)
{
  wlist_t *tmp = head->next;
  int x = 0;
  int y = 0;

  while (tmp)
    {
      // Prevent icons from running off the screen
      if (x + IWIDTH > SWIDTH)
	{
	  x = 0;
	  y += IHEIGHT;
	}

      XMoveWindow (dpy, tmp->icon, x, y);

      XClearWindow (dpy, tmp->icon);
      XDrawString (dpy, tmp->icon, tmp->gc, 0, IHEIGHT,
		   (tmp->title ? tmp->title : 0), 
		   MIN ((tmp->title ? strlen (tmp->title) : 0), 10));

      tmp = tmp->next;
      x += IWIDTH;
    }
}

void
paintIcon (Display * dpy, Window icon)
{
  wlist_t *tmp = findList (icon);

  XClearWindow (dpy, tmp->icon);
  XDrawString (dpy, tmp->icon, tmp->gc, 0, IHEIGHT, 
		   (tmp->title ? tmp->title : 0),
	       MIN ((tmp->title ? strlen (tmp->title) :  0), 25));
}
