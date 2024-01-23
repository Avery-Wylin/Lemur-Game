#ifndef CLIENTMENUS_H
#define CLIENTMENUS_H

// #include "ClientWindow.h"
#include "definitions.h"
#include "GUI.h"
#include "ClientConfig.h"

class Client;

/**
 * Implementation of default Element Sets for managing certain GUI screens.
 * Menus all contain an ElementSet that can be bound and unbound.
 * Furthermore, all Menus receive an update pass each frame.
 */

struct MenuBase {
    ElementSet elements;

    MenuBase() {
    }

    virtual ~MenuBase() {
    }

    virtual void open(){

    }

    virtual void close(){

    }

    virtual void update() {
    }
};

namespace Menu {
    extern MenuBase *active;
    void activate( MenuBase *m );
    void deactivate();
    void update();
}

struct MenuMultiplayer : public MenuBase {
    ElementTextInput
    elem_ip,
    elem_username,
    elem_passkey;
    ElementNumInput elem_port;
    ElementButton
    elem_connect,
    elem_return;
    ElementText elem_msg;

    ~MenuMultiplayer(){
        elements.remove_all();
    }

    void init();
    void update() override;
    void open() override;
};

struct MenuHome : public MenuBase {
    ElementText elem_title;
    ElementButton
    elem_single,
    elem_multi,
    elem_settings;

    ~MenuHome(){
        elements.remove_all();
    }

    void init();
    void update();
};

// Choose a save slot and play an island, port is currently required for hosting reasons
struct MenuSingleplayer : public MenuBase {
    ElementButton
    elem_return,                          // Cancel and go back to home
    elem_play,                            // Play selected slot, creates a new world on empty slot files, loads a world if the slot has a save file
    elem_erase;                           // Erase selected slot
    ElementOption elem_save;              // Option to choose available saves
    ElementNumInput elem_port;

    ~MenuSingleplayer(){
        elements.remove_all();
    }

    void init();
    void update() override;
    void open() override;
};

// Configure settings such as keybinds and volume
struct MenuSettings : public MenuBase {
    ElementBar elem_global_volume;
    ElementKeyCapture elem_keybinds[KEY_COUNT];
    ElementButton elem_save;
    ElementButton elem_return;
    ElementOption elem_theme;

    ~MenuSettings(){
        elements.remove_all();
    }

    void init();
    void update() override;
    void open() override;
};

struct MenuInGame : public MenuBase {
    ElementBar elem_global_volume;
    ElementKeyCapture elem_keybinds[KEY_COUNT];
    ElementButton elem_save;
    ElementButton elem_return;

    ~MenuInGame(){
        elements.remove_all();
    }

    void init();
    void update() override;
};

struct MenuMessageScreen : public MenuBase{
    ElementText elem_msg;
    ElementButton elem_return;
    MenuBase *return_menu;

    ~MenuMessageScreen(){
        elements.remove_all();
    }

    void init();
    void set_message(std::string msg, MenuBase *return_menu = nullptr);
    void update() override;
};


#endif // CLIENTMENUS_H
