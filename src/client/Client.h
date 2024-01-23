#ifndef CLIENT_H
#define CLIENT_H

#include "glad_common.h"

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include <cmath>

#include "FBO.h"
#include "Scene.h"
#include "Texture.h"
#include "VAO.h"
#include "View.h"
#include "Armature.h"
#include "DebugDraw.h"
#include "ClientConfig.h"
#include "ClientConnection.h"
#include "ClientMenus.h"
#include "Server.h"
#include "Player.h"

class Client;
extern Client* active_client;

class Client {
    public:

        GLFWwindow *window = nullptr;
        GLFWmonitor *monitor;
        const GLFWvidmode *vidmode;

        int
        window_width = 0,
        window_height = 0;
        float guiscale;
        float fbo_ratio = 1;

        FBO scenefbo;
        Texture scenetex;

        // Background
        Texture background;
        Shader background_shader;
        VAO quad;
        float background_time = 0;

        Scene scene;

        // Connection and local server
        ClientConnection connection;
        Server local_server;

        // Menus
        MenuHome menu_home;
        MenuSingleplayer menu_single;
        MenuMultiplayer menu_multi;
        MenuSettings menu_settings;
        MenuMessageScreen menu_message;

        // Configuration
        ClientConfig config;

    public:
        Client();
        virtual ~Client();
        void init();
        void run();

        // GLFW Callback functions
        static void keyCallback( GLFWwindow *window, int key, int scancode, int action, int mods );
        static void resizeCallback( GLFWwindow *window, int width, int height );
        static void charCallback( GLFWwindow *window, unsigned int codepoint );
        static void cursorMoveCallback( GLFWwindow *window, double xpos, double ypos );
        static void clickCallback( GLFWwindow *window, int key, int action, int mods );
        void enableCursor();
        void disableCursor();

    private:

};

#endif // CLIENT_H
