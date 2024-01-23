#include "Player.h"
#include "Packet.h"
#include "Mesh.h"

VAO player_vao;
Mesh player_mesh;
ArmatureInfo armature_info;

void Player::init_assets(){
    player_mesh.append_PLY("Mongoz");
    player_mesh.to_VAO(&player_vao);
    // player_vao.loadPLY("Mongoz");
}

void Player::close_assets(){
    player_vao.free();
}


Player::Player() {
    collision_shape.pos[0] = TERRAIN_DIM*TERRAIN_SCALE*.5;
    collision_shape.pos[1] = 0;
    collision_shape.pos[2] = TERRAIN_DIM*TERRAIN_SCALE*.5;
    collision_shape.radius = .5f;
    collision_shape.y_extension = .5;
}

void Player::clear(){
    *this = Player();
    collision_shape.pos[0] = TERRAIN_DIM*TERRAIN_SCALE*.5;
    collision_shape.pos[1] = 0;
    collision_shape.pos[2] = TERRAIN_DIM*TERRAIN_SCALE*.5;
}

uint8_t PlayerSet::get_slot( std::string username ) {
    for( uint8_t i = 0; i < MAX_PLAYERS; ++i ) {
        if( players[i].username == username )
            return i;
    }

    return MAX_PLAYERS;
}

uint8_t PlayerSet::get_save( std::string username ) {
    for( uint8_t i = 0; i < player_saves.size(); ++i ) {
        if( player_saves[i].username == username )
            return i;
    }
    return MAX_PLAYER_SAVES;
}

uint8_t PlayerSet::create_player_save( std::string username, std::string passkey) {
    if( player_saves.size() >= MAX_PLAYER_SAVES )
        return MAX_PLAYER_SAVES;

    uint8_t save_id = get_save(username);

    // Player already has a save, return
    if(save_id != MAX_PLAYER_SAVES)
        return MAX_PLAYER_SAVES;

    Player p;
    p.username = username;
    p.passkey = passkey;
    player_saves.push_back( p );
    return player_saves.size() - 1;
}

Player *PlayerSet::get_active(){
    if( active_player_slot != MAX_PLAYERS )
        return &players[active_player_slot];
    return nullptr;
}

Armature* PlayerSet::get_active_armature(){
    if( active_player_slot != MAX_PLAYERS )
        return &armatures[active_player_slot];
    return nullptr;
}

uint8_t PlayerSet::set_active(std::string username){
    active_player_slot = get_slot(username);
    return active_player_slot;
}

uint8_t PlayerSet::login( std::string username, std::string passkey, ServerConnection *connection, ENetPeer *peer) {
    std::string kick_reason;
    // Server is full
    if( player_count >= MAX_PLAYERS ){
        kick_reason = "Server is full.";
        Packet::send_kick(kick_reason, peer);
        return MAX_PLAYERS;
    }

    // Username is already playing
    if( get_slot( username ) != MAX_PLAYERS ){
        kick_reason = "Username is taken.";
        Packet::send_kick(kick_reason, peer);
        return MAX_PLAYERS;
    }

    uint8_t save_id = get_save(username);

    // Username has a save
    if(save_id != MAX_PLAYER_SAVES){
        // Passkey matches
        if(player_saves[save_id].passkey == passkey){
            players[player_count] = player_saves[save_id];
            peer->data = &players[player_count];
            peers[player_count] = peer;
            ++player_count;
            Packet::broadcast_player_status_synch(this, connection->host_server);
        }
        // Passkey does not match
        else {
            kick_reason = "Invalid passkey.";
            Packet::send_kick( kick_reason, peer );
            return MAX_PLAYERS;
        }
    }
    // Username does not have a save
    else{
        save_id = create_player_save(username, passkey);

        // Save made
        if(save_id != MAX_PLAYER_SAVES){
            players[player_count] = player_saves[save_id];
            peers[player_count] = peer;
            peer->data = &players[player_count];
            ++player_count;
            Packet::broadcast_player_status_synch(this, connection->host_server);
            return save_id;
        }
        // Save not made
        else{
            kick_reason = "Could not make a new save, server saves are full.";
            Packet::send_kick( kick_reason, peer );
            return MAX_PLAYERS;
        }
    }
}

