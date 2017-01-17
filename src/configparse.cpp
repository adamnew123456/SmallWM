/** @file */
#include "configparse.h"

/**
 * Loads a configuration file and parses it.
 */
void WMConfig::load()
{
    // This is meant to help out the unit tests, which would otherwise have
    // 'reset()' calls sprinkled about
    reset();

    std::string config_path = get_config_path();
    const char *c_filename = const_cast<const char*>(config_path.c_str());

    ini_parse(c_filename, &WMConfig::config_parser, this);
}

/**
 * Resets the configuration - this is used mostly for testing purposes.
 */
void WMConfig::reset()
{
    shell.assign("/usr/bin/xterm");
    num_desktops = 5;
    icon_width = 75;
    icon_height = 20;
    border_width = 4;
    show_icons = true;
    log_mask = LOG_UPTO(LOG_WARNING);
    hotkey = HK_MOUSE;

    key_commands.reset();
    classactions.clear();
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
    WMConfig *self = static_cast<WMConfig*>(user);
    const std::string section(c_section);
    const std::string name(c_name);
    const std::string value(c_value);

    if (section == std::string("smallwm"))
    {
        if (name == std::string("log-level"))
        {
#define SYSLOG_MACRO_CHECK(level) do {\
        if (value == std::string(#level)) \
            self->log_mask = LOG_UPTO(LOG_##level); \
    } while (0);
            SYSLOG_MACRO_CHECK(EMERG);
            SYSLOG_MACRO_CHECK(ALERT);
            SYSLOG_MACRO_CHECK(CRIT);
            SYSLOG_MACRO_CHECK(ERR);
            SYSLOG_MACRO_CHECK(WARNING);
            SYSLOG_MACRO_CHECK(NOTICE);
            SYSLOG_MACRO_CHECK(INFO);
            SYSLOG_MACRO_CHECK(DEBUG);
#undef SYSLOG_MACRO_CHECK
        }
        else if (name == std::string("hotkey-mode"))
        {
            if (value == std::string("focus"))
                self->hotkey = HK_FOCUS;
            else if (value == std::string("mouse"))
                self->hotkey = HK_MOUSE;
            else
                self->hotkey = HK_MOUSE;
        }
        else if (name == std::string("shell"))
        {
            if (value.size() > 0)
                self->shell = value;
        }
        else if (name == std::string("desktops"))
        {
            unsigned long long old_value = self->num_desktops;
            self->num_desktops = try_parse_ulong_nonzero(value.c_str(), old_value);
        }
        else if (name == std::string("icon-width"))
        {
            Dimension old_value = self->icon_width;
            self->icon_width = try_parse_ulong_nonzero(value.c_str(), old_value);
        }
        else if (name == std::string("icon-height"))
        {
            Dimension old_value = self->icon_height;
            self->icon_height = try_parse_ulong_nonzero(value.c_str(), old_value);
        }
        else if (name == std::string("border-width"))
        {
            Dimension old_value = self->border_width;
            self->border_width = try_parse_ulong_nonzero(value.c_str(), old_value);
        }
        else if (name == std::string("icon-icons"))
        {
            bool old_value = self->show_icons;
            self->show_icons =
                try_parse_ulong(value.c_str(),
                     static_cast<unsigned long>(old_value)) != 0;
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

        // Catch an empty configuration setting (which returns NULL) before it
        // gets into the loop below, which will cause a crash. However, we still
        // have to assign the empty ClassAction, so we can't just return here.
        if (!option)
            goto finish_actions;

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
                action.snap = DIR_LEFT;
            }
            else if (!strcmp(stripped, "snap:right"))
            {
                action.actions |= ACT_SNAP;
                action.snap = DIR_RIGHT;
            }
            else if (!strcmp(stripped, "snap:top"))
            {
                action.actions |= ACT_SNAP;
                action.snap = DIR_TOP;
            }
            else if (!strcmp(stripped, "snap:bottom"))
            {
                action.actions |= ACT_SNAP;
                action.snap = DIR_BOTTOM;
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
            else if (!strncmp(stripped, "xpos:", 5))
            {
                double relative_x = strtod(stripped + 5, NULL) / 100.0;
                if (relative_x > 0.0 && relative_x < 1.0)
                {
                    action.actions |= ACT_MOVE_X;
                    action.relative_x = relative_x;
                }
            }
            else if (!strncmp(stripped, "ypos:", 5))
            {
                double relative_y = strtod(stripped + 5, NULL) / 100.0;
                if (relative_y > 0.0 && relative_y < 1.0)
                {
                    action.actions |= ACT_MOVE_Y;
                    action.relative_y = relative_y;
                }
            }
            else if (!strcmp(stripped, "nofocus"))
            {
                // This looks different, because it is not an action, but a
                // persistent setting which is respected in multiple places
                self->no_autofocus.push_back(name);
            }

            delete[] stripped;
        } while (option = strtok(NULL, ","));


finish_actions:
        free(copied_value);
        self->classactions[name] = action;
    }

    // All of the keyboard bindings are handled here - see configparse.h, and
    // more specifically KeyboardConfig.
    else if (section == std::string("keyboard"))
    {
        KeyboardConfig &kb_config = self->key_commands;
        KeyboardAction action = kb_config.action_names[name];

        bool uses_secondary_action = (value[0] == '!');

        std::string key_value = value;
        if (uses_secondary_action)
            key_value.erase(0, 1);

        KeySym key = XStringToKeysym(key_value.c_str());
        // Fail if the key is not recognized by Xlib
        if (key == NoSymbol)
            return 0;

        KeyBinding binding(key, uses_secondary_action);

        // If this new binding is in use by another entry, then fail
        if (kb_config.binding_to_action[binding] != INVALID_ACTION)
            return 0;

        // If an old binding exists for this action, then remove it
        KeyBinding old_binding = kb_config.action_to_binding[action];
        if (old_binding.first != NoSymbol)
            kb_config.binding_to_action.erase(old_binding);

        kb_config.action_to_binding[action] = binding;
        kb_config.binding_to_action[binding] = action;
    }

    return 0;
}
