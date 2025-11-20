#include "Physics/PhysicsSystem.hpp"
#include <algorithm>

void PhysicsSystem::Update(World& world, float deltaTime)
{
    // Update all entities with physics
    for (auto& [id, entity] : world.Entities)
    {
        if (!entity.isActive) continue;
        UpdateEntity(entity, world, deltaTime);
    }
}

void PhysicsSystem::UpdateEntity(Entity& entity, const World& world, float deltaTime)
{
    // Update sector based on position
    SectorID newSector = FindSectorOfPoint(entity.position, world);
    if (newSector != NULL_SECTOR)
    {
        entity.currentSectorId = newSector;
    }

    // Apply gravity if enabled
    if (entity.hasGravity)
    {
        ApplyGravity(entity, deltaTime);
    }

    // Apply friction
    bool isGrounded = IsGrounded(entity, world);
    ApplyFriction(entity, isGrounded, deltaTime);

    // Update position based on velocity
    entity.position.x += entity.velocity.x * deltaTime;
    entity.position.y += entity.velocity.y * deltaTime;
    entity.elevation += entity.verticalVelocity * deltaTime;

    // Resolve collisions
    if (entity.hasCollision)
    {
        ResolveFloorCollision(entity, world);
        ResolveCeilingCollision(entity, world);
    }
}

void PhysicsSystem::UpdateCamera(RaycastingCamera& camera, const World& world, float deltaTime)
{
    // Update camera sector
    SectorID newSector = FindSectorOfPoint(camera.position, world);
    if (newSector != NULL_SECTOR)
    {
        camera.currentSectorId = newSector;
    }

    // Simple camera grounding (no gravity, just snapping)
    if (camera.currentSectorId != NULL_SECTOR)
    {
        const Sector& sector = world.Sectors.at(camera.currentSectorId);
        float floorHeight = sector.zFloor * 1000.0f; // Assuming 1000 units = full height
        float ceilingHeight = sector.zCeiling * 1000.0f;

        // Keep camera within sector bounds
        camera.elevation = Clamp(camera.elevation, floorHeight + 50.0f, ceilingHeight - 50.0f);
    }
}

void PhysicsSystem::ApplyGravity(Entity& entity, float deltaTime)
{
    // Apply gravity acceleration
    entity.verticalVelocity -= config.gravity * deltaTime;

    // Clamp to terminal velocity
    entity.verticalVelocity = Clamp(entity.verticalVelocity, -config.terminalVelocity, config.terminalVelocity);
}

void PhysicsSystem::ApplyFriction(Entity& entity, bool isGrounded, float deltaTime)
{
    float frictionCoeff = isGrounded ? config.groundFriction : config.airFriction;

    // Apply friction to horizontal velocity
    entity.velocity.x *= frictionCoeff;
    entity.velocity.y *= frictionCoeff;

    // Stop very small velocities
    if (fabsf(entity.velocity.x) < 0.01f) entity.velocity.x = 0.0f;
    if (fabsf(entity.velocity.y) < 0.01f) entity.velocity.y = 0.0f;
}

void PhysicsSystem::ResolveFloorCollision(Entity& entity, const World& world)
{
    if (entity.currentSectorId == NULL_SECTOR) return;

    float floorHeight = GetFloorHeight(entity.position, entity.currentSectorId, world);

    if (entity.elevation <= floorHeight)
    {
        entity.elevation = floorHeight;
        entity.verticalVelocity = 0.0f;
    }
}

void PhysicsSystem::ResolveCeilingCollision(Entity& entity, const World& world)
{
    if (entity.currentSectorId == NULL_SECTOR) return;

    float ceilingHeight = GetCeilingHeight(entity.position, entity.currentSectorId, world);

    if (entity.elevation + entity.spriteHeight >= ceilingHeight)
    {
        entity.elevation = ceilingHeight - entity.spriteHeight;
        if (entity.verticalVelocity > 0)
        {
            entity.verticalVelocity = 0.0f;
        }
    }
}

bool PhysicsSystem::IsGrounded(const Entity& entity, const World& world)
{
    if (entity.currentSectorId == NULL_SECTOR) return false;

    float floorHeight = GetFloorHeight(entity.position, entity.currentSectorId, world);
    return fabsf(entity.elevation - floorHeight) < config.groundSnapDistance;
}

bool PhysicsSystem::IsGrounded(const RaycastingCamera& camera, const World& world)
{
    if (camera.currentSectorId == NULL_SECTOR) return false;

    const Sector& sector = world.Sectors.at(camera.currentSectorId);
    float floorHeight = sector.zFloor * 1000.0f;
    return fabsf(camera.elevation - floorHeight) < config.groundSnapDistance;
}

float PhysicsSystem::GetFloorHeight(Vector2 position, SectorID sectorId, const World& world)
{
    if (sectorId == NULL_SECTOR) return 0.0f;

    auto it = world.Sectors.find(sectorId);
    if (it == world.Sectors.end()) return 0.0f;

    const Sector& sector = it->second;
    return sector.zFloor * 1000.0f; // Convert normalized height to world units
}

float PhysicsSystem::GetCeilingHeight(Vector2 position, SectorID sectorId, const World& world)
{
    if (sectorId == NULL_SECTOR) return 1000.0f;

    auto it = world.Sectors.find(sectorId);
    if (it == world.Sectors.end()) return 1000.0f;

    const Sector& sector = it->second;
    return sector.zCeiling * 1000.0f; // Convert normalized height to world units
}
