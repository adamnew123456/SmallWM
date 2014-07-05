/** @file */
#ifndef __SMALLWM_DESKTOP_TYPE__
#define __SMALLWM_DESKTOP_TYPE__
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
};

/**
 * A 'real' desktop which the user directly interacts with.
 */
struct UserDesktop : public Desktop
{
    UserDesktop(unsigned long long _desktop) :
        desktop(_desktop)
    {};

    bool is_user_desktop() const
    { return true; }

    const unsigned long long deskop;
};

/**
 * A virtual desktop describing windows which are visible on all 'real'
 * desktops.
 */
struct AllDesktops : public Desktop
{
    bool is_all_desktop const
    { return true; }
};

/**
 * A virtual desktop describing windows which are currently hidden.
 */
struct IconDesktop : public Desktop
{
    bool is_icon_desktop() const
    { return true; }
};

/**
 * A virtual desktop for a window which is currently being moved.
 */
struct MovingDesktop : public Desktop
{
    bool is_moving_desktop() const
    { return true; }
};

/**
 * A virtual desktop for a window which is currently being resized.
 */
struct ResizingDesktop : public Desktop
{
    bool is_resizing_desktop() const
    { return true; }
};
#endif
