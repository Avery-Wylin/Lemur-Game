#include "ClientConnection.h"
#include "Client.h"

ClientConnection::~ClientConnection(){
}

void ClientConnection::init(Client *owner){

    this->owner = owner;

    // Initialize ENet Host
    host_client = enet_host_create(
            NULL, // Server Address
            1,    // One Connection
            2,    // Two Channels
            0,    // Any incoming bandwidth
            0     // Any outgoing bandwidth
        );

    if( host_client == nullptr ) {
        puts( "Failed to create ENet Client Host.\n" );
        exit( 1 );
    }
}


void ClientConnection::connect(std::string ip, int port){
    // Create the server
    ENetAddress server_address;

    if(status != ClientConnection::PENDING){

        this->ip = ip;
        this->port = port;

        // Set the server address
        if( peer_server != nullptr )
            enet_peer_reset( peer_server );
        enet_address_set_host( &server_address, ip.c_str() );
        server_address.port = port;
        peer_server = enet_host_connect( host_client, &server_address, 2, 0 );
        status = ClientConnection::DISCONNECTED;

        Menu::activate(&active_client->menu_message);
        if( peer_server == nullptr ) {
            active_client->menu_message.set_message("Peer is not available.", &active_client->menu_multi);
            status = ClientConnection::DISCONNECTED;
            return;
        }

        active_client->menu_message.set_message("Pending connection.", nullptr);
        status = ClientConnection::PENDING;
        attempts = 0;
    }
}

void ClientConnection::disconnect(bool force, std::string reason){
    status = ClientConnection::DISCONNECTED;

    // Show the message screen
    Menu::activate(&active_client->menu_message);

    // If the client disconnected from its own server
    if(active_client->local_server.running()){
        active_client->local_server.stop();
        active_client->menu_message.set_message(reason, &active_client->menu_single);
    }
    // If the client disconnected from a remote server
    else{
        active_client->menu_message.set_message(reason, &active_client->menu_home);

    }

    // If there isnt a server, return
    if(!peer_server )
        return;

    // Force immediate disconnection
    enet_peer_disconnect_now( peer_server, 0);

    // Skip acknowledging the server if the disconnection was forced
    if(force){
        enet_peer_reset( peer_server );
        return;
    }

    ENetEvent event;

    /* Pause 1 second for incoming packets, drop those that are not disconnect
      This will halt the main loop, so it is only when trying to cleanly exit a server,
      otherwise ENet will fail to keep the connection alive and the server will automatically disconnect
    */
    while (enet_host_service (host_client, &event, 1000) > 0 )
    {
        switch( event.type ) {
            // Server sent a packet
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy( event.packet );
                break;

            // Server acknowledged the disconnect
            case ENET_EVENT_TYPE_DISCONNECT:
                puts( "Client: disconnected." );
                fflush(stdout);
                return;

            // Not expected
            case ENET_EVENT_TYPE_CONNECT:
            case ENET_EVENT_TYPE_NONE:
                break;
        }
    }

    // Reset the peer
    enet_peer_reset( peer_server );
}

void ClientConnection::update(){

    // Perform an attempt if the status is pending
    if( status == ClientConnection::PENDING ) {
        ++attempts;
        if( attempts > 500 ) {
            active_client->menu_message.set_message( "Connection timeout, server not found.", &active_client->menu_home );
            enet_peer_reset( peer_server );
            status = ClientConnection::DISCONNECTED;
            return;
        }
    }

    ENetEvent event;
    while(enet_host_service(host_client, &event, 0) > 0){
        switch( event.type ) {
            case ENET_EVENT_TYPE_CONNECT:
                if(status == ClientConnection::PENDING){
                    active_client->menu_message.set_message( "Validating username with server." );
                    Packet::send_login(username, passkey, peer_server);
                    status = ClientConnection::VALIDATING;
                    return;
                }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                disconnect(true, "disconnected");
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                interpret_packet(event.packet);
                break;

            default:
                break;
        }
    }

}

void ClientConnection::interpret_packet( ENetPacket *packet ) {
    uint8_t type = ( uint8_t )packet->data[0];

    Player *p = owner->scene.player_set.get_active();

    switch( type ) {
        case Packet::PACKET_KICK: {
            std::string reason;
            Packet::receive_kick( reason, packet );
            reason = "Kicked: " + reason;
            disconnect( true, reason );
            break;
        }

        case Packet::PACKET_PLAYER_STATUS_SYNCH: {
            Packet::receive_player_status_synch(&owner->scene.player_set, packet);
            // If the username was found
            if(owner->scene.player_set.set_active(username) != MAX_PLAYERS){
                // Only when switching from a non-playing state
                if(status != PLAYING){
                    Menu::deactivate();
                    status = PLAYING;
                }
            }
            break;
        }

        case Packet::PACKET_PLAYER_SYNCH: {
            Packet::receive_player_synch( &owner->scene.player_set, packet );
            break;
        }

        default:
            break;
    }

    enet_packet_destroy( packet );
}


