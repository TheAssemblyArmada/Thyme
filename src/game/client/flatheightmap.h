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

    virtual ~FlatHeightMapRenderObjClass() override;
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void On_Frame_Update() override;
    virtual int Init_Height_Data(
        int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline) override;
    virtual int Free_Map_Resources() override;
    virtual void Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights) override;
    virtual void Adjust_Terrain_LOD(int adj) override;
    virtual void Do_Partial_Update(
        IRegion2D &partial_range, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) override;
    virtual void Static_Lighting_Changed() override;
    virtual void Oversize_Terrain(int tiles_to_oversize) override {}
    virtual void Reset() override;
    virtual int Update_Block(
        int x0, int y0, int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) override
    {
        return 0;
    }

    virtual void Release_Resources() override;
    virtual void Re_Acquire_Resources() override;

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
