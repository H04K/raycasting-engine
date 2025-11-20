#pragma once

#include <raylib.h>
#include <vector>
#include "Renderer/World.hpp"
#include "Renderer/RaycastingMath.hpp"

class PortalVisualizationTool
{
public:
    PortalVisualizationTool() = default;

    void Update(float dt, class WorldEditor& editor);
    void Render(const World& world) const;
    void DrawGUI();

    bool IsActive() const { return isActive; }
    void Activate() { isActive = true; }
    void Deactivate() { isActive = false; selectedWall = nullptr; }

    // Portal creation/modification
    void CreatePortal(World& world, SectorID fromSector, size_t wallIndex, SectorID toSector);
    void RemovePortal(World& world, SectorID sectorId, size_t wallIndex);

private:
    bool isActive = false;

    // Selection
    SectorID selectedSectorId = NULL_SECTOR;
    Wall* selectedWall = nullptr;
    size_t selectedWallIndex = 0;

    // Portal creation
    SectorID targetSectorId = NULL_SECTOR;

    // Visualization options
    bool showPortalConnections = true;
    bool showPortalDirections = true;
    Color portalConnectionColor = GREEN;
    Color solidWallColor = RED;

    void RenderPortalConnections(const World& world) const;
    void RenderWallHighlight(const Wall& wall, Color color) const;
};
