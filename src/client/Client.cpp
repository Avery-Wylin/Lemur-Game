#include "Client.h"
#include <thread>
#include <cstring>

// Init the extern client
Client *active_client;

static void GLAPIENTRY glMessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam ) {
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
        type, severity, message );
 }

static void glfwErrorCallback( int error, const char *description ) {
    fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}


Client::Client() {
}

Client::~Client() {

}


void Client::init() {
    active_client = this;
    if( !glfwInit() )
        exit( 1 );

    glfwSetErrorCallback( glfwErrorCallback );

    // Initialize GLFW objects
    window = glfwCreateWindow( 1, 1, "Lemur Island", nullptr, nullptr );
    if( !window ) {
        fprintf( stderr, "Failed to create GLFW Window.\n" );
        exit( 1 );
    }
    glfwMakeContextCurrent( window );
    monitor = glfwGetPrimaryMonitor();
    vidmode = glfwGetVideoMode( monitor );
    glfwSetWindowSize( window, vidmode->width / 2, vidmode->height / 2 );
    glfwGetFramebufferSize( window, &window_width, &window_height );
    glfwSetWindowUserPointer( window, this );
    glfwSetKeyCallback( window, keyCallback );
    glfwSetWindowSizeCallback( window, resizeCallback );
    glfwSetCharCallback( window, charCallback );
    glfwSetCursorPosCallback( window, cursorMoveCallback );
    glfwSetMouseButtonCallback( window, clickCallback );

    // Enable OpenGL and configure it
    gladLoadGL();
    glDebugMessageCallback( glMessageCallback, 0 );
    glEnable( GL_DEBUG_OUTPUT );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glEnable(GL_FRAMEBUFFER_SRGB);
    glLineWidth(3);

    // Vsync
    glfwSwapInterval( 1 );

    // Init debug draw
    DebugDraw::init_assets();

    // Initialize the FBO that the scene renders to the display (this is can't be eadily changed later because GUI would need reformated)
    fbo_ratio = (float)vidmode->width/vidmode->height;
    fbo_ratio = fmin(fmax(1.5,fbo_ratio),2);
    // Create FBO with roughly half of the screen resolution (full res get expensive sometimes?)
    scenefbo.allocate( vidmode->height/2*fbo_ratio, vidmode->height/2 );

    // Create a depth attachment for depth test
    scenefbo.createDepthAttachment();

    // Set the texture to the fbo
    scenetex.from_FBO( scenefbo, 0, GL_NEAREST );
    scenefbo.bindDefault( window_width, window_height );

    // Background
    float pos[8] = {-1, -1, 1, -1, 1, 1, -1, 1};
    quad.loadAttributeFloat( Attribute::ATTRB_POS, 0, 0, 2, 8, pos );
    background.allocate();
    background.load_png("background");
    background_shader.load("background");

    // Client Connection
    connection.init(this);

    // Configuration
    config.load_settings();

    // GUI
    GUI::init_assets();
    GUI::ratio = fbo_ratio;

    // Menus
    menu_single.init();
    menu_multi.init();
    menu_settings.init();
    menu_home.init();
    menu_message.init();
    Menu::activate(&menu_home);

    // Scene
    scene.init_client(this);

}


