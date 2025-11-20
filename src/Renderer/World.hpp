#pragma once

#include <unordered_map>
#include <vector>

#include "RaycastingMath.hpp"
#include "Entity.hpp"

struct World
{
    World();

    std::unordered_map<SectorID, Sector> Sectors = 
    {
        {
            3,
            {
                .walls = {
                    {
                        .segment = { { 500, 600 }, { 500, 700 } }, 
                        .toSector = 1U
                    },
                    {
                        .segment = { { 500, 600 }, { 300, 600 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 300, 600 }, { 300, 700 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 300, 700 }, { 500, 700 } }, 
                        .color = WHITE,
                    },
                },
                .zFloor = 0.95
            }
        },
        {
            1, 
            {
                .walls = {
                    {
                        .segment = { { 500, 500 }, { 500, 600 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 500, 700 }, { 500, 600 } }, 
                        .toSector = 3U,
                        .color = PURPLE,
                    },
                    { 
                        .segment = { { 500, 700 }, { 700, 700 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 500, 500 }, { 700, 700 } },
                        .toSector = 2U
                    },
                }
            },
        },
        { 
            2, 
            {
                .walls = {
                    { 
                        .segment = { { 700, 700 }, { 700, 500 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 700, 500 }, { 500, 500 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 700, 700 }, { 500, 500 } },
                        .toSector = 1U
                    },
                },
                .floorColor = RED,
                .ceilingColor = BLUE,
                .zCeiling = 0.60f,
                .zFloor = 0.75f,
            },
        },
    };

    // Entity management
    std::unordered_map<EntityID, Entity> Entities;
    EntityID nextEntityId = 0;

    EntityID AddEntity(const Entity& entity);
    void RemoveEntity(EntityID id);
    Entity* GetEntity(EntityID id);
    const Entity* GetEntity(EntityID id) const;
    std::vector<Entity*> GetEntitiesInSector(SectorID sectorId);

    void InitWorld();
};

void RearrangeWallListToPolygon(std::vector<Wall>& walls);
uint32_t FindSectorOfPoint(Vector2 point, const World& world);