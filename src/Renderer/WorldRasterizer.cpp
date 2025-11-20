#include <limits>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "Renderer/WorldRasterizer.hpp"
#include "Renderer/RaycastingMath.hpp"
#include "Utils/ColorHelper.hpp"
#include "WorldRasterizer.hpp"

void RasterizeInRenderArea(RasterizeWorldContext& ctx, SectorRenderContext renderContext)
{
    std::unordered_map<SectorID, SectorRenderContext> renderAreaToPushInStack;

    const auto& [ sectorId, renderArea ] = renderContext;
    
    const Sector& currentSector = ctx.world->Sectors.at(sectorId);

    for(uint32_t x = renderArea.xBegin; x <= renderArea.xEnd; ++x)
    {
        MinMaxUint32& yMinMax = ctx.yBoundaries.at(x);

        float rayAngle = RayAngleForScreenXCam(x, *ctx.cam, ctx.RenderTargetWidth);

        RasterRay rasterRay = {
            .position = ctx.cam->position,
            .direction = Vector2DirectionFromAngle((rayAngle * DEG2RAD) + ctx.cam->yaw),
        };

        struct RaycastHitData
        {
            float distance = std::numeric_limits<float>::max();
            Vector2 position;
            const Wall* wall = nullptr;
        } bestHitData;

        for(const Wall& wall : currentSector.walls)
        {
            HitInfo hitInfo;
            if(RayToSegmentCollision(rasterRay, wall.segment, hitInfo))
            {
                if(wall.toSector != NULL_SECTOR 
                    && PointSegmentSide(ctx.cam->position, wall.segment.a, wall.segment.b) <= 0)
                {
                    continue;
                }

                if(bestHitData.distance > hitInfo.distance)
                {
                    bestHitData = {
                        .distance = hitInfo.distance,
                        .position = hitInfo.position,
                        .wall = &wall,
                    };
                }
            }
        }

        if(bestHitData.distance < std::numeric_limits<float>::max())
        {
            // Floor / ceiling rendering with proper perspective
            {
                RenderFloorAndCeiling(ctx, currentSector, x, yMinMax, bestHitData.distance);
            }

            // Means this is a solid wall
            if(bestHitData.wall->toSector == NULL_SECTOR)
            {
                // Calculate U coordinate along the wall
                const Segment& wallSeg = bestHitData.wall->segment;
                float wallLength = Vector2Distance(wallSeg.a, wallSeg.b);
                float hitU = Vector2Distance(wallSeg.a, bestHitData.position) / wallLength;

                CameraYLineData cameraWallYData =
                    ComputeCameraYAxis(*ctx.cam, x, bestHitData.distance,
                        ctx.FloorVerticalOffset, ctx.CamCurrentSectorElevationOffset,
                        ctx.RenderTargetWidth, ctx.RenderTargetHeight,
                        yMinMax.max,
                        yMinMax.min,
                        currentSector.zFloor, currentSector.zCeiling
                    );

                cameraWallYData.hitU = hitU;
                cameraWallYData.wallLength = wallLength;

                // Use textured rendering if texture is available
                if (bestHitData.wall->textureId != NULL_TEXTURE)
                {
                    RenderCameraYLineTextured(cameraWallYData, bestHitData.wall->textureId,
                                              bestHitData.wall->textureScale, WHITE);
                }
                else
                {
                    RenderCameraYLine(cameraWallYData, bestHitData.wall->color);
                }
            }
            else
            {
                // "Try to render a next sector with an invalid SectorID"
                assert(ctx.world->Sectors.contains(bestHitData.wall->toSector));

                const SectorID nextSectorId = bestHitData.wall->toSector;
                const Sector& nextSector = ctx.world->Sectors.at(nextSectorId);

                RenderNextAreaBorders(ctx, yMinMax, currentSector, nextSector, x, bestHitData.distance, bestHitData.wall, bestHitData.position);

                // Create / update NextRenderArea

                if(!renderAreaToPushInStack.contains(nextSectorId))
                {
                    SectorRenderContext nextRenderAreaContext = {
                        .sectorId = nextSectorId,
                        .renderArea = {
                            .xBegin = x,
                            .xEnd = x
                        },
                    };

                    renderAreaToPushInStack.emplace(nextSectorId, nextRenderAreaContext);
                }
                else
                {
                    renderAreaToPushInStack.at(nextSectorId).renderArea.xEnd = x;
                }
            
                // Draw a Purple placeholder where next sector will be drawn
                DrawLineV({(float)x, (float)yMinMax.min}, { (float)x, (float)yMinMax.max }, PURPLE);
            }
        }
    }

    // current render is over pop it
    ctx.renderStack.pop();

    for(const auto& [ key, renderAreaCtx ] : renderAreaToPushInStack)
    {
        ctx.renderStack.push(renderAreaCtx);
    }
}

