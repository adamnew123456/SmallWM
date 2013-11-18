/*
 * TinyWM is written by Nick Welch <mack@incise.org>, 2005. 
 *
 * SmallWM was hacked out of TinyWM by Adam Marchetti <adamnew123456@gmail.com>, 2010.
 *
 * This software is in the public domain and is provided AS IS, with NO
 * WARRANTY.
 */

#include "smallwm.h"

char *SHELL = NULL;

// The X error handler
int x_error_handler(Display * dpy, XErrorEvent * error)
{
    char err_desc[500];
    XGetErrorText(dpy, error->error_code, err_desc, 500);

    printf("\n****************\n"
           "X Gave An Error\n"
           "=================\n"
           " - Display: %s\n"
           " - Serial Number: %lu\n"
           " - Error Code: %d (%s)\n"
           " - Request Code: %d\n"
           " - Minor Code: %d\n",
           XDisplayName(NULL),
           error->serial,
           error->error_code,
           err_desc, error->request_code, error->minor_code);
}

// Prints out a message of the current function that is being run, and then
// crashes
void die(const char *call, int line)
{
    printf("Line %d: Failed when calling %s\n", line, call);
    exit(1);
}

// Waits for a child process to exit
void sigchld(int signal)
{
    int status;
    wait(&status);
}

// Get a list of all existing windows and place them under SmallWM's control
void getexisting(Display * dpy, Window root)
{
    Window w_;
    Window *childs;
    unsigned int nchilds;
    XQueryTree(dpy, root, &w_, &w_, &childs, &nchilds);

    int i = 0;
    for (i = 0; i < nchilds; i++) {
        if (childs[i] == root)
            continue;
        create(dpy, childs[i]);
    }

    XFree(childs);
}

static int config_handler(void *user,
              const char *section,
              const char *name, const char *value)
{
    if (!strcmp(section, "smallwm") && !strcmp(name, "shell")) {
        SHELL = strdup(value);
    } else {
        return 0;
    }

    return 1;
}

int main()
{
    signal(SIGCHLD, sigchld);
    XSetErrorHandler(x_error_handler);

    // Load the shell name from the configuration file
    char *path =
        malloc(strlen(getenv("HOME")) + strlen("/.config/smallwm"));
    sprintf(path, "%s/.config/smallwm", getenv("HOME"));
    ini_parse(path, config_handler, NULL);

    if (SHELL == NULL) {
        SHELL = "xterm";
    }

    XEvent *ev = malloc(sizeof(XEvent));

    if (!(dpy = XOpenDisplay(NULL)))
        return 1;

    root = DefaultRootWindow(dpy);
    XSelectInput(dpy, root, PointerMotionMask | SubstructureNotifyMask);
    getexisting(dpy, root);

    int i;
    for (i = 0; i < NSHORTCUTS; i++) {
        int keysym = XKeysymToKeycode(dpy, SHORTCUTS[i].ksym);
        XGrabKey(dpy, keysym, MASK, root, True, GrabModeAsync,
             GrabModeAsync);
    }

    for (i = 0; i < NKEYBINDS; i++) {
        int keysym = XKeysymToKeycode(dpy, KEYBINDS[i]);
        XGrabKey(dpy, keysym, MASK, root, True, GrabModeAsync,
             GrabModeAsync);
    }

    // The move and resize buttons (also used for other stuff)
    XGrabButton(dpy, MOVE, MASK, root, True, ButtonPressMask,
            GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(dpy, RESZ, MASK, root, True, ButtonPressMask,
            GrabModeAsync, GrabModeAsync, None, None);

    // Run a shell upon startup
    if (!fork()) {
        execlp(SHELL, SHELL, NULL);
        exit(0);
    }

    while (1) {
        XNextEvent(dpy, ev);
        client_t *cli;

        switch (ev->type) {
        case KeyPress:
            eKeyPress(dpy, ev);
            break;
        case ButtonPress:
            eButtonPress(dpy, ev);
            break;
        case ButtonRelease:
            eButtonRelease(dpy, ev);
            break;
        case MotionNotify:
            eMotionNotify(dpy, ev);
            break;
        case MapNotify:
            eMapNotify(dpy, ev);
            break;
        case Expose:
            cli = fromicon(ev->xexpose.window);
            paint(cli);    // No need to check - only icons
            break;    // will give  us this event
        case DestroyNotify:
            cli = fromwin(ev->xdestroywindow.window);
            if (!cli)
                break;
            destroy(cli, 1);
            break;
        }
    }
}
