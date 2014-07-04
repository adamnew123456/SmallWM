#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <syslog.h>

#include <UnitTest++.h>
#include "actions.h"
#include "configparse.h"

std::string *config_path = (std::string*)0;

class CustomFileWMConfig : public WMConfig
{
protected:
    std::string get_config_path() const
    {
        return *config_path;
    };
};

CustomFileWMConfig config;
struct WMConfigFixture 
{
    WMConfigFixture() 
    { config.reset(); };

    ~WMConfigFixture() 
    {};
};

void write_config_file(std::string filename, const char *text)
{
    std::ofstream config(filename.c_str());
    config << text;
    config.close();
}

SUITE(WMConfigSuitePlainOptions)
{
    TEST_FIXTURE(WMConfigFixture, test_default_shell)
    {
        // Ensure that an empty config file provides the default shell
        write_config_file(*config_path, "\n");

        config.load();
        CHECK_EQUAL(std::string("/usr/bin/xterm"), config.shell);
    }

    TEST_FIXTURE(WMConfigFixture, test_shell) 
    {
        // Make sure that any particular setting is acceptable
        write_config_file(*config_path,
            "[smallwm]\nshell=some-terminal \n");

        config.load();
        CHECK_EQUAL(std::string("some-terminal"), config.shell);
    }
    
    TEST_FIXTURE(WMConfigFixture, test_shell_empty)
    {
        // Make sure that an empty configuration option is not accepted
        write_config_file(*config_path,
            "[smallwm]\nshell= \n");
        
        config.load();
        CHECK_EQUAL(std::string("/usr/bin/xterm"), config.shell);
    }

    TEST_FIXTURE(WMConfigFixture, test_default_num_desktops)
    {
        // Ensure that an empty config file provides the default number of
        // desktops
        write_config_file(*config_path, "\n");

        config.load();
        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST_FIXTURE(WMConfigFixture, test_num_desktops)
    {
        // Make sure that a valid number of desktops sets the option
        write_config_file(*config_path,
            "[smallwm]\ndesktops= 42 \n");

        config.load();
        CHECK_EQUAL(42, config.num_desktops);
    }

    TEST_FIXTURE(WMConfigFixture, test_num_desktops_zero)
    {
        // Zero desktops are invalid, so make sure they revert to the default
        write_config_file(*config_path,
            "[smallwm]\ndesktops=0\n");

        config.load();
        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST_FIXTURE(WMConfigFixture, test_num_desktops_negative)
    {
        // Negative desktops are also invalid
        write_config_file(*config_path,
            "[smallwm]\ndesktops=-32\n");

        config.load();
        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST_FIXTURE(WMConfigFixture, test_num_desktops_non_numeric)
    {
        // Non-numbers should give back the default value
        write_config_file(*config_path,
            "[smallwm]\ndesktops=not a number\n");

        config.load();
        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST_FIXTURE(WMConfigFixture, test_default_icon_width)
    {

        write_config_file(*config_path, "\n");

        config.load();
        CHECK_EQUAL(75, config.icon_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_width)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width= 42 \n");

        config.load();
        CHECK_EQUAL(42, config.icon_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_width_zero)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width=0\n");

        config.load();
        CHECK_EQUAL(75, config.icon_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_width_negative)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width=-42\n");

        config.load();
        CHECK_EQUAL(75, config.icon_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_width_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width=not a number\n");

        config.load();
        CHECK_EQUAL(75, config.icon_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_default_icon_height)
    {

        write_config_file(*config_path, "\n");

        config.load();
        CHECK_EQUAL(20, config.icon_height);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_height)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height= 42 \n");

        config.load();
        CHECK_EQUAL(42, config.icon_height);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_height_zero)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height=0\n");

        config.load();
        CHECK_EQUAL(20, config.icon_height);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_height_negative)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height=-42\n");

        config.load();
        CHECK_EQUAL(20, config.icon_height);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_height_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height=not a number\n");

        config.load();
        CHECK_EQUAL(20, config.icon_height);
    }

    TEST_FIXTURE(WMConfigFixture, test_default_border_width)
    {

        write_config_file(*config_path, "\n");

        config.load();
        CHECK_EQUAL(4, config.border_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_border_width)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width= 42 \n");

        config.load();
        CHECK_EQUAL(42, config.border_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_border_width_zero)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width=0\n");

        config.load();
        CHECK_EQUAL(4, config.border_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_border_width_negative)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width=-42\n");

        config.load();
        CHECK_EQUAL(4, config.border_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_border_width_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width=not a number\n");

        config.load();
        CHECK_EQUAL(4, config.border_width);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_icons_default)
    {
        write_config_file(*config_path, "\n");

        config.load();
        CHECK_EQUAL(true, config.show_icons);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_icons_yes)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons= 1 \n");

        config.load();
        CHECK_EQUAL(true, config.show_icons);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_icons_no)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons= 0 \n");

        config.load();
        CHECK_EQUAL(false, config.show_icons);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_icons_invalid)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons= 42 \n");

        config.load();
        CHECK_EQUAL(true, config.show_icons);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_icons_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-icons=not a number\n");

        config.load();
        CHECK_EQUAL(true, config.show_icons);
    }

    TEST_FIXTURE(WMConfigFixture, test_syslog_level_default)
    {
        write_config_file(*config_path, "\n");

        config.load();
        CHECK_EQUAL(LOG_UPTO(LOG_WARNING), config.log_mask);
    }

    TEST_FIXTURE(WMConfigFixture, test_syslog_level_valid)
    {
        const char *log_names[] = {
            "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO",
            "DEBUG", NULL
        };
        int log_levels[] = {
            LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE,
            LOG_INFO, LOG_DEBUG
        };

        for (int i = 0; log_names[i] != NULL; i++)
        {
            const char *log_name = log_names[i];
            int log_level = LOG_UPTO(log_levels[i]);

            std::stringstream stream;
            stream << "[smallwm]\nlog-level= " << log_name << " \n";
            write_config_file(*config_path, stream.str().c_str());

            config.load();
            CHECK_EQUAL(log_level, config.log_mask);
        }
    }

    TEST_FIXTURE(WMConfigFixture, test_syslog_invalid)
    {
        write_config_file(*config_path,
            "[smallwm]\nlog-level=not a log level\n");

        config.load();
        CHECK_EQUAL(LOG_UPTO(LOG_WARNING), config.log_mask);
    }
};

SUITE(WMConfigSuiteActions)
{
    TEST_FIXTURE(WMConfigFixture, test_empty_actions)
    {
        // First, ensure that no class actions are set by default
        write_config_file(*config_path, "");

        config.load();
        CHECK_EQUAL(0, config.classactions.size());
    }

    TEST_FIXTURE(WMConfigFixture, test_default_actions)
    {
        // Check that we get an entry in the class actions mapping, but with
        // the default values
        write_config_file(*config_path, 
            "[actions]\ntest-class=\n");

        config.load();
        ClassActions &action = config.classactions[
            std::string("test-class")];
        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_SETLAYER);
    }

    TEST_FIXTURE(WMConfigFixture, test_invalid_actions)
    {
        // Check that we get an entry in the class actions mapping, but with
        // the default values, whenever the configuration encounters an
        // invalid value
        write_config_file(*config_path, 
            "[actions]\ntest-class=not an action\n");

        config.load();
        ClassActions &action = config.classactions[
            std::string("test-class")];
        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_SETLAYER);
    }

    TEST_FIXTURE(WMConfigFixture, test_stick)
    {
        // Check that the created class action sticks the window and nothing
        // else
        write_config_file(*config_path, 
            "[actions]\ntest-class= stick \n");

        config.load();
        ClassActions &action = config.classactions[
            std::string("test-class")];
        CHECK(action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_SETLAYER);
    }

    TEST_FIXTURE(WMConfigFixture, test_maximize)
    {

        // Check that the created class action sticks the window and nothing
        // else
        write_config_file(*config_path, 
            "[actions]\ntest-class= maximize \n");

        config.load();
        ClassActions &action = config.classactions[
            std::string("test-class")];
        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK(action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_SETLAYER);
    }

    TEST_FIXTURE(WMConfigFixture, test_set_valid_layers)
    {
        // Set the layer using some valid layers
        Layer layers[] = {MIN_LAYER, MAX_LAYER, DEF_LAYER, 87, 22, 41};
        for (int idx = 0; idx < sizeof(layers) / sizeof(*layers); idx++)
        {
            config.reset();

            std::stringstream stream;
            stream << "[actions]\ntest-class= layer:" << (int)layers[idx] << " \n";
            write_config_file(*config_path, stream.str().c_str());
            config.load();

            ClassActions &action = config.classactions[
                std::string("test-class")];

            // Ensure that only the layer is changed...
            CHECK_EQUAL(0, action.actions & ACT_STICK);
            CHECK_EQUAL(0, action.actions & ACT_SNAP);
            CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
            CHECK(action.actions & ACT_SETLAYER);

            // ... and that it is changed correctly
            CHECK_EQUAL(action.layer, layers[idx]);
        }
    }

    TEST_FIXTURE(WMConfigFixture, test_set_invalid_layers)
    {
        // Set the layer using some invalid layers
        Layer layers[] = {DIALOG_LAYER, 120, 168, 114, 210};

        for (int idx = 0; idx < sizeof(layers) / sizeof(*layers); idx++)
        {
            config.reset();

            std::stringstream stream;
            stream << "[actions]\ntest-class= layer:" << (int)layers[idx] << " \n";
            write_config_file(*config_path, stream.str().c_str());
            config.load();

            ClassActions &action = config.classactions[
                std::string("test-class")];

            // Ensure that invalid layers do not cause the layer to be
            // changed
            CHECK_EQUAL(0, action.actions & ACT_STICK);
            CHECK_EQUAL(0, action.actions & ACT_SNAP);
            CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
            CHECK_EQUAL(0, action.actions & ACT_SETLAYER);
        }

        // Test with something non-numeric
        config.reset();

        write_config_file(*config_path, "[actions]\ntest-class= layer:not a layer\n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        // Ensure that invalid layers do not cause the layer to be
        // changed
        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_SETLAYER);
    }

    TEST_FIXTURE(WMConfigFixture, test_set_snap_sides)
    {
        // Snap toward all valid sides
        SnapDir snaps[] = {SNAP_TOP, SNAP_BOTTOM, SNAP_LEFT, SNAP_RIGHT};
        const char *snap_names[] = {"top", "bottom", "left", "right"};

        for (int idx = 0; idx < sizeof(snaps) / sizeof(*snaps); idx++)
        {
            config.reset();

            std::stringstream stream;
            stream << "[actions]\ntest-class= snap:" << 
                snap_names[idx] << " \n";
            write_config_file(*config_path, stream.str().c_str());
            config.load();

            ClassActions &action = config.classactions[
                std::string("test-class")];

            CHECK_EQUAL(0, action.actions & ACT_STICK);
            CHECK(action.actions & ACT_SNAP);
            CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
            CHECK_EQUAL(0, action.actions & ACT_SETLAYER);

            CHECK_EQUAL(action.snap, snaps[idx]);
        }
    }

    TEST_FIXTURE(WMConfigFixture, test_set_invalid_snap)
    {
        // Snap some invalid sides
        const char *invalid_snaps[] = {"LEFT", "lEfT", "not a layer", "42"};

        for (int idx = 0; 
                idx < sizeof(invalid_snaps) / sizeof(*invalid_snaps);
                idx++)
        {
            config.reset();

            std::stringstream stream;
            stream << "[actions]\ntest-class= snap:" << 
                invalid_snaps[idx] << " \n";
            write_config_file(*config_path, stream.str().c_str());
            config.load();

            ClassActions &action = config.classactions[
                std::string("test-class")];

            CHECK_EQUAL(0, action.actions & ACT_STICK);
            CHECK_EQUAL(0, action.actions & ACT_SNAP);
            CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
            CHECK_EQUAL(0, action.actions & ACT_SETLAYER);
        }
    }

    TEST_FIXTURE(WMConfigFixture, test_combiations)
    {
        // Test a few combinations of different comma-separated options
        config.reset();

        write_config_file(*config_path,
            "[actions]\ntest-class= maximize, snap:left, layer:42, stick \n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK(action.actions & ACT_STICK);
        CHECK(action.actions & ACT_SNAP);
        CHECK(action.actions & ACT_MAXIMIZE);
        CHECK(action.actions & ACT_SETLAYER);

        CHECK_EQUAL(action.snap, SNAP_LEFT);
        CHECK_EQUAL(action.layer, 42);
    }

    TEST_FIXTURE(WMConfigFixture, test_combiations_with_duplicates)
    {
        // Test a few combinations of different comma-separated options
        config.reset();

        write_config_file(*config_path,
            "[actions]\ntest-class= maximize, snap:left, layer:42, stick, layer:43 \n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK(action.actions & ACT_STICK);
        CHECK(action.actions & ACT_SNAP);
        CHECK(action.actions & ACT_MAXIMIZE);
        CHECK(action.actions & ACT_SETLAYER);

        CHECK_EQUAL(action.snap, SNAP_LEFT);
        CHECK_EQUAL(action.layer, 43);
    }
};

struct DefaultBinding
{
    KeyboardAction action;
    KeySym keysym;
};

DefaultBinding shortcuts[] = {
    { CLIENT_NEXT_DESKTOP, XK_bracketright },
    { CLIENT_PREV_DESKTOP, XK_bracketleft },
    { NEXT_DESKTOP, XK_period },
    { PREV_DESKTOP, XK_comma },
    { TOGGLE_STICK, XK_backslash },
    { ICONIFY, XK_h },
    { MAXIMIZE, XK_m },
    { REQUEST_CLOSE, XK_c },
    { FORCE_CLOSE, XK_x },
    { K_SNAP_TOP, XK_Up },
    { K_SNAP_BOTTOM, XK_Down },
    { K_SNAP_LEFT, XK_Left },
    { K_SNAP_RIGHT, XK_Right },
    { LAYER_ABOVE, XK_Page_Up },
    { LAYER_BELOW, XK_Page_Down },
    { LAYER_TOP, XK_Home },
    { LAYER_BOTTOM, XK_End },
    { LAYER_1, XK_1 },
    { LAYER_2, XK_2 },
    { LAYER_3, XK_3 },
    { LAYER_4, XK_4 },
    { LAYER_5, XK_5 },
    { LAYER_6, XK_6 },
    { LAYER_7, XK_7 },
    { LAYER_8, XK_8 },
    { LAYER_9, XK_9 },
    { EXIT_WM, XK_Escape },
};

// Note that all the key bindings tested here used "layer-1" through
// "layer-9"
SUITE(WMConfigSuiteKeyboardOptions)
{
    TEST_FIXTURE(WMConfigFixture, test_default_bindngs)
    {
        write_config_file(*config_path, "");
        config.load();
        for (int i = 0; i < sizeof(shortcuts) / sizeof(*shortcuts); i++)
        {
            KeyboardAction action = shortcuts[i].action;
            KeySym keysym = shortcuts[i].keysym;
            CHECK_EQUAL(config.key_commands.bindings[action], keysym);
            CHECK_EQUAL(config.key_commands.reverse_bindings[keysym], action);
        }
    }

    TEST_FIXTURE(WMConfigFixture, test_unused_bindings)
    {
        // Makes sure that unused bindings are assigned properly
        write_config_file(*config_path,
            "[keyboard]\nlayer-1=asciitilde\nlayer-2=colon\n");
        config.load();

        CHECK_EQUAL(XK_asciitilde, config.key_commands.bindings[LAYER_1]);
        CHECK_EQUAL(LAYER_1, 
            config.key_commands.reverse_bindings[XK_asciitilde]);

        CHECK_EQUAL(XK_colon, config.key_commands.bindings[LAYER_2]);
        CHECK_EQUAL(LAYER_2, config.key_commands.reverse_bindings[XK_colon]);
    }
};

int main()
{
    char *filename = tempnam("/tmp", "smallwm");
    config_path = new std::string(filename);
    std::free(filename);

    UnitTest::RunAllTests();
    remove(config_path->c_str());
}
