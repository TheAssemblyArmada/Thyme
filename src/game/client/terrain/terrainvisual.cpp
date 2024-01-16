/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Terrain Visual
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "terrainvisual.h"
#include "xfer.h"

#ifndef GAME_DLL
TerrainVisual *g_theTerrainVisual;
#endif

TerrainVisual::TerrainVisual() {}

TerrainVisual::~TerrainVisual() {}

void TerrainVisual::CRC_Snapshot(Xfer *xfer) {}

void TerrainVisual::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
}

void TerrainVisual::Load_Post_Process() {}

bool TerrainVisual::Load(Utf8String filename)
{
    if (filename.Is_Empty()) {
        return false;
    } else {
        m_filenameString = filename;
        return true;
    }
}

bool TerrainVisual::Intersect_Terrain(Coord3D *ray_start, Coord3D *ray_end, Coord3D *result)
{
    return false;
}

WorldHeightMap *TerrainVisual::Get_Logic_Height_Map()
{
    return nullptr;
}

WorldHeightMap *TerrainVisual::Get_Client_Height_Map()
{
    return nullptr;
}

void TerrainVisual::Init() {}

void TerrainVisual::Reset()
{
    m_filenameString.Clear();
}

void TerrainVisual::Update() {}
