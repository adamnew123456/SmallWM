/** @file */
#ifndef __SMALLWM_CONFIG__
#define __SMALLWM_CONFIG__

#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "ini.h"
#include "actions.h"
#include "utils.h"

/**
 * Reads and manages configuration options in the SmallWM configure option.
 */
class WMConfig
{
public:
    /// Loads a couple of defaults for the WMConfig
    WMConfig() : shell("/usr/bin/xterm"), num_desktops(5), 
        icon_width(75), icon_height(20), border_width(2) {};
    
    void load();
    
    /// The shell to run 
    std::string shell;

    /// The number of available desktops
    Desktop num_desktops;

    /// The width and height of hidden icons
    Dimension icon_width, 
              icon_height;

    /// The width of the window border
    Dimension border_width;

    /// Handles all the configured class actions
    std::map<std::string, ClassActions> classactions;

protected:
    virtual std::string get_config_path();

private:
    static int config_parser(void *user, const char *c_section, const char *c_name, const char *c_value);
};

#endif
