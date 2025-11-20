#pragma once

#include <raylib.h>
#include <vector>
#include "Renderer/World.hpp"
#include "Renderer/Entity.hpp"
#include "Renderer/RaycastingMath.hpp"

struct Ray3D
{
    Vector2 origin2D;         // XY origin
    float originZ;            // Z origin (elevation)
    Vector2 direction2D;      // XY direction (normalized)
    float directionZ;         // Z direction component
    float maxDistance;        // Maximum raycast distance
};

enum class HitType
{
    None,
    Wall,
    Entity,
    Floor,
    Ceiling
};

struct RaycastHit3D
{
    bool hit = false;
    HitType hitType = HitType::None;
    Vector2 position2D { 0, 0 };
    float positionZ = 0.0f;
    float distance = 0.0f;
    Vector2 normal { 0, 0 };

    // Hit information
    const Wall* wall = nullptr;
    const Entity* entity = nullptr;
    SectorID sectorId = NULL_SECTOR;
};

class Raycast3D
{
public:
    // Cast a ray in 3D space and return the first hit
    static RaycastHit3D Cast(const Ray3D& ray, const World& world, bool hitEntities = true);

    // Cast a ray and return all hits along the path
    static std::vector<RaycastHit3D> CastAll(const Ray3D& ray, const World& world, bool hitEntities = true);

    // Check if there's a clear line of sight between two points
    static bool HasLineOfSight(Vector2 from, float fromZ, Vector2 to, float toZ, const World& world);

    // Find all entities within a radius
    static std::vector<Entity*> OverlapSphere(Vector2 center, float centerZ, float radius, World& world);

    // Check if a point is inside a sector's floor/ceiling bounds
    static bool IsPointInSectorBounds(Vector2 position, float positionZ, SectorID sectorId, const World& world);

private:
    // Helper functions
    static bool RayIntersectsWall(const Ray3D& ray, const Wall& wall, float wallFloorZ, float wallCeilingZ, RaycastHit3D& hit);
    static bool RayIntersectsEntity(const Ray3D& ray, const Entity& entity, RaycastHit3D& hit);
};
