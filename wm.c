/* Routines for managing core window manager state. */
#include "wm.h"

// A callback for the configuration parser which sets the appropriate config_* variable
// as well as to build the actions table
char *config_shell = NULL,
     *config_num_desktops = NULL,
     *config_icon_width = NULL,
     *config_icon_height = NULL,
     *config_border_width = NULL;

#define SET_CONFIG_VAR(key, var) if (!strcmp(name, (key))) {\
        var = strdup(value); \
    }
#define STR_EQUAL(a, b) (!(strcmp((a), (b))))

static int config_handler(void *user,
        const char *section,
        const char *name, const char *value)
{
    if (STR_EQUAL(section, "smallwm")) 
    {
        SET_CONFIG_VAR("shell", config_shell);
        SET_CONFIG_VAR("desktops", config_num_desktops);
        SET_CONFIG_VAR("icon_width", config_icon_width);
        SET_CONFIG_VAR("icon_height", config_icon_height);
        SET_CONFIG_VAR("border_width", config_border_width);
    } 
    else if (STR_EQUAL(section, "actions")) 
    {
        table_t *actions = user;
        classaction_t *action = malloc(sizeof(classaction_t));

        // Add one to make room for the terminator value
        unsigned int num_actions = count_occurences(value, ',') + 1;
        action->actions = malloc(sizeof(action_t) * num_actions);

        int action_idx = 0;

        // strtok() is destructive, so make sure that inih doesn't have to
        // deal with a butchered string
        char *copied_value = strdup(value);

        char *option = strtok(copied_value, ",");
        do
        {
            char *stripped = strip_string(option, " \t\r\n");

            if (STR_EQUAL(stripped, "stick"))
            {
                action->actions[action_idx] = ACT_STICK;
                action_idx++;
            }
            else if (STR_EQUAL(stripped, "maximize"))
            {
                action->actions[action_idx] = ACT_MAXIMIZE;
                action_idx++;
            }
            else if (STR_EQUAL(stripped, "snapleft"))
            {
                action->actions[action_idx] = ACT_SNAPLEFT;
                action_idx++;
            }
            else if (STR_EQUAL(stripped, "snapright"))
            {
                action->actions[action_idx] = ACT_SNAPRIGHT;
                action_idx++;
            }
            else if (STR_EQUAL(stripped, "snaptop"))
            {
                action->actions[action_idx] = ACT_SNAPTOP;
                action_idx++;
            }
            else if (STR_EQUAL(stripped, "snapbottom"))
            {
                action->actions[action_idx] = ACT_SNAPBOTTOM;
                action_idx++;
            }
            else if (stripped[0] == 'L')
            {
                status_t convert_status;
                action->layer = string_to_long(stripped + 1, &convert_status);
                if (convert_status == SUCCESS)
                {
                    action->actions[action_idx] = ACT_SETLAYER;
                    action_idx++;
                }
            }

            free(stripped);
        } while (option = strtok(NULL, ","));

        action->actions[action_idx] = ACT_END;
        add_table(actions, string_hash(name), action);

        free(copied_value);
    }
}
#undef SET_CONFIG_VAR

