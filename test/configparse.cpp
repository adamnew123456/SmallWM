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
    TEST_FIXTURE(WMConfigFixture, test_parse_shell) 
    {
        // Make sure that any particular setting is acceptable
        write_config_file(*config_path,
            "[smallwm]\nshell=some-terminal\n");

        config.load();
        CHECK_EQUAL(std::string("some-terminal"), config.shell);
    }
    
    TEST_FIXTURE(WMConfigFixture, test_parse_shell_empty)
    {
        // Make sure that an empty configuration option is not accepted
        write_config_file(*config_path,
            "[smallwm]\nshell=\n");
        
        config.load();
        CHECK_EQUAL(std::string("/usr/bin/xterm"), config.shell);
    }
};

int main()
{
    char *filename = tempnam("/tmp", "smallwm");
    config_path = new std::string(filename);
    std::free(filename);

    UnitTest::RunAllTests();
}
