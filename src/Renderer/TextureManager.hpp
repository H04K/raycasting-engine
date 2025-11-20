#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <memory>

using TextureID = uint32_t;
constexpr TextureID NULL_TEXTURE { static_cast<TextureID>(-1) };

struct TextureData
{
    Texture2D texture;
    std::string path;
    int width;
    int height;
};

class TextureManager
{
public:
    static TextureManager& Instance()
    {
        static TextureManager instance;
        return instance;
    }

    TextureID LoadTexture(const std::string& path);
    void UnloadTexture(TextureID id);
    const TextureData* GetTexture(TextureID id) const;
    Texture2D GetTexture2D(TextureID id) const;

    void UnloadAll();

    const std::unordered_map<TextureID, TextureData>& GetAllTextures() const { return textures; }

private:
    TextureManager() = default;
    ~TextureManager() { UnloadAll(); }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    std::unordered_map<TextureID, TextureData> textures;
    TextureID nextId = 0;
};
