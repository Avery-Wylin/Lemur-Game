#include "Server.h"

#include <enet/enet.h>
#include <chrono>
#include <thread>
#include "Scene.h"
#include "ServerConfig.h"
#include "Packet.h"

// Function for pthread to use when starting a server
void *server_run_func( void *arg ) {
    Server *server = ( Server * )arg;
    server->run();
    return nullptr;
}

// Forks the server by creating its own thread
void Server::start(uint16_t port, std::string save_name){
    this->save_name = save_name;
    puts("Server: Started.");
    fflush(stdout);
    if(is_running){
        puts("Server already running");
        fflush(stdout);
        return;
    }
    is_running = true;
    connection.set_port(port);
    connection.set_owner(this);
    pthread_create( &running_thread, nullptr, server_run_func, this);
}

// The main run function for the server
void Server::run() {
    // Start hosting without init
    // This is fine because players will not receive feedback until the next poll in the main loop
    connection.start_host();
    if(!is_running){
        puts("Server could not create a host.");
        fflush(stdout);
        return;
    }

    // Initialize the Scene
    scene.init_server(this);

    puts("Server: Initialized.");
    fflush(stdout);

    // Declare variables used in runtime
    std::chrono::time_point<std::chrono::steady_clock> start, stop;
    std::chrono::duration<double> elapsed;           // The time elapsed for a single run loop
    double update_time = 1.0 / ( STEPS_PER_SECOND ), // The time used per update
           deferred_time = 0;                        // The amount of time to be used
    uint8_t updates = 0,                             // The number of updates to run
            update_cap = 10;

    while( is_running ) {
        start = std::chrono::steady_clock::now();


        if(updates > 0){
            // Poll client signals
            connection.poll_packets();

            // Update the scene using the step count
            for( int i = 0; i < updates; ++i ) {
                scene.update( );
            }

            // Send synchronize packets
            Packet::broadcast_player_synch(&scene.player_set, connection.host_server);
        }


        // TEST artificial latency
        // std::this_thread::sleep_for((std::chrono::duration<double> ).1);

        // Read the start/stop times and get the elapsed
        stop = std::chrono::steady_clock::now();
        elapsed = stop - start;

        // Sleep any extra time
        std::this_thread::sleep_until( stop + ( std::chrono::duration<double> )update_time - elapsed );

        // Recalculate the slept time
        stop = std::chrono::steady_clock::now();
        elapsed = stop - start;

        // Append the deferred time and clear the number of updates
        deferred_time += elapsed.count();

        // If there is enough time to fill to cap
        if( deferred_time >= update_cap * update_time ) {
            deferred_time -= update_cap * update_time;
            updates = update_cap;
        }

        // If there is some time, but not enough to fill to cap
        else if( deferred_time >= update_time ) {
            updates = floor( deferred_time / update_time );
            deferred_time -= updates * update_time;
        }

        // If the deffered time is not enough for an update
        else {
            updates = 0;
        }
    }
    // Kick all players and flush
    scene.player_set.kick_all();
    connection.poll_packets();

    // Clear the player set
    scene.player_set.clear();

    // Close the scene (saves it)
    scene.close_server();
}

void Server::stop(){
    if(!is_running)
        return;
    puts("Server: Stopped.");
    fflush(stdout);
    is_running = false;
    pthread_join(running_thread, nullptr);
}
