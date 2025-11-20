#include "Editor/SpriteEditor.hpp"
#include "Editor/WorldEditor.hpp"
#include <imgui.h>

void SpriteEditor::Update(float dt, WorldEditor& editor)
{
    if (!isActive) return;

    // Get mouse position in world space
    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseViewport = editor.ScreenToViewportPosition(mouseScreen);
    Vector2 mouseWorld = editor.ScreenToWorldPosition(mouseViewport);

    // Left click to create entity
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        CreateEntity(editor.world, mouseWorld);
    }

    // Right click to select entity (simple distance check)
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        float minDist = 50.0f;
        EntityID closest = NULL_ENTITY;

        for (const auto& [id, entity] : editor.world.Entities)
        {
            float dist = Vector2Distance(mouseWorld, entity.position);
            if (dist < minDist)
            {
                minDist = dist;
                closest = id;
            }
        }

        if (closest != NULL_ENTITY)
        {
            SelectEntity(closest);
        }
    }

    // Delete selected entity with Delete key
    if (IsKeyPressed(KEY_DELETE) && selectedEntityId != NULL_ENTITY)
    {
        DeleteEntity(editor.world, selectedEntityId);
    }
}

void SpriteEditor::Render(const World& world) const
{
    if (!isActive) return;

    // Render all entities with gizmos
    for (const auto& [id, entity] : world.Entities)
    {
        RenderEntityGizmo(entity);

        // Highlight selected entity
        if (id == selectedEntityId)
        {
            DrawCircleLines(entity.position.x, entity.position.y, entity.radius + 5, YELLOW);
            DrawCircleV(entity.position, 3.0f, YELLOW);
        }
    }
}

void SpriteEditor::DrawGUI(World& world)
{
    ImGui::Begin("Sprite Editor");

    if (ImGui::Button(isActive ? "Deactivate" : "Activate"))
    {
        if (isActive)
            Deactivate();
        else
            Activate();
    }

    if (isActive)
    {
        ImGui::Separator();
        ImGui::Text("Left Click: Create Entity");
        ImGui::Text("Right Click: Select Entity");
        ImGui::Text("Delete Key: Delete Selected");

        ImGui::Separator();
        ImGui::Text("New Entity Properties:");

        const char* entityTypes[] = { "Static", "Dynamic", "Player", "Enemy", "Item", "Projectile" };
        int currentType = static_cast<int>(entityType);
        if (ImGui::Combo("Entity Type", &currentType, entityTypes, 6))
        {
            entityType = static_cast<EntityType>(currentType);
        }

        int texId = static_cast<int>(spriteTexture);
        if (ImGui::InputInt("Sprite Texture ID", &texId))
        {
            if (texId < 0)
                spriteTexture = NULL_TEXTURE;
            else
                spriteTexture = static_cast<TextureID>(texId);
        }

        ImGui::SliderFloat("Sprite Scale", &spriteScale, 0.1f, 10.0f);
        ImGui::InputFloat("Sprite Width", &spriteWidth);
        ImGui::InputFloat("Sprite Height", &spriteHeight);
        ImGui::InputFloat("Elevation", &elevation);
        ImGui::Checkbox("Billboard", &isBillboard);
        ImGui::Checkbox("Has Gravity", &hasGravity);
        ImGui::Checkbox("Has Collision", &hasCollision);
        ImGui::ColorEdit4("Tint", (float*)&tint);

        ImGui::Separator();
        ImGui::Text("Entity List (%zu):", world.Entities.size());

        if (ImGui::BeginListBox("##Entities", ImVec2(-1, 150)))
        {
            for (const auto& [id, entity] : world.Entities)
            {
                bool isSelected = (id == selectedEntityId);
                char label[64];
                snprintf(label, sizeof(label), "Entity %u (Type: %d)", id, static_cast<int>(entity.type));

                if (ImGui::Selectable(label, isSelected))
                {
                    SelectEntity(id);
                }
            }
            ImGui::EndListBox();
        }

        // Edit selected entity
        if (selectedEntityId != NULL_ENTITY)
        {
            Entity* entity = world.GetEntity(selectedEntityId);
            if (entity)
            {
                ImGui::Separator();
                ImGui::Text("Selected Entity %u:", selectedEntityId);
                EditEntityProperties(*entity);
            }
        }
    }

    ImGui::End();
}

EntityID SpriteEditor::CreateEntity(World& world, Vector2 position)
{
    Entity newEntity;
    newEntity.type = entityType;
    newEntity.position = position;
    newEntity.elevation = elevation;
    newEntity.spriteTextureId = spriteTexture;
    newEntity.spriteScale = spriteScale;
    newEntity.spriteWidth = spriteWidth;
    newEntity.spriteHeight = spriteHeight;
    newEntity.isBillboard = isBillboard;
    newEntity.hasGravity = hasGravity;
    newEntity.hasCollision = hasCollision;
    newEntity.tint = tint;

    // Find current sector
    newEntity.currentSectorId = FindSectorOfPoint(position, world);

    return world.AddEntity(newEntity);
}

void SpriteEditor::DeleteEntity(World& world, EntityID id)
{
    world.RemoveEntity(id);
    if (selectedEntityId == id)
    {
        selectedEntityId = NULL_ENTITY;
    }
}

void SpriteEditor::RenderEntityGizmo(const Entity& entity) const
{
    // Draw collision radius
    if (entity.hasCollision)
    {
        DrawCircleLines(entity.position.x, entity.position.y, entity.radius, BLUE);
    }

    // Draw entity position
    DrawCircleV(entity.position, 5.0f, GREEN);

    // Draw direction indicator if not billboard
    if (!entity.isBillboard)
    {
        Vector2 dir = {
            cosf(entity.rotation) * 20.0f,
            sinf(entity.rotation) * 20.0f
        };
        Vector2 endPoint = Vector2Add(entity.position, dir);
        DrawLineV(entity.position, endPoint, RED);
    }
}

void SpriteEditor::EditEntityProperties(Entity& entity)
{
    ImGui::PushID(entity.id);

    const char* entityTypes[] = { "Static", "Dynamic", "Player", "Enemy", "Item", "Projectile" };
    int currentType = static_cast<int>(entity.type);
    if (ImGui::Combo("Type", &currentType, entityTypes, 6))
    {
        entity.type = static_cast<EntityType>(currentType);
    }

    ImGui::InputFloat2("Position", (float*)&entity.position);
    ImGui::InputFloat("Elevation", &entity.elevation);
    ImGui::InputFloat("Rotation", &entity.rotation);

    int texId = static_cast<int>(entity.spriteTextureId);
    if (ImGui::InputInt("Texture ID", &texId))
    {
        entity.spriteTextureId = (texId < 0) ? NULL_TEXTURE : static_cast<TextureID>(texId);
    }

    ImGui::SliderFloat("Scale", &entity.spriteScale, 0.1f, 10.0f);
    ImGui::InputFloat("Width", &entity.spriteWidth);
    ImGui::InputFloat("Height", &entity.spriteHeight);
    ImGui::Checkbox("Billboard", &entity.isBillboard);
    ImGui::Checkbox("Gravity", &entity.hasGravity);
    ImGui::Checkbox("Collision", &entity.hasCollision);
    ImGui::InputFloat("Radius", &entity.radius);
    ImGui::ColorEdit4("Tint", (float*)&entity.tint);
    ImGui::Checkbox("Active", &entity.isActive);
    ImGui::Checkbox("Visible", &entity.isVisible);

    ImGui::PopID();
}
