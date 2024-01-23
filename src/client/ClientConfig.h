#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <unordered_map>
#include <inttypes.h>
#include <sys/types.h>
#include <string>
#include <GLFW/glfw3.h>

enum KeyActions{
    KEY_FORWARD,
    KEY_BACKWARD,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LEAP,
    KEY_TEXT,
    KEY_COUNT
};

struct Keybind{
    std::string name;
    int32_t key = GLFW_KEY_UNKNOWN;
    uint8_t mods = 0;
    inline void set(int32_t k, uint8_t m, std::string n){
        key = k;
        mods = m;
        name = n;
    }

    inline void set(int32_t k, uint8_t m){
        key = k;
        mods = m;
    }
};

struct ClientConfig{
    Keybind keybinds[KEY_COUNT];
    ClientConfig(){};

    void default_keybinds();
    void load_settings();
    void save_settings();
};

#endif // CLIENTCONFIG_H
