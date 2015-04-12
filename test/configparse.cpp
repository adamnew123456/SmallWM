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

std::string *config_path = static_cast<std::string*>(0);

class CustomFileWMConfig : public WMConfig
{
protected:
    std::string get_config_path() const
    {
        return *config_path;
    };
};

CustomFileWMConfig config;
void write_config_file(std::string filename, const char *text)
{
    std::ofstream config(filename.c_str());
    config << text;
    config.close();
}

SUITE(WMConfigSuitePlainOptions)
{
    TEST(test_default_shell)
    {
        // Ensure that an empty config file provides the default shell
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(std::string("/usr/bin/xterm"), config.shell);
    }

    TEST(test_shell) 
    {
        // Make sure that any particular setting is acceptable
        write_config_file(*config_path,
            "[smallwm]\nshell=some-terminal \n");
        config.load();

        CHECK_EQUAL(std::string("some-terminal"), config.shell);
    }
    
    TEST(test_shell_empty)
    {
        // Make sure that an empty configuration option is not accepted
        write_config_file(*config_path,
            "[smallwm]\nshell= \n");
        config.load();

        CHECK_EQUAL(std::string("/usr/bin/xterm"), config.shell);
    }

    TEST(test_default_num_desktops)
    {
        // Ensure that an empty config file provides the default number of
        // desktops
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST(test_num_desktops)
    {
        // Make sure that a valid number of desktops sets the option
        write_config_file(*config_path,
            "[smallwm]\ndesktops= 42 \n");
        config.load();

        CHECK_EQUAL(42, config.num_desktops);
    }

    TEST(test_num_desktops_zero)
    {
        // Zero desktops are invalid, so make sure they revert to the default
        write_config_file(*config_path,
            "[smallwm]\ndesktops=0\n");
        config.load();

        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST(test_num_desktops_negative)
    {
        // Negative desktops are also invalid
        write_config_file(*config_path,
            "[smallwm]\ndesktops=-32\n");
        config.load();

        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST(test_num_desktops_non_numeric)
    {
        // Non-numbers should give back the default value
        write_config_file(*config_path,
            "[smallwm]\ndesktops=not a number\n");
        config.load();

        CHECK_EQUAL(5, config.num_desktops);
    }

    TEST(test_default_icon_width)
    {
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(75, config.icon_width);
    }

    TEST(test_icon_width)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width= 42 \n");
        config.load();

        CHECK_EQUAL(42, config.icon_width);
    }

    TEST(test_icon_width_zero)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width=0\n");
        config.load();

        CHECK_EQUAL(75, config.icon_width);
    }

    TEST(test_icon_width_negative)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width=-42\n");
        config.load();

        CHECK_EQUAL(75, config.icon_width);
    }

    TEST(test_icon_width_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-width=not a number\n");
        config.load();

        CHECK_EQUAL(75, config.icon_width);
    }

    TEST(test_default_icon_height)
    {
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(20, config.icon_height);
    }

    TEST(test_icon_height)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height= 42 \n");
        config.load();

        CHECK_EQUAL(42, config.icon_height);
    }

    TEST(test_icon_height_zero)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height=0\n");
        config.load();

        CHECK_EQUAL(20, config.icon_height);
    }

    TEST(test_icon_height_negative)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height=-42\n");
        config.load();

        CHECK_EQUAL(20, config.icon_height);
    }

    TEST(test_icon_height_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-height=not a number\n");
        config.load();

        CHECK_EQUAL(20, config.icon_height);
    }

    TEST(test_default_border_width)
    {
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(4, config.border_width);
    }

    TEST(test_border_width)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width= 42 \n");
        config.load();

        CHECK_EQUAL(42, config.border_width);
    }

    TEST(test_border_width_zero)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width=0\n");
        config.load();

        CHECK_EQUAL(4, config.border_width);
    }

    TEST(test_border_width_negative)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width=-42\n");
        config.load();

        CHECK_EQUAL(4, config.border_width);
    }

    TEST(test_border_width_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nborder-width=not a number\n");
        config.load();

        CHECK_EQUAL(4, config.border_width);
    }

    TEST(test_icon_icons_default)
    {
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(true, config.show_icons);
    }

    TEST(test_icon_icons_yes)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons= 1 \n");
        config.load();

        CHECK_EQUAL(true, config.show_icons);
    }

    TEST(test_icon_icons_no)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons= 0 \n");

        config.load();
        CHECK_EQUAL(false, config.show_icons);
    }

    TEST(test_icon_icons_invalid)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons= 42 \n");
        config.load();

        CHECK_EQUAL(true, config.show_icons);
    }

    TEST(test_icon_icons_not_numeric)
    {
        write_config_file(*config_path,
            "[smallwm]\nicon-icons=not a number\n");
        config.load();

        CHECK_EQUAL(true, config.show_icons);
    }

    TEST(test_syslog_level_default)
    {
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(LOG_UPTO(LOG_WARNING), config.log_mask);
    }

    TEST(test_syslog_level_valid)
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

    TEST(test_syslog_invalid)
    {
        write_config_file(*config_path,
            "[smallwm]\nlog-level=not a log level\n");
        config.load();

        CHECK_EQUAL(LOG_UPTO(LOG_WARNING), config.log_mask);
    }
};

