#include "ServerConnection.h"
#include "Packet.h"
#include <cstdio>
#include "Server.h"

void ServerConnection::start_host() {
    _ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    host_server = enet_host_create(
            &address,
            MAX_PLAYERS, // Maximum number of clients
            2,           // Number of channels
            0,           // Allow any amount of incoming bandwidth
            0            // Allow any amount of outgoing bandwidth
        );

    if( host_server == nullptr ) {
        puts("Failed to create ENet server host." );
        owner->stop();
        return;
    }
}

void ServerConnection::poll_packets() {
    ENetEvent event;
    while( enet_host_service( host_server, &event, 0 ) > 0 ) {
        char address_name[16];
        enet_address_get_host_ip( &( event.peer->address ), &( address_name[0] ), 16 );

        switch( event.type ) {

            case ENET_EVENT_TYPE_CONNECT: {
                printf( "Server: %s:%hu connected.\n", address_name, event.peer->address.port );
                fflush( stdout );

                // Return if too many players
                if( owner->scene.player_set.count() >= MAX_PLAYERS ) {
                    printf( "Server: %s:%hu rejected, too many players.\n", address_name, event.peer->address.port );
                    fflush( stdout );
                    continue;
                }
            }
            break;

            case ENET_EVENT_TYPE_RECEIVE:
                interpret_packets(event.packet, event.peer);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf( "Server: %s disconnected.\n", address_name );
                fflush( stdout );
                owner->scene.player_set.logout(this, event.peer);
                break;

            case ENET_EVENT_TYPE_NONE:
                break;
        }
    }
}

void ServerConnection::interpret_packets( ENetPacket *packet, ENetPeer *peer ) {
    packet_type type = ( uint8_t )packet->data[0];
    Player* p = reinterpret_cast<Player*>(peer->data);

    switch( type ) {
        case Packet::PACKET_LOGIN: {
            //TEST this just checks that you have the right name
            std::string username, passkey;
            Packet::receive_login( username, passkey, packet );
            printf( "Server: Validating user of name %s and passkey %s\n", username.c_str(), passkey.c_str() );
            fflush( stdout );
            // Call the player set login function, it will send packet responses
            owner->scene.player_set.login(username, passkey, this, peer);
            break;
        }

        case Packet::PACKET_PLAYER_INPUT: {
            Packet::receive_player_input(p, packet);
            break;
        }
    }

    enet_packet_destroy( packet );
}

