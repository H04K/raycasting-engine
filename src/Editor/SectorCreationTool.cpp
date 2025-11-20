#include "Editor/SectorCreationTool.hpp"
#include "Editor/WorldEditor.hpp"
#include <imgui.h>
#include <rlImGui.h>

void SectorCreationTool::Update(float dt, WorldEditor& editor)
{
    if (!IsActive()) return;

    // Get mouse position in world space
    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseViewport = editor.ScreenToViewportPosition(mouseScreen);
    Vector2 mouseWorld = editor.ScreenToWorldPosition(mouseViewport);

    if (snapToGrid)
    {
        mouseWorld = SnapToGrid(mouseWorld);
    }

    // Left click to add point
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        AddPoint(mouseWorld);
    }

    // Right click to remove last point
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        RemoveLastPoint();
    }

    // Press Enter to complete sector
    if (IsKeyPressed(KEY_ENTER) && points.size() >= 3)
    {
        CompleteSector(editor.world);
    }

    // Press Escape to cancel
    if (IsKeyPressed(KEY_ESCAPE))
    {
        Deactivate();
    }
}

void SectorCreationTool::Render() const
{
    if (!IsActive() || points.empty()) return;

    // Draw lines between points
    for (size_t i = 0; i < points.size(); ++i)
    {
        Vector2 p1 = points[i];
        Vector2 p2 = (i + 1 < points.size()) ? points[i + 1] : points[0];

        Color lineColor = (i + 1 < points.size()) ? GREEN : YELLOW;
        DrawLineV(p1, p2, lineColor);
        DrawCircleV(p1, 5.0f, RED);
    }

    // Draw preview line to mouse
    if (!points.empty())
    {
        Vector2 lastPoint = points.back();
        Vector2 mousePos = GetMousePosition();
        DrawLineV(lastPoint, mousePos, ColorAlpha(YELLOW, 0.5f));
    }
}

void SectorCreationTool::DrawGUI()
{
    ImGui::Begin("Sector Creation Tool");

    if (ImGui::Button(IsActive() ? "Deactivate" : "Activate"))
    {
        if (IsActive())
            Deactivate();
        else
            Activate();
    }

    if (IsActive())
    {
        ImGui::Separator();
        ImGui::Text("Points: %zu", points.size());
        ImGui::Text("Left Click: Add Point");
        ImGui::Text("Right Click: Remove Last");
        ImGui::Text("Enter: Complete Sector");
        ImGui::Text("Escape: Cancel");

        ImGui::Separator();
        ImGui::Checkbox("Snap to Grid", &snapToGrid);
        if (snapToGrid)
        {
            ImGui::SliderFloat("Grid Size", &gridSize, 10.0f, 200.0f);
        }

        ImGui::Separator();
        ImGui::Text("New Sector Properties:");
        ImGui::ColorEdit3("Floor Color", (float*)&floorColor);
        ImGui::ColorEdit3("Ceiling Color", (float*)&ceilingColor);
        ImGui::ColorEdit3("Top Border", (float*)&topBorderColor);
        ImGui::ColorEdit3("Bottom Border", (float*)&bottomBorderColor);
        ImGui::SliderFloat("Ceiling Height", &zCeiling, 0.0f, 1.0f);
        ImGui::SliderFloat("Floor Height", &zFloor, 0.0f, 1.0f);

        if (ImGui::Button("Clear Points"))
        {
            Clear();
        }

        if (points.size() >= 3)
        {
            ImGui::SameLine();
            if (ImGui::Button("Complete Sector"))
            {
                // Will be called with world reference
            }
        }
    }

    ImGui::End();
}

void SectorCreationTool::AddPoint(Vector2 worldPos)
{
    points.push_back(worldPos);
}

void SectorCreationTool::RemoveLastPoint()
{
    if (!points.empty())
    {
        points.pop_back();
    }
}

void SectorCreationTool::CompleteSector(World& world)
{
    if (points.size() < 3) return;

    // Create walls from points
    std::vector<Wall> walls;
    for (size_t i = 0; i < points.size(); ++i)
    {
        Vector2 p1 = points[i];
        Vector2 p2 = (i + 1 < points.size()) ? points[i + 1] : points[0];

        Wall wall;
        wall.segment.a = p1;
        wall.segment.b = p2;
        wall.color = WHITE;
        wall.toSector = NULL_SECTOR;
        walls.push_back(wall);
    }

    // Create new sector
    Sector newSector;
    newSector.walls = walls;
    newSector.floorColor = floorColor;
    newSector.ceilingColor = ceilingColor;
    newSector.topBorderColor = topBorderColor;
    newSector.bottomBorderColor = bottomBorderColor;
    newSector.zCeiling = zCeiling;
    newSector.zFloor = zFloor;

    // Add to world
    SectorID newId = 0;
    if (!world.Sectors.empty())
    {
        newId = world.Sectors.rbegin()->first + 1;
    }
    world.Sectors[newId] = newSector;

    // Rearrange walls
    RearrangeWallListToPolygon(world.Sectors[newId].walls);

    Clear();
}

void SectorCreationTool::Clear()
{
    points.clear();
}

Vector2 SectorCreationTool::SnapToGrid(Vector2 pos) const
{
    return {
        roundf(pos.x / gridSize) * gridSize,
        roundf(pos.y / gridSize) * gridSize
    };
}