void RenderNextAreaBorders(RasterizeWorldContext& worldContext, MinMaxUint32& yMinMax, const Sector& currentSector, const Sector& nextSector, uint32_t x, float hitDistance, const Wall* wall, const Vector2& hitPosition)
{
    // TODO :
    // zCeilling shloud not be < to current zFloor
    // And this is the same in the other way
    // zFloor should not be > to current zCeiling

    // Calculate U coordinate for texture mapping
    float hitU = 0.0f;
    float wallLength = 0.0f;
    if (wall != nullptr)
    {
        const Segment& wallSeg = wall->segment;
        wallLength = Vector2Distance(wallSeg.a, wallSeg.b);
        hitU = Vector2Distance(wallSeg.a, hitPosition) / wallLength;
    }

    // Top Border
    {
        bool nextSectCelingHigher = nextSector.zCeiling >= currentSector.zCeiling;

        const Sector& zSizesSector = (nextSectCelingHigher) ? currentSector : nextSector;

        CameraYLineData topBorderLineData = ComputeCameraYAxis(*worldContext.cam, x, hitDistance,
            worldContext.FloorVerticalOffset, worldContext.CamCurrentSectorElevationOffset,
            worldContext.RenderTargetWidth, worldContext.RenderTargetHeight,
            yMinMax.max, yMinMax.min,
            0, zSizesSector.zCeiling
        );

        topBorderLineData.hitU = hitU;
        topBorderLineData.wallLength = wallLength;

        if(!nextSectCelingHigher)
        {
            bool topEdge = !nextSectCelingHigher;
            if (nextSector.topBorderTextureId != NULL_TEXTURE)
            {
                RenderCameraYLineTextured(topBorderLineData, nextSector.topBorderTextureId,
                                          1.0f, WHITE, topEdge, true);
            }
            else
            {
                RenderCameraYLine(topBorderLineData, nextSector.topBorderColor, topEdge, true);
            }
        }

        // Apply Y min
        yMinMax.min = topBorderLineData.bottom.y;
    }

    // Bottom Border
    {
        bool nextSectFloorHigher = nextSector.zFloor >= currentSector.zFloor;

        const Sector& zSizesSector = (nextSectFloorHigher) ? currentSector : nextSector;

        CameraYLineData bottomBorderLineData = ComputeCameraYAxis(*worldContext.cam, x, hitDistance,
            worldContext.FloorVerticalOffset, worldContext.CamCurrentSectorElevationOffset,
            worldContext.RenderTargetWidth, worldContext.RenderTargetHeight,
            yMinMax.max, yMinMax.min,
            zSizesSector.zFloor, 0
        );

        bottomBorderLineData.hitU = hitU;
        bottomBorderLineData.wallLength = wallLength;

        if(!nextSectFloorHigher)
        {
            bool bottomEdge = !nextSectFloorHigher;
            if (nextSector.bottomBorderTextureId != NULL_TEXTURE)
            {
                RenderCameraYLineTextured(bottomBorderLineData, nextSector.bottomBorderTextureId,
                                          1.0f, WHITE, true, bottomEdge);
            }
            else
            {
                RenderCameraYLine(bottomBorderLineData, nextSector.bottomBorderColor, true, bottomEdge);
            }
        }

        // Apply Y max
        yMinMax.max = bottomBorderLineData.top.y;
    }
}

float ComputeVerticalOffset(const RaycastingCamera& cam, uint32_t RenderTargetHeight)
{
    return round(0.5f * RenderTargetHeight * (tanf(cam.pitch)) / tanf(0.5f * cam.fovVectical));
}

float ComputeElevationOffset(const RaycastingCamera& cam, const World& world, uint32_t RenderTargetHeight)
{ 
    // TODO : const float OneSectorHeight = RenderTargetHeight * cam.nearPlaneDistance;
    
    const Sector& currentSector = world.Sectors.at(cam.currentSectorId);
    return Lerp((float)RenderTargetHeight, 0.f, currentSector.zFloor);
}

