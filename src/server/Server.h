#ifndef SERVER_H
#define SERVER_H

#include "definitions.h"
#include <pthread.h>
#include <inttypes.h>
#include "ServerConnection.h"
#include <string>
#include "Scene.h"

class Server {
    bool is_running = false;
public:
    ServerConnection connection;
    std::string save_name;
    pthread_t running_thread;
    Scene scene;
    void start( uint16_t port, std::string save_name );
    void run();
    inline bool running() {return is_running;};
    void stop();
};

#endif // SERVER_H
