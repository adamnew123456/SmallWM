#if 0
#include <csignal>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>

#include "actions.h"
#include "configparse.h"
#include "clientmanager.h"
#include "common.h"
#include "logging.h"
#include "model/client-model.h"
#include "model/x-model.h"
#include "xdata.h"

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
#endif
