/** @file */
#ifndef __SMALLWM_CLIENT_CONTAINER__
#define __SMALLWM_CLIENT_CONTAINER__

#include <map>

#include "common.h"
#include "shared.h"

/**
 * A complete listing of all the possible states that each client is capable of
 * being in.
 *
 * Keeping an explicit FSM in the code makes it _much_ easier to modify. The FSM
 * itself is implemented in ClientManager::state_transition
 */
enum ClientState {
    /// Active clients are visible to the user and can be interacted with
    CS_ACTIVE = 1,
    /// Visible clients are visible but do not have the input focused
    CS_VISIBLE,
    /// Iconified clients are not visible, but have an icon on the top of the screen
    CS_ICON,
    /// Invisible clients are on a different desktop from the one the user is 
    /// currently on
    CS_INVISIBLE,
    /// Clients which are currently being moved by the user
    CS_MOVING,
    /// Clients which are currently being resized by the user
    CS_RESIZING,
    /// A client has already been destroyed
    CS_DESTROY,
    /// A client has voluntarily asked to be totally invisible
    CS_WITHDRAWN,
};

/**
 * A mapping between client windows and their states.
*/
class ClientContainer
{
public:
    ClientContainer(WMShared &shared) : 
        m_shared(shared)
    {};

    /// Iterator for accessing the client state mapping
    typedef std::map<Window,ClientState>::iterator iterator;

    bool is_client(Window);
    bool is_visible(Window);

    ClientContainer::iterator clients_begin();
    ClientContainer::iterator clients_end();

    ClientState get_state(Window);

    // Yes, this is a horrible hack that lets the interface of the base
    // class leak through. It is probably not a good idea, design wise,
    // but it's better than the overall design its replacing.
    virtual void state_transition(Window, ClientState) = 0;
    virtual void relayer() = 0;
    virtual void redesktop() = 0;

protected:
    void set_state(Window, ClientState);
    void delete_state(Window);

private:
    /// A mapping of all clients to their states
    std::map<Window, ClientState> m_clients;

    /// The shared window manager data
    WMShared &m_shared;
};

#endif
