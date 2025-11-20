#include "Editor/PortalVisualizationTool.hpp"
#include "Editor/WorldEditor.hpp"
#include <imgui.h>

void PortalVisualizationTool::Update(float dt, WorldEditor& editor)
{
    if (!isActive) return;

    // Tool logic for selecting walls and creating portals
    // This would involve mouse picking in the editor
}

void PortalVisualizationTool::Render(const World& world) const
{
    if (!isActive) return;

    if (showPortalConnections)
    {
        RenderPortalConnections(world);
    }

    // Highlight selected wall
    if (selectedWall != nullptr)
    {
        RenderWallHighlight(*selectedWall, YELLOW);
    }
}

void PortalVisualizationTool::DrawGUI()
{
    ImGui::Begin("Portal Visualization Tool");

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
        ImGui::Checkbox("Show Portal Connections", &showPortalConnections);
        ImGui::Checkbox("Show Portal Directions", &showPortalDirections);

        ImGui::ColorEdit3("Portal Color", (float*)&portalConnectionColor);
        ImGui::ColorEdit3("Solid Wall Color", (float*)&solidWallColor);

        ImGui::Separator();
        ImGui::Text("Portal Creation:");

        int sectorFrom = static_cast<int>(selectedSectorId);
        if (ImGui::InputInt("From Sector", &sectorFrom))
        {
            selectedSectorId = static_cast<SectorID>(sectorFrom);
        }

        ImGui::InputInt("Wall Index", (int*)&selectedWallIndex);

        int sectorTo = static_cast<int>(targetSectorId);
        if (ImGui::InputInt("To Sector", &sectorTo))
        {
            targetSectorId = static_cast<SectorID>(sectorTo);
        }

        if (ImGui::Button("Create Portal"))
        {
            // Create portal - needs world reference
            ImGui::Text("Click in viewport to create portal");
        }

        if (ImGui::Button("Remove Portal"))
        {
            // Remove portal
            ImGui::Text("Click wall to remove portal");
        }

        ImGui::Separator();
        ImGui::Text("Instructions:");
        ImGui::BulletText("Select walls in the map editor");
        ImGui::BulletText("Set target sector ID");
        ImGui::BulletText("Click 'Create Portal' to connect");
        ImGui::BulletText("Green lines show portal connections");
    }

    ImGui::End();
}

void PortalVisualizationTool::CreatePortal(World& world, SectorID fromSector, size_t wallIndex, SectorID toSector)
{
    auto it = world.Sectors.find(fromSector);
    if (it == world.Sectors.end()) return;

    if (wallIndex >= it->second.walls.size()) return;

    it->second.walls[wallIndex].toSector = toSector;
}

void PortalVisualizationTool::RemovePortal(World& world, SectorID sectorId, size_t wallIndex)
{
    auto it = world.Sectors.find(sectorId);
    if (it == world.Sectors.end()) return;

    if (wallIndex >= it->second.walls.size()) return;

    it->second.walls[wallIndex].toSector = NULL_SECTOR;
}

void PortalVisualizationTool::RenderPortalConnections(const World& world) const
{
    for (const auto& [sectorId, sector] : world.Sectors)
    {
        for (const Wall& wall : sector.walls)
        {
            Vector2 midpoint = {
                (wall.segment.a.x + wall.segment.b.x) / 2.0f,
                (wall.segment.a.y + wall.segment.b.y) / 2.0f
            };

            if (wall.toSector != NULL_SECTOR)
            {
                // Draw portal connection
                auto targetIt = world.Sectors.find(wall.toSector);
                if (targetIt != world.Sectors.end())
                {
                    Vector2 targetCenter = FindInsidePoint(targetIt->second.walls);
                    DrawLineV(midpoint, targetCenter, portalConnectionColor);

                    if (showPortalDirections)
                    {
                        DrawCircleV(midpoint, 8.0f, portalConnectionColor);
                    }
                }
            }
            else
            {
                // Draw solid wall indicator
                DrawCircleV(midpoint, 4.0f, solidWallColor);
            }
        }
    }
}

void PortalVisualizationTool::RenderWallHighlight(const Wall& wall, Color color) const
{
    DrawLineEx(wall.segment.a, wall.segment.b, 3.0f, color);
    DrawCircleV(wall.segment.a, 6.0f, color);
    DrawCircleV(wall.segment.b, 6.0f, color);
}
