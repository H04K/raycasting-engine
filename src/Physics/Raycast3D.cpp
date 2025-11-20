#include "Physics/Raycast3D.hpp"
#include <limits>
#include <cmath>

RaycastHit3D Raycast3D::Cast(const Ray3D& ray, const World& world, bool hitEntities)
{
    RaycastHit3D closestHit;
    closestHit.hit = false;
    closestHit.distance = std::numeric_limits<float>::max();

    // Create a 2D ray for initial wall testing
    RasterRay ray2D = {
        .position = ray.origin2D,
        .direction = ray.direction2D
    };

    // Test against all sectors
    for (const auto& [sectorId, sector] : world.Sectors)
    {
        // Get sector floor and ceiling heights
        float floorZ = sector.zFloor * 1000.0f;
        float ceilingZ = sector.zCeiling * 1000.0f;

        // Test each wall in the sector
        for (const Wall& wall : sector.walls)
        {
            RaycastHit3D hit;
            if (RayIntersectsWall(ray, wall, floorZ, ceilingZ, hit))
            {
                if (hit.distance < closestHit.distance && hit.distance <= ray.maxDistance)
                {
                    closestHit = hit;
                    closestHit.hitType = HitType::Wall;
                    closestHit.wall = &wall;
                    closestHit.sectorId = sectorId;
                }
            }
        }
    }

    // Test against entities if requested
    if (hitEntities)
    {
        for (const auto& [id, entity] : world.Entities)
        {
            if (!entity.isActive || !entity.hasCollision) continue;

            RaycastHit3D hit;
            if (RayIntersectsEntity(ray, entity, hit))
            {
                if (hit.distance < closestHit.distance && hit.distance <= ray.maxDistance)
                {
                    closestHit = hit;
                    closestHit.hitType = HitType::Entity;
                    closestHit.entity = &entity;
                }
            }
        }
    }

    return closestHit;
}

std::vector<RaycastHit3D> Raycast3D::CastAll(const Ray3D& ray, const World& world, bool hitEntities)
{
    std::vector<RaycastHit3D> hits;

    // Test against all sectors
    for (const auto& [sectorId, sector] : world.Sectors)
    {
        float floorZ = sector.zFloor * 1000.0f;
        float ceilingZ = sector.zCeiling * 1000.0f;

        for (const Wall& wall : sector.walls)
        {
            RaycastHit3D hit;
            if (RayIntersectsWall(ray, wall, floorZ, ceilingZ, hit))
            {
                if (hit.distance <= ray.maxDistance)
                {
                    hit.hitType = HitType::Wall;
                    hit.wall = &wall;
                    hit.sectorId = sectorId;
                    hits.push_back(hit);
                }
            }
        }
    }

    // Test against entities
    if (hitEntities)
    {
        for (const auto& [id, entity] : world.Entities)
        {
            if (!entity.isActive || !entity.hasCollision) continue;

            RaycastHit3D hit;
            if (RayIntersectsEntity(ray, entity, hit))
            {
                if (hit.distance <= ray.maxDistance)
                {
                    hit.hitType = HitType::Entity;
                    hit.entity = &entity;
                    hits.push_back(hit);
                }
            }
        }
    }

    // Sort by distance
    std::sort(hits.begin(), hits.end(), [](const RaycastHit3D& a, const RaycastHit3D& b) {
        return a.distance < b.distance;
    });

    return hits;
}

bool Raycast3D::HasLineOfSight(Vector2 from, float fromZ, Vector2 to, float toZ, const World& world)
{
    // Calculate ray direction
    Vector2 dir2D = Vector2Subtract(to, from);
    float distance2D = Vector2Length(dir2D);
    if (distance2D < 0.001f) return true;

    dir2D = Vector2Normalize(dir2D);
    float dirZ = (toZ - fromZ) / distance2D;

    Ray3D ray = {
        .origin2D = from,
        .originZ = fromZ,
        .direction2D = dir2D,
        .directionZ = dirZ,
        .maxDistance = distance2D
    };

    RaycastHit3D hit = Cast(ray, world, false);
    return !hit.hit || hit.distance >= distance2D;
}

