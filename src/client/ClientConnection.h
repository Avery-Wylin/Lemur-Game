#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include "definitions.h"
#include <enet/enet.h>
#include "GUI.h"
#include "Packet.h"

// Foreward declare client
class Client;

struct ClientConnection {

        enum ConnectionStatus : uint8_t {
            DISCONNECTED,
            PENDING,
            VALIDATING,
            INITIALIZING,
            PLAYING
        };
        ENetHost *host_client = nullptr;
        ENetPeer *peer_server = nullptr;
        string ip = "127.0.0.1";
        int port = CONNECTION_DEFAULT_PORT;
        char message[32] = "";
        unsigned int attempts = 0;
        Client *owner;
        std::string username = "Username", passkey = "Passkey";

        ClientConnection(){
        };

        ~ClientConnection();

        // Initialize the connection
        void init(Client *owner);

        // Connect to a remote server
        void connect(std::string ip, int port);

        // Wait for a connection and validate, call each time step while waiting
        // void pend_connection(Element *msg, unsigned int max_attempts);

        // Disconnect from the server, set force to true for immediate disconnect
        void disconnect(bool force = false, std::string reason = "disconnected");

        // Get the status of the connection
        inline ConnectionStatus get_status(){return status;}

        // Called successivley to keep the connection alive and to poll events
        void update();

        // Interpret a packet
        void interpret_packet(ENetPacket *packet);

private:
       ConnectionStatus status = ClientConnection::DISCONNECTED;

};

#endif // CLIENTCONNECTION_H
