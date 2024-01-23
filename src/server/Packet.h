#ifndef PACKET_H
#define PACKET_H

#include <inttypes.h>
#include <string>
#include <enet/enet.h>
#include <Player.h>

/*
 * Packets are created by calling the packet functions and giving the required args.
 * The packet is then constructed and passed to a connection object.
 */
typedef uint8_t packet_type;

namespace Packet{

    /*
     * Server Bound
     * Requests a login using a username and passkey.
     * If the login is sucessful, an activate player packet is broadcast.
     * If the login fails, a kick is sent to the sender.
     */
    const packet_type PACKET_LOGIN = 0;
    void send_login(std::string &username, std::string &passkey, ENetPeer *dest);
    void receive_login(std::string &username, std::string &passkey, ENetPacket *packet);

    /*
     * Client Bound
     * Request that the player disconnects and sends a message why.
     * Calling send_kick() disconnects the peer after flushing packets.
     */
    const packet_type PACKET_KICK = 1;
    void send_kick(std::string &reason, ENetPeer *dest);
    void receive_kick(std::string &reason, ENetPacket *packet);

    /*
     * Server Bound
     * Sends the input flags of a player to the server.
     * The clients change with key events and send all input flags to the server.
     */
    const packet_type PACKET_PLAYER_INPUT = 2;
    void send_player_input(Player *p, ENetPeer *dest);
    void receive_player_input(Player *p,  ENetPacket *packet);

    /*
    * Client Bound
    * Synchronizes the status of players.
    * This include the player count, player usernames, and other infrequently changing values.
    */
    const packet_type PACKET_PLAYER_STATUS_SYNCH = 3;
    void broadcast_player_status_synch( PlayerSet *player_set, ENetHost *host);
    void receive_player_status_synch(PlayerSet *player_set,  ENetPacket *packet);

    /*
     * Client Bound
     * Synchronizes all active players.
     * Clients read the player data and display it.
     * Clients may predict motion, and generally show one step behind real-time.
     * Armatures are automatically interpolated when calling the interpolate() function.
     */
    const packet_type PACKET_PLAYER_SYNCH = 4;
    void broadcast_player_synch( PlayerSet *player_set, ENetHost *host);
    void receive_player_synch(PlayerSet *player_set,  ENetPacket *packet);
};



#endif // PACKET_H