std::vector<Entity*> Raycast3D::OverlapSphere(Vector2 center, float centerZ, float radius, World& world)
{
    std::vector<Entity*> result;

    for (auto& [id, entity] : world.Entities)
    {
        if (!entity.isActive) continue;

        // Calculate 2D distance
        float dist2D = Vector2Distance(center, entity.position);
        float distZ = fabsf(centerZ - entity.elevation);

        // Calculate 3D distance
        float dist3D = sqrtf(dist2D * dist2D + distZ * distZ);

        if (dist3D <= radius + entity.radius)
        {
            result.push_back(&entity);
        }
    }

    return result;
}

bool Raycast3D::IsPointInSectorBounds(Vector2 position, float positionZ, SectorID sectorId, const World& world)
{
    auto it = world.Sectors.find(sectorId);
    if (it == world.Sectors.end()) return false;

    const Sector& sector = it->second;
    float floorZ = sector.zFloor * 1000.0f;
    float ceilingZ = sector.zCeiling * 1000.0f;

    return positionZ >= floorZ && positionZ <= ceilingZ && IsPointInSector(position, sector);
}

bool Raycast3D::RayIntersectsWall(const Ray3D& ray, const Wall& wall, float wallFloorZ, float wallCeilingZ, RaycastHit3D& hit)
{
    // First test 2D intersection
    RasterRay ray2D = {
        .position = ray.origin2D,
        .direction = ray.direction2D
    };

    HitInfo hit2D;
    if (!RayToSegmentCollision(ray2D, wall.segment, hit2D))
    {
        return false;
    }

    // Calculate the Z coordinate at the hit point
    float hitZ = ray.originZ + ray.directionZ * hit2D.distance;

    // Check if the hit point is within the wall's vertical bounds
    if (hitZ < wallFloorZ || hitZ > wallCeilingZ)
    {
        // If this is a portal, the ray might pass through
        if (wall.toSector != NULL_SECTOR)
        {
            return false;
        }
        return false;
    }

    // Calculate normal (perpendicular to wall segment)
    Vector2 wallDir = Vector2Subtract(wall.segment.b, wall.segment.a);
    Vector2 normal = { -wallDir.y, wallDir.x };
    normal = Vector2Normalize(normal);

    hit.hit = true;
    hit.position2D = hit2D.position;
    hit.positionZ = hitZ;
    hit.distance = hit2D.distance;
    hit.normal = normal;

    return true;
}

bool Raycast3D::RayIntersectsEntity(const Ray3D& ray, const Entity& entity, RaycastHit3D& hit)
{
    // Simplified cylinder collision for entities
    // Test against entity's cylindrical collision volume

    Vector2 toEntity = Vector2Subtract(entity.position, ray.origin2D);
    float proj = Vector2DotProduct(toEntity, ray.direction2D);

    // Entity is behind the ray
    if (proj < 0) return false;

    // Find closest point on ray to entity center
    Vector2 closestPoint = Vector2Add(ray.origin2D, Vector2Scale(ray.direction2D, proj));
    float dist2D = Vector2Distance(closestPoint, entity.position);

    // Check if ray passes through entity's cylinder radius
    if (dist2D > entity.radius) return false;

    // Calculate Z at closest point
    float closestZ = ray.originZ + ray.directionZ * proj;

    // Check vertical bounds
    if (closestZ < entity.elevation || closestZ > entity.elevation + entity.spriteHeight)
    {
        return false;
    }

    // Calculate actual hit distance accounting for radius
    float hitDistance = proj - sqrtf(entity.radius * entity.radius - dist2D * dist2D);
    if (hitDistance < 0) hitDistance = 0;

    hit.hit = true;
    hit.position2D = Vector2Add(ray.origin2D, Vector2Scale(ray.direction2D, hitDistance));
    hit.positionZ = ray.originZ + ray.directionZ * hitDistance;
    hit.distance = hitDistance;
    hit.normal = Vector2Normalize(Vector2Subtract(hit.position2D, entity.position));

    return true;
}
