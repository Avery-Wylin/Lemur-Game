#ifndef GUI_H
#define GUI_H

#include <vector>
#include <string>
#include <inttypes.h>
#include "Text.h"
#include "Audio.h"
#include <cglm/vec3.h>

class ElementSet;
struct ElementColor;

// Create a single namespace for the GUI interface
// All extern definitions are in the GUI.cpp file
namespace GUI {
    extern FontInfo font;
    extern Shader text_shader, background_shader;
    extern VAO quad_vao;
    extern float ratio, bevel, volume;
    extern ElementSet *selection;
    extern SoundSource sound_source;
    extern SoundBuffer sound_select;
    extern ElementColor colors;

    void init_assets();
    void close_assets();
    void select_set( ElementSet *es );
    void deselect_set();
    void draw();
    void char_input( char c );
    void select( float x, float y );
    void key_input( uint32_t key_value, uint8_t modifiers_value );
    void play_sound_select();
    void play_sound_deselect();
};

/*
 * Default colors used for an element set.
 * This is currently a soft biege and blue theme.
 */
struct ElementColor {
    vec3
    // Dark Default
    background_active = {.05,.05,.05},
    background_inactive = {.1,.1,.1},
    outline_active = {1,1,1},
    outline_inactive = {.2,.2,.2},
    text_active = {1,1,1},
    text_inactive = {.75,.75,.75},
    decorator_active = {.23,.54,0},
    decorator_inactive = {.07,.21,.02};

    ElementColor(){
        theme_leaf();
    }

    #define vec3_compose(dest, x, y, z); dest[0] = x; dest[1] = y; dest[2] = z;

    void theme_leaf(){
        vec3_compose( background_active, .15, .25, .1 );
        vec3_compose( background_inactive, .7, .65, .4 );
        vec3_compose( outline_active, 0.3, 1, 0 );
        vec3_compose( outline_inactive, 0, 0, 0 );
        vec3_compose( text_active, 0.3, 1, 0 );
        vec3_compose( text_inactive, 0, 0, 0 );
        vec3_compose( decorator_active, .33, .77, 0 );
        vec3_compose( decorator_inactive, .13, .47, 0.1 );
    }

    void theme_sand(){
        vec3_compose( background_active, .59,.54,.53 );
        vec3_compose( background_inactive, .77,.69,.63 );
        vec3_compose( outline_active, 1, 1, 1 );
        vec3_compose( outline_inactive, .05,.04,.03 );
        vec3_compose( text_active, 1, 1, 1 );
        vec3_compose( text_inactive, .05,.04,.03 );
        vec3_compose( decorator_active, .90,.58,.36 );
        vec3_compose( decorator_inactive, .17,.29,.50 );
    }

    void theme_sunset(){
        vec3_compose( background_active, .0, .05, .1 );
        vec3_compose( background_inactive, .0, .1, .2 );
        vec3_compose( outline_active, .86, .35, .0 );
        vec3_compose( outline_inactive, 0, 0, 0 );
        vec3_compose( text_active, .86, .35, .0 );
        vec3_compose( text_inactive, 1,1,1 );
        vec3_compose( decorator_active, .3, .05, 0 );
        vec3_compose( decorator_inactive, .0, .25, .25 );
    }

    void theme_day(){
        vec3_compose( background_active, .8,.8,.8 );
        vec3_compose( background_inactive, 1, 1, 1 );
        vec3_compose( outline_active, .1, .12, .15  );
        vec3_compose( outline_inactive, 0, 0 ,0  );
        vec3_compose( text_active, 0, 0, .1 );
        vec3_compose( text_inactive, 0, 0, 0 );
        vec3_compose( decorator_active, .5, .65, .8 );
        vec3_compose( decorator_inactive, .6, .75, .9 );
    }

