#include "Packet.h"
#include "ClientConnection.h"
#include "ServerConnection.h"
#include <string.h>

// NOTE Does not support inter-system endian changes, all systems must be little endian.
// Each encode and decode function shifts the pointer the amount used, do not simply pass in the original data pointer
// It is important that a packet correctly preallocates its size for ENet packets

// Encode a single type, this type should be a POD type only
template<typename T> void encode(T t, ENetPacket *pac, unsigned int &offset ){
    if( pac->dataLength < offset + sizeof(T))
        enet_packet_resize( pac, pac->dataLength + ( offset+sizeof(T) - pac->dataLength) );
    T *tp = (T*)(pac->data + offset);
    *tp = t;
    offset += sizeof(T);
};

// Encode an array of exact size that the sender and reciever agree on
template<typename T> void encode_array(T *t, uint8_t count, ENetPacket *pac, unsigned int &offset){
    unsigned int size = sizeof(T)*count;
    if( pac->dataLength < offset + size )
        enet_packet_resize( pac, pac->dataLength + ( offset + size - pac->dataLength) );
    memcpy(pac->data + offset, t, size );
    offset += size;
};

// Encode a vector of values up to 255 values
template<typename T> void encode_vec(std::vector<T> &t, ENetPacket *pac, unsigned int &offset){
    uint8_t length = std::min(t.size, UINT8_MAX);
    unsigned int size = length * sizeof(T);
    if( pac->dataLength < offset + size + 1)
        enet_packet_resize( pac, pac->dataLength + ( offset + size + 1 - pac->dataLength) );
    pac->data[offset] = length;
    ++offset;
    memcpy(pac->data + offset, t, size );
    offset += size;
};

// Encode a string up to 255 chars
void encode_string(std::string &str, ENetPacket *pac, unsigned int &offset){
    uint8_t length = std::min((int)str.size(), UINT8_MAX);
    if( pac->dataLength < offset + length + 1 )
        enet_packet_resize( pac, pac->dataLength + ( offset + length + 1 - pac->dataLength) );
    pac->data[offset] = length;
    ++offset;
    memcpy(pac->data + offset, str.data(), length );
    offset += length;
};

// Decode a single type
template<typename T> void decode(T &t, ENetPacket *pac, unsigned int &offset){
    T *tp = (T*)( pac->data + offset );
    t = *tp;
    offset += sizeof(T);
};

// Decode a fixed-size array of types
template<typename T> void decode_array(T *t, uint8_t count, ENetPacket *pac, unsigned int &offset){
    unsigned int s = sizeof(T)*count;
    s = std::min(s,(unsigned int)pac->dataLength);
    memcpy(t, pac->data + offset, s);
    offset += s;
};

// Decode a variable-sized array of types
template<typename T> void decode_vec(std::vector<T> &t, ENetPacket *pac, unsigned int &offset){
    uint8_t count = pac->data[offset];
    ++offset;
    t.resize(count);
    unsigned int s = sizeof(T) * count;
    s = std::min(s,(unsigned int)pac->dataLength);
    memcpy(t.data(), pac->data + offset, s);
    offset += s;
};

void decode_string(std::string &str, ENetPacket *pac, unsigned int &offset){
    uint8_t count = pac->data[offset];
    ++offset;
    str.resize(count);
    count = std::min((unsigned int)count,(unsigned int)pac->dataLength);
    memcpy(str.data(), pac->data + offset, count);
    offset += count;
};

#define packet_create(packet_size) unsigned int offset = 0; ENetPacket *packet = enet_packet_create(nullptr, packet_size, ENET_PACKET_FLAG_RELIABLE);
#define packet_send enet_peer_send(dest, 0, packet);
#define packet_broadcast enet_host_broadcast(host, 0, packet);
namespace Packet{

    /*
     * Send functions take in the desired arguments and make a packet.
     * Broadcast functions are server only and broadcast instead of sending.
     * Receive functions decode the packet into the parameters references passed.
     *
     * Sending/broadcasting can be done anywhere in the code as seen fit,
     * as long as the broadcast host/destination peer is given.
     * Each end should place the desired receive functions into a switch statement.
     */

