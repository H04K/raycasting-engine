#include "Renderer/LightingSystem.hpp"
#include "Renderer/World.hpp"
#include <cmath>
#include <algorithm>

LightID LightingSystem::AddLight(const Light& light)
{
    LightID id = nextLightId++;
    Light newLight = light;
    newLight.id = id;
    lights[id] = newLight;
    return id;
}

void LightingSystem::RemoveLight(LightID id)
{
    lights.erase(id);
}

Light* LightingSystem::GetLight(LightID id)
{
    auto it = lights.find(id);
    if (it != lights.end())
    {
        return &it->second;
    }
    return nullptr;
}

const Light* LightingSystem::GetLight(LightID id) const
{
    auto it = lights.find(id);
    if (it != lights.end())
    {
        return &it->second;
    }
    return nullptr;
}

void LightingSystem::SetAmbientLight(Color color, float intensity)
{
    ambientColor = color;
    ambientIntensity = Clamp(intensity, 0.0f, 1.0f);
}

LightingResult LightingSystem::CalculateLighting(Vector2 position, float positionZ, const Vector2& normal, const World& world) const
{
    LightingResult result;

    // Start with ambient light
    float r = ambientColor.r * ambientIntensity;
    float g = ambientColor.g * ambientIntensity;
    float b = ambientColor.b * ambientIntensity;
    float totalBrightness = ambientIntensity;

    // Accumulate contributions from all active lights
    for (const auto& [id, light] : lights)
    {
        if (!light.isActive) continue;

        float contribution = CalculateLightContribution(light, position, positionZ, normal);

        if (contribution > 0.0f)
        {
            r += light.color.r * light.intensity * contribution;
            g += light.color.g * light.intensity * contribution;
            b += light.color.b * light.intensity * contribution;
            totalBrightness += contribution;
        }
    }

    // Clamp final color
    result.finalColor.r = static_cast<unsigned char>(Clamp(r, 0.0f, 255.0f));
    result.finalColor.g = static_cast<unsigned char>(Clamp(g, 0.0f, 255.0f));
    result.finalColor.b = static_cast<unsigned char>(Clamp(b, 0.0f, 255.0f));
    result.finalColor.a = 255;
    result.brightness = Clamp(totalBrightness, 0.0f, 1.0f);

    return result;
}

float LightingSystem::CalculateLightContribution(const Light& light, Vector2 position, float positionZ, const Vector2& normal) const
{
    if (light.type == LightType::Ambient)
    {
        return light.intensity;
    }

    // Calculate distance to light
    Vector2 toLight = Vector2Subtract(light.position, position);
    float distance2D = Vector2Length(toLight);
    float distanceZ = light.elevation - positionZ;
    float distance3D = sqrtf(distance2D * distance2D + distanceZ * distanceZ);

    // Check if within light radius
    if (distance3D > light.radius) return 0.0f;

    // Calculate attenuation based on distance
    float attenuation = 1.0f - powf(distance3D / light.radius, light.falloffExponent);
    attenuation = Clamp(attenuation, 0.0f, 1.0f);

    // For point lights
    if (light.type == LightType::Point)
    {
        // Calculate angle between surface normal and light direction
        if (distance2D > 0.001f)
        {
            Vector2 lightDir = Vector2Normalize(toLight);
            float angle = Vector2DotProduct(normal, lightDir);
            angle = Clamp(angle, 0.0f, 1.0f); // Only positive contributions

            return attenuation * angle * light.intensity;
        }
        return attenuation * light.intensity;
    }

    // For spotlights
    if (light.type == LightType::Spot)
    {
        if (distance2D > 0.001f)
        {
            Vector2 lightDir = Vector2Normalize(toLight);

            // Check if point is within spotlight cone
            float angle = Vector2DotProduct(Vector2Negate(lightDir), light.direction);
            float spotCutoff = cosf(light.spotAngle * DEG2RAD);

            if (angle < spotCutoff)
            {
                return 0.0f; // Outside spotlight cone
            }

            // Calculate spotlight falloff
            float spotFalloff = (angle - spotCutoff) / (1.0f - spotCutoff);
            spotFalloff = powf(spotFalloff, 1.0f / light.spotSoftness);

            // Surface angle
            float surfaceAngle = Vector2DotProduct(normal, lightDir);
            surfaceAngle = Clamp(surfaceAngle, 0.0f, 1.0f);

            return attenuation * spotFalloff * surfaceAngle * light.intensity;
        }
    }

    // For directional lights
    if (light.type == LightType::Directional)
    {
        Vector2 lightDir = Vector2Normalize(light.direction);
        float angle = Vector2DotProduct(normal, Vector2Negate(lightDir));
        angle = Clamp(angle, 0.0f, 1.0f);
        return angle * light.intensity;
    }

    return 0.0f;
}