    void theme_night(){
        vec3_compose( background_active, .05,.05,.05 );
        vec3_compose( background_inactive, .1,.1,.1 );
        vec3_compose( outline_active, .5,.5,.5 );
        vec3_compose( outline_inactive, 0, 0, 0 );
        vec3_compose( text_active, .75, .84, 1 );
        vec3_compose( text_inactive, 1,1,1 );
        vec3_compose( decorator_active, .2, .2, .4  );
        vec3_compose( decorator_inactive, .15, .15, .30 );
    }

};


enum ElementType : uint8_t {
    NONE,       // Undeclared type
    TEXT,       // Simple text container, no interaction
    BUTTON,     // Can be enabled but only disabled by code
    OPTION,     // Switches between a list of options
    TOGGLE,     // Can be enabled/diabled by clicking again
    TEXT_INPUT, // Text that can be selected and edited
    NUM_INPUT,  // Number that can be selected and edited
    BAR,        // Select a float value based on x position
    KEY_CAPTURE // Capture whichever key is pressed
};

struct Element {
        static const uint8_t
        ACTIVE = 1,         // Determines if the element is selected or enabled, renders in different colors
        CHANGED = 2,        // Whether the element has changed since it was last read
        HIDDEN = 4,         // Hides the element when rendering and selecting
        DISABLED = 8,       // Disables selection
        LEFT_ALIGN = 16;    // Aligns display text leftmost, otherwise centered

        /*
         */
        Text *text = nullptr;                // Text drawn inside of an element
        ElementSet *owner = nullptr;         // The owner of this element
        uint8_t flags = 0;                   // Flags shared by all element types
        ElementType type = NONE;             // The element type
        float x = 0, y = 0, w = .2, h = .2;  // Position and dimensions of element


        Element() {};
        virtual ~Element() {
            if( owner ) {
                // This will cause a fatal error eventually, force the program to close
                printf( "Warning: An Element was deleted before its containing ElementSet.\n" );
                exit( 1 );
            }

            if( text ) {
                delete text;
            }
        };

        // When the element is clicked after another/none is selected
        virtual void event_select( float x, float y ) {
            if( !( flags & ACTIVE ) )
                GUI::play_sound_select();

            flags |= ACTIVE;
        }

        // When the element is selected and another/none is selected
        virtual void event_deselect() {
            flags &= ~ACTIVE;
        };

        // When the element is selected and clicked again
        virtual void event_reselect( float x, float y ) {};

        // When the element is selected and a key combo is pressed
        virtual void event_keypress( int32_t key_value, uint8_t modifiers_value ) {};

        virtual void event_char( char c ) {};

        inline bool is_active() {
            return flags & ACTIVE;
        }

        inline void set_hidden( bool b ) {
            if( b )
                flags |= HIDDEN;
            else
                flags &= ~HIDDEN;
        }

        void localize_coordinates( float &sx, float &sy ) {
            sx = ( sx - x ) / ( w / GUI::ratio );
            sy = ( sy - y ) / h;
        }

        bool contains_point( float sx, float sy, float &lx, float &ly ) {

            float b = fmax( GUI::bevel, 1.0f / fmin( w, h ) );

            // Convert the screen coordinates into the local space
            float ux, uy;
            lx = sx;
            ly = sy;
            localize_coordinates( lx, ly );

            // Shift the local coordinates so the origin is in the center
            // Absolute the coordinates and subtract a half, this inverts the values,
            // Multiply by the bevel and width, this will make a larger negative number closer to the edge
            // Add back half,
            ux = w * b * ( abs( lx - .5 ) -  0.5 ) + 0.5;
            uy = h * b * ( abs( ly - .5 ) -  0.5 ) + 0.5;

            // Clamp any negative numbers to 0
            ux = fmax( 0, ux );
            uy = fmax( 0, uy );

            return  ux * ux + uy * uy < .25;
        }

        void set_owner( ElementSet *owner ) {
            if( this->owner != nullptr ) {
                printf( "Can not reasign owner of an Element.\nUnlink all elements before destructor using remove_all()\n" );
                return;
            }

            this->owner = owner;
        };