CameraYLineData ComputeCameraYAxis(
    const RaycastingCamera& cam, uint32_t renderTargetX, float hitDistance, 
    float FloorVerticalOffset, float CamCurrentSectorElevationOffset,
    uint32_t RenderTargetWidth, uint32_t RenderTargetHeight,
    uint32_t YHigh, uint32_t YLow,
    float topOffsetPercentage, float bottomOffsetPercentage
)
{
    const float depth = Clamp(hitDistance, 0, cam.farPlaneDistance);
    // Normalize distance to [0, 1]
    const float normalizedDepth = depth / cam.farPlaneDistance;

    const float rayDirectionDeg = cam.fov * (floor(0.5 * RenderTargetWidth) - renderTargetX) / RenderTargetWidth;
    const float rayProjectionPositionInScreen = 0.5 * tanf(rayDirectionDeg * DEG2RAD) / tanf((0.5 * cam.fov) * DEG2RAD);
    const float objectHeight = round(RenderTargetHeight * cam.nearPlaneDistance / (depth * cosf(rayDirectionDeg * DEG2RAD)));

    // Rendering
    const float heightDelta = RenderTargetHeight - objectHeight;
    const float halfHeightDelta = heightDelta / 2;
    
    const float fullSizeTopY = (halfHeightDelta - FloorVerticalOffset);
    
    float topY    = fullSizeTopY + (objectHeight * topOffsetPercentage);
    float bottomY = (fullSizeTopY + objectHeight) - (objectHeight * bottomOffsetPercentage);

    topY += CamCurrentSectorElevationOffset;
    bottomY += CamCurrentSectorElevationOffset;

    return {
        .top    = { static_cast<float>(renderTargetX), Clamp(topY, YLow, YHigh) }, 
        .bottom = { static_cast<float>(renderTargetX), Clamp(bottomY, YLow, YHigh) },
        .depth  = depth,
        .normalizedDepth = normalizedDepth
    };
}

void RenderCameraYLine(CameraYLineData renderData, Color color, bool topEdge, bool bottomEdge)
{
    float darkness = Lerp(1, 0, renderData.normalizedDepth);

    DrawLineV(
        renderData.top,
        renderData.bottom,
        ColorDarken(color, renderData.normalizedDepth)
    );

    if(topEdge)
        DrawRectangle(renderData.top.x - 1, renderData.top.y - 1, 3, 3, GRAY);
    if(bottomEdge)
        DrawRectangle(renderData.bottom.x - 1, renderData.bottom.y - 1, 3, 3, GRAY);
}

