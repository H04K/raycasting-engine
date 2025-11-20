#include "Project/ProjectCreator.hpp"
#include <raylib.h>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

bool ProjectCreator::CreateProject(const std::string& projectName, const std::string& projectPath, const ProjectTemplate& templateConfig)
{
    // Validate inputs
    if (!IsValidProjectName(projectName))
    {
        lastError = "Invalid project name";
        return false;
    }

    if (!IsValidPath(projectPath))
    {
        lastError = "Invalid project path";
        return false;
    }

    // Create directory structure
    if (!CreateDirectoryStructure(projectPath))
    {
        lastError = "Failed to create directory structure";
        return false;
    }

    // Create project files
    if (!CreateCMakeFile(projectPath, projectName))
    {
        lastError = "Failed to create CMakeLists.txt";
        return false;
    }

    if (!CreateMainFile(projectPath, templateConfig))
    {
        lastError = "Failed to create main.cpp";
        return false;
    }

    if (!CreateDefaultAssets(projectPath, templateConfig))
    {
        lastError = "Failed to create default assets";
        return false;
    }

    if (!CreateReadme(projectPath, projectName))
    {
        lastError = "Failed to create README.md";
        return false;
    }

    if (!CreateGitIgnore(projectPath))
    {
        lastError = "Failed to create .gitignore";
        return false;
    }

    return true;
}

std::vector<ProjectTemplate> ProjectCreator::GetAvailableTemplates()
{
    return {
        {
            "Empty Project",
            "A minimal project with basic engine setup",
            false, false, false
        },
        {
            "Basic Game",
            "Includes example map and basic physics",
            true, true, false
        },
        {
            "Full Template",
            "Complete template with map, physics, and entities",
            true, true, true
        }
    };
}

bool ProjectCreator::IsValidProjectName(const std::string& name)
{
    if (name.empty()) return false;
    if (name.length() > 64) return false;

    // Check for valid characters (alphanumeric, dash, underscore)
    return std::all_of(name.begin(), name.end(), [](char c) {
        return std::isalnum(c) || c == '-' || c == '_';
    });
}

bool ProjectCreator::IsValidPath(const std::string& path)
{
    if (path.empty()) return false;

    try
    {
        fs::path p(path);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool ProjectCreator::CreateDirectoryStructure(const std::string& projectPath)
{
    try
    {
        fs::create_directories(projectPath);
        fs::create_directories(projectPath + "/src");
        fs::create_directories(projectPath + "/assets");
        fs::create_directories(projectPath + "/assets/textures");
        fs::create_directories(projectPath + "/assets/maps");
        fs::create_directories(projectPath + "/build");
        return true;
    }
    catch (const std::exception& e)
    {
        lastError = e.what();
        return false;
    }
}

bool ProjectCreator::CreateCMakeFile(const std::string& projectPath, const std::string& projectName)
{
    std::string cmakeContent = R"(cmake_minimum_required(VERSION 3.21)
project()" + projectName + R"()

set(CMAKE_CXX_STANDARD 20)

# Find raylib
find_package(raylib CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Add raycasting-engine as a library or subdirectory
# add_subdirectory(raycasting-engine)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE raylib
    PRIVATE imgui::imgui
    # PRIVATE raycasting-engine
)

# Copy assets to build directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
)";

    std::ofstream file(projectPath + "/CMakeLists.txt");
    if (!file.is_open()) return false;

    file << cmakeContent;
    file.close();
    return true;
}

bool ProjectCreator::CreateMainFile(const std::string& projectPath, const ProjectTemplate& config)
{
    std::string mainContent = R"(#include <raylib.h>
#include <iostream>

// Include raycasting-engine headers
// #include "Renderer/World.hpp"
// #include "Renderer/RaycastingCamera.hpp"
// #include "Renderer/WorldRasterizer.hpp"

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, ")" + std::string("Raycasting Game") + R"(");
    SetTargetFPS(60);

    // Initialize your game here
    // World world;
    // RaycastingCamera camera;

    while (!WindowShouldClose())
    {
        // Update
        float dt = GetFrameTime();

        // Render
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Raycasting Engine Project", 20, 20, 20, WHITE);
        DrawFPS(10, 10);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
)";

    std::ofstream file(projectPath + "/src/main.cpp");
    if (!file.is_open()) return false;

    file << mainContent;
    file.close();
    return true;
}

bool ProjectCreator::CreateDefaultAssets(const std::string& projectPath, const ProjectTemplate& config)
{
    // Create a placeholder texture file
    std::ofstream placeholderFile(projectPath + "/assets/textures/README.txt");
    if (placeholderFile.is_open())
    {
        placeholderFile << "Place your texture files here (.png, .jpg, etc.)\n";
        placeholderFile.close();
    }

    // Create a placeholder map file
    std::ofstream mapFile(projectPath + "/assets/maps/README.txt");
    if (mapFile.is_open())
    {
        mapFile << "Place your map files here (.map)\n";
        mapFile.close();
    }

    return true;
}

bool ProjectCreator::CreateReadme(const std::string& projectPath, const std::string& projectName)
{
    std::string readmeContent = "# " + projectName + R"(

A raycasting game project created with the Raycasting Engine.

## Building

1. Configure CMake:
   ```bash
   cmake -B build
   ```

2. Build:
   ```bash
   cmake --build build
   ```

3. Run:
   ```bash
   ./build/)" + projectName + R"(
   ```

## Project Structure

- `src/` - Source code
- `assets/` - Game assets (textures, maps, etc.)
- `build/` - Build output directory

## License

Your license here.
)";

    std::ofstream file(projectPath + "/README.md");
    if (!file.is_open()) return false;

    file << readmeContent;
    file.close();
    return true;
}

bool ProjectCreator::CreateGitIgnore(const std::string& projectPath)
{
    std::string gitignoreContent = R"(# Build directories
build/
out/
cmake-build-*/

# IDE files
.vscode/
.idea/
*.swp
*.swo
*~

# OS files
.DS_Store
Thumbs.db

# Compiled files
*.exe
*.o
*.obj
*.a
*.lib
*.so
*.dylib
)";

    std::ofstream file(projectPath + "/.gitignore");
    if (!file.is_open()) return false;

    file << gitignoreContent;
    file.close();
    return true;
}