        void set_location( float x, float y ) {
            this->x = x;
            this->y = y;
        };

        void set_size( float w, float h ) {
            this->w = w;
            this->h = h;
        }

        inline void set_text( std::string s ) {
            if( text )
                text->set_text( s );
        }

        inline void set_text( const char *s, uint32_t count ) {
            if( text )
                text->set_text( s, count );
        }

        inline std::string get_text() {
            if( text )
                return text->getText();
        }

        inline bool changed() {
            if( flags & CHANGED ) {
                flags &= ~CHANGED;
                return true;
            }
            else {
                return false;
            }
        }

        inline void hide(){
            flags |= HIDDEN;
        }

        inline void show(){
            flags &= ~HIDDEN;
        }

    private:
        Element &operator=( const Element & );
};

/*
 * Keeps track of elements as well as providing a theme and operations.
 * Element sets that are active are drawn and input is passed from the global GUI functions.
 */
class ElementSet {
        // NOTE elements are allocated elsewhere and are not cleaned up by the element set
        std::vector<Element *> elements;
        Element *selection = nullptr;
        bool bool_had_input = false;

    public:

        void deselect();
        void select( float x, float y );
        void add( Element *element );
        void remove( Element *element );
        void remove_all();
        void draw();
        void char_input( char c );
        void key_input( uint32_t key_value, uint8_t modifiers_value );
        void compact( uint32_t start, float left, float right, float top, float spacing, float line_spacing );
        void compact_center( uint32_t start, float center, float max_width, float top, float spacing, float line_spacing );
        inline uint32_t size() {return elements.size();}
        inline bool had_input() {
            if( bool_had_input ) {
                bool_had_input = false;
                return true;
            }
            else {
                return false;
            }
        }
};

// Simple non-selectable text
class ElementText : public Element {

    public :
        ElementText() {
            type = TEXT;
            text = new Text();
            text->set_text( "Text", 0 );
            flags |= DISABLED;
        }
};

// A Button that is enabled by a click and disabled when the if state is read
class ElementButton : public Element {

    public :

        ElementButton() {
            type = BUTTON;
            text = new Text();
            text->set_text( "Button", 0 );
        }

        virtual void event_select( float x, float y ) override{
            if( !( flags & CHANGED ) ) {
                GUI::play_sound_select();
            }

            flags |= CHANGED;
        }

        // Also trigger active on reselect
        virtual void event_reselect( float x, float y ) override {
            event_select( x, y );
        }


        // Do nothing on deselect
        virtual void event_deselect() override {};
};

// Cycles between options when clicked
class ElementOption : public Element {
        uint8_t selected_option;
        std::vector<std::string> options;
    public :

        ElementOption() {
            type = OPTION;
            text = new Text();
            options = {"Option 1", "Option 2", "Option 3"};
            selected_option = 0;
            text->set_text( this->options[selected_option] );
        }

        void event_reselect( float x, float y ) override {
            GUI::play_sound_select();
            flags |= CHANGED;
            selected_option = ( selected_option + 1 ) % options.size();
            text->set_text( this->options[selected_option] );
        }

        void event_keypress( int32_t key, uint8_t modifier ) override {
            switch( key ) {
                case GLFW_KEY_DOWN:
                case GLFW_KEY_LEFT:
                    selected_option = ( ( options.size() + selected_option ) - 1 ) % options.size();
                    text->set_text( this->options[selected_option] );
                    GUI::play_sound_deselect();
                    flags |= CHANGED;
                    break;

                case GLFW_KEY_UP:
                case GLFW_KEY_RIGHT:
                    selected_option = ( selected_option + 1 ) % options.size();
                    text->set_text( options[selected_option] );
                    GUI::play_sound_select();
                    flags |= CHANGED;
                    break;

                default:
                    break;
            }
        }

        void select_option( uint32_t index ) {
            selected_option = index % options.size();;
            flags |= CHANGED;
            text->set_text( options[selected_option] );
        }

