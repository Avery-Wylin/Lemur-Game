#include "ClientMenus.h"
#include "Client.h"

namespace Menu {
    MenuBase *active = nullptr;
    void activate( MenuBase *m ) {
        if( m ) {
            active = m;
            GUI::select_set( &( m->elements ) );
            m->open();
            active_client->enableCursor();
        }
    };

    void deactivate() {
        if(!active)
            return;
        active->close();
        GUI::deselect_set();
        active = nullptr;
        active_client->disableCursor();
    };

    void update() {
        if( active ) {
            active->update();
        }
    }
}


// Multiplayer Menu

void MenuMultiplayer::init() {
    elem_username.set_text( "name" );
    elem_username.set_size( .4, .075 );
    elements.add( &elem_username );

    elem_passkey.set_text( "key" );
    elem_passkey.set_size( .4, .075 );
    elements.add( &elem_passkey );

    elem_ip.set_text( "127.0.0.1" );
    elem_ip.set_size( .4, .075 );
    elements.add( &elem_ip );

    elem_port.set_integer( true );
    elem_port.set_value( CONNECTION_DEFAULT_PORT );
    elem_port.set_size( .2, .075 );
    elements.add( &elem_port );

    elements.compact_center(0, .5, .801, .8, .01, .01 );

    elem_connect.set_text( "Connect" );
    elem_connect.set_size( .2, .075 );
    elements.add( &elem_connect );

    elem_msg.set_size( 1.2, .075 );
    elements.add( &elem_msg );

    elements.compact_center( elements.size()-2, .5, .9, .7, .01, .01 );

    elem_return.set_text("Return");
    elem_return.set_size(.2,.075);
    elements.add(&elem_return);

    elements.compact_center( elements.size()-1, .5, .9, .6, .01, .01 );
}

void MenuMultiplayer::update() {
    ClientConnection *connection = &active_client->connection;

    // Attempt to make a connection when clicked
    if( elem_connect.changed() ) {
        // Set the username and passkey to the respective field
        active_client->connection.username = elem_username.get_text();
        active_client->connection.passkey = elem_passkey.get_text();

        // Attempt to connect to the given ip
        connection->connect( elem_ip.get_text(), elem_port.get_int() );

        // Switch to a message screen while the connection is pending
        // NOTE currently inescapable while waiting for connection
        Menu::activate(&active_client->menu_message);
    }

    if(elem_return.changed()){
        connection->disconnect(true);// NOTE not needed, cant escape while pending anyways
        // Switch to the home menu if returning
        Menu::activate(&active_client->menu_home);
    }

};

void MenuMultiplayer::open() {
    // Reset the status message
    elem_msg.set_text( "Input name, key, server IP, and port number." );
}


// Home Menu

void MenuHome::init() {
    elem_title.set_text( "Lemur Island" );
    elem_title.set_size( 1, .2 );
    elements.add( &elem_title );

    elem_single.set_text( "Host Game" );
    elem_single.set_size( .7, .1 );
    elements.add( &elem_single );

    elem_multi.set_text( "Join Game" );
    elem_multi.set_size( .7, .1 );
    elements.add( &elem_multi );

    elem_settings.set_text( "Settings" );
    elem_settings.set_size( .7, .1 );
    elements.add( &elem_settings );

    elements.compact_center( 0, .5, .1, .9, .01, .01 );
}

void MenuHome::update() {
    // More than one element change events are read, it is more efficient to check if the element set had input first
    if(!elements.had_input())
        return;

    if( elem_single.changed() ) {
        Menu::activate( &active_client->menu_single );
        return;
    }

    if( elem_multi.changed() ) {
        Menu::activate( &active_client->menu_multi );
        return;
    }

    if( elem_settings.changed() ) {
        Menu::activate( &active_client->menu_settings );
        return;
    }
}

// Singleplayer Menu

void MenuSingleplayer::init() {

    // TODO load file info frome saves
    elem_save.resize_options(3);
    for(unsigned int i = 0; i < elem_save.option_count(); ++i){
        elem_save.set_option(i, "Island " + std::to_string(i+1) + "\nPlayers: 0\nPlant Species: 0\nSeason: Spring\n\n\n");
    }
    elem_save.flags |= Element::LEFT_ALIGN;
    elem_save.set_size(.8,.4);
    elements.add(&elem_save);

    elements.compact_center(0, .5, 0, .9, .025, .025 );

    elem_play.set_text( "Play" );
    elem_play.set_size( .4, .075 );
    elements.add( &elem_play );

    elem_port.set_size(.15,.075);
    elem_port.set_integer(true);
    elem_port.set_value(CONNECTION_DEFAULT_PORT);
    elements.add(&elem_port);

    elem_erase.set_text( "Erase" );
    elem_erase.set_size( .2, .075 );
    elements.add( &elem_erase );

    elem_return.set_text( "Return" );
    elem_return.set_size( .2, .075 );
    elements.add( &elem_return );

    elements.compact_center(elements.size()-4, .5, .6, .25, .025, .025 );

}

