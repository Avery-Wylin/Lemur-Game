#include "GUI.h"
#include "../graphics/Shader.h"

#include <iostream>

namespace GUI {
    FontInfo font;
    Shader text_shader, background_shader;
    VAO quad_vao;
    float ratio = 2.0f, bevel = 20.0f, volume = .8;
    ElementSet *selection = nullptr;
    SoundBuffer sound_select;
    SoundSource sound_source;
    ElementColor colors;
}

void GUI::init_assets() {
    font.load( "liberation-mono" );
    text_shader.load( "text2D" );
    background_shader.load( "gui" );
    float pos[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    float uv[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    quad_vao.loadAttributeFloat( Attribute::ATTRB_POS, 0, 0, 2, 8, pos );
    quad_vao.loadAttributeFloat( Attribute::ATTRB_UV, 0, 0, 2, 8, uv );
    sound_select.load( "click" );
    sound_source.allocate();
}

void GUI::close_assets() {
    text_shader.free();
    background_shader.free();
    quad_vao.free();
    font.free_texture();
    sound_select.free();
    sound_source.free();
}

void GUI::select_set( ElementSet *es ) {
    selection = es;
}

void GUI::deselect_set() {
    selection = nullptr;
}

void GUI::draw() {
    if( selection ) {
        selection->draw();
    }
}

void GUI::char_input( char c ) {
    if( selection )
        selection->char_input( c );
}

void GUI::select( float x, float y ) {
    if( selection )
        selection->select( x, y );
}

void GUI::key_input( uint32_t key_value, uint8_t modifiers_value ) {
    if( selection )
        selection->key_input( key_value, modifiers_value );
}

void GUI::play_sound_select() {
    if( sound_source.is_playing() )
        return;

    sound_source.set_sound( sound_select );
    sound_source.set_volume( volume );
    sound_source.set_pitch( 1 );
    sound_source.play();
}

void GUI::play_sound_deselect() {
    if( sound_source.is_playing() )
        return;

    sound_source.set_sound( sound_select );
    sound_source.set_volume( volume );
    sound_source.set_pitch( .8 );
    sound_source.play();
}

void ElementSet::deselect() {
    if( selection )
        selection->event_deselect();

    selection = nullptr;
}

void ElementSet::select( float x, float y ) {
    for( Element *element : elements ) {
        if( element->flags & Element::DISABLED || element->flags & Element::HIDDEN )
            continue;

        float lx, ly;

        if( element->contains_point( x, y, lx, ly ) ) {
            if( selection == element ) {
                bool_had_input = true;
                element->event_reselect( lx, ly );
            }
            else {
                if( selection )
                    selection->event_deselect();

                bool_had_input = true;
                element->event_select( lx, ly );
                selection = element;
            }

            return;
        }
    }

    deselect();
}

void ElementSet::add( Element *element ) {
    if( !element )
        return;

    if( element->owner ) {
        printf( "Element already has an owner.\n" );
        return;
    }

    element->set_owner( this );
    elements.push_back( element );
}

void ElementSet::remove( Element *element ) {
    for( unsigned int i = 0; i < elements.size(); ++i ) {
        if( elements[i] == element ) {
            element->owner = nullptr;
            elements.erase( elements.begin() + i );
            return;
        }
    }
}

void ElementSet::remove_all() {
    for( unsigned int i = 0; i < elements.size(); ++i ) {
        elements[i]->owner = nullptr;
    }

    elements.clear();
}

// NOTE Shader and VAO should be pre-bound before this call
void ElementSet::draw() {
    mat4 ortho;
    glm_ortho( 0, GUI::ratio, 0, 1, 0, 1, ortho );

    // Draw Backgrounds
    GUI::quad_vao.bind();
    Shader::bind( GUI::background_shader );
    Shader::uniformMat4f( UNIFORM_CAMERA, ortho );

    for( Element *e : elements ) {
        if( e->flags & Element::HIDDEN )
            continue;

        // Background
        Shader::uniformVec4f( UNIFORM_TRANSFORM, vec4{e->x * GUI::ratio, e->y, e->w, e->h} );
        Shader::uniformFloat( UNIFORM_FACTOR, fmax( GUI::bevel, 1.0f / fmin( e->w, e->h ) ) );
        if(e->is_active()){
            Shader::uniformVec3f( UNIFORM_COLOR,GUI::colors.background_active );
            Shader::uniformVec3f( UNIFORM_COLOR2,GUI::colors.outline_active );
        }
        else{
            Shader::uniformVec3f( UNIFORM_COLOR,GUI::colors.background_inactive );
            Shader::uniformVec3f( UNIFORM_COLOR2,GUI::colors.outline_inactive );

        }
        glDrawArrays( GL_QUADS, 0, 4 );

        // Addition decorator for bars
        if( e->type == BAR ) {
            float v = static_cast<ElementBar *>( e )->get_normalized_value();
            Shader::uniformVec4f( UNIFORM_TRANSFORM,
                vec4{e->x * GUI::ratio, e->y, e->w * v, e->h}
            );
            Shader::uniformVec3f( UNIFORM_COLOR, e->is_active() ? GUI::colors.decorator_active : GUI::colors.decorator_inactive );
            glDrawArrays( GL_QUADS, 0, 4 );
        }
    }

    // Draw Texts
    Shader::bind( GUI::text_shader );
    Shader::uniformMat4f( UNIFORM_CAMERA, ortho );
    GUI::font.fontTexture.bind( 0 );
    float scale;

    Shader::uniformVec3f( UNIFORM_COLOR2, GUI::colors.decorator_active );

    for( Element *e : elements ) {
        if( e->flags & Element::HIDDEN || !e->text )
            continue;

        e->text->bind( GUI::font );
        //only fill to 9 height
        scale = fmin( e->w / e->text->getWidth(), e->h / e->text->getHeight() * .9 );
        // Adjust scale into uniform sizes, this keeps the UI consistent
        scale = scale > .2 ? floor( scale * 10 ) / 10 : scale;
        // Text scale is pulled away from the edges by setting it to .9
        scale *= .9f;

        // Text transform is 2d screen position and scale
        Shader::uniformVec3f( UNIFORM_TRANSFORM, vec3{
            // If left align, use half the fraction of the box not filled (1-.9)/2, if center align, use half the difference
            e->x * GUI::ratio + ( e->flags & Element::LEFT_ALIGN ? .05f * e->w : 0.5f * ( e->w - e->text->getWidth()*scale ) ),
            e->y + 0.5f * ( e->h + scale * e->text->getHeight() ),
            scale}
        );
        if(e->is_active()){
            Shader::uniformVec3f( UNIFORM_COLOR, GUI::colors.text_active );
        }
        else{
            Shader::uniformVec3f( UNIFORM_COLOR, GUI::colors.text_inactive );
        }
        glDrawArrays( GL_QUADS, 0, e->text->getVertexCount() );
    }

    Shader::unbind();
}

void ElementSet::char_input( char c ) {
    if( !selection )
        return;

    bool_had_input = true;
    selection->event_char( c );
}

void ElementSet::key_input( uint32_t key_value, uint8_t modifiers_value ) {
    if( !selection )
        return;

    bool_had_input = true;
    selection->event_keypress( key_value, modifiers_value );
}

// Spaces elements apart horizontally
void ElementSet::compact( unsigned int start, float left, float right, float top, float spacing, float line_spacing ) {
    float w = left, h = top, max_height = 0;
    unsigned int line_start = start, line_end = start;

    for( unsigned int i = start; i < elements.size(); ++i ) {
        if( w + elements[i]->w / GUI::ratio > right ) {
            w = left;

            for( unsigned int i = line_start; i < line_end; ++i ) {
                elements[i]->y -= max_height;
            }

            h -= max_height + line_spacing;
            line_start = line_end;
            max_height = 0;
        }

        elements[i]->set_location( w, h );
        w += ( elements[i]->w + spacing ) / GUI::ratio;
        max_height = max_height < elements[i]->h ? elements[i]->h : max_height;
        ++line_end;
    }

    for( unsigned int i = line_start; i < line_end; ++i ) {
        elements[i]->y -= max_height;
    }
}

void ElementSet::compact_center( unsigned int start, float center, float max_width, float top, float spacing, float line_spacing ) {
    float w = center, h = top, line_width = 0, line_height = 0;
    unsigned int line_start = start, line_end = start;

    for( unsigned int i = start; i < elements.size(); ++i ) {
        // The line exceeds the maximum width, align all to center
        if( line_width > max_width ) {
            w = center;
            h -= line_height + line_spacing;

            // Push the line down by its height and shift over to center
            for( unsigned int j = line_start; j < line_end; ++j ) {
                elements[j]->x -= line_width / 2.0f;
                elements[j]->y -= line_height;
            }

            line_width = 0;
            line_height = 0;
            line_start = line_end;
        }

        elements[i]->set_location( w, h );
        line_width += ( elements[i]->w + spacing ) / GUI::ratio;
        w += ( elements[i]->w + spacing ) / GUI::ratio;
        line_height = line_height < elements[i]->h ? elements[i]->h : line_height;


        line_end++;
    }

    for( unsigned int i = line_start; i < line_end; ++i ) {
        elements[i]->x -= line_width / 2.0f;
        elements[i]->y -= line_height;
    }
}
