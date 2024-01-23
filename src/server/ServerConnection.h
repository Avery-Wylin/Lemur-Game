#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <definitions.h>
#include <enet/enet.h>
#include <inttypes.h>

class Server;
class ServerConnection {
    uint16_t port;
    Server *owner = nullptr;

    // Process a packet performing the action on the server
    void interpret_packets(ENetPacket *p, ENetPeer *peer);
public:
    ENetHost *host_server;

    // Create an ENetHost for the server
    void start_host();

    // Set the port before hosting
    inline void set_port(uint16_t port){this->port = port;}

    inline void set_owner(Server *owner){this->owner = owner;};

    // Poll for any incoming packets
    void poll_packets();
};

#endif // SERVERCONNECTION_H
