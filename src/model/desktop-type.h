/** @file */
#ifndef __SMALLWM_DESKTOP_TYPE__
#define __SMALLWM_DESKTOP_TYPE__

#include <ostream>

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

    virtual ~Desktop()
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

    bool operator<(const Desktop &other) const
    { return sort_key < other.sort_key; }

    bool operator==(const Desktop &other) const
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

static std::ostream &operator<<(std::ostream &out, const UserDesktop &desktop)
{
    out << "[UserDesktop " << desktop.desktop << "]";
    return out;
}

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

static std::ostream &operator<<(std::ostream &out, const AllDesktops &desktop)
{
    out << "[All Desktops]";
    return out;
}

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

static std::ostream &operator<<(std::ostream &out, const IconDesktop &desktop)
{
    out << "[Icon Desktop]";
    return out;
}

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

static std::ostream &operator<<(std::ostream &out, const MovingDesktop &desktop)
{
    out << "[Moving Desktop]";
    return out;
}

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

static std::ostream &operator<<(std::ostream &out, const ResizingDesktop &desktop)
{
    out << "[Resizing Desktop]";
    return out;
}

static std::ostream &operator<<(std::ostream &out, const Desktop &desktop)
{
    if (desktop.is_user_desktop())
        out << dynamic_cast<const UserDesktop&>(desktop);
    else if (desktop.is_all_desktop())
        out << dynamic_cast<const AllDesktops&>(desktop);
    else if (desktop.is_icon_desktop())
        out << dynamic_cast<const IconDesktop&>(desktop);
    else if (desktop.is_moving_desktop())
        out << dynamic_cast<const MovingDesktop&>(desktop);
    else if (desktop.is_resizing_desktop())
        out << dynamic_cast<const ResizingDesktop&>(desktop);
    else
        out << "[Desktop]";

    return out;
}

/**
 * Like std::less<T>, but for pointers.
 */
template <typename T>
struct PointerLess
{
    /**
     * Compares a pair of Desktop pointers. This is used because Desktop must be
     * stored in structures as a pointer, due to the need for downcasting to
     * access appropriate members.
     *
     * Note that this also handles NULL pointers, by saying that a NULL pointer
     * is less than any other pointer, with the exception of other null
     * pointers.
     */
    bool operator()(const T *a, const T* b) const
    {
        if (!a && !b)
            return false;
        else if (!a)
            return true;
        else if (!b)
            return false;
        else
            return *a < *b;
    }
};

#endif