        void set_option( uint32_t index, string option ) {
            if( index >= options.size() )
                return;

            options[index] = option;

            if( index == selected_option )
                text->set_text( options[selected_option] );
        }

        void set_options( std::initializer_list<string> list ) {
            options = list;
            selected_option %= list.size();
            text->set_text( options[selected_option] );
        }

        void resize_options( uint32_t count ) {
            options.resize( count );
            selected_option %= options.size();
        }

        void delete_option( uint32_t index ) {
            if( index >= options.size() || options.size() == 1 )
                return;

            if( selected_option >= index )
                selected_option--;

            options.erase( options.begin() + index );
        }

        inline uint32_t option_count() {return options.size();}
        inline uint32_t selected() {return selected_option;}
};

// Turns on when clicked, turns off when clicked again, retains active state
class ElementToggle : public Element {
    public:

        ElementToggle() {
            type = TOGGLE;
            text = new Text();
            text->set_text( "Toggle", 0 );
        }

        ElementToggle( const char *label, uint32_t length, bool active ) {
            type = TOGGLE;
            text = new Text();
            text->set_text( label, length );

            if( active )
                flags |= ACTIVE;
        }

        // Toggle on select
        void event_select( float x, float y ) override {
            if( !( flags & ACTIVE ) )
                GUI::play_sound_select();
            else
                GUI::play_sound_deselect();

            flags |= CHANGED;
            flags ^= ACTIVE;
        }

        // Toggle on reselect
        void event_reselect( float x, float y ) override {
            event_select( x, y );
        }

        // Keep state on deselection
        void event_deselect() override {};

};

// Allows for typing a single line of text
class ElementTextInput : public Element {
        uint8_t max_length;

    public:
        ElementTextInput() {
            type = TEXT_INPUT;
            text = new Text();
            text->set_text( "Text Input", 0 );
            flags |= LEFT_ALIGN;
            max_length = 255;
        }

        ElementTextInput( std::string default_text, uint8_t max_length ) {
            type = TEXT_INPUT;
            text = new Text();
            text->set_text( default_text );
            this->max_length = max_length;
        }

        // Reset the cursor position on selection
        void event_select( float x, float y ) override {
            if( !( flags & ACTIVE ) )
                GUI::play_sound_select();

            text->setCursor( text->getText().size() );
            flags |= ACTIVE;
        }

        // Unselect text when deselected
        void event_deselect() override {
            text->deselectAll();
            flags &= ~ACTIVE;
        }

        // Select all text when reselected
        void event_reselect( float x, float y ) override {
            text->selectAll();
        }

        // Additional text manipulation keys
        void event_keypress( int32_t key, uint8_t modifier ) override {
            switch( key ) {
                case GLFW_KEY_BACKSPACE:
                    text->erase();
                    flags |= CHANGED;
                    break;

                case GLFW_KEY_LEFT:
                    if( modifier & GLFW_MOD_SHIFT ) {
                        text->selectMore( -1 );
                    }
                    else {
                        text->moveCursor( -1 );
                    }

                    break;

                case GLFW_KEY_RIGHT:
                    if( modifier & GLFW_MOD_SHIFT ) {
                        text->selectMore( 1 );
                    }
                    else {
                        text->moveCursor( 1 );
                    }

                    break;
            }
        }

        // Text insertion when typing
        void event_char( char c ) override {
            if( text->getText().size() < max_length || text->getSelectionCount() > 0 ) {
                text->overwrite( c );
                flags |= CHANGED;
            }
        }
};

// Allows for typing a number
// NOTE This is not very precise, still has rounding errors
class ElementNumInput : public Element {
        static const uint8_t max_chars = 8;
        float value = 0;
        bool is_int =  false;

        void parse_value() {
            float new_val =  value;

            try {
                new_val = std::stof( text->getText() );
            }
            catch( std::invalid_argument &ex) {
            }

            value = new_val;
            flags |= CHANGED;
            update_text();
        }