void Client::run() {

    // Declare variables used in runtime
    std::chrono::time_point<std::chrono::steady_clock> start, stop;
    std::chrono::duration<double> elapsed;                // The time elapsed for a single run loop
    double update_time = 1.0 / ( STEPS_PER_SECOND ),      // The time used per update
           deferred_time = 0;                             // The amount of time to be used
    uint8_t updates = 0,                                  // The number of updates to run
            update_cap = 10;                              // The maximum amount of updates run per loop


    // Start client loop
    while( !glfwWindowShouldClose( window ) ) {
        start = std::chrono::steady_clock::now();

        // Update menus
        Menu::update();

        if( updates > 0 ) {
            glfwPollEvents();
            // Read server packets
            connection.update();
        }



        // If the client has switched to the playing status
        if( connection.get_status() == ClientConnection::PLAYING ) {

            // Skip this section if only rendering
            if( updates > 0 ) {

                Player *active_player = scene.player_set.get_active();

                // Update the scene using the step count (prediction)
                for( int i = 0; i < updates; ++i ) {
                    scene.update( );
                    scene.player_set.update_armatures();
                    if( active_player ) {
                        Packet::send_player_input(active_player, connection.peer_server);
                    }
                }

                // Send synch packets
                if( active_player ) {

                }

            }

            // Draw the scene
            scenefbo.bind();
            glClearColor( 0.5, 0.5, 0.5, 1 );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glEnable( GL_DEPTH_TEST );
            glEnable(GL_CULL_FACE);

            // scene.draw(1);
            scene.draw(fmin(deferred_time/update_time,1));

            // Draw debug
            DebugDraw::draw(scene.view);

            Shader::unbind();
        }

        // Switch to default FBO
        FBO::bindDefault( window_width, window_height );

        // Get information about the window size
        glfwGetFramebufferSize( window, &window_width, &window_height );

        // Bound by width
        if( window_width > window_height * fbo_ratio ) {
            glViewport( ( window_width - window_height * fbo_ratio ) / 2, 0, window_height * fbo_ratio, window_height );
        }
        // Bound by height
        else {
            glViewport( 0, ( window_height - window_width / fbo_ratio ) / 2, window_width, window_width / fbo_ratio );
        }

        // Draw the fbo over the background
        glClearColor( 0, 0, 0, 0 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );

        // Bind the rendered scene texture and draw a single quad, draw the scene if playing
        if( connection.get_status() == ClientConnection::PLAYING){
            scenetex.bind( 0 );
            glBegin( GL_QUADS );
            glColor3f( 1, 1, 1 );
            glTexCoord2f( 0, 0 );
            glVertex2f( -1, -1 );

            glColor3f( 1, 1, 1 );
            glTexCoord2f( 1, 0 );
            glVertex2f( 1, -1 );

            glColor3f( 1, 1, 1 );
            glTexCoord2f( 1, 1 );
            glVertex2f( 1, 1 );

            glColor3f( 1, 1, 1 );
            glTexCoord2f( 0, 1 );
            glVertex2f( -1, 1 );
            glEnd();
        }
        else{
            Texture::unbind();
            // background.bind(0);
            Shader::bind(background_shader);
            background_time = fmod(background_time + .001, 1);
            glUniform1f( 1, background_time );
            glUniform3fv( 2, 1, GUI::colors.decorator_inactive );
            glUniform3fv( 3, 1, GUI::colors.background_inactive );
            glUniform3fv( 4, 1, GUI::colors.decorator_active );
            quad.bind();
            glDrawArrays( GL_QUADS, 0, 4 );
        }


         // Draw the gui using the same bound scene texture (useful for transparency effects)
        glDisable(GL_DEPTH_TEST);
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        GUI::draw();
        // glDisable(GL_BLEND);

        glfwSwapBuffers( window );


        // NOTE Disabling this block allows FPS to exceed the update rate, interpolation will be used on non-update draw calls
        // GLFW by default uses 60 fps
        // Read the start/stop times and get the elapsed
        // stop = std::chrono::steady_clock::now();
        // elapsed = stop - start;
        // Sleep any extra time
        // std::this_thread::sleep_until( stop + ( std::chrono::duration<double> )update_time - elapsed);


        // Recalculate the slept time
        stop = std::chrono::steady_clock::now();
        elapsed = stop - start;

        deferred_time += elapsed.count();
        // If there is enough time to fill to cap
        if( deferred_time >= update_cap*update_time ) {
            deferred_time -= update_cap * update_time;
            updates = update_cap;
        }
        // If there is some time, but not enough to fill to cap
        else if( deferred_time >= update_time ) {
            updates = floor( deferred_time / update_time );
            deferred_time -= updates * update_time;
        }
        // If the deffered time is not enough for an update
        else{
            updates = 0;
        }
    }


    // Close the local server if running
    local_server.stop();

    // Force the connection to close
    connection.disconnect(true);

    // Close the scene (client side)
    scene.close_client();

    // Close scene assets
    scene.close_assets();

    // Close GUI assets
    GUI::close_assets();

    // Free debug draw assets
    DebugDraw::free_assets();
}

#define cf_key(action) client->config.keybinds[action].key == key && client->config.keybinds[action].mods == mods

