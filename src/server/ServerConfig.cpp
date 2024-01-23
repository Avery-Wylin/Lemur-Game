#include "ServerConfig.h"
#include "toml.hpp"
#include "definitions.h"
#include "Server.h"

void ServerConfig::load_players(Server *server){
    // Get the server save path and load the toml file
    toml::table table;

    try
    {
        table = toml::parse_file((std::string)DIR_SAVES + server->save_name);
    }
    catch (const toml::parse_error& err)
    {
        return;
    }
}

void ServerConfig::save_players(Server *server){

}


// Call all specified load/save functions

void ServerConfig::load_configs(Server *server){
    load_players(server);
}

void ServerConfig::save_configs(Server *server){
    save_players(server);
}

