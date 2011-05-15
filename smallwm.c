/* TinyWM is written by Nick Welch <mack@incise.org>, 2005.
 * SmallWM was hacked out of TinyWM by Adam Marchetti <adamnew123456@gmail.com>, 2010.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include "smallwm.h"

int
window_exist(Window win)
{
 Window dump, dump2;
 Window *childs;
 unsigned int nchilds;

 XQueryTree(dpy, root, &dump, &dump2, &childs, &nchilds);
 for (;nchilds > 0; --nchilds)
 {
  if (childs[nchilds - 1] == win){
    free(childs);
    return 1;
  }
 }
 free(childs);
 return 0;
}

void
focus ()
{
  // Sets the focus to wherever the pointer 
  // is (avoids focus stealing and other nastiness)
  //
  // One outstanding bug plagues this place - I can't
  // see if a window exists, so weird apps (git gui)
  // crash SmallWM when they are closed
  Window dump, child;
  int rx, ry, cx, cy;
  unsigned int mask;

  XQueryPointer (dpy, root, &dump, &child, &rx, &ry, &cx, &cy, &mask);

  if (window_exist(child))
    XSetInputFocus (dpy, child, RevertToNone, CurrentTime);
}

int
main ()
{
  XEvent ev;

  if (!(dpy = XOpenDisplay (NULL)))
    return 1;

  root = DefaultRootWindow (dpy);
  XSelectInput (dpy, root, KeyPressMask |
		ButtonPressMask |
		ButtonReleaseMask |
		PointerMotionMask | SubstructureNotifyMask);

  // Loops through all the key shortcuts in event.h and grabs them
  int i;
  for (i = 0; i < NSHORTCUTS; i++)
    {
      XGrabKey (dpy, XKeysymToKeycode (dpy, SHORTCUTS[i].ksym), MASK, root,
		True, GrabModeAsync, GrabModeAsync);
    }
  // Exit key combo
  XGrabKey (dpy, XKeysymToKeycode (dpy, XK_Escape), MASK, root, True,
	    GrabModeAsync, GrabModeAsync);

  // The move and resize buttons (also used for other stuff)
  XGrabButton (dpy, 1, MASK, root, True, ButtonPressMask, GrabModeAsync,
	       GrabModeAsync, None, None);
  XGrabButton (dpy, 3, MASK, root, True, ButtonPressMask, GrabModeAsync,
	       GrabModeAsync, None, None);

  if (!fork ())
    {
      execlp (SHELL, SHELL, NULL);
      exit (0);
    }

  initList ();

  while (1)
    {
      XNextEvent (dpy, &ev);

      wlist_t *tmp = NULL;
      switch (ev.type)
	{
	case KeyPress:
	  eKeyPress (dpy, ev);
	  break;
	case ButtonPress:
	  eButtonPress (dpy, ev);
	  break;
	case ButtonRelease:
	  eButtonRelease (dpy, ev);
	  break;
	case MotionNotify:
	  eMotionNotify (dpy, ev);
	  break;
	case MapNotify:
	  eMapNotify (dpy, ev);
	  break;
	case Expose:
	  paintIcon (dpy, ev.xexpose.window);
	  break;
	case DestroyNotify:
	  tmp = revList (ev.xdestroywindow.window);
	  if (tmp)
	    unHideWindow (dpy, tmp->icon, 1);
	}

      focus ();
    }
}
