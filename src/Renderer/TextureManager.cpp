#include "Renderer/TextureManager.hpp"
#include <iostream>

TextureID TextureManager::LoadTexture(const std::string& path)
{
    // Check if texture is already loaded
    for (const auto& [id, data] : textures)
    {
        if (data.path == path)
        {
            return id;
        }
    }

    // Load new texture
    Texture2D texture = ::LoadTexture(path.c_str());

    if (texture.id == 0)
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return NULL_TEXTURE;
    }

    TextureID id = nextId++;
    textures[id] = TextureData{
        .texture = texture,
        .path = path,
        .width = texture.width,
        .height = texture.height
    };

    std::cout << "Loaded texture: " << path << " (ID: " << id << ")" << std::endl;
    return id;
}

void TextureManager::UnloadTexture(TextureID id)
{
    auto it = textures.find(id);
    if (it != textures.end())
    {
        ::UnloadTexture(it->second.texture);
        textures.erase(it);
    }
}

const TextureData* TextureManager::GetTexture(TextureID id) const
{
    auto it = textures.find(id);
    if (it != textures.end())
    {
        return &it->second;
    }
    return nullptr;
}

Texture2D TextureManager::GetTexture2D(TextureID id) const
{
    const TextureData* data = GetTexture(id);
    if (data)
    {
        return data->texture;
    }
    return Texture2D{0};
}

void TextureManager::UnloadAll()
{
    for (auto& [id, data] : textures)
    {
        ::UnloadTexture(data.texture);
    }
    textures.clear();
}
