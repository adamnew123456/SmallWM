/** @file */
#ifndef __SMALLWM_MAYBE_TYPE__
#define __SMALLWM_MAYBE_TYPE__

/**
 * This is a base class for representing either a present value or an absent
 * value.
 *
 * This is used primarily for representing the currently focused window in the
 * client data model, but it is generic enough that it may be of use
 * elsewhere.
 *
 * An instance of `Maybe` must be either a `Just` or a `Nothing`. A `Just`
 * carries a value, while a `Nothing` does not.
 */
struct Maybe
{ virtual bool has_value() const = 0; };

template <typename T>
struct Just : Maybe
{
    Just(T const &value)
    {
        this->value = value;
    }

    bool has_value() const
    { return true; }

    T const &value;
};

struct Nothing : Maybe
{
    bool has_value() const
    { return false; }
};

#endif
