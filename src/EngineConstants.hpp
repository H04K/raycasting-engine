#pragma once

// Common constants for the raycasting engine
// Eliminates magic numbers and centralizes configuration

namespace EngineConstants
{
    // Height conversion
    // Sector heights are normalized (0.0-1.0), this converts to world units
    constexpr float HEIGHT_SCALE = 1000.0f;
    constexpr float INV_HEIGHT_SCALE = 1.0f / HEIGHT_SCALE;

    // Color operations
    constexpr int COLOR_MAX = 255;
    constexpr int COLOR_SHIFT = 8; // For fast division: value >> 8 instead of / 255

    // Default player height
    constexpr float DEFAULT_PLAYER_HEIGHT = 50.0f;

    // Texture sampling
    constexpr int MAX_TEXTURE_CACHE_SIZE = 64; // Maximum cached texture images

    // Physics
    constexpr float DEFAULT_GRAVITY = 980.0f;           // units/s²
    constexpr float DEFAULT_GROUND_FRICTION = 0.9f;
    constexpr float DEFAULT_AIR_FRICTION = 0.99f;
    constexpr float DEFAULT_TERMINAL_VELOCITY = 1000.0f;
    constexpr float DEFAULT_GROUND_SNAP_DISTANCE = 10.0f;

    // Rendering
    constexpr float DEFAULT_FOV = 60.0f;
    constexpr float DEFAULT_VERTICAL_FOV = 120.0f;
    constexpr float DEFAULT_FAR_PLANE = 900.0f;
    constexpr float DEFAULT_NEAR_PLANE = 100.0f;

    // Entity defaults
    constexpr float DEFAULT_ENTITY_RADIUS = 16.0f;
    constexpr float DEFAULT_SPRITE_WIDTH = 64.0f;
    constexpr float DEFAULT_SPRITE_HEIGHT = 64.0f;
}
