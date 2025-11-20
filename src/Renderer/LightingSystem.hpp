#pragma once

#include <raylib.h>
#include <vector>
#include <unordered_map>
#include "Renderer/RaycastingMath.hpp"

using LightID = uint32_t;
constexpr LightID NULL_LIGHT { static_cast<LightID>(-1) };

enum class LightType
{
    Point,        // Omnidirectional point light
    Spot,         // Directional spotlight
    Directional,  // Parallel directional light
    Ambient       // Global ambient light
};

struct Light
{
    LightID id = NULL_LIGHT;
    LightType type = LightType::Point;

    // Transform
    Vector2 position { 0, 0 };
    float elevation = 100.0f;

    // Light properties
    Color color = WHITE;
    float intensity = 1.0f;
    float radius = 500.0f;          // Light radius/range
    float falloffExponent = 2.0f;   // Falloff power (2 = inverse square)

    // Spotlight properties
    Vector2 direction { 1, 0 };
    float spotAngle = 45.0f;        // Cone angle in degrees
    float spotSoftness = 0.5f;      // Edge softness

    // State
    bool isActive = true;
    bool castsShadows = true;

    // Sector association
    SectorID sectorId = NULL_SECTOR;
};

struct LightingResult
{
    Color finalColor;
    float brightness;  // 0-1
};

class LightingSystem
{
public:
    LightingSystem() = default;

    // Light management
    LightID AddLight(const Light& light);
    void RemoveLight(LightID id);
    Light* GetLight(LightID id);
    const Light* GetLight(LightID id) const;

    // Calculate lighting for a point
    LightingResult CalculateLighting(Vector2 position, float positionZ, const Vector2& normal, const struct World& world) const;

    // Calculate light contribution from a single light
    float CalculateLightContribution(const Light& light, Vector2 position, float positionZ, const Vector2& normal) const;

    // Ambient lighting
    void SetAmbientLight(Color color, float intensity);
    Color GetAmbientColor() const { return ambientColor; }
    float GetAmbientIntensity() const { return ambientIntensity; }

    // Get all lights
    const std::unordered_map<LightID, Light>& GetAllLights() const { return lights; }

private:
    std::unordered_map<LightID, Light> lights;
    LightID nextLightId = 0;

    Color ambientColor = { 30, 30, 40, 255 };
    float ambientIntensity = 0.2f;
};
