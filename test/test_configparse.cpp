#include "test_configparse.h"

/**
 * An override to the WMConfig::get_config_path() method which uses a local 
 * configuration file, rather than the user's configuration file.
 *
 * @return The path to the sample configuration file
 */
std::string TestWMConfig::get_config_path() const
{
    return std::string("./sample.conf");
}

template<typename T>
void show_error(const char *config_option, T true_value, T actual_value)
{
    std::cout << "[" << config_option << "] Failure - expected '"
        << true_value << "', got '" << actual_value << "'\n";
}

int main()
{
    // Load up the configuration file 'sample.conf' and check its properties
    TestWMConfig config;
    config.load();

    if (config.shell != std::string("/usr/bin/foo -bar --baz"))
    {
        show_error<std::string>("smallwm.shell", 
                std::string("/usr/bin/foo -bar --baz"), config.shell);
    }
    
    // num_desktops is not set in the configuration file - this is configparse.cpp's default
    if (config.num_desktops != 5)
    {
        show_error<Desktop>("smallwm.desktops", 5, config.num_desktops);
    }

    if (config.icon_width != 1)
    {
        show_error<Dimension>("smallwm.icon-width", 1, config.icon_width);
    }
    
    if (config.icon_height != 2)
    {
        show_error<Dimension>("smallwm.icon-height", 2, config.icon_height);
    }

    if (config.border_width != 3)
    {
        show_error<Dimension>("smallwm.border-width", 3, config.border_width);
    }

    ClassActions class_a = config.classactions[std::string("class-a")];
    if (!(class_a.actions & ACT_STICK))
    {
        std::cout << "[actions.class-a] Does not include a STICK action\n";
    }

    if (!(class_a.actions & ACT_MAXIMIZE))
    {
        std::cout << "[actions.class-a] Does not include a MAXIMIZE action\n";
    }

    if (!(class_a.actions & ACT_SNAP))
    {
        std::cout << "[actions.class-a] Does not include a SNAP action\n";
    }

    if (!(class_a.actions & ACT_SETLAYER))
    {
        std::cout << "[actions.class-a] Does not include a SETLAYER action\n";
    }

    if (class_a.snap != SNAP_LEFT)
    {
        std::cout << "[actions.class-a] Does not snap left\n";
    }

    if (class_a.layer != 2)
    {
        std::cout << "[actions.class-a] Does not set the layer to '2'\n";
    }

    ClassActions class_b = config.classactions[std::string("class-b")];
    if (!(class_b.actions & ACT_SNAP))
    {
        std::cout << "[actions.class-b] Does not include a SNAP action\n";
    }

    if (!(class_b.actions & ACT_SETLAYER))
    {
        std::cout << "[actions.class-b] Does not include a SETLAYER action\n";
    }

    if (class_b.snap != SNAP_RIGHT)
    {
        std::cout << "[actions.class-b] Does not snap right\n";
    }

    if (class_b.layer != 7)
    {
        std::cout << "[actions.class-b] Does not set the layer to 7\n";
    }
    
    ClassActions class_c = config.classactions[std::string("class-c")];
    if (!(class_c.actions & ACT_SNAP))
    {
        std::cout << "[actions.class-c] Does not include a SNAP action\n";
    }

    if (!(class_c.actions & ACT_SETLAYER))
    {
        std::cout << "[actions.class-c] Does not include a SETLAYER action\n";
    }

    if (class_c.snap != SNAP_TOP)
    {
        std::cout << "[actions.class-c] Does not snap top\n";
    }

    if (class_c.layer != 4)
    {
        std::cout << "[actions.class-c] Does not set the layer to 4\n";
    }
}
