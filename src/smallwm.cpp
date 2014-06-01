/** @file */
#include <csignal>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>

#include "actions.h"
#include "configparse.h"
#include "clientmanager.h"
#include "common.h"
#include "events.h"
#include "shared.h"

/**
 * Registers to receive RandR events, and returns the offset at which RandR 
 * events start.
 *
 * @return The RandR event offset
 */
int register_xrandr(WMShared &shared)
{
    // Initialize XRandR
    int xrandr_evt_base, xrandr_err_base;
    Bool xrandr_state = XRRQueryExtension(shared.display, &xrandr_evt_base, &xrandr_err_base);
    if (xrandr_state == false)
    {
        shared.logger.set_priority(LOG_ERR) << 
            "Unable to initialize XRandR extension - terminating" << SysLog::endl;
        shared.logger.stop();

        std::exit(3);
    }
    else
    {
        // Version 1.4 is about two-ish years old now, so this version seems
        // to be a reasonable choice
        int major_version = 1, minor_version = 4;
        XRRQueryVersion(shared.display, &major_version, &minor_version);

        // Get the initial screen information
        int nsizes;
        XRRScreenConfiguration *screen_config = XRRGetScreenInfo(shared.display, shared.root);
        XRRScreenSize *screen_size = XRRConfigSizes(screen_config, &nsizes);

        shared.screen_size = Dimension2D(screen_size->width, screen_size->height);

        // Register the event hook to update the screen information later
        XRRSelectInput(shared.display, shared.root,
                RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask 
                | RROutputChangeNotifyMask | RROutputPropertyNotifyMask);
    }
    
    return xrandr_evt_base;
}

/**
 * Copies over data which is common to WMConfig and WMShared
 *
 * @param config The configuration data loaded from file
 * @param shared The shared location to copy into
 * @param clients The client manager to copy the actions into
 */
void copy_config(WMConfig &config, WMShared &shared, ClientManager &clients)
{

    shared.logger.set_identity("smallwm");
    shared.logger.set_priority(LOG_INFO);
    shared.logger.set_facility(LOG_USER);
    shared.logger.set_log_mask(config.log_mask);
    shared.logger.start();

    shared.shell = config.shell;
    shared.icon_size = Dimension2D(config.icon_width, config.icon_height);
    shared.border_width = config.border_width;
    shared.max_desktops = config.num_desktops;
    shared.show_icons = config.show_icons;

    for (std::map<std::string,ClassActions>::iterator actions_iter 
            = config.classactions.begin();
            actions_iter != config.classactions.end();
            actions_iter++)
    {
        clients.register_action(actions_iter->first, actions_iter->second);
    }
}

/**
 * Interns all necessary X atoms.
 */
const int n_atoms = 4;
void intern_atoms(WMShared &shared)
{
    const char *atom_names[n_atoms] = {
        "WM_PROTOCOLS",
        "WM_DELETE_WINDOW",
        "WM_STATE",
        "SMALLWM_LAYER"
    };

    Atom atoms[n_atoms];

    XInternAtoms(shared.display, const_cast<char**>(atom_names), n_atoms, false, atoms);
    for (int i = 0; i < n_atoms; i++)
    {
        std::string atom_as_string(atom_names[i]);
        shared.atoms[atom_as_string] = atoms[i];
    }
}

/**
 * Reap dead child processes to avoid zombies.
 * @param signal The UNIX signal being sent to this process.
 */
void reap_child(int signal)
{
    int _unused;
    wait(&_unused);
}

/**
 * Prints out X errors to enable diagnosis, but doesn't kill us.
 * @param display The display the error occurred on
 * @param event The error event that happened
 */
int x_error_handler(Display *display, XErrorEvent *event)
{
    char err_desc[500];
    XGetErrorText(display, event->error_code, err_desc, 500);

    std::cout << "X Error\n";
/*
 * If you want diagnostics, then go ahead and uncomment this. However, SmallWM
 * will trigger X errors that aren't fatal, and much of the output will be
 * garbage.
 *
    std::cout << "X Error: {\n\tdisplay = " << XDisplayName(NULL) 
        << "\n\tserial = " << event->serial
        << "\n\terror = '" << err_desc 
        << "'\n\trequest = " << (int)event->request_code 
        << "\n\tminor = " << (int)event->minor_code << "\n}\n";
*/

    return 0;
}

int main()
{
    XSetErrorHandler(x_error_handler);
    signal(SIGCHLD, reap_child);
        
    WMConfig config;
    config.load();

    WMShared shared;
    ClientManager clients(shared);
    copy_config(config, shared, clients);

    shared.display = XOpenDisplay(NULL);
    if (!shared.display)
    {
        shared.logger.set_priority(LOG_ERR) << 
            "Could not open X display - terminating" << SysLog::endl;
        shared.logger.stop();

        std::exit(2);
    }

    shared.root = DefaultRootWindow(shared.display);
    shared.screen = DefaultScreen(shared.display);

    XSelectInput(shared.display, shared.root, 
            PointerMotionMask | StructureNotifyMask | SubstructureNotifyMask);

    Window _unused1;
    Window *children;
    unsigned int nchildren;

    XQueryTree(shared.display, shared.root, &_unused1, &_unused1, &children, &nchildren);
    int idx;
    for (idx = 0; idx < nchildren; idx++)
    {
        if (children[idx] != shared.root)
            clients.create(children[idx]);
    }

    XFree(children);

    intern_atoms(shared);
    int randr_offset = register_xrandr(shared);
    XEvents events(shared, clients, config.key_commands, randr_offset);
    events.run();

    return 0;
}
