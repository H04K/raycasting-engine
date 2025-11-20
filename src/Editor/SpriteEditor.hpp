#pragma once

#include <raylib.h>
#include <vector>
#include "Renderer/World.hpp"
#include "Renderer/Entity.hpp"

class SpriteEditor
{
public:
    SpriteEditor() = default;

    void Update(float dt, class WorldEditor& editor);
    void Render(const World& world) const;
    void DrawGUI(World& world);

    bool IsActive() const { return isActive; }
    void Activate() { isActive = true; }
    void Deactivate() { isActive = false; selectedEntityId = NULL_ENTITY; }

    // Entity management
    EntityID CreateEntity(World& world, Vector2 position);
    void DeleteEntity(World& world, EntityID id);
    void SelectEntity(EntityID id) { selectedEntityId = id; }

private:
    bool isActive = false;
    EntityID selectedEntityId = NULL_ENTITY;

    // Entity creation properties
    EntityType entityType = EntityType::Static;
    TextureID spriteTexture = NULL_TEXTURE;
    float spriteScale = 1.0f;
    float spriteWidth = 64.0f;
    float spriteHeight = 64.0f;
    float elevation = 0.0f;
    bool isBillboard = true;
    bool hasGravity = false;
    bool hasCollision = true;
    Color tint = WHITE;

    void RenderEntityGizmo(const Entity& entity) const;
    void EditEntityProperties(Entity& entity);
};
