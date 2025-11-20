#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include "Renderer/TextureManager.hpp"

struct TextureEntry
{
    TextureID id;
    std::string name;
    std::string path;
    int width;
    int height;
};

class TextureBrowser
{
public:
    TextureBrowser();

    void DrawGUI();
    void Update();

    bool IsActive() const { return isActive; }
    void Activate() { isActive = true; }
    void Deactivate() { isActive = false; }

    // Texture management
    TextureID LoadTexture(const std::string& path);
    void UnloadTexture(TextureID id);
    void RefreshTextureList();

    // Selection
    TextureID GetSelectedTexture() const { return selectedTextureId; }

private:
    bool isActive = false;
    TextureID selectedTextureId = NULL_TEXTURE;

    std::vector<TextureEntry> textureEntries;

    // UI state
    char filePathBuffer[256] = "";
    int thumbnailSize = 128;
    bool showTextureInfo = true;

    void RenderTextureGrid();
    void RenderTextureInfo();
    void LoadDefaultTextures();
};