SUITE(WMConfigSuiteActions)
{
    TEST(test_empty_actions)
    {
        // First, ensure that no class actions are set by default
        write_config_file(*config_path, "");
        config.load();

        CHECK_EQUAL(0, config.classactions.size());
    }

    TEST(test_default_actions)
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
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);
    }

    TEST(test_invalid_actions)
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
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);
    }

    TEST(test_stick)
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
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);
    }

    TEST(test_maximize)
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
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);
    }

    TEST(test_set_valid_layers)
    {
        // Set the layer using some valid layers
        Layer layers[] = {MIN_LAYER, MAX_LAYER, DEF_LAYER, 6, 4, 9};
        for (int idx = 0; idx < sizeof(layers) / sizeof(*layers); idx++)
        {

            std::stringstream stream;
            stream << "[actions]\ntest-class= layer:" << 
                static_cast<int>(layers[idx]) << " \n";
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

    TEST(test_set_invalid_layers)
    {
        // Set the layer using some invalid layers
        Layer layers[] = {DIALOG_LAYER, 12, 16, 22, 19};

        for (int idx = 0; idx < sizeof(layers) / sizeof(*layers); idx++)
        {

            std::stringstream stream;
            stream << "[actions]\ntest-class= layer:" << 
                static_cast<int>(layers[idx]) << " \n";
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

    TEST(test_set_snap_sides)
    {
        // Snap toward all valid sides
        SnapDir snaps[] = {SNAP_TOP, SNAP_BOTTOM, SNAP_LEFT, SNAP_RIGHT};
        const char *snap_names[] = {"top", "bottom", "left", "right"};

        for (int idx = 0; idx < sizeof(snaps) / sizeof(*snaps); idx++)
        {

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

    TEST(test_set_invalid_snap)
    {
        // Snap some invalid sides
        const char *invalid_snaps[] = {"LEFT", "lEfT", "not a layer", "42"};

        for (int idx = 0; 
                idx < sizeof(invalid_snaps) / sizeof(*invalid_snaps);
                idx++)
        {

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

    TEST(test_x_pos)
    {
        write_config_file(*config_path, 
            "[actions]\ntest-class=xpos:57.32\n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK(action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);

        CHECK_CLOSE(action.relative_x, 57.32 / 100.0, 0.001);
    }

    TEST(test_invalid_x_pos)
    {
        // The only valid range is in [0, 100], so test two values outside of
        // that range
        write_config_file(*config_path, 
            "[actions]\ntest-class=xpos:-15.05\n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);

        write_config_file(*config_path, 
            "[actions]\ntest-class=xpos:-15.05\n");
        config.load();

        action = config.classactions[
            std::string("test-class")];

        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);
    }
    
    TEST(test_y_pos)
    {
        write_config_file(*config_path, 
            "[actions]\ntest-class=ypos:57.32\n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK(action.actions & ACT_MOVE_Y);

        CHECK_CLOSE(action.relative_y, 57.32 / 100.0, 0.001);
    }

    TEST(test_invalid_y_pos)
    {
        // The only valid range is in [0, 100], so test two values outside of
        // that range
        write_config_file(*config_path, 
            "[actions]\ntest-class=ypos:-15.05\n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);

        write_config_file(*config_path, 
            "[actions]\ntest-class=ypos:109.52\n");
        config.load();

        action = config.classactions[
            std::string("test-class")];

        CHECK_EQUAL(0, action.actions & ACT_STICK);
        CHECK_EQUAL(0, action.actions & ACT_SNAP);
        CHECK_EQUAL(0, action.actions & ACT_MAXIMIZE);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_X);
        CHECK_EQUAL(0, action.actions & ACT_MOVE_Y);
    }

    TEST(test_hotkey_default)
    {
        write_config_file(*config_path, "\n");
        config.load();

        CHECK_EQUAL(config.hotkey, HK_MOUSE);
    }

    TEST(test_hotkey_focus)
    {
        write_config_file(*config_path,
            "[smallwm]\nhotkey-mode=focus\n");
        config.load();

        CHECK_EQUAL(config.hotkey, HK_FOCUS);
    }

    TEST(test_hotkey_mouse)
    {
        write_config_file(*config_path,
            "[smallwm]\nhotkey-mode=mouse\n");
        config.load();

        CHECK_EQUAL(config.hotkey, HK_MOUSE);
    }

    TEST(test_invalid_hotkey)
    {
        write_config_file(*config_path,
            "[smallwm]\nhotkey-mode=blargh\n");
        config.load();

        CHECK_EQUAL(config.hotkey, HK_MOUSE);
    }

    TEST(test_combiations)
    {
        // Test a few combinations of different comma-separated options

        write_config_file(*config_path,
            "[actions]\ntest-class= maximize, snap:left, layer:4, stick \n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK(action.actions & ACT_STICK);
        CHECK(action.actions & ACT_SNAP);
        CHECK(action.actions & ACT_MAXIMIZE);
        CHECK(action.actions & ACT_SETLAYER);

        CHECK_EQUAL(action.snap, SNAP_LEFT);
        CHECK_EQUAL(action.layer, 4);
    }

    TEST(test_combiations_with_duplicates)
    {
        // Test a few combinations of different comma-separated options

        write_config_file(*config_path,
            "[actions]\ntest-class= maximize, snap:left, layer:4, stick, layer:5 \n");
        config.load();

        ClassActions &action = config.classactions[
            std::string("test-class")];

        CHECK(action.actions & ACT_STICK);
        CHECK(action.actions & ACT_SNAP);
        CHECK(action.actions & ACT_MAXIMIZE);
        CHECK(action.actions & ACT_SETLAYER);

        CHECK_EQUAL(action.snap, SNAP_LEFT);
        CHECK_EQUAL(action.layer, 5);
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
    TEST(test_default_bindngs)
    {
        write_config_file(*config_path, "");
        config.load();
        for (int i = 0; i < sizeof(shortcuts) / sizeof(*shortcuts); i++)
        {
            KeyboardAction action = shortcuts[i].action;
            KeySym keysym = shortcuts[i].keysym;
            CHECK_EQUAL(config.key_commands.action_to_keysym[action], keysym);
            CHECK_EQUAL(config.key_commands.keysym_to_action[keysym], action);
        }
    }

    TEST(test_unused_bindings)
    {
        // Makes sure that unused bindings are assigned properly
        write_config_file(*config_path,
            "[keyboard]\nlayer-1=asciitilde\nlayer-2=colon\n");
        config.load();

        CHECK_EQUAL(XK_asciitilde, 
            config.key_commands.action_to_keysym[LAYER_1]);
        CHECK_EQUAL(LAYER_1, 
            config.key_commands.keysym_to_action[XK_asciitilde]);

        CHECK_EQUAL(XK_colon, config.key_commands.action_to_keysym[LAYER_2]);
        CHECK_EQUAL(LAYER_2, config.key_commands.keysym_to_action[XK_colon]);
    }

    TEST(test_duplicate_bindings)
    {
        // Tests to make sure that duplicate bindings get reverted, to avoid
        // bindings which are not attached to any key
        write_config_file(*config_path,
            "[keyboard]\nlayer-1=x\n");
        config.load();

        // Remember that the binding remains for the original Super+x binding
        CHECK_EQUAL(XK_x, config.key_commands.action_to_keysym[FORCE_CLOSE]);
        CHECK_EQUAL(FORCE_CLOSE, config.key_commands.keysym_to_action[XK_x]);

        // Similarly, make sure that the old binding remains for the original
        // LAYER_1 action
        CHECK_EQUAL(XK_1, config.key_commands.action_to_keysym[LAYER_1]);
        CHECK_EQUAL(LAYER_1, config.key_commands.keysym_to_action[XK_1]);
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