void Client::keyCallback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    Client *client = static_cast<Client *>( glfwGetWindowUserPointer( window ) );
    if(action == GLFW_PRESS && key == GLFW_KEY_ESCAPE && mods & GLFW_MOD_ALT)
        glfwSetWindowShouldClose(window,GLFW_TRUE);

    // Player Controls
    Player *p = client->scene.player_set.get_active();

    if( p ) {
        uint16_t& iflag = p->input_flag;
        if( cf_key( KEY_FORWARD ) ) {
            if( action == GLFW_PRESS )
                iflag = iflag | Player::FORWARD;
            else if( action == GLFW_RELEASE )
                iflag = iflag & ~Player::FORWARD;
        }
        else if( cf_key( KEY_BACKWARD ) ) {
            if( action == GLFW_PRESS )
                iflag = iflag | Player::BACKWARD;
            else if( action == GLFW_RELEASE )
                iflag = iflag & ~Player::BACKWARD;
        }
        else if( cf_key( KEY_LEFT ) ) {
            if( action == GLFW_PRESS )
                iflag = iflag | Player::LEFT;
            else if( action == GLFW_RELEASE )
                iflag = iflag & ~Player::LEFT;
        }
        else if( cf_key( KEY_RIGHT ) ) {
            if( action == GLFW_PRESS )
                iflag = iflag | Player::RIGHT;
            else if( action == GLFW_RELEASE )
                iflag = iflag & ~Player::RIGHT;
        }
        else if( cf_key( KEY_LEAP ) ) {
            if( action == GLFW_PRESS )
                iflag = iflag | Player::LEAP;
            else if( action == GLFW_RELEASE )
                iflag = iflag & ~Player::LEAP;
        }
    }

    if( action == GLFW_REPEAT ) {
        GUI::key_input(key, mods);
    }
    else if(action == GLFW_PRESS){

        GUI::key_input(key, mods);
    }
}

void Client::resizeCallback( GLFWwindow *window, int width, int height ) {
    // ClientWindow *client = static_cast<ClientWindow *>( glfwGetWindowUserPointer( window ) );
    // if( !client->run_scene )
    //     return;
}

void Client::charCallback( GLFWwindow *window, unsigned int codepoint ) {
    Client *client = static_cast<Client *>( glfwGetWindowUserPointer( window ) );
    GUI::char_input(codepoint);
    // if( !client->run_scene )
        // return;
}

void Client::cursorMoveCallback( GLFWwindow *window, double xpos, double ypos ) {
    Client *client = static_cast<Client *>( glfwGetWindowUserPointer( window ) );
    if( glfwGetInputMode(client->window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        return;


    // NOTE View navigation, should be moved to external function
    int width, height;
    glfwGetWindowSize( window, &width, &height );
    versor r = GLM_QUAT_IDENTITY_INIT, s;
    vec3 vx = {1,0,0};
    glm_quat_rotatev(client->scene.view.rot, vx,vx);
    glm_quatv( r, (ypos-height*.5)*-.01, vx);
    glm_quat( s, (xpos-width*.5)*-.01, 0, 1, 0);
    glm_quat_mul(s,r,r);
    glm_quat_mul(r,client->scene.view.rot,client->scene.view.rot);
    client->scene.view.update();

    glfwSetCursorPos( window, width / 2, height / 2 );
}

void Client::clickCallback( GLFWwindow *window, int button, int action, int mods ) {
    Client *client = static_cast<Client *>( glfwGetWindowUserPointer( window ) );

    double x,y;
    int w,h;
    glfwGetCursorPos(window, &x, &y);
    glfwGetWindowSize(window, &w,&h);

    if(action != GLFW_PRESS)
        return;

    // Offset the input to that of the subwindow
    if(w > h * client->fbo_ratio){
        x -= (w-client->fbo_ratio*h)/2;
        w = h*client->fbo_ratio;
    }
    else{
        y -= (h-w/client->fbo_ratio)/2;
        h = w/client->fbo_ratio;
    }

    // Convert to a 0 to 1 scale and flip y
    x /= w;
    y = 1-(y/h);

    GUI::select(x,y);
}

void Client::enableCursor(){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Client::disableCursor(){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos( window, window_width / 2, window_height / 2 );

}