void PlayerSet::logout(ServerConnection *connection, ENetPeer *peer){
    // If only one player, remove
    if(player_count == 1){
        if(peer->data == players){
            printf("%s %s %s\n","Server:", players[0].username.c_str(), "logged out.");
            fflush(stdout);
            player_count = 0;
            player_saves[players[0].save_id] = players[0];
            players[0].clear();
            return;
        }
        else{
            puts("Server: Last player is not host.");
            fflush(stdout);
            return;
        }
    }
    // Find the matching player
    for(uint8_t i = 0; i < player_count; ++i){
        if(players+i == peer->data){
            printf("%s %s %s\n","Server:", players[i].username.c_str(), "logged out.");
            fflush(stdout);

            // Copy the player to its save slot
            player_saves[players[i].save_id] = players[i];

            // Shift players to replace the removed player
            enet_peer_reset(peers[i]);
            for(uint8_t j = i; j < player_count - 1; ++j){
                players[j] = players[j+1];
                peers[j]->data = peers[j+1]->data;
            }
            // Decrement the player count and clear the unsused player
            --player_count;
            players[player_count].clear();
            peers[player_count] = nullptr;

            // Synchronize to all peers, they will find their username and update their slot
            Packet::broadcast_player_status_synch(this, connection->host_server);
            return;
        }
    }
    // Failed to log out
     puts("Server: No player data found for peer on logout.");
     fflush(stdout);
    return;
}

void PlayerSet::clear() {
    for(uint8_t i = 0; i < MAX_PLAYERS; ++i){
        players[i].clear();
        peers[i] = nullptr;
    }
    player_count = 0;
}

void PlayerSet::kick_all(){
    std::string reason = "Server Closed";
    for(uint8_t i = 0; i < player_count; ++i){
        Packet::send_kick(reason, peers[i]);
    }
}

void Player::update_logic() {
    // TODO player logic implementation goes here
}

void PlayerSet::update_logic() {
    // TODO update each player's logic
}

// TODO this function is rubbish and needs overhauled
void Player::update_motion(){

    // Get motion inputs based on flag values
    vec3 motion = GLM_VEC3_ZERO_INIT;
    motion[0] += ( ( input_flag & RIGHT ) > 0 );
    motion[0] -= ( ( input_flag & LEFT ) > 0 );
    motion[2] += ( ( input_flag & BACKWARD ) > 0 );
    motion[2] -= ( ( input_flag & FORWARD ) > 0 );

    // Rotate the motion input by the look vector, makes control view-based
    glm_quat_rotatev( look_rot, motion, motion );

    // Remove any motion on the y component to force z-only rotation and normalize the speed
    motion[1] = 0;
    glm_vec3_normalize( motion );

    // If moving, align the look rotation to the motion and slowly return the players rotation to the look direction
    if( ( input_flag & ( RIGHT | LEFT | BACKWARD | FORWARD ) ) > 0 ) {
        vec3 up = {0, 1, 0}; // Pslayer's up direction
        versor mrot;
        glm_quat_for( motion, up, mrot );  // Convert motion into a quat
        glm_quat_nlerp( collision_shape.rot, mrot, .2, collision_shape.rot );   // Interpolate current rotation and motion
        // Update collision shape rotation
        glm_quat_inv( collision_shape.rot, collision_shape.inv_rot );

    }



    switch(move_mode){
        case WALK:{

            // Leaping is faster
            if(input_flag & LEAP){
                glm_vec3_scale( motion, .2, motion );
                velocity[1] = 1.5;
            }
            else{
                // Ground speed
                glm_vec3_scale( motion, speed, motion );
            }

            // Ground friction
            glm_vec3_scale( velocity, .5, velocity );
            break;
        }

        case IN_AIR:{

            // Gravity
            velocity[1] -= 0.1;

            // Air motion control
            glm_vec3_scale( motion, .1, motion );

            // Air friction
            glm_vec3_scale( velocity, .9, velocity );
            break;
        }

        case SWIM:{
            // Water Speed
            glm_vec3_scale( motion, .1, motion );

            // Water Friction
            glm_vec3_scale( velocity, .4, velocity );
            break;
        }

        case CLIMB:{
            if(climbing_plant == PLANT_NULL){
                move_mode = IN_AIR;
                break;
            }


        }
    }

    // Add motion to velocity
    glm_vec3_add( velocity, motion, velocity );
    glm_vec3_add( collision_shape.pos, velocity, collision_shape.pos );
}

void PlayerSet::update_motion(){
    // Apply velocities
    for( uint8_t i = 0; i < player_count; ++i ) {
        players[i].update_motion();
    }
}

void PlayerSet::update_collision(){
    //TODO replace with proper collision
    for( unsigned int i = 0; i < player_count; ++i ) {
        vec3 resolve = GLM_VEC3_ZERO_INIT;
        vec3 down = {0,-1,0};
        for( unsigned int j = 0; j < player_count; ++j ) {
            if( i == j )
                continue;
            if( CollisionShape::gjk( players[i].collision_shape, players[j].collision_shape, resolve ) ) {
                glm_vec3_scale(resolve,.5,resolve);
                glm_vec3_sub(players[i].collision_shape.pos, resolve, players[i].collision_shape.pos );
                glm_vec3_add(players[j].collision_shape.pos, resolve, players[j].collision_shape.pos );
                glm_vec3_normalize(resolve);
                if(glm_vec3_dot(resolve,down) > .8)
                    players[i].move_mode = Player::WALK;
            }
        }
    }
}