void MenuSingleplayer::update(){

    // Stop changes while pending
    if(active_client->connection.get_status() == ClientConnection::PENDING)
        return;

    // Stop if no input
    if(!elements.had_input())
        return;

    // Return to home menu
    if( elem_return.changed() ) {
        Menu::activate( &active_client->menu_home );
        return;
    }

    // Create local server and start pending a connection to it
    if( elem_play.changed() ) {

        // Start the local server
        // TODO The server should know which file to open when runnning and deserialize the data for that save
        active_client->local_server.start(elem_port.get_int(), "save-1");

        // Connect to the local server
        active_client->connection.connect("127.0.0.1", elem_port.get_int());

        // TODO The window should switch to Scene mode when the scene gets an initialize signal from the server
        // Menu::window->run_scene = true;
        // Menu::deactivate();
    }


}

void MenuSingleplayer::open(){
        //TODO read the island saves and update the option list
}

// Settings Menu

void MenuSettings::init() {
    elem_global_volume.set_label( "Global Volume" );
    elem_global_volume.set_range(0,2);
    elem_global_volume.set_value(1);
    elem_global_volume.set_size( 1, .075 );
    elements.add( &elem_global_volume );

    elem_theme.set_options({"Theme Leaf","Theme Sand", "Theme Sunset", "Theme Day", "Theme Night"});
    elem_theme.set_size(.2, .075);
    elements.add(&elem_theme);

    elements.compact_center(0, .5, .9, .99, .01, .01);

    Keybind *k;
    for(unsigned int i = 0; i < KEY_COUNT; ++i){
        k = &active_client->config.keybinds[i];
        elem_keybinds[i].set_label( k->name);
        elem_keybinds[i].set_key(k->key, k->mods);
        elem_keybinds[i].set_size( .2, .075 );
        elements.add( &elem_keybinds[i] );
    }

    elements.compact_center(2, .5, .9, .805, .01, .01);

    elem_save.set_text("Save");
    elem_save.set_size(.2,.1);
    elements.add(&elem_save);

    elem_return.set_text("Return");
    elem_return.set_size(.2,.1);
    elements.add(&elem_return );

    elements.compact_center(elements.size()-2, .5, .6, .2, .025, .025);

}

void MenuSettings::update() {
    if(!elements.had_input())
        return;

    if(elem_global_volume.changed())
        Audio::set_master_volume( elem_global_volume.get_value());

    // Iterate between KEY_NONE and KEY_COUNT
    for(unsigned int i = 0; i < KEY_COUNT; ++i){
        if(elem_keybinds[i].changed()){
            active_client->config.keybinds[i].set(elem_keybinds[i].get_key(), elem_keybinds[i].get_mods());
        }
    }

    if(elem_return.changed()){
        Menu::activate(&active_client->menu_home);
        return;
    }

    if(elem_theme.changed()){
        switch(elem_theme.selected()){
            case 0:
                GUI::colors.theme_leaf();
                break;
            case 1:
                GUI::colors.theme_sand();
                break;
            case 2:
                GUI::colors.theme_sunset();
                break;
            case 3:
                GUI::colors.theme_day();
                break;
            case 4:
                GUI::colors.theme_night();
                break;
        }
    }

}

void MenuSettings::open() {
    // Update keybind elements in case of change
    Keybind *k;
    for(unsigned int i = 0; i < KEY_COUNT; ++i){
        k = &active_client->config.keybinds[i];
        elem_keybinds[i].set_key(k->key, k->mods);
    }
}

// Message Screen Menu

void MenuMessageScreen::init() {
    elem_msg.set_size(1,.075);
    elements.add(&elem_msg);

    elem_return.set_text("Return");
    elem_return.set_size(.3,.075);
    elements.add(&elem_return);

    elements.compact_center(0, .5, .1, .6, 0, .1);

    return_menu = &active_client->menu_home;
}


void MenuMessageScreen::set_message( std::string msg, MenuBase *return_menu) {
    elem_msg.set_text(msg);
    if(return_menu == nullptr){
        elem_return.hide();
    }
    else{
        elem_return.show();
        this->return_menu = return_menu;
    }

}

void MenuMessageScreen::update() {
    if(!elements.had_input())
        return;

    if(elem_return.changed())
        Menu::activate(return_menu);
}



