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
        if (name == std::string("log-level"))
        {
            if (value == "EMERG")
                self->log_mask = LOG_UPTO(LOG_EMERG);
            else if (value == "ALERT")
                self->log_mask = LOG_UPTO(LOG_ALERT);
            else if (value == "CRIT")
                self->log_mask = LOG_UPTO(LOG_CRIT);
            else if (value == "ERR")
                self->log_mask = LOG_UPTO(LOG_ERR);
            else if (value == "WARNING")
                self->log_mask = LOG_UPTO(LOG_WARNING);
            else if (value == "NOTICE")
                self->log_mask = LOG_UPTO(LOG_NOTICE);
            else if (value == "INFO")
                self->log_mask = LOG_UPTO(LOG_INFO);
            else if (value == "DEBUG")
                self->log_mask = LOG_UPTO(LOG_DEBUG);
        }
        else if (name == std::string("shell"))
        {
            if (value.size() > 0)
                self->shell = value;
        }
        else if (name == std::string("desktops"))
        {
            Desktop old_value = self->num_desktops;
            self->num_desktops = try_parse_ulong(value.c_str(), old_value);
        }
        else if (name == std::string("icon-width"))
        {
            Dimension old_value = self->icon_width;

            self->icon_width = strtoul(value.c_str(), NULL, 0);
            if (self->icon_width == 0)
                self->icon_width = old_value;
        }
        else if (name == std::string("icon-height"))
        {
            Dimension old_value = self->icon_height;

            self->icon_height = strtoul(value.c_str(), NULL, 0);
            if (self->icon_height == 0)
                self->icon_height = old_value;
        }
        else if (name == std::string("border-width"))
        {
            Dimension old_value = self->border_width;

            self->border_width = strtoul(value.c_str(), NULL, 0);
            if (self->border_width == 0)
                self->border_width = old_value;
        }
        else if (name == std::string("icon-icons"))
        {
            bool old_value = self->show_icons;

            unsigned long as_long = strtoul(value.c_str(), NULL, -1);
            if (as_long != 0 && as_long != 1)
                self->show_icons = old_value;
            else
                self->show_icons = as_long == 1 ? true : false;
        }
    }

    else if (section == std::string("actions"))
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
            opt_length = std::strlen(option);

            stripped = new char[opt_length];
            strip_string(option, " \n\r\t", stripped);

            if (!strcmp(stripped, "stick"))
            {
                action.actions |= ACT_STICK;
            }
            else if (!strcmp(stripped, "maximize"))
            {
                action.actions |= ACT_MAXIMIZE;
            }
            else if (!strcmp(stripped, "snap:left"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_LEFT;
            }
            else if (!strcmp(stripped, "snap:right"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_RIGHT;
            }
            else if (!strcmp(stripped, "snap:top"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_TOP;
            }
            else if (!strcmp(stripped, "snap:bottom"))
            {
                action.actions |= ACT_SNAP;
                action.snap = SNAP_BOTTOM;
            }
            else if (!strncmp(stripped, "layer:", 6))
            {
                Layer layer = strtoul(stripped + 6, NULL, 0);
                if (layer >= MIN_LAYER && layer <= MAX_LAYER)
                {
                    action.actions |= ACT_SETLAYER;
                    action.layer = layer;
                }
            }

            delete[] stripped;
        } while((option = strtok(NULL, ",")));

        self->classactions[name] = action;
    }

    // All of the keyboard bindings are handled here - see configparse.h, and
    // more specifically KeyboardConfig.
    else if (section == std::string("keyboard"))
    {
        KeyboardConfig &kb_config = self->key_commands;

        // If there is already a binding for that action, then fail
        KeyboardAction action = kb_config.action_names[name];
        if (action == INVALID_ACTION)
            return 0;

        // If an old binding exists for this action, then remove it
        if (kb_config.bindings[action] != NoSymbol)
        {
            KeySym old_binding = kb_config.bindings[action];
            kb_config.bindings.erase(action);
            kb_config.reverse_bindings.erase(old_binding);
        }

        KeySym binding = XStringToKeysym(value.c_str());
        if (binding == NoSymbol)
            return 0;

        // If the key that is being bound is already in use, then fail
        if (kb_config.reverse_bindings[binding] != INVALID_ACTION)
            return 0;

        kb_config.bindings[action] = binding;
    }

    return 0;
}
