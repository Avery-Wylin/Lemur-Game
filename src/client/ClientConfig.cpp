#include "ClientConfig.h"
#include <fstream>

void ClientConfig::default_keybinds() {
    keybinds[KeyActions::KEY_FORWARD].set( GLFW_KEY_W, 0, "Forward" );
    keybinds[KeyActions::KEY_BACKWARD].set( GLFW_KEY_S, 0, "Backward");
    keybinds[KeyActions::KEY_LEFT].set( GLFW_KEY_A, 0, "Left" );
    keybinds[KeyActions::KEY_RIGHT].set( GLFW_KEY_D, 0, "Right" );
    keybinds[KeyActions::KEY_LEAP].set( GLFW_KEY_SPACE, 0, "Leap");
    keybinds[KeyActions::KEY_TEXT].set( GLFW_KEY_T, 0, "Text" );
}


void ClientConfig::load_settings() {
    // Read the config file
    std::ifstream filereader;
    filereader.open( "../config/keybinds.cfg", std::ios::in);

    //TODO load keybinds from file

    // Load  default keybinds and return if file is not found
    if( !filereader.is_open() ) {
        default_keybinds();
        return;
    }

    filereader.close();
}

void ClientConfig::save_settings() {
    std::ofstream filewriter("../config/keybinds.cfg", std::ios::out);

    //TODO write keybinds to file
    filewriter.close();
}
