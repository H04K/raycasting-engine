#include "Editor/TextureBrowser.hpp"
#include <imgui.h>
#include <rlImGui.h>

TextureBrowser::TextureBrowser()
{
    LoadDefaultTextures();
}

void TextureBrowser::Update()
{
    if (!isActive) return;

    // Update logic if needed
}

void TextureBrowser::DrawGUI()
{
    if (!ImGui::Begin("Texture Browser", &isActive))
    {
        ImGui::End();
        return;
    }

    // Load texture section
    ImGui::Text("Load New Texture:");
    ImGui::InputText("File Path", filePathBuffer, sizeof(filePathBuffer));
    ImGui::SameLine();

    if (ImGui::Button("Load"))
    {
        if (filePathBuffer[0] != '\0')
        {
            LoadTexture(std::string(filePathBuffer));
            filePathBuffer[0] = '\0'; // Clear buffer
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh List"))
    {
        RefreshTextureList();
    }

    ImGui::Separator();

    // Thumbnail size slider
    ImGui::SliderInt("Thumbnail Size", &thumbnailSize, 64, 256);
    ImGui::Checkbox("Show Info", &showTextureInfo);

    ImGui::Separator();

    // Texture count
    ImGui::Text("Loaded Textures: %zu", textureEntries.size());

    ImGui::Separator();

    // Texture grid
    RenderTextureGrid();

    // Selected texture info
    if (selectedTextureId != NULL_TEXTURE && showTextureInfo)
    {
        ImGui::Separator();
        RenderTextureInfo();
    }

    ImGui::End();
}

TextureID TextureBrowser::LoadTexture(const std::string& path)
{
    TextureID id = TextureManager::Instance().LoadTexture(path);

    if (id != NULL_TEXTURE)
    {
        RefreshTextureList();
    }

    return id;
}

void TextureBrowser::UnloadTexture(TextureID id)
{
    TextureManager::Instance().UnloadTexture(id);
    RefreshTextureList();

    if (selectedTextureId == id)
    {
        selectedTextureId = NULL_TEXTURE;
    }
}

void TextureBrowser::RefreshTextureList()
{
    textureEntries.clear();

    const auto& allTextures = TextureManager::Instance().GetAllTextures();
    for (const auto& [id, texData] : allTextures)
    {
        TextureEntry entry;
        entry.id = id;
        entry.path = texData.path;

        // Extract filename from path
        size_t lastSlash = texData.path.find_last_of("/\\");
        entry.name = (lastSlash != std::string::npos) ?
                     texData.path.substr(lastSlash + 1) :
                     texData.path;

        entry.width = texData.width;
        entry.height = texData.height;

        textureEntries.push_back(entry);
    }
}

void TextureBrowser::RenderTextureGrid()
{
    if (textureEntries.empty())
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No textures loaded");
        return;
    }

    float windowWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(windowWidth / (thumbnailSize + 10)));

    if (ImGui::BeginTable("TextureGrid", columns, ImGuiTableFlags_SizingStretchSame))
    {
        int column = 0;

        for (const auto& entry : textureEntries)
        {
            if (column == 0)
            {
                ImGui::TableNextRow();
            }

            ImGui::TableSetColumnIndex(column);

            ImGui::PushID(entry.id);

            bool isSelected = (entry.id == selectedTextureId);

            // Texture preview
            const TextureData* texData = TextureManager::Instance().GetTexture(entry.id);
            if (texData)
            {
                ImVec4 tintColor = isSelected ? ImVec4(1, 1, 0, 1) : ImVec4(1, 1, 1, 1);

                if (ImGui::ImageButton(
                    entry.name.c_str(),
                    (void*)(intptr_t)texData->texture.id,
                    ImVec2(thumbnailSize, thumbnailSize),
                    ImVec2(0, 0), ImVec2(1, 1),
                    ImVec4(0, 0, 0, 1),
                    tintColor))
                {
                    selectedTextureId = entry.id;
                }

                // Tooltip on hover
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("ID: %u", entry.id);
                    ImGui::Text("Name: %s", entry.name.c_str());
                    ImGui::Text("Size: %dx%d", entry.width, entry.height);
                    ImGui::EndTooltip();
                }
            }

            // Texture name
            ImGui::TextWrapped("%s", entry.name.c_str());

            ImGui::PopID();

            column = (column + 1) % columns;
        }

        ImGui::EndTable();
    }
}

void TextureBrowser::RenderTextureInfo()
{
    const TextureData* texData = TextureManager::Instance().GetTexture(selectedTextureId);
    if (!texData) return;

    ImGui::Text("Selected Texture Info:");
    ImGui::Separator();

    ImGui::Text("ID: %u", selectedTextureId);
    ImGui::Text("Path: %s", texData->path.c_str());
    ImGui::Text("Dimensions: %dx%d", texData->width, texData->height);

    // Large preview
    float previewSize = 256.0f;
    ImGui::Image(
        (void*)(intptr_t)texData->texture.id,
        ImVec2(previewSize, previewSize),
        ImVec2(0, 0), ImVec2(1, 1),
        ImVec4(1, 1, 1, 1),
        ImVec4(1, 1, 1, 1));

    if (ImGui::Button("Unload Texture"))
    {
        UnloadTexture(selectedTextureId);
    }

    ImGui::SameLine();
    if (ImGui::Button("Copy ID"))
    {
        // Would copy ID to clipboard in a real implementation
        ImGui::SetClipboardText(std::to_string(selectedTextureId).c_str());
    }
}

void TextureBrowser::LoadDefaultTextures()
{
    // Load some default/example textures if they exist
    const char* defaultPaths[] = {
        "ressources/textures/wall1.png",
        "ressources/textures/wall2.png",
        "ressources/textures/floor1.png",
        "ressources/textures/ceiling1.png"
    };

    for (const char* path : defaultPaths)
    {
        if (FileExists(path))
        {
            LoadTexture(path);
        }
    }
}
