#pragma once

#include "Project/ProjectCreator.hpp"
#include <string>

class ProjectCreatorUI
{
public:
    ProjectCreatorUI() = default;

    void DrawGUI();

    bool IsActive() const { return isActive; }
    void Show() { isActive = true; }
    void Hide() { isActive = false; }

private:
    bool isActive = false;

    // Project settings
    char projectNameBuffer[128] = "MyRaycastingGame";
    char projectPathBuffer[512] = "./";
    int selectedTemplate = 0;

    // UI state
    bool showSuccessMessage = false;
    bool showErrorMessage = false;
    std::string statusMessage;

    ProjectCreator creator;

    void RenderTemplateSelection();
    void RenderProjectSettings();
    void CreateProject();
};