        void update_text() {
            char s[max_chars + 1];

            if( is_int ) {
                snprintf( s, max_chars + 1, "%d", ( int )value );
            }
            else
                snprintf( s, max_chars + 1, "%.4f", value );

            value = std::stof( s );

            text->set_text( s, 0 );
        }

    public:

        ElementNumInput() {
            type = TEXT_INPUT;
            text = new Text();
            flags |= LEFT_ALIGN;
            parse_value();
        }

        // Reset the cursor position on selection
        void event_select( float x, float y ) override {
            if( !( flags & ACTIVE ) )
                GUI::play_sound_select();

            text->setCursor( text->getText().size() );
            flags |= ACTIVE;
        }

        // Unselect text when deselected
        void event_deselect() override {
            parse_value();
            text->deselectAll();
            flags &= ~ACTIVE;
        }

        // Select all text when reselected
        void event_reselect( float x, float y ) override {
            text->selectAll();
        }

        // Additional text manipulation keys
        void event_keypress( int32_t key, uint8_t modifier ) override {
            switch( key ) {
                case GLFW_KEY_BACKSPACE:
                    text->erase();
                    break;

                case GLFW_KEY_LEFT:
                    if( modifier & GLFW_MOD_SHIFT ) {
                        text->selectMore( -1 );
                    }
                    else {
                        text->moveCursor( -1 );
                    }

                    break;

                case GLFW_KEY_RIGHT:
                    if( modifier & GLFW_MOD_SHIFT ) {
                        text->selectMore( 1 );
                    }
                    else {
                        text->moveCursor( 1 );
                    }

                    break;
            }
        }

        // Text insertion when typing
        void event_char( char c ) override {
            if( ( c < '0' || c > '9' ) && !( c == '.' || c == '-' ) )
                return;

            if( text->getText().size() < max_chars || text->getSelectionCount() > 0 ) {
                text->overwrite( c );
            }
        }

        void set_value( float v ) {
            value = v;
            update_text();
        }

        void set_integer( bool b ) {
            is_int = b;
            parse_value();
        }

        inline float get_float() {
            return value;
        }

        inline int get_int() {
            return ( int )value;
        }
};

// A bar that stores a float value between min and max
class ElementBar : public Element {
        static const uint8_t max_characters = 8;
        float value, min, max, t;
        string label = "Value";

        void update_value() {
            value = ( 1 - t ) * min + t * max;
            char num_label[max_characters];
            snprintf( num_label, max_characters, "%.2f", value );
            string new_label = label + ": " + num_label;
            text->set_text( new_label );
            flags |= CHANGED;
        }

    public:
        ElementBar() {
            type = BAR;
            text = new Text();
            value = 0;
            t = 0;
            min = 0;
            max = 1;
            update_value();
        }

        // Only change value on reselection
        void event_reselect( float x, float y ) override {
            if( t < x )
                GUI::play_sound_select();
            else
                GUI::play_sound_deselect();

            t = x;
            update_value();
        };

        // Additional key manipulation
        void event_keypress( int32_t key, uint8_t modifier ) override {
            switch( key ) {
                case GLFW_KEY_LEFT:
                    if( modifier & GLFW_MOD_SHIFT ) {
                        t -= .01f;
                    }
                    else {
                        t -= .1f;
                    }

                    t = fmin( fmax( t, 0 ), 1 );
                    update_value();
                    GUI::play_sound_deselect();
                    break;

                case GLFW_KEY_RIGHT:
                    if( modifier & GLFW_MOD_SHIFT ) {
                        t += .01f;
                    }
                    else {
                        t += .1f;
                    }

                    t = fmin( fmax( t, 0 ), 1 );
                    update_value();
                    GUI::play_sound_select();
                    break;
            }
        }

        inline void set_label( string s ) {
            label = s;
            update_value();
        }

        inline void set_range( float min_val, float max_val ) {
            min = min_val;
            max = max_val;
            update_value();
        }