/* Initializes the window manager state:
 *
 *  - Loads configuration file information (shell program, number of desktops, etc)
 *  - Opens the display, loads the root window
 *  - Sets up the event, client, and icon tables
 *  - Initializes XRandR
 *  - Grabs WM-related events
 *  - Grabs keys and buttons for events
 *  - Takes ownership over all existing windows
*/
smallwm_t *init_wm()
{
    smallwm_t *state = malloc(sizeof(smallwm_t));

    state->classactions = new_table();

    // Loads the configuration file via the inih API
    char *path = malloc(strlen(getenv("HOME")) + strlen("/.config/smallwm"));
    sprintf(path, "%s/.config/smallwm", getenv("HOME"));
    ini_parse(path, config_handler, state->classactions);

    // Set defaults for values which are not in the configuration file
#define SET_VAR_DEFAULT(var, defval) if (!var) var = (defval)
    SET_VAR_DEFAULT(config_shell, "xterm");
    SET_VAR_DEFAULT(config_num_desktops, "5");
    SET_VAR_DEFAULT(config_icon_width, "75");
    SET_VAR_DEFAULT(config_icon_height, "20");
    SET_VAR_DEFAULT(config_border_width, "4");
#undef SET_VAR_DEFAULT

    // Copy over everything into the WM state structure, doing numeric conversions as necessary
    status_t conversion_state;
#define SET_STATE_NUM(state_var, config_var, default_var) \
    state_var = string_to_long(config_var, &conversion_state); \
    if (conversion_state == FAIL) state_var = (default_var);

    state->leftclick_launch = (config_shell == NULL ? "xterm" : config_shell);
    SET_STATE_NUM(state->num_desktops, config_num_desktops, 5);
    SET_STATE_NUM(state->icon_width, config_icon_width, 75);
    SET_STATE_NUM(state->icon_height, config_icon_height, 20);
    SET_STATE_NUM(state->border_width, config_border_width, 4);

    if (state->num_desktops == 0)
        state->num_desktops++;

    if (state->icon_width <= 0)
        state->icon_width = 75;

    if (state->icon_height <= 0)
        state->icon_height = 20;
#undef SET_STATE_NUM

    // Get information about the display and root window
    state->display = XOpenDisplay(NULL);
    if (!state->display)
        die("Unable to open X display");

    state->root = DefaultRootWindow(state->display);
    state->screen = DefaultScreen(state->display);
    state->movement.state = MR_NONE;
    state->current_desktop = 0;

    // Initialize the tables (XRandR has an update event which needs to go inside)
    state->clients = new_table();
    state->icons = new_table();
    state->dialogs = new_table();
    state->focused_window = None;

    // Initialize XRandR
    int xrandr_evt_base, xrandr_err_base;
    Bool xrandr_state = XRRQueryExtension(state->display, &xrandr_evt_base, &xrandr_err_base);
    if (xrandr_state == False)
    {
        die("Unable to initialize XRandR");
    }
    else
    {
        // Add the event base to the window manager state
        state->xrandr_event_offset = xrandr_evt_base;

        // Version 1.4 is what is currently on my Saucy Salamander machine
        int major_version = 1, minor_version = 4;
        XRRQueryVersion(state->display, &major_version, &minor_version);

        // Get the initial screen information
        int nsizes;
        XRRScreenConfiguration *screen_config = XRRGetScreenInfo(state->display, state->root);
        XRRScreenSize *screen_size = XRRConfigSizes(screen_config, &nsizes);

        state->width = screen_size->width;
        state->height = screen_size->height;

        // Register the event hook to update the screen information later
        XRRSelectInput(state->display, state->root,
                RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask | RROutputChangeNotifyMask | RROutputPropertyNotifyMask);
    }

    // Asks X to report all interesting events to us (PointerMotion is used for moving/resizing clients)
    XSelectInput(state->display, state->root, PointerMotionMask | SubstructureNotifyMask);

    // Collect all the children and move them under our ownership
    Window _unused1;
    Window *children;
    unsigned int nchildren;

    XQueryTree(state->display, state->root, &_unused1, &_unused1, &children, &nchildren);

    int idx;
    for (idx = 0; idx < nchildren; idx++)
    {
        if (children[idx] != state->root)
            add_client_wm(state, children[idx]);
    }

    XFree(children);

    return state;
}

// Updates the size of the screen - called when xrandr changes the screen dimensions
void set_size_wm(smallwm_t *state, XEvent *event)
{
    // Get the new screen information
    int nsizes;
    XRRScreenConfiguration *screen_config = XRRGetScreenInfo(state->display, state->root);
    XRRScreenSize *screen_size = XRRConfigSizes(screen_config, &nsizes);

    state->width = screen_size->width;
    state->height = screen_size->height;
}

// Launches the program given by the left click
void shell_launch_wm(smallwm_t *state)
{
    if (!fork())
    {
        execlp(state->leftclick_launch, state->leftclick_launch, NULL);
        exit(1);
    }
}

// Reposisions all the icons while painting them
void update_icons_wm(smallwm_t *state)
{
    int n_icons;
    void **icons = to_list_table(state->icons, &n_icons);

    int idx;
    int x = 0, y = 0;

    for (idx = 0; idx < n_icons; idx++)
    {
        if (x + state->icon_width > state->width)
        {
            x = 0;
            y += state->icon_height;
        }

        icon_t *icon = icons[idx];
        place_icon(icon, x, y);
        paint_icon(icon);

        x += state->icon_width;
    }
}

// Changes the currently focused window. This was snatched from 9wm, which grabs a button on all
// unfocused windows so that they can be refocused when clicked on next. Otherwise, there is no
// grab and the window gets keyboard input.
void refocus_wm(smallwm_t *state, Window window)
{
    if (state->focused_window != None)
        XGrabButton(state->display, AnyButton, AnyModifier, state->focused_window,
                False, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
                None, None);
        
    XWindowAttributes attr;
    XGetWindowAttributes(state->display, window, &attr);

    if (attr.class != InputOnly && attr.map_state == IsViewable)
    {
        XUngrabButton(state->display, AnyButton, AnyModifier, window);
        XSetInputFocus(state->display, window, RevertToPointerRoot, CurrentTime);

        Window new_focus;
        int _unused1;
        XGetInputFocus(state->display, &new_focus, &_unused1);

        // If the new window is not actually focused, something has gone wrong and the focus
        // should be reverted
        if (new_focus != window)
        {
            XGrabButton(state->display, AnyButton, AnyModifier, window,
                    False, ButtonPressMask | ButtonReleaseMask,
                    GrabModeAsync, GrabModeAsync, None, None);
            state->focused_window = None;
        } else
            state->focused_window = window;
    } 
}

