#include <cstdio>
#include <pthread.h>
#include <enet/enet.h>
#include <bit>

#include "Audio.h"
#include "Client.h"
#include "Server.h"

void client_run() {

    // Prevent destructors that remove gl elements from being called after the gl context is destroyed
    Audio::init();
    GLFWwindow *window;
    Client *c = new Client();
    c->init();
    window = c->window;
    c->run();
    delete c;

    // gl elements called their destructors, the context can be destroyed now that the window is deleted
    glfwDestroyWindow( window );
    glfwTerminate();
}

int main( int, char ** ) {

    // Enforce that the processer is little endian. Otherwise assets will load incorrectly and inter-machine communications will fail.
    if( std::endian::native != std::endian::little ) {
        puts("ERROR: System must be little endian.");
        exit(EXIT_FAILURE);
    }

    // Initialize Enet
    if( enet_initialize() != 0 ) {
        printf( "Enet initialization error." );
        return 1;
    }
    atexit( enet_deinitialize );


    // Create the server
    // pthread_t server_thread;
    // Server *server = new Server();
    // Server::create(&server_thread, server);

    // Run the client
    client_run();

    // Initialize OpenAL


    // pthread_join( server_thread, nullptr );

    Audio::close();


}