    // Sending functions for packet creation
    void send_login(std::string &username, std::string &passkey, ENetPeer *dest){
        packet_create(1)
        encode( PACKET_LOGIN, packet, offset);
        encode_string(username, packet, offset);
        encode_string(passkey, packet, offset);
        packet_send
    }

    void receive_login(std::string &username, std::string &passkey, ENetPacket *packet){
        unsigned int offset = 1; // First byte is packet type
        decode_string(username, packet, offset);
        decode_string(passkey, packet, offset);
    }

    void send_kick(std::string &reason, ENetPeer *dest){
        packet_create(1)
        encode( PACKET_KICK, packet, offset);
        encode_string(reason, packet, offset);
        packet_send
        // When kicking, force a disconnect with the destination
        enet_peer_disconnect_later(dest,0);
    }

    void receive_kick(std::string &reason, ENetPacket *packet){
        unsigned int offset = 1; // First byte is packet type
        decode_string(reason, packet, offset);
        // The host has disconnected
    }

    void send_player_input(Player *p, ENetPeer *dest){
        packet_create(1)
        encode( PACKET_PLAYER_INPUT, packet, offset);
        encode(p->input_flag, packet, offset);
        encode_array(p->look_rot, 4, packet, offset);
        packet_send
    }

    void receive_player_input(Player *p,  ENetPacket *packet){
        unsigned int offset = 1;
        decode(p->input_flag, packet, offset);
        decode_array(p->look_rot, 4, packet, offset);
    }



    void broadcast_player_status_synch( PlayerSet *player_set, ENetHost *host){
        packet_create(1)
        encode( PACKET_PLAYER_STATUS_SYNCH, packet, offset); // Packet type
        encode( player_set->count(), packet, offset);   // Specify the player count
        // For each player, place the required status data
        Player* p;
        for(uint8_t i = 0; i < player_set->count(); ++i){
            p = &player_set->at(i);
            encode_string(p->username, packet, offset);
        }
        packet_broadcast
    }

    void receive_player_status_synch(PlayerSet *player_set,  ENetPacket *packet){
        unsigned int offset = 1;
        uint8_t player_count;
        decode(player_count, packet, offset);
        player_set->reserve(player_count);
        for(uint8_t i = 0; i < player_set->count(); ++i){
            decode_string(player_set->at(i).username, packet, offset);
        }
    }

    void broadcast_player_synch( PlayerSet *player_set, ENetHost *host){
        packet_create(1);
        encode( PACKET_PLAYER_SYNCH, packet, offset);   // Packet type
        encode( player_set->count(), packet, offset); // Specify the player count
        // For each player, place the required data
        Player* p;
        for(uint8_t i = 0; i < player_set->count(); ++i){
            p = &player_set->at(i);
            encode(p->move_mode, packet, offset);
            encode(p->input_flag, packet, offset);
            encode_array(p->collision_shape.pos, 3, packet, offset);
            encode_array(p->collision_shape.rot, 4, packet, offset);
            encode_array(p->look_rot, 4, packet, offset);
            encode_array(p->velocity, 3, packet, offset);
        }
        packet_broadcast
    }

    void receive_player_synch(PlayerSet *player_set,  ENetPacket *packet){
        unsigned int offset = 1;
        uint8_t player_count;
        decode( player_count, packet, offset); // Specify the player count
        player_set->reserve(player_count);

        // For each player, place the required data
        Player *p;
        uint8_t active_slot = player_set->get_active_slot();
        for(uint8_t i = 0; i < player_set->count(); ++i){
            p = &player_set->at(i);

            // Active Player (write fewer predicted or known states)
            if(i == active_slot){
                offset += sizeof(p->move_mode);    // Skip move mode
                offset += sizeof(p->input_flag);    // Skip input flag
                decode_array(p->collision_shape.pos, 3, packet, offset);
                decode_array(p->collision_shape.rot, 4, packet, offset);
                offset += sizeof(p->look_rot);    // Skip look rotation
                decode_array(p->velocity, 3, packet, offset);
            }
            // Other Players
            else{
                decode(p->move_mode, packet, offset);
                decode(p->input_flag, packet, offset);
                decode_array(p->collision_shape.pos, 3, packet, offset);
                decode_array(p->collision_shape.rot, 4, packet, offset);
                decode_array(p->look_rot, 4, packet, offset);
                decode_array(p->velocity, 3, packet, offset);

            }


        }
    }


}