// Sending data to the compare_window_layers function to avoid using qsort_r
smallwm_t *__cmp_wm;

// Helps sort windows by the layers
int compare_window_layers(const void *windowA, const void* windowB)
{
    const Window *a = windowA;
    const Window *b = windowB;

    // Get the stacking levels of the clients
    client_t *clientA = get_table(__cmp_wm->clients, *a);
    client_t *clientB = get_table(__cmp_wm->clients, *b);

    // This is backwards of strcmp, since XRestackWindows takes windows in the opposite order
    if (clientA->layer < clientB->layer)
        return 1;

    if (clientA->layer > clientB->layer)
        return -1;

    return 0;
}

// Shows/ hides windows that are/aren't visible on the current desktop
void update_desktop_wm(smallwm_t *state)
{
    int n_clients;
    void **clients = to_list_table(state->clients, &n_clients);

    // This for setting the stacking order
    int n_win_idx = 0;
    Window *windows = malloc(sizeof(Window) * n_clients);

    int idx;
    XWindowAttributes attr;

    for (idx = 0; idx < n_clients; idx++)
    {
        client_t *client = clients[idx];
        XGetWindowAttributes(state->display, client->window, &attr);
        Bool should_be_viewable = (
                client->sticky || client->desktop == state->current_desktop);

        if (attr.map_state != IsViewable && should_be_viewable)
            XMapWindow(state->display, client->window);

        if (attr.map_state == IsViewable && !should_be_viewable)
            XUnmapWindow(state->display, client->window);

        if (should_be_viewable)
            windows[n_win_idx++] = client->window;
    }

    
    // Sort the windows by stacking order and then layer them
    if (n_win_idx)
    {
        __cmp_wm = state;
        qsort(windows, n_win_idx, sizeof(Window), compare_window_layers);
        XRestackWindows(state->display, windows, n_win_idx);
    }

    // Then place all the dialogs on top of all the others
    int n_dialogs = 0;
    void **dialogs = to_list_table(state->dialogs, &n_dialogs);

    for (idx = 0; idx < n_dialogs; idx++)
    {
        Window dialog = (Window)dialogs[idx];
        XRaiseWindow(state->display, dialog);
    }

    free(dialogs);
    free(windows);
    free(clients);
}

// Make sure that SmallWM ignores this window
void ignore_window_wm(smallwm_t *state, Window window)
{
    XClassHint *classhint = XAllocClassHint();
    classhint->res_class = "SMALLWM_IGNORE";
    XSetClassHint(state->display, window, classhint);
    XFree(classhint);
}

// Ask if a particular window should be ignored
Bool should_ignore_window_wm(smallwm_t *state, Window window)
{
    XClassHint *classhint = XAllocClassHint();
    XGetClassHint(state->display, window, classhint);
    Bool should_ignore = (
        classhint->res_class && !strcmp(classhint->res_class, "SMALLWM_IGNORE"));
    XFree(classhint);

    return should_ignore;
}

// Creates a client from a particular window
void add_client_wm(smallwm_t *state, Window window)
{
    // Make sure that this client wants to be managed - override_redirect means that a client does
    // not want to be managed by us
    XWindowAttributes attr;
    XGetWindowAttributes(state->display, window, &attr);

    // Ignore windows internal to SmallWM
    if (should_ignore_window_wm(state, window))
        return;

    if (attr.override_redirect)
    {
        // Do not manage these windows - they are things like combo boxes, etc.
        // which are private to applications.
        return;
    }

    // Detect dialogs is via the WM_TRANSIENT_FOR property
    Bool is_transient = False;

    Window transient_for;
    if (XGetTransientForHint(state->display, window, &transient_for) && transient_for != None)
    {
        add_table(state->dialogs, window, (Window*)window);
        is_transient = True;
    }

    // Make sure that this window is not being duplicated
    if (get_table(state->clients, window))
        return;

    client_t *client = malloc(sizeof(client_t));
    client->wm = state;
    client->window = window;
    client->state = C_VISIBLE;

    client->sticky = False;
    client->desktop = state->current_desktop;

    client->layer = 5;

    XSetWindowBorder(state->display, window, BlackPixel(state->display, state->screen));
    XSetWindowBorderWidth(state->display, window, state->border_width);

    add_table(state->clients, window, client);
    
    // Run all the window class actions, unless a window is a dialog
    if (!is_transient)
        do_actions_client(client);

    refocus_wm(state, window);
    update_desktop_wm(state);
}