void PlayerSet::update_terrain_collision(Terrain *terrain){
    // TODO this is only point collision, fix to account for gjk?
    vec3 gpos;
     for(unsigned int i = 0; i < player_count; ++i){
        vec3 &pos = players[i].collision_shape.pos;
        glm_vec3_copy(pos, gpos);
        terrain->pointProjection(gpos);
        gpos[1]+=1;

        // Ease in if slightly above the surface
        if(pos[1] <= gpos[1] + .2){
            if(players[i].move_mode == Player::IN_AIR || players[i].move_mode == Player::SWIM)
                players[i].move_mode = Player::WALK;
            pos[1] = fmax(pos[1]-.1,gpos[1]);
        }
        else{
            players[i].move_mode = Player::IN_AIR;
        }
    }
}

void PlayerSet::apply_bouyant_force(float water_level){
    for(unsigned int i = 0; i < player_count; ++i){
        if(players[i].collision_shape.pos[1] < water_level){
            players[i].move_mode = Player::SWIM;
            players[i].velocity[1] = .95* players[i].velocity[1] + .2*(water_level - players[i].collision_shape.pos[1]);
        }
    }
}

void PlayerSet::update_armatures() {
    for(uint8_t i = 0; i < player_count; ++i){
        // Play animations

        static uint16_t moving_flags = Player::LEFT | Player::RIGHT | Player::FORWARD | Player::BACKWARD;

        if(players[i].input_flag & Player::LEAP){
            armatures[i].play_animation(armatures[i].get_animation("Leap"), AnimationPlayData::CLAMPED, 1.0f, true);
        }
        else if(players[i].move_mode != Player::IN_AIR){
            armatures[i].stop_animation(armatures[i].get_animation("Leap"));
        }

        if( players[i].move_mode == Player::WALK && players[i].input_flag & moving_flags  ){
            armatures[i].play_animation(armatures[i].get_animation("Walk"), AnimationPlayData::LOOP, 2.0f, true);
        }
        else{
            armatures[i].stop_animation(armatures[i].get_animation("Walk"));
        }

        // Set the root transform of the armature
        armatures[i].set_root_transform(players[i].collision_shape.pos, players[i].collision_shape.rot);

        // Update the transform buffer and constraints
        armatures[i].update();
    }
}

void PlayerSet::init_armatures(){
    armature_info.load("Mongoz");
    for(uint8_t i = 0; i < MAX_PLAYERS; ++i){
        armatures[i].assign(&armature_info);
        armatures[i].add_softbody("tail_base",0.8, .1, .4, .1, 10);
        armatures[i].add_softbody("tail_1", 0.8, .1, .4, 0.1, 10);
        armatures[i].add_softbody("tail_2", 0.8, .1, .4, 0.1, 10);
        armatures[i].add_softbody("tail_3", 0.8, .1, .4, 0.1, 10);
        armatures[i].add_softbody("tail_4", 0.8, .1, .4, 0.1, 10);
        armatures[i].add_softbody("tail_5", 0.8, .1, .4, 0.1, 10);
        armatures[i].add_softbody("tail_6", 0.8, .1, .4, 0.1, 10);
        armatures[i].add_softbody("tail_7", 0.8, .1, .4, 0.1, 10);
        armatures[i].play_animation(armatures[i].get_animation("Idle"), AnimationPlayData::LOOP, 1.0f);
    }
}

void PlayerSet::draw(float interp_fac){
    player_vao.bind();

    for(uint8_t i = 0; i < player_count; ++i){
        armatures[i].interpolate(interp_fac);
        Shader::uniformMat4f(UNIFORM_TRANSFORM, armatures[i].get_transform_buffer()[0]);
        Shader::uniformMat4fArray(UNIFORM_JOINTS, armatures[i].get_transform_buffer(), armatures[i].getJoints().size());

        // glm_quat_mat4( players[i].collision_shape.rot, players[i].transform);
        // glm_vec3_copy(players[i].collision_shape.pos, players[i].transform[3]);
        // Shader::uniformMat4f(UNIFORM_TRANSFORM, players[i].transform);

        glDrawElements( GL_TRIANGLES, player_vao.getIndexCount(), GL_UNSIGNED_INT, 0 );
    }
}
