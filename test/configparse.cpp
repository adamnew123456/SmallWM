#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <UnitTest++.h>
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

struct WMConfigFixture 
{
    WMConfigFixture() 
    {};

    ~WMConfigFixture() 
    {};

    CustomFileWMConfig config;
};

void write_config_file(std::string filename, const char *text)
{
    std::ofstream config(filename.c_str());
    config << text;
    config.close();
}

SUITE(WMConfigSuite)
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
            "[smallwm]\nshell=some-terminal\n");

        config.load();
        CHECK_EQUAL(std::string("some-terminal"), config.shell);
    }
    
    TEST_FIXTURE(WMConfigFixture, test_shell_empty)
    {
        // Make sure that an empty configuration option is not accepted
        write_config_file(*config_path,
            "[smallwm]\nshell=\n");
        
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
            "[smallwm]\ndesktops=42\n");

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
            "[smallwm]\nicon-width=42\n");

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
            "[smallwm]\nicon-height=42\n");

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
            "[smallwm]\nborder-width=42\n");

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
            "[smallwm]\nicon-icons=1\n");

        config.load();
        CHECK_EQUAL(true, config.show_icons);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_icons_no)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons=0\n");

        config.load();
        CHECK_EQUAL(false, config.show_icons);
    }

    TEST_FIXTURE(WMConfigFixture, test_icon_icons_invalid)
    {
        write_config_file(*config_path, 
            "[smallwm]\nicon-icons=42\n");

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
};

int main()
{
    char *filename = tempnam("/tmp", "smallwm");
    config_path = new std::string(filename);
    std::free(filename);

    UnitTest::RunAllTests();
    remove(config_path->c_str());
}
