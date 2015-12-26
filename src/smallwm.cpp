#include <csignal>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>

#include "actions.h"
#include "clientmodel-events.h"
#include "configparse.h"
#include "common.h"
#include "logging/logging.h"
#include "logging/syslog.h"
#include "model/changes.h"
#include "model/client-model.h"
#include "model/screen.h"
#include "model/x-model.h"
#include "xdata.h"
#include "x-events.h"

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

    std::cerr << "X Error: \n"
        "\tdisplay = " << XDisplayName(NULL) << "'\n"
        "\tserial = " << event->serial << "'\n"
        "\terror = '" << err_desc << "\n"
        "\trequest = " << static_cast<int>(event->request_code) << "\n"
        "\tminor = " << static_cast<int>(event->minor_code) << "\n";

    return 0;
}

int main()
{
    XSetErrorHandler(x_error_handler);
    signal(SIGCHLD, reap_child);
        
    WMConfig config;
    config.load();

    SysLog logger;
    logger.set_identity("SmallWM");
    logger.set_facility(LOG_USER);
    logger.set_log_mask(LOG_UPTO(config.log_mask));
    logger.start();

    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        logger.log(LOG_ERR) << 
            "Could not open X display - terminating" << Log::endl;
        logger.stop();

        std::exit(2);
    }

    Window default_root = DefaultRootWindow(display);
    XData xdata(logger, display, default_root, DefaultScreen(display));
    xdata.select_input(default_root, 
        PointerMotionMask | StructureNotifyMask | SubstructureNotifyMask);

    CrtManager crt_manager;
    std::vector<Box> screens;
    xdata.get_screen_boxes(screens);
    crt_manager.rebuild_graph(screens);

    ChangeStream changes;
    ClientModel clients(changes, crt_manager, config.num_desktops);

    std::vector<Window> existing_windows;
    xdata.get_windows(existing_windows);

    XModel xmodel;
    FocusCycle focus_cycle(&logger);
    XEvents x_events(config, xdata, clients, xmodel, focus_cycle);

    for (std::vector<Window>::iterator win_iter = existing_windows.begin();
         win_iter != existing_windows.end();
         win_iter++)
    {
        if (*win_iter != default_root)
            x_events.add_window(*win_iter);
    }


    ClientModelEvents client_events(config, logger, changes, 
                                    xdata, clients, xmodel, 
                                    focus_cycle);

    // Make sure to process all the changes produced by the class actions for
    // the first set of windows
    client_events.handle_queued_changes();

    while (x_events.step())
        client_events.handle_queued_changes();

    return 0;
}
