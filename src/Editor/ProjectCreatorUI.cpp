#include "Editor/ProjectCreatorUI.hpp"
#include <imgui.h>

void ProjectCreatorUI::DrawGUI()
{
    if (!isActive) return;

    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Project Creator", &isActive))
    {
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("Create a new raycasting game project in one click!");
    ImGui::Separator();

    // Project settings
    RenderProjectSettings();

    ImGui::Separator();

    // Template selection
    RenderTemplateSelection();

    ImGui::Separator();

    // Create button
    if (ImGui::Button("Create Project", ImVec2(-1, 40)))
    {
        CreateProject();
    }

    // Status messages
    if (showSuccessMessage)
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Success!");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }

    if (showErrorMessage)
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error!");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }

    ImGui::End();
}

void ProjectCreatorUI::RenderProjectSettings()
{
    ImGui::Text("Project Settings:");

    ImGui::InputText("Project Name", projectNameBuffer, sizeof(projectNameBuffer));

    if (!ProjectCreator::IsValidProjectName(projectNameBuffer))
    {
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Invalid project name (use alphanumeric, -, _)");
    }

    ImGui::InputText("Project Path", projectPathBuffer, sizeof(projectPathBuffer));

    if (ImGui::Button("Browse..."))
    {
        // File dialog would go here in a real implementation
        ImGui::TextWrapped("File browser not implemented - enter path manually");
    }

    ImGui::TextWrapped("Full path: %s/%s", projectPathBuffer, projectNameBuffer);
}

void ProjectCreatorUI::RenderTemplateSelection()
{
    ImGui::Text("Select Template:");

    auto templates = ProjectCreator::GetAvailableTemplates();

    for (size_t i = 0; i < templates.size(); ++i)
    {
        const auto& templ = templates[i];

        ImGui::PushID(i);

        bool isSelected = (selectedTemplate == static_cast<int>(i));

        if (ImGui::Selectable(templ.name.c_str(), isSelected, 0, ImVec2(0, 0)))
        {
            selectedTemplate = static_cast<int>(i);
        }

        if (isSelected)
        {
            ImGui::Indent();
            ImGui::TextWrapped("%s", templ.description.c_str());

            ImGui::Text("Includes:");
            if (templ.includeExampleMap)
                ImGui::BulletText("Example map");
            if (templ.includePhysics)
                ImGui::BulletText("Physics system");
            if (templ.includeEntities)
                ImGui::BulletText("Entity system");

            ImGui::Unindent();
        }

        ImGui::PopID();
    }
}

void ProjectCreatorUI::CreateProject()
{
    showSuccessMessage = false;
    showErrorMessage = false;

    std::string projectName = projectNameBuffer;
    std::string projectPath = std::string(projectPathBuffer) + "/" + projectName;

    auto templates = ProjectCreator::GetAvailableTemplates();
    if (selectedTemplate < 0 || selectedTemplate >= static_cast<int>(templates.size()))
    {
        statusMessage = "Invalid template selection";
        showErrorMessage = true;
        return;
    }

    const auto& selectedTemplateConfig = templates[selectedTemplate];

    bool success = creator.CreateProject(projectName, projectPath, selectedTemplateConfig);

    if (success)
    {
        statusMessage = "Project created successfully at:\n" + projectPath +
                       "\n\nNext steps:\n"
                       "1. Navigate to the project directory\n"
                       "2. Run: cmake -B build\n"
                       "3. Run: cmake --build build\n"
                       "4. Run your game!";
        showSuccessMessage = true;
    }
    else
    {
        statusMessage = "Failed to create project: " + creator.GetLastError();
        showErrorMessage = true;
    }
}
