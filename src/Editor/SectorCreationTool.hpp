#pragma once

#include <raylib.h>
#include <vector>
#include "Renderer/World.hpp"
#include "Renderer/RaycastingMath.hpp"
#include "Utils/ColorHelper.hpp"

enum class SectorCreationMode
{
    None,
    DrawingWalls,       // Drawing walls for a new sector
    EditingExisting     // Editing an existing sector
};

class SectorCreationTool
{
public:
    SectorCreationTool() = default;

    void Update(float dt, class WorldEditor& editor);
    void Render() const;
    void DrawGUI();

    // Tool state
    bool IsActive() const { return mode != SectorCreationMode::None; }
    void Activate() { mode = SectorCreationMode::DrawingWalls; }
    void Deactivate() { mode = SectorCreationMode::None; Clear(); }

    // Sector creation
    void AddPoint(Vector2 worldPos);
    void RemoveLastPoint();
    void CompleteSector(World& world);
    void Clear();

    // Get current state
    const std::vector<Vector2>& GetPoints() const { return points; }
    SectorCreationMode GetMode() const { return mode; }

private:
    SectorCreationMode mode = SectorCreationMode::None;
    std::vector<Vector2> points;

    // New sector properties
    Color floorColor = MY_DARK_BLUE;
    Color ceilingColor = MY_BEIGE;
    Color topBorderColor = MY_PURPLE;
    Color bottomBorderColor = MY_RED;
    float zCeiling = 1.0f;
    float zFloor = 1.0f;

    // Grid snapping
    bool snapToGrid = true;
    float gridSize = 50.0f;

    Vector2 SnapToGrid(Vector2 pos) const;
};