void RenderEntities(const World& world, const RaycastingCamera& cam, uint32_t renderTargetWidth, uint32_t renderTargetHeight)
{
    if (world.Entities.empty()) return;

    // Collect visible entities with rendering data
    std::vector<EntityRenderData> visibleEntities;

    for (const auto& [id, entity] : world.Entities)
    {
        if (!entity.isActive || !entity.isVisible) continue;
        if (entity.spriteTextureId == NULL_TEXTURE) continue;

        // Calculate entity position relative to camera
        Vector2 relativePos = Vector2Subtract(entity.position, cam.position);
        float distance = Vector2Length(relativePos);

        // Cull if too far
        if (distance > cam.farPlaneDistance) continue;

        // Calculate angle to entity
        float entityAngle = atan2f(relativePos.y, relativePos.x);
        float angleFromCam = entityAngle - cam.yaw;

        // Normalize angle to -PI to PI
        while (angleFromCam > PI) angleFromCam -= 2 * PI;
        while (angleFromCam < -PI) angleFromCam += 2 * PI;

        // Cull if outside FOV
        float halfFOV = (cam.fov / 2.0f) * DEG2RAD;
        if (angleFromCam < -halfFOV - 0.5f || angleFromCam > halfFOV + 0.5f) continue;

        // Calculate screen position
        float screenX = (angleFromCam / (cam.fov * DEG2RAD) + 0.5f) * renderTargetWidth;

        // Calculate screen scale based on distance
        float screenScale = (renderTargetHeight / distance) * cam.nearPlaneDistance;

        EntityRenderData renderData = {
            .entityId = id,
            .distance = distance,
            .screenPosition = { screenX, static_cast<float>(renderTargetHeight) / 2.0f },
            .screenScale = screenScale,
            .entity = &entity
        };

        visibleEntities.push_back(renderData);
    }

    // Sort entities by distance (far to near for proper depth)
    std::sort(visibleEntities.begin(), visibleEntities.end());

    // Cache for texture images
    static std::unordered_map<TextureID, Image> spriteImageCache;

    // Render each entity
    for (const EntityRenderData& renderData : visibleEntities)
    {
        const Entity& entity = *renderData.entity;
        const TextureData* texData = TextureManager::Instance().GetTexture(entity.spriteTextureId);
        if (!texData) continue;

        // Load image for sampling if not cached
        if (spriteImageCache.find(entity.spriteTextureId) == spriteImageCache.end())
        {
            spriteImageCache[entity.spriteTextureId] = LoadImageFromTexture(texData->texture);
        }
        const Image& spriteImage = spriteImageCache[entity.spriteTextureId];

        // Calculate sprite dimensions on screen
        float spriteScreenWidth = entity.spriteWidth * renderData.screenScale * entity.spriteScale;
        float spriteScreenHeight = entity.spriteHeight * renderData.screenScale * entity.spriteScale;

        int startX = static_cast<int>(renderData.screenPosition.x - spriteScreenWidth / 2);
        int endX = static_cast<int>(renderData.screenPosition.x + spriteScreenWidth / 2);
        int startY = static_cast<int>(renderData.screenPosition.y - spriteScreenHeight / 2);
        int endY = static_cast<int>(renderData.screenPosition.y + spriteScreenHeight / 2);

        // Clamp to screen bounds
        startX = Clamp(startX, 0, static_cast<int>(renderTargetWidth) - 1);
        endX = Clamp(endX, 0, static_cast<int>(renderTargetWidth) - 1);
        startY = Clamp(startY, 0, static_cast<int>(renderTargetHeight) - 1);
        endY = Clamp(endY, 0, static_cast<int>(renderTargetHeight) - 1);

        // Render sprite billboard
        for (int x = startX; x <= endX; ++x)
        {
            for (int y = startY; y <= endY; ++y)
            {
                // Calculate texture coordinates
                float u = static_cast<float>(x - startX) / spriteScreenWidth;
                float v = static_cast<float>(y - startY) / spriteScreenHeight;

                int texX = static_cast<int>(u * spriteImage.width) % spriteImage.width;
                int texY = static_cast<int>(v * spriteImage.height) % spriteImage.height;

                Color pixelColor = GetImageColor(spriteImage, texX, texY);

                // Skip transparent pixels
                if (pixelColor.a < 10) continue;

                // Apply tint
                pixelColor.r = (pixelColor.r * entity.tint.r) / 255;
                pixelColor.g = (pixelColor.g * entity.tint.g) / 255;
                pixelColor.b = (pixelColor.b * entity.tint.b) / 255;
                pixelColor.a = (pixelColor.a * entity.tint.a) / 255;

                // Apply distance darkening
                float normalizedDepth = Clamp(renderData.distance / cam.farPlaneDistance, 0.0f, 1.0f);
                pixelColor = ColorDarken(pixelColor, normalizedDepth);

                DrawPixel(x, y, pixelColor);
            }
        }
    }
}

