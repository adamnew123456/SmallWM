#ifndef __SMALLWM_TEST_CONFIGPARSE__
#define __SMALLWM_TEST_CONFIGPARSE__

#include <cstdlib>
#include <iostream>
#include <string>

#include "actions.h"
#include "configparse.h"

/**
 * An override for the standard WMConfig class to use a custom config file to test on.
 */
class TestWMConfig : public WMConfig
{
protected:
    virtual std::string get_config_path() const;
};

#endif
