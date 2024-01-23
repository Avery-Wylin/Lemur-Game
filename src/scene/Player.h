#ifndef PLAYER_H
#define PLAYER_H

#include "definitions.h"
#include <enet/enet.h>
#include <string>
#include "Armature.h"
#include "CollisionShape.h"
#include "DBVH.h"
#include "ServerConnection.h"
#include "Terrain.h"

class Player {

public:
    // Flags marking player controls
    static const uint16_t
    FORWARD = 0x01,
    BACKWARD = 0x02,
    RIGHT = 0x04,
    LEFT = 0x08,
    LEAP = 0x10,
    CROUCH = 0x20;

    static const uint8_t
    ACTIVE = 0x01;

    // The state of the player's motion, affects how the player is rendered and moves
    enum MotionMode : uint8_t {
        WALK,       // On the ground and can walk
        IN_AIR,     // In the air and can slightly move
        SWIM,       // In sufficiently deep water and floating
        CLIMB       // Climbing a surface
    };

    // The collision shape of the player, a capsule moves smoothly
    Capsule collision_shape;

    // Collision shapes do not have a transform matrix, they use a vec3 and quat
    mat4 transform = GLM_MAT4_IDENTITY_INIT;

    // The velocity of the player
    vec3 velocity = GLM_VEC3_ZERO_INIT;

    // The direction the camera is looking, the player will face this when moving
    versor look_rot = GLM_QUAT_IDENTITY_INIT;

    // The ground speed of the player
    float speed = 0.1;

    // The save ID of the player (index in player_saves)
    uint8_t save_id = 0;

    // Player input
    uint16_t input_flag = 0;

    // The current motion mode of the player
    MotionMode move_mode = IN_AIR;

    // The name and passkey, the server will not synchronize the passkey
    // Players without a username are considered erased
    std::string username = "", passkey = "";

    uint32_t climbing_plant = PLANT_NULL;

    Player();
    void update_logic();
    void update_motion();

    static void init_assets();
    static void close_assets();
    void clear();
};

class Server;
class Client;


/*
* A set of players
* Players are stored compactly in a normal array.
* When logging in, the server checks against a list of usernames and passkeys.
* If the username is not present, then the passkey is saved.
* If the username is present, then it must match the passkey.
* All player data is loaded on startup to a MAX_PLAYER_SAVES size.
* Player saves are then copied over to the player set upon login.
*/
class PlayerSet {

    DBVH player_dbvh;                      // A DBVH specifically for players
    Player players[MAX_PLAYERS];           // List of players
    vector<Player> player_saves;           // List of player saves (server only)
    ENetPeer *peers[MAX_PLAYERS];          // Peers corresponding to players (server only)
    Armature armatures[MAX_PLAYERS];       // Armatures corresponding to players (client only)
    uint8_t player_count = 0;                   // The number of current players
    uint8_t active_player_slot = MAX_PLAYERS;   // Client only, tells the client which player to focus on as well as if the game has started

    // Create a new player at the given slot (Server only)
    uint8_t create_player_save(std::string username, std::string passkey);

public:

    // Accessors
    // Get the slot of a current player by username, return MAX_PLAYERS on null
    uint8_t get_slot(std::string username);
    // Return the save position for a given username, returns MAX_PLAYER_SAVES on null
    uint8_t get_save(std::string username);
    // Return pointer to client's player, returns nullptr on null
    Player* get_active();
    Armature* get_active_armature();
    // Clientside, finds the index of the username, returns MAX_PLAYERS on null
    uint8_t set_active(std::string username);
    inline uint8_t count(){return player_count;}
    Player& at( uint8_t i ) {
        if( i >= player_count ) {
            puts( "ERROR: Player access out of bounds." );
            exit( 0 );
        }

        return players[i];
    }
    inline void reserve(uint8_t amount){
        if(amount < MAX_PLAYERS)
            player_count = amount;
        else
            player_count = MAX_PLAYERS;
    }

    inline uint8_t get_active_slot(){
        return active_player_slot;
    }


    // Login/out
    // Serverside, attempts to log a user in, returns the player slot,  returns MAX_PLAYERS on null
    uint8_t login(std::string username, std::string passkey, ServerConnection *connection, ENetPeer *peer);

    // Serverside, logs out a player and saves them to the save buffer.
    void logout(ServerConnection *connection, ENetPeer *peer);

    // Clears the player set, removing all players. This should be called on a client when logging out and before hosting.
    void clear();

    // Kick all players
    void kick_all();

        // Update Functions (in application order)

        // Applies any player logic, it is safe to query and modify the state
        void update_logic();

        // Applies player motion from control or carried over velocity
        void update_motion();

        // Calculates Player-Player and Player-Object collisions
        void update_collision();

        // Applies terrain collision correction, this simply clamps any colliding object back
        void update_terrain_collision( Terrain *terrain );

        // Applies an upwards force for players below given water level, safe to do after collision
        void apply_bouyant_force( float water_level );

        // Clientside, updates armatures
        void update_armatures();

    // Draw Functions
    void init_armatures();
    void draw(float interp_fac);                        // Clientside, draws all players interpolated, assumes correct shader is loaded with global uniforms

};



#endif // PLAYER_H
