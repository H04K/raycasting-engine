#pragma once

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include "Renderer/TextureManager.hpp"
#include "Renderer/RaycastingMath.hpp"
#include "EngineConstants.hpp"

using EntityID = uint32_t;
constexpr EntityID NULL_ENTITY { static_cast<EntityID>(-1) };

enum class EntityType
{
    Static,     // Non-moving decorative entity
    Dynamic,    // Moving entity with physics
    Player,     // Player entity
    Enemy,      // AI-controlled enemy
    Item,       // Pickup item
    Projectile  // Projectile entity
};

struct Entity
{
    EntityID id = NULL_ENTITY;
    EntityType type = EntityType::Static;

    // Transform
    Vector2 position { 0, 0 };      // XY position in world
    float elevation = 0.0f;          // Z position (height)
    float rotation = 0.0f;           // Rotation in radians (for non-billboard)

    // Rendering
    TextureID spriteTextureId = NULL_TEXTURE;
    bool isBillboard = true;         // Always face camera
    float spriteScale = 1.0f;        // Sprite scaling
    float spriteWidth = EngineConstants::DEFAULT_SPRITE_WIDTH;
    float spriteHeight = EngineConstants::DEFAULT_SPRITE_HEIGHT;
    Color tint = WHITE;

    // Physics
    Vector2 velocity { 0, 0 };
    float verticalVelocity = 0.0f;
    bool hasGravity = false;
    bool hasCollision = true;
    float radius = EngineConstants::DEFAULT_ENTITY_RADIUS;

    // Gameplay
    SectorID currentSectorId = NULL_SECTOR;
    bool isActive = true;
    bool isVisible = true;

    // User data
    std::string name;
    void* userData = nullptr;
};

struct EntityRenderData
{
    EntityID entityId;
    float distance;
    Vector2 screenPosition;
    float screenScale;
    const Entity* entity;

    bool operator<(const EntityRenderData& other) const
    {
        return distance > other.distance; // Sort far to near for rendering
    }
};
