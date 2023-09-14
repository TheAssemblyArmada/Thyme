/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Terrain Logic
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dterrainlogic.h"
#include "ai.h"
#include "aipathfind.h"
#include "baseheightmap.h"
#include "cachedfileinputstream.h"
#include "gameclient.h"
#include "globaldata.h"
#include "maputil.h"
#include "worldheightmap.h"
#include "xfer.h"

W3DTerrainLogic::W3DTerrainLogic() : m_mapMinZ(0.0f), m_mapMaxZ(1.0f)
{
    m_mapData = nullptr;
}

W3DTerrainLogic::~W3DTerrainLogic() {}

void W3DTerrainLogic::Init()
{
    TerrainLogic::Init();
    m_mapDX = 0;
    m_mapDY = 0;
    m_mapMinZ = 0.0f;
    m_mapMaxZ = 1.0f;
}

void W3DTerrainLogic::Reset()
{
    TerrainLogic::Reset();
    m_mapDX = 0;
    m_mapDY = 0;
    m_mapMinZ = 0.0f;
    m_mapMaxZ = 1.0f;
    WorldHeightMap::Free_List_Of_Map_Objects();
}

void W3DTerrainLogic::Update()
{
    TerrainLogic::Update();
}

void W3DTerrainLogic::CRC_Snapshot(Xfer *xfer)
{
    TerrainLogic::CRC_Snapshot(xfer);
}

void W3DTerrainLogic::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    TerrainLogic::Xfer_Snapshot(xfer);
}

void W3DTerrainLogic::Load_Post_Process()
{
    TerrainLogic::Load_Post_Process();
}

bool W3DTerrainLogic::Load_Map(Utf8String filename, bool query)
{
    if (g_theMapCache == nullptr) {
        return false;
    }

    CachedFileInputStream stream;

    if (!stream.Open(filename)) {
        return false;
    }

    WorldHeightMap *map = new WorldHeightMap(&stream, true);

    if (map == nullptr) {
        return false;
    }

    m_mapDX = map->Get_X_Extent();
    m_mapDY = map->Get_Y_Extent();
    m_boundaries = map->Get_All_Boundaries();
    m_activeBoundary = 0;
    int min_z = 255;
    int max_z = 0;

    for (int y = 0; y < m_mapDY; y++) {
        for (int x = 0; x < m_mapDX; x++) {
            unsigned char data = map->Get_Height(x, y);

            if (data < min_z) {
                min_z = data;
            }

            if (max_z < data) {
                max_z = data;
            }
        }
    }

    m_mapMinZ = min_z * 0.625f;
    m_mapMaxZ = max_z * 0.625f;
    map->Release_Ref();

    if (TerrainLogic::Load_Map(filename, query)) {
        if (g_theWriteableGlobalData->Set_Time_Of_Day(g_theWriteableGlobalData->m_timeOfDay)) {
            g_theGameClient->Set_Time_Of_Day(g_theWriteableGlobalData->m_timeOfDay);
        }

        return true;
    }

    return false;
}

void W3DTerrainLogic::New_Map(bool b)
{
    g_theTerrainRenderObject->Load_Roads_And_Bridges(this, b);
    TerrainLogic::New_Map(b);
}

float W3DTerrainLogic::Get_Ground_Height(float x, float y, Coord3D *n) const
{
    if (g_theTerrainRenderObject != nullptr) {
        return g_theTerrainRenderObject->Get_Height_Map_Height(x, y, n);
    }

    if (n != nullptr) {
        n->x = 0.0f;
        n->y = 0.0f;
        n->z = 1.0f;
    }

    return 0.0f;
}

float W3DTerrainLogic::Get_Layer_Height(float x, float y, PathfindLayerEnum layer, Coord3D *n, bool b) const
{
    if (g_theTerrainRenderObject != nullptr) {
        float height = g_theTerrainRenderObject->Get_Height_Map_Height(x, y, n);

        if (layer == LAYER_GROUND) {
            return height;
        }

        Coord3D c;
        c.x = x;
        c.y = y;
        c.z = height;

        if (layer != LAYER_WALLS) {
            Bridge *bridge = Find_Bridge_Layer_At(&c, layer, b);

            if (bridge != nullptr) {
                float bridge_height = bridge->Get_Bridge_Height(&c, n);

                if (bridge_height > height) {
                    return bridge_height;
                }
            }

            return height;
        }

        if (!b || g_theAI->Get_Pathfinder()->Is_Point_On_Wall(&c)) {
            return g_theAI->Get_Pathfinder()->Get_Wall_Height();
        } else {
            return height;
        }
    }

    if (n != nullptr) {
        n->x = 0.0f;
        n->y = 0.0f;
        n->z = 1.0f;
    }

    return 0.0f;
}

void W3DTerrainLogic::Get_Extent(Region3D *extent) const
{
    extent->lo.x = 0.0f;
    extent->lo.y = 0.0f;

    if (m_boundaries.size() != 0) {
        extent->hi.x = m_boundaries[m_activeBoundary].x * 10.0f;
        extent->hi.y = m_boundaries[m_activeBoundary].y * 10.0f;
    } else {
        extent->hi.x = 0.0f;
        extent->hi.y = 0.0f;
    }

    extent->lo.z = m_mapMinZ;
    extent->hi.z = m_mapMaxZ;
}

void W3DTerrainLogic::Get_Extent_Including_Border(Region3D *extent) const
{
    extent->lo.x = 0.0f;
    extent->lo.y = 0.0f;
    float size = g_theTerrainRenderObject->Get_Map()->Border_Size() * 10.0f;
    extent->lo.x = 0.0f - size;
    extent->lo.y = 0.0f - size;
    extent->hi.x = (m_mapDX * 10.0f) - size;
    extent->hi.y = (m_mapDY * 10.0f) - size;
}

void W3DTerrainLogic::Get_Maximum_Pathfind_Extent(Region3D *extent) const
{
    extent->lo.x = 0.0f;
    extent->lo.y = 0.0f;
    extent->hi.x = 0.0f;
    extent->hi.y = 0.0f;

    for (auto it = m_boundaries.begin(); it != m_boundaries.end(); it++) {
        float x = it->x * 10.0f;

        if (x > extent->hi.x) {
            extent->hi.x = x;
        }

        float y = it->y * 10.0f;

        if (y > extent->hi.y) {
            extent->hi.y = y;
        }
    }

    extent->lo.z = m_mapMinZ;
    extent->hi.z = m_mapMaxZ;
}

bool W3DTerrainLogic::Is_Clear_Line_Of_Sight(const Coord3D &pos1, const Coord3D &pos2) const
{
    if (g_theTerrainRenderObject != nullptr) {
        return g_theTerrainRenderObject->Is_Clear_Line_Of_Sight(pos1, pos2);
    }

    return false;
}

bool W3DTerrainLogic::Is_Cliff_Cell(float x, float y) const
{
    return g_theTerrainRenderObject->Is_Cliff_Cell(x, y);
}
