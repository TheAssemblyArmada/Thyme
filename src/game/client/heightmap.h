/**
/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Height Map Render Object
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
class W3DDynamicLight;

class HeightMapRenderObjClass : public BaseHeightMapRenderObjClass
{
public:
#ifdef GAME_DLL
    HeightMapRenderObjClass *Hook_Ctor() { return new (this) HeightMapRenderObjClass; }
#endif

    HeightMapRenderObjClass();

    virtual ~HeightMapRenderObjClass() override;
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
    virtual void Oversize_Terrain(int tiles_to_oversize) override;
    virtual void Reset() override;
    virtual int Get_Num_Extra_Blend_Tiles(bool b) override
    {
        if (b) {
            return m_extraBlendTilesRendered;
        } else {
            return m_numExtraBlendTiles;
        }
    }
    virtual int Update_Block(
        int x0, int y0, int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) override;

    virtual void Release_Resources() override;
    virtual void Re_Acquire_Resources() override;

    void Free_Index_Vertex_Buffers();
    void Do_The_Dynamic_Light(VertexFormatXYZDUV2 *vb,
        VertexFormatXYZDUV2 *vb_mirror,
        Vector3 *light,
        Vector3 *normal,
        W3DDynamicLight *lights,
        int num_lights);
    int Get_X_With_Origin(int x);
    int Get_Y_With_Origin(int y);
    int Update_VB(DX8VertexBufferClass *vb,
        char *data,
        int x0,
        int y0,
        int x1,
        int y1,
        int origin_x,
        int origin_y,
        WorldHeightMap *map,
        RefMultiListIterator<RenderObjClass> *lights);
    int Update_VB_For_Light(DX8VertexBufferClass *vb,
        char *data,
        int x0,
        int y0,
        int x1,
        int y1,
        int origin_x,
        int origin_y,
        W3DDynamicLight **lights,
        int num_lights);
    int Update_VB_For_Light_Optimized(DX8VertexBufferClass *vb,
        char *data,
        int x0,
        int y0,
        int x1,
        int y1,
        int origin_x,
        int origin_y,
        W3DDynamicLight **lights,
        int num_lights);
    void Render_Terrain_Pass(CameraClass *camera);
    void Render_Extra_Blend_Tiles();

private:
    unsigned int *m_extraBlendTilePositions;
    int m_numExtraBlendTiles;
    int m_extraBlendTilesRendered;
    int m_extraBlendTilePositionsSize;
    DX8VertexBufferClass **m_vertexBufferTiles;
    char **m_vertexBufferBackup;
    int m_originX;
    int m_originY;
    DX8IndexBufferClass *m_indexBuffer;
    int m_numVBTilesX;
    int m_numVBTilesY;
    int m_numVertexBufferTiles;
    int m_numBlockColumnsInLastVB;
    int m_numBlockRowsInLastVB;
};

#ifdef GAME_DLL
extern HeightMapRenderObjClass *&g_theHeightMap;
#else
extern HeightMapRenderObjClass *g_theHeightMap;
#endif
