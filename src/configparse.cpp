/** @file */
#include "configparse.h"

/**
 * Loads a configuration file and parses it.
 */
void WMConfig::load()
{
    std::string config_path = get_config_path();
    const char *c_filename = const_cast<const char*>(config_path.c_str());

    ini_parse(c_filename, &WMConfig::config_parser, this);
}

/**
 * Gets the path to the configuration file. This is a virtual method so that
 * unit tests can override this to present a dummy configuration file.
 */
std::string WMConfig::get_config_path() const
{
    std::string homedir(std::getenv("HOME"));
    homedir += "/.config/smallwm";
    return homedir;
}

/**
 * Converts a key name into a KeySym, or returns the default.
 * @param key The name of the key as a string.
 * @param default_key The default KeySym to return if conversion fails.
 * @return Either the key as a KeySym, or the default.
 */
KeySym string_to_keysym(std::string key, KeySym default_key)
{
    KeySym result = XStringToKeysym(key.c_str());
    if (result == NoSymbol)
        return default_key;
    else
        return result;
}

/**
 * A callback for the inih library, which handles a singular key-value pair.
 *
 * @param user The WMConfig object this parser belongs to (necessary because
 *             this API has to be called from C).
 * @param c_section The section name the option is found under.
 * @param c_name The name of the configuration option.
 * @param c_value The value of the configuration option.
 */
int WMConfig::config_parser(void *user, const char *c_section, 
        const char *c_name, const char *c_value)
{
    WMConfig *self = (WMConfig*)user;
    const std::string section(c_section);
    const std::string name(c_name);
    const std::string value(c_value);

    if (section == std::string("smallwm"))
    {
        if (name == std::string("shell"))
        {
            self->shell = value;
        }
        if (name == std::string("desktops"))
        {
            // Save the default value in case something goes wrong here
            Desktop old_value = self->num_desktops;

            self->num_desktops = strtoul(value.c_str(), NULL, 0);
            if (self->num_desktops == 0)
            {
                self->num_desktops = old_value;
            }
        }
        if (name == std::string("icon-width"))
        {
            Dimension old_value = self->icon_width;

            self->icon_width = strtoul(value.c_str(), NULL, 0);
            if (self->icon_width == 0)
            {
                self->icon_width = old_value;
            }
        }
        if (name == std::string("icon-height"))
        {
            Dimension old_value = self->icon_height;

            self->icon_height = strtoul(value.c_str(), NULL, 0);
            if (self->icon_height == 0)
            {
                self->icon_height = old_value;
            }
        }
        if (name == std::string("border-width"))
        {
            Dimension old_value = self->border_width;

            self->border_width = strtoul(value.c_str(), NULL, 0);
            if (self->border_width == 0)
            {
                self->border_width = old_value;
            }
        }
    }

    if (section == std::string("actions"))
    {
        ClassActions action;

        // Make sure not to butcher the value inside the contained string, to
        // make sure that the destructor doesn't do anything weird
        char *copied_value = strdup(value.c_str());

        // All the configuration options are separated by commas
        char *option = strtok(copied_value, ",");

        // The configuration values are stripped of spaces
        char *stripped;
        int opt_length;
        do
        {
            int opt_length = std::strlen(option);
            stripped = new char[opt_length];
            strip_string(option, " \n\r\t", stripped);

            if (!strcmp(stripped, "stick"))
            {
                action.actions |= ACT_STICK;
            }
            if (!strcmp(stripped, "maximize"))
            {
                action.actions |= ACT_MAXIMIZE;
            }
            if (!strcmp(stripped, "snap:left"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_LEFT;
            }
            if (!strcmp(stripped, "snap:right"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_RIGHT;
            }
            if (!strcmp(stripped, "snap:top"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_TOP;
            }
            if (!strcmp(stripped, "snap:bottom"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_BOTTOM;
            }
            if (!strncmp(stripped, "layer:", 6))
            {
                Layer layer = strtoul(stripped + 6, NULL, 0);
                if (layer >= 1 && layer <= 9)
                {
                    action.actions |= ACT_SETLAYER;
                    action.layer = layer;
                }
            }

            delete[] stripped;
        } while((option = strtok(NULL, ",")));

        self->classactions[name] = action;
    }

    // All of the keyboard bindings are hanled here - see configparse.h, and
    // more specifically KeyboardConfig.
    if (section == std::string("keyboard"))
    {
        if (name == std::string("client-next-desktop"))
            self->key_commands.client_next_desktop =
                string_to_keysym(value, XK_bracketright);

        if (name == std::string("client-prev-desktop"))
            self->key_commands.client_prev_desktop =
                string_to_keysym(value, XK_bracketleft);

        if (name == std::string("next-desktop"))
            self->key_commands.next_desktop =
                string_to_keysym(value, XK_period);

        if (name == std::string("prev-desktop"))
            self->key_commands.prev_desktop =
                string_to_keysym(value, XK_comma);

        if (name == std::string("toggle-stick"))
            self->key_commands.toggle_stick =
                string_to_keysym(value, XK_backslash);

        if (name == std::string("iconify"))
            self->key_commands.iconify =
                string_to_keysym(value, XK_h);

        if (name == std::string("maximize"))
            self->key_commands.maximize =
                string_to_keysym(value, XK_m);

        if (name == std::string("request-close"))
            self->key_commands.request_close =
                string_to_keysym(value, XK_c);

        if (name == std::string("force-close"))
            self->key_commands.force_close =
                string_to_keysym(value, XK_x);

        if (name == std::string("snap-top"))
            self->key_commands.snap_top = 
                string_to_keysym(value, XK_Up);

        if (name == std::string("snap-bottom"))
            self->key_commands.snap_bottom =
                string_to_keysym(value, XK_Down);

        if (name == std::string("snap-left"))
            self->key_commands.snap_left =
                string_to_keysym(value, XK_Left);

        if (name == std::string("snap-right"))
            self->key_commands.snap_right =
                string_to_keysym(value, XK_Right);

        if (name == std::string("layer-above"))
            self->key_commands.layer_above = 
                string_to_keysym(value, XK_Page_Up);

        if (name == std::string("layer-below"))
            self->key_commands.layer_below =
                string_to_keysym(value, XK_Page_Down);

        if (name == std::string("layer-top"))
            self->key_commands.layer_top =
                string_to_keysym(value, XK_Home);

        if (name == std::string("layer-bottom"))
            self->key_commands.layer_bottom =
                string_to_keysym(value, XK_End);

        if (name == std::string("exit-wm"))
            self->key_commands.exit_wm =
                string_to_keysym(value, XK_Escape);
    }

    return 0;
}
