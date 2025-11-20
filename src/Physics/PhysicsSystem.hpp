#pragma once

#include "Renderer/World.hpp"
#include "Renderer/Entity.hpp"
#include "Renderer/RaycastingCamera.hpp"

struct PhysicsConfig
{
    float gravity = 980.0f;              // Gravity acceleration (units/s²)
    float groundFriction = 0.9f;         // Ground friction coefficient
    float airFriction = 0.99f;           // Air resistance
    float terminalVelocity = 1000.0f;    // Maximum falling speed
    float groundSnapDistance = 10.0f;    // Distance to snap to ground
};

class PhysicsSystem
{
public:
    PhysicsSystem() = default;

    void Update(World& world, float deltaTime);
    void UpdateEntity(Entity& entity, const World& world, float deltaTime);
    void UpdateCamera(RaycastingCamera& camera, const World& world, float deltaTime);

    // Collision detection
    bool IsGrounded(const Entity& entity, const World& world);
    bool IsGrounded(const RaycastingCamera& camera, const World& world);
    float GetFloorHeight(Vector2 position, SectorID sectorId, const World& world);
    float GetCeilingHeight(Vector2 position, SectorID sectorId, const World& world);

    // Physics configuration
    PhysicsConfig& GetConfig() { return config; }
    const PhysicsConfig& GetConfig() const { return config; }

private:
    PhysicsConfig config;

    void ApplyGravity(Entity& entity, float deltaTime);
    void ApplyFriction(Entity& entity, bool isGrounded, float deltaTime);
    void ResolveFloorCollision(Entity& entity, const World& world);
    void ResolveCeilingCollision(Entity& entity, const World& world);
};