void RenderFloorAndCeiling(RasterizeWorldContext& ctx, const Sector& sector, uint32_t x, const MinMaxUint32& yMinMax, float wallDistance)
{
    const RaycastingCamera& cam = *ctx.cam;

    // Calculate ray angle for this column
    float rayAngle = RayAngleForScreenXCam(x, cam, ctx.RenderTargetWidth);
    float rayDir = (rayAngle * DEG2RAD) + cam.yaw;

    float centerY = (ctx.RenderTargetHeight / 2.0f) - ctx.FloorVerticalOffset + ctx.CamCurrentSectorElevationOffset;

    // Cache texture data if textures are used
    const TextureData* floorTexData = nullptr;
    const TextureData* ceilingTexData = nullptr;
    Image floorImage, ceilingImage;
    bool hasFloorTex = false;
    bool hasCeilingTex = false;

    static std::unordered_map<TextureID, Image> imageCache;

    if (sector.floorTextureId != NULL_TEXTURE)
    {
        floorTexData = TextureManager::Instance().GetTexture(sector.floorTextureId);
        if (floorTexData)
        {
            if (imageCache.find(sector.floorTextureId) == imageCache.end())
            {
                imageCache[sector.floorTextureId] = LoadImageFromTexture(floorTexData->texture);
            }
            floorImage = imageCache[sector.floorTextureId];
            hasFloorTex = true;
        }
    }

    if (sector.ceilingTextureId != NULL_TEXTURE)
    {
        ceilingTexData = TextureManager::Instance().GetTexture(sector.ceilingTextureId);
        if (ceilingTexData)
        {
            if (imageCache.find(sector.ceilingTextureId) == imageCache.end())
            {
                imageCache[sector.ceilingTextureId] = LoadImageFromTexture(ceilingTexData->texture);
            }
            ceilingImage = imageCache[sector.ceilingTextureId];
            hasCeilingTex = true;
        }
    }

    // Render floor (from center to bottom)
    for (int y = static_cast<int>(centerY); y <= static_cast<int>(yMinMax.max); ++y)
    {
        if (y < 0 || y >= static_cast<int>(ctx.RenderTargetHeight)) continue;

        // Calculate distance to floor point
        float rowDistance = (ctx.RenderTargetHeight * cam.nearPlaneDistance) /
                           (2.0f * (y - centerY));

        if (rowDistance < 0 || rowDistance > wallDistance) continue;

        // Calculate floor point in world space
        float floorX = cam.position.x + cosf(rayDir) * rowDistance;
        float floorY = cam.position.y + sinf(rayDir) * rowDistance;

        Color floorColor = sector.floorColor;

        if (hasFloorTex)
        {
            // Sample texture
            int texX = static_cast<int>(floorX * sector.floorTextureScale) % floorImage.width;
            int texY = static_cast<int>(floorY * sector.floorTextureScale) % floorImage.height;
            if (texX < 0) texX += floorImage.width;
            if (texY < 0) texY += floorImage.height;

            floorColor = GetImageColor(floorImage, texX, texY);
        }

        // Apply distance-based darkening
        float normalizedDepth = Clamp(rowDistance / cam.farPlaneDistance, 0.0f, 1.0f);
        floorColor = ColorDarken(floorColor, normalizedDepth);

        DrawPixel(x, y, floorColor);
    }

    // Render ceiling (from center to top)
    for (int y = static_cast<int>(centerY); y >= static_cast<int>(yMinMax.min); --y)
    {
        if (y < 0 || y >= static_cast<int>(ctx.RenderTargetHeight)) continue;

        // Calculate distance to ceiling point
        float rowDistance = (ctx.RenderTargetHeight * cam.nearPlaneDistance) /
                           (2.0f * (centerY - y));

        if (rowDistance < 0 || rowDistance > wallDistance) continue;

        // Calculate ceiling point in world space
        float ceilingX = cam.position.x + cosf(rayDir) * rowDistance;
        float ceilingY = cam.position.y + sinf(rayDir) * rowDistance;

        Color ceilingColor = sector.ceilingColor;

        if (hasCeilingTex)
        {
            // Sample texture
            int texX = static_cast<int>(ceilingX * sector.ceilingTextureScale) % ceilingImage.width;
            int texY = static_cast<int>(ceilingY * sector.ceilingTextureScale) % ceilingImage.height;
            if (texX < 0) texX += ceilingImage.width;
            if (texY < 0) texY += ceilingImage.height;

            ceilingColor = GetImageColor(ceilingImage, texX, texY);
        }

        // Apply distance-based darkening
        float normalizedDepth = Clamp(rowDistance / cam.farPlaneDistance, 0.0f, 1.0f);
        ceilingColor = ColorDarken(ceilingColor, normalizedDepth);

        DrawPixel(x, y, ceilingColor);
    }
}

