/** @file */
#ifndef __SMALLWM_DESKTOP_TYPE__
#define __SMALLWM_DESKTOP_TYPE__

/*
 * These constants are used to make sure that different instances of
 * desktop classes with the same parameters are equal, and are stored in
 * the same place in an std::map
 */
const unsigned long long DESKTOP_SORT_KEY = 1,
      ALL_DESKTOP_SORT_KEY = 2,
      ICON_DESKTOP_SORT_KEY = 3,
      MOVING_DESKTOP_SORT_KEY = 4,
      RESIZING_DESKTOP_SORT_KEY = 5,
      USER_DESKTOP_SORT_KEY = 6;

/**
 * This describes both 'real' desktops that the user interacts with, and
 * 'virtual' desktops which are used as a part of the data model.
 *
 * In essence, the term 'desktop' is used to describe any group of windows
 * to which a window must belong exclusively - it cannot be on two desktops
 * at once. This causes some issues (hence `AllDesktops`) but it works well
 * in practice.
 */
struct Desktop
{
    Desktop(unsigned long long _sort_key) : sort_key(_sort_key)
    {};

    Desktop() : sort_key(DESKTOP_SORT_KEY)
    {};

    virtual bool is_user_desktop() const
    { return false; }

    virtual bool is_all_desktop() const
    { return false; }

    virtual bool is_icon_desktop() const
    { return false; }

    virtual bool is_moving_desktop() const
    { return false; }

    virtual bool is_resizing_desktop() const
    { return false; }

    virtual bool operator<(const Desktop &other) const
    { return sort_key < other.sort_key; }

    virtual bool operator==(const Desktop &other) const
    { return sort_key == other.sort_key; }

    unsigned long long sort_key;
};

/**
 * A 'real' desktop which the user directly interacts with.
 */
struct UserDesktop : public Desktop
{
    UserDesktop(unsigned long long _desktop) :
        desktop(_desktop), Desktop(_desktop + USER_DESKTOP_SORT_KEY)
    {};

    bool is_user_desktop() const
    { return true; }

    unsigned long long desktop;
};

/**
 * A virtual desktop describing windows which are visible on all 'real'
 * desktops.
 */
struct AllDesktops : public Desktop
{
    AllDesktops() : Desktop(ALL_DESKTOP_SORT_KEY)
    {};

    bool is_all_desktop() const
    { return true; }
};

/**
 * A virtual desktop describing windows which are currently hidden.
 */
struct IconDesktop : public Desktop
{
    IconDesktop() : Desktop(ICON_DESKTOP_SORT_KEY)
    {};

    bool is_icon_desktop() const
    { return true; }
};

/**
 * A virtual desktop for a window which is currently being moved.
 */
struct MovingDesktop : public Desktop
{
    MovingDesktop() : Desktop(MOVING_DESKTOP_SORT_KEY)
    {};

    bool is_moving_desktop() const
    { return true; }
};

/**
 * A virtual desktop for a window which is currently being resized.
 */
struct ResizingDesktop : public Desktop
{
    ResizingDesktop() : Desktop(RESIZING_DESKTOP_SORT_KEY)
    {};

    bool is_resizing_desktop() const
    { return true; }
};
#endif
