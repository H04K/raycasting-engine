#pragma once

#include <string>
#include <vector>

struct ProjectTemplate
{
    std::string name;
    std::string description;
    bool includeExampleMap;
    bool includePhysics;
    bool includeEntities;
};

class ProjectCreator
{
public:
    ProjectCreator() = default;

    // Create a new project
    bool CreateProject(const std::string& projectName, const std::string& projectPath, const ProjectTemplate& templateConfig);

    // Get available templates
    static std::vector<ProjectTemplate> GetAvailableTemplates();

    // Validation
    static bool IsValidProjectName(const std::string& name);
    static bool IsValidPath(const std::string& path);

    std::string GetLastError() const { return lastError; }

private:
    bool CreateDirectoryStructure(const std::string& projectPath);
    bool CreateCMakeFile(const std::string& projectPath, const std::string& projectName);
    bool CreateMainFile(const std::string& projectPath, const ProjectTemplate& config);
    bool CreateDefaultAssets(const std::string& projectPath, const ProjectTemplate& config);
    bool CreateReadme(const std::string& projectPath, const std::string& projectName);
    bool CreateGitIgnore(const std::string& projectPath);

private:
    std::string lastError;
};
