#ifndef CFG_VARS_H
#define CFG_VARS_H

// Terrain
#define TERRAIN_DIM 255
#define TERRAIN_SCALE 2.0f
#define TERRAIN_HEIGHT_SCALE 0.25f

// Water Plane
#define WATER_WAVE_MOD 100
#define WATER_WAVE_SPEED .005f

// Audio
#define AUDIO_POOL_SIZE 16

// Armature
#define ARMATURE_MAX_JOINTS 150 // Must match shader uniform
#define ARMATURE_CONSTRAINT_TIMESTEP .05f

// FBO
#define FBO_MAX_COLOR_ATTACHMENTS 8

// View
#define VIEW_NEAR .01f
#define VIEW_FAR 80.0f

// Connection
#define CONNECTION_DEFAULT_PORT 53687

// Server
#define MAX_PLAYERS 16
#define MAX_PLAYER_SAVES 64
#define STEPS_PER_SECOND 20

// Player

// Plant
#define PLANT_NULL UINT16_MAX
#define PLANT_MAX_SPECIES 100       // Can be up to 127 (2^7)-2
#define PLANT_MAX_INSTANCES 500     // Can be up to 511 (2^9)-2
#define PLANT_SPECIES_MASK 0xfd
#define PLANT_INSTANCE_MASK 0x1ff

// File Directories
#define DIR_CONFIGS   "../config/"
#define DIR_SHADERS   "../assets/shaders/"
#define DIR_SAVES     "../saves/"
#define DIR_TEXTURES  "../assets/textures/"
#define DIR_MODELS    "../assets/models/"
#define DIR_ARMATURES "../assets/armatures/"
#define DIR_SOUNDS    "../assets/sounds/"
#define DIR_FONTS     "../assets/fonts/"

#endif // CFG_VARS_H
