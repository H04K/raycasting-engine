#pragma once

#include <raylib.h>
#include "Renderer/World.hpp"
#include "Renderer/RaycastingMath.hpp"

class WallCreationTool
{
public:
    WallCreationTool() = default;

    void Update(float dt, class WorldEditor& editor);
    void Render() const;
    void DrawGUI();

    bool IsActive() const { return isActive; }
    void Activate() { isActive = true; }
    void Deactivate() { isActive = false; Reset(); }

private:
    bool isActive = false;
    bool isDrawing = false;
    Vector2 startPoint { 0, 0 };
    Vector2 endPoint { 0, 0 };

    // Wall properties
    SectorID targetSector = NULL_SECTOR;
    SectorID portalToSector = NULL_SECTOR;
    Color wallColor = WHITE;
    TextureID wallTexture = NULL_TEXTURE;
    float textureScale = 1.0f;

    // Grid snapping
    bool snapToGrid = true;
    float gridSize = 50.0f;

    void Reset();
    Vector2 SnapToGrid(Vector2 pos) const;
    void CreateWall(World& world);
};
