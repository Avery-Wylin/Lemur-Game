#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>

class Server;
namespace ServerConfig{
    void load_players(Server *server);
    void save_players(Server *server);

    void load_configs(Server *server);
    void save_configs(Server *server);
}

#endif //SERVERCONFIG_H