        inline void set_value( float v ) {
            t = fmax( fmin( ( v - min ) / ( max - min ), 1 ), 0 );
            update_value();
        }

        inline float get_normalized_value() {
            return t;
        }

        inline float get_value() {
            return value;
        }
};

// Displays a key combination typed while selected, then deselects itself
class ElementKeyCapture : public Element {
        int32_t key = GLFW_KEY_UNKNOWN;
        uint8_t modifiers = 0;
        string label = "Keybind";

        void update_label() {
            std::string new_label = label + ": ";

            // Modifier Values
            if( modifiers & GLFW_MOD_CONTROL )
                new_label.append( "ctrl " );

            if( modifiers & GLFW_MOD_ALT )
                new_label.append( "alt " );

            if( modifiers & GLFW_MOD_SHIFT )
                new_label.append( "shift " );

            if( key > 96 )
                switch( key ) {
                    case GLFW_KEY_ENTER:new_label.append( "enter " );break;
                    case GLFW_KEY_TAB:new_label.append( "tab" );break;
                    case GLFW_KEY_BACKSPACE:new_label.append( "backspace" );break;
                    case GLFW_KEY_INSERT:new_label.append( "insert" );break;
                    case GLFW_KEY_DELETE:new_label.append( "delete" );break;
                    case GLFW_KEY_RIGHT:new_label.append( "right" );break;
                    case GLFW_KEY_LEFT:new_label.append( "left" );break;
                    case GLFW_KEY_DOWN:new_label.append( "down" );break;
                    case GLFW_KEY_UP:new_label.append( "up" );break;
                    case GLFW_KEY_PAGE_UP:new_label.append( "page up" );break;
                    case GLFW_KEY_PAGE_DOWN:new_label.append( "page down" );break;
                    case GLFW_KEY_HOME:new_label.append( "home" );break;
                    case GLFW_KEY_END:new_label.append( "end" );break;
                    case GLFW_KEY_CAPS_LOCK:new_label.append( "caps lock" );break;
                    case GLFW_KEY_SCROLL_LOCK:new_label.append( "scroll lock" );break;
                    case GLFW_KEY_NUM_LOCK:new_label.append( "num lock" );break;
                    case GLFW_KEY_PRINT_SCREEN:new_label.append( "print screen" );break;
                    case GLFW_KEY_PAUSE:new_label.append( "pause" );break;
                }

            else if( key == GLFW_KEY_UNKNOWN )
                new_label.append( "none" );

            // Print space character (it is not visible)
            else if( key == GLFW_KEY_SPACE )
                new_label.append( "space" );

            // Assume the rest of ASCII is printable
            else
                new_label.push_back( key );

            text -> set_text( new_label );
        }

    public:

        ElementKeyCapture() {
            type = KEY_CAPTURE;
            text = new Text();
            update_label();
        };

        inline bool set_key(int32_t key_value, uint8_t modifiers_value){
            // If the key is escape, then capture it and set the keybind to none
            if( key_value == GLFW_KEY_ESCAPE )
                key_value = GLFW_KEY_UNKNOWN;

            // Exit if the input values are invalid (non-ASCII) or nav keys, exclude escape
            else if( key_value < 32 || ( key_value > 96 && key_value < 257 ) || ( key_value > 284 ) )
                return false;

            // Return if there is no change
            if( key == key_value && modifiers == modifiers_value )
                return false;

            // If there is a change mark it and update the text
            key = key_value;
            modifiers = modifiers_value;
            update_label();
            return true;
        }

        void event_keypress( int32_t key_value, uint8_t modifiers_value ) override {
            if(set_key(key_value, modifiers_value)){
                flags |= CHANGED;
                if( owner )
                    owner->deselect();
            }
        };

        void set_label( string s ) {
            label = s;
            update_label();
        }

        inline int32_t get_key() {
            return key;
        }

        inline uint8_t get_mods() {
            return modifiers;
        }
};

#endif // GUI_H
