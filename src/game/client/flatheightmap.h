/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Flat Height Map Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"
#include "baseheightmap.h"
class W3DTerrainBackground;

class FlatHeightMapRenderObjClass : public BaseHeightMapRenderObjClass
{
public:
    FlatHeightMapRenderObjClass();

    ~FlatHeightMapRenderObjClass() override;
    void Render(RenderInfoClass &rinfo) override;
    void On_Frame_Update() override;
    int Init_Height_Data(
        int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline) override;
    int Free_Map_Resources() override;
    void Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights) override;
    void Adjust_Terrain_LOD(int adj) override;
    void Do_Partial_Update(
        IRegion2D &partial_range, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) override;
    void Static_Lighting_Changed() override;
    void Oversize_Terrain(int tiles_to_oversize) override {}
    void Reset() override;
    int Update_Block(
        int x0, int y0, int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) override
    {
        return 0;
    }

    void Release_Resources() override;
    void Re_Acquire_Resources() override;

    void Release_Tiles();

private:
    W3DTerrainBackground *m_tiles;
    int m_tileCount;
    int m_tileWidth;
    int m_tileHeight;
    int m_state;
};

#ifdef GAME_DLL
extern FlatHeightMapRenderObjClass *&g_theFlatHeightMap;
#else
extern FlatHeightMapRenderObjClass *g_theFlatHeightMap;
#endif
