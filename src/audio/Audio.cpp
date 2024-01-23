#include "Audio.h"

#include <stdlib.h>
#include <cstdio>

#include <AL/alc.h>
#include <string.h>

namespace Audio {
    ALCdevice *al_device = nullptr;
    ALCcontext *al_context = nullptr;
    SoundSource source_pool[AUDIO_POOL_SIZE];
    SoundSource music_source;
}

void Audio::init() {

    // Open the default audio device
    al_device = nullptr;
    al_device = alcOpenDevice( nullptr );

    if( !al_device ) {
        puts( " Failed to find default audio device." );
        exit( EXIT_FAILURE );
    }

    // Create the audio context, only 1 is needed
    al_context = alcCreateContext( al_device, nullptr );

    if( !al_context ) {
        puts( " Failed to create audio context." );
        exit( EXIT_FAILURE );
    }

    // Make the context current
    if( !alcMakeContextCurrent( al_context ) ) {
        puts( " Failed to make audio context current." );
        exit( EXIT_FAILURE );
    }

    // Allocate source pool
    for( unsigned int i = 0; i < AUDIO_POOL_SIZE; ++i ) {
        source_pool[i].allocate();
    }
}

void Audio::close() {
    for( unsigned int i = 0; i < AUDIO_POOL_SIZE; ++i ) {
        source_pool[i].free();
    }

    alcDestroyContext( al_context );
    alcCloseDevice( al_device );
}

unsigned int Audio::get_available_source() {
    for( unsigned int i = 0; i < AUDIO_POOL_SIZE; ++i ) {
        if( !source_pool[i].is_playing() )
            return i;
    }

    return AUDIO_POOL_SIZE;
}

void Audio::set_master_volume( float v ) {
    alListenerf( AL_GAIN, v );
}

void Audio::set_listener_transform( View &view ) {
    mat4 transform;
    view.getTransform( transform );

    // Extract xyz of view transform, this is the last column of the affine transform
    alListenerfv( AL_POSITION, transform[3] );

    // Extract orthogonal vectors of view transform, this is -z forward and +y up columns
    float orientation[6];
    glm_vec3_inv_to( transform[2], *reinterpret_cast<vec3 *>( &orientation[0] ) ); // -Z component
    glm_vec3_copy( transform[1], *reinterpret_cast<vec3 *>( &orientation[3] ) ); // +Y component
    alListenerfv( AL_ORIENTATION, orientation );
}

void Audio::play_music( SoundBuffer &buffer, float volume, float pitch ) {
    music_source.set_sound( buffer );
    music_source.set_pitch( pitch );
    music_source.set_volume( volume );
    music_source.play();
}

void Audio::stop_music() {
    music_source.stop();
}

SoundSource *Audio::play_sound_3D( SoundBuffer &buffer, float volume, float pitch, float x, float y, float z ) {
    unsigned int i = get_available_source();

    if( i == AUDIO_POOL_SIZE )
        return nullptr;

    source_pool[i].set_sound( buffer );
    source_pool[i].set_pitch( pitch );
    source_pool[i].set_volume( volume );
    source_pool[i].set_relative( true );
    source_pool[i].set_position( x, y, z );
    source_pool[i].play();
    return source_pool + i;
}

SoundSource *Audio::play_sound( SoundBuffer &buffer, float volume, float pitch ) {
    unsigned int i = get_available_source();

    if( i == AUDIO_POOL_SIZE )
        return nullptr;

    source_pool[i].set_sound( buffer );
    source_pool[i].set_pitch( pitch );
    source_pool[i].set_volume( volume );
    source_pool[i].set_relative( false );
    source_pool[i].play();
    return source_pool + i;
}
