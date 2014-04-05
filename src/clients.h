/** @file */
#ifndef __SMALLWM_CLIENT_CONTAINER__
#define __SMALLWM_CLIENT_CONTAINER__

#include <map>

#include "common.h"

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
};

/**
 * A mapping between client windows and their states.
*/
class ClientContainer
{
public:
    bool is_client(Window);
    bool is_visible(Window);

    std::map<Window,ClientState>::iterator clients_begin();
    std::map<Window,ClientState>::iterator clients_end();

    ClientState get_state(Window);
protected:
    void set_state(Window, ClientState);
    void delete_state(Window);

private:
    std::map<Window, ClientState> m_clients;
};

#endif