void RenderCameraYLineTextured(CameraYLineData renderData, TextureID textureId, float textureScale, Color tint, bool topEdge, bool bottomEdge)
{
    const TextureData* texData = TextureManager::Instance().GetTexture(textureId);
    if (!texData)
    {
        // Fallback to color rendering if texture not found
        RenderCameraYLine(renderData, tint, topEdge, bottomEdge);
        return;
    }

    const Texture2D& texture = texData->texture;

    // Load image once for sampling (cached by raylib)
    static std::unordered_map<TextureID, Image> imageCache;
    if (imageCache.find(textureId) == imageCache.end())
    {
        imageCache[textureId] = LoadImageFromTexture(texture);
    }
    const Image& image = imageCache[textureId];

    // Calculate texture coordinates
    float u = fmodf(renderData.hitU * textureScale * texture.width, texture.width);

    // Draw vertical line with texture sampling
    int startY = static_cast<int>(renderData.top.y);
    int endY = static_cast<int>(renderData.bottom.y);
    int lineHeight = endY - startY;

    if (lineHeight <= 0) return;

    int x = static_cast<int>(renderData.top.x);
    int texX = static_cast<int>(u) % texture.width;

    for (int y = startY; y <= endY; ++y)
    {
        float v = static_cast<float>(y - startY) / static_cast<float>(lineHeight);
        v *= texture.height;

        int texY = static_cast<int>(v) % texture.height;

        // Sample texture color
        Color texColor = GetImageColor(image, texX, texY);

        // Apply depth darkening
        Color finalColor = ColorDarken(texColor, renderData.normalizedDepth);

        // Apply tint
        finalColor.r = (finalColor.r * tint.r) / 255;
        finalColor.g = (finalColor.g * tint.g) / 255;
        finalColor.b = (finalColor.b * tint.b) / 255;
        finalColor.a = (finalColor.a * tint.a) / 255;

        DrawPixel(x, y, finalColor);
    }

    if(topEdge)
        DrawRectangle(renderData.top.x - 1, renderData.top.y - 1, 3, 3, GRAY);
    if(bottomEdge)
        DrawRectangle(renderData.bottom.x - 1, renderData.bottom.y - 1, 3, 3, GRAY);
}


WorldRasterizer::WorldRasterizer(uint32_t renderTargetWidth, uint32_t renderTargetHeight, const World& world, const RaycastingCamera& cam)
{
    Reset(renderTargetWidth, renderTargetHeight, world, cam);
}

void WorldRasterizer::Reset(uint32_t renderTargetWidth, uint32_t renderTargetHeight, const World &world, const RaycastingCamera &cam)
{
    ctx.world = &world;
    ctx.cam = &cam;
    ctx.FloorVerticalOffset = ComputeVerticalOffset(cam, renderTargetHeight);
    ctx.CamCurrentSectorElevationOffset = ComputeElevationOffset(cam, world, renderTargetHeight);
    ctx.RenderTargetWidth = renderTargetWidth;
    ctx.RenderTargetHeight = renderTargetHeight;
    ctx.currentRenderItr = 0;

    ctx.yBoundaries.resize(renderTargetWidth);

    std::fill(ctx.yBoundaries.begin(), ctx.yBoundaries.end(), MinMax<uint32_t> {
        .max = renderTargetHeight,
        .min = 0,
    });

    // "Try to InitRasterizeWorldContext with an invalid SectorID"
    assert(world.Sectors.contains(ctx.cam->currentSectorId));

    // Clear the render stack
    if(ctx.renderStack.size() > 0)
    {
        std::stack<SectorRenderContext>().swap(ctx.renderStack);    
    }

    ctx.renderStack.push({
        .sectorId = ctx.cam->currentSectorId,
        .renderArea = {
            .xBegin = 0,
            .xEnd = renderTargetWidth > 0 ? (renderTargetWidth - 1) : 0,
        }
    });
}

void WorldRasterizer::RasterizeWorldInTexture(const RenderTexture& renderTexture)
{
    assert(ctx.RenderTargetWidth == renderTexture.texture.width && ctx.RenderTargetHeight == renderTexture.texture.height);

    BeginTextureMode(renderTexture);
        RasterizeWorld();
    EndTextureMode();
}

void WorldRasterizer::RasterizeWorld()
{
    ClearBackground(MY_BLACK);

    while(IsRenderIterationRemains())
    {
        RenderIteration();
    }

    // Render entities after all walls (for proper depth)
    RenderEntities(*ctx.world, *ctx.cam, ctx.RenderTargetWidth, ctx.RenderTargetHeight);
}

bool WorldRasterizer::IsRenderIterationRemains() const
{
    return (!ctx.renderStack.empty() && ctx.currentRenderItr < ctx.cam->maxRenderItr);
}

void WorldRasterizer::RenderIteration()
{
    // "Rendering is ended, RenderIteration should not be called"
    assert(IsRenderIterationRemains());

    RasterizeInRenderArea(ctx, ctx.renderStack.top());

    ctx.currentRenderItr++;
}