#include "Editor/WallCreationTool.hpp"
#include "Editor/WorldEditor.hpp"
#include <imgui.h>

void WallCreationTool::Update(float dt, WorldEditor& editor)
{
    if (!isActive) return;

    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseViewport = editor.ScreenToViewportPosition(mouseScreen);
    Vector2 mouseWorld = editor.ScreenToWorldPosition(mouseViewport);

    if (snapToGrid)
    {
        mouseWorld = SnapToGrid(mouseWorld);
    }

    // Start drawing wall
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isDrawing)
    {
        startPoint = mouseWorld;
        isDrawing = true;
    }

    // Update end point while drawing
    if (isDrawing)
    {
        endPoint = mouseWorld;
    }

    // Finish drawing wall
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDrawing)
    {
        CreateWall(editor.world);
        isDrawing = false;
    }

    // Cancel with right click
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        isDrawing = false;
    }

    // Escape to deactivate
    if (IsKeyPressed(KEY_ESCAPE))
    {
        Deactivate();
    }
}

void WallCreationTool::Render() const
{
    if (!isActive) return;

    if (isDrawing)
    {
        DrawLineV(startPoint, endPoint, GREEN);
        DrawCircleV(startPoint, 5.0f, RED);
        DrawCircleV(endPoint, 5.0f, BLUE);
    }
}

void WallCreationTool::DrawGUI()
{
    ImGui::Begin("Wall Creation Tool");

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
        ImGui::Text("Left Click: Start/End Wall");
        ImGui::Text("Right Click: Cancel");
        ImGui::Text("Escape: Deactivate");

        ImGui::Separator();
        ImGui::Checkbox("Snap to Grid", &snapToGrid);
        if (snapToGrid)
        {
            ImGui::SliderFloat("Grid Size", &gridSize, 10.0f, 200.0f);
        }

        ImGui::Separator();
        ImGui::Text("Wall Properties:");

        // Target sector selection
        int sectorId = static_cast<int>(targetSector);
        if (ImGui::InputInt("Target Sector", &sectorId))
        {
            targetSector = static_cast<SectorID>(sectorId);
        }

        // Portal connection
        int portalId = static_cast<int>(portalToSector);
        if (ImGui::InputInt("Portal to Sector (-1 = solid)", &portalId))
        {
            if (portalId < 0)
                portalToSector = NULL_SECTOR;
            else
                portalToSector = static_cast<SectorID>(portalId);
        }

        ImGui::ColorEdit3("Wall Color", (float*)&wallColor);

        int texId = static_cast<int>(wallTexture);
        if (ImGui::InputInt("Texture ID (-1 = none)", &texId))
        {
            if (texId < 0)
                wallTexture = NULL_TEXTURE;
            else
                wallTexture = static_cast<TextureID>(texId);
        }

        if (wallTexture != NULL_TEXTURE)
        {
            ImGui::SliderFloat("Texture Scale", &textureScale, 0.1f, 10.0f);
        }
    }

    ImGui::End();
}

void WallCreationTool::Reset()
{
    isDrawing = false;
    startPoint = { 0, 0 };
    endPoint = { 0, 0 };
}

Vector2 WallCreationTool::SnapToGrid(Vector2 pos) const
{
    return {
        roundf(pos.x / gridSize) * gridSize,
        roundf(pos.y / gridSize) * gridSize
    };
}

void WallCreationTool::CreateWall(World& world)
{
    if (targetSector == NULL_SECTOR) return;

    auto it = world.Sectors.find(targetSector);
    if (it == world.Sectors.end()) return;

    Wall newWall;
    newWall.segment.a = startPoint;
    newWall.segment.b = endPoint;
    newWall.toSector = portalToSector;
    newWall.color = wallColor;
    newWall.textureId = wallTexture;
    newWall.textureScale = textureScale;

    it->second.walls.push_back(newWall);
    RearrangeWallListToPolygon(it->second.walls);
}
