/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Base Height Map Render Object
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
#include "asciistring.h"
#include "coord.h"
#include "dx8wrapper.h"
#include "gametype.h"
#include "rendobj.h"
#include "snapshot.h"
#include <vector>

class CloudMapTerrainTextureClass;
class W3DTreeBuffer;
class W3DWaypointBuffer;
class W3DRoadBuffer;
class W3DBridgeBuffer;
class W3DShroud;
class ShorelineTile;
class ShorelineSortInfo;
class WorldHeightMap;
class DX8VertexBufferClass;
class DX8IndexBufferClass;
struct VertexFormatXYZDUV2
{
    float x;
    float y;
    float z;
    unsigned int diffuse;
    float u1;
    float v1;
    float u2;
    float v2;
};

struct TScorch
{
    Vector3 m_location;
    float m_radius;
    int m_scorchType;
};

class BaseHeightMapRenderObjClass : public RenderObjClass, public DX8_CleanupHook, public SnapShot
{
public:
    virtual ~BaseHeightMapRenderObjClass();
    virtual RenderObjClass *Clone();
    virtual int Class_ID() const;
    virtual void On_Frame_Update();
    virtual void Notify_Added(SceneClass *scene);
    virtual bool Cast_Ray(RayCollisionTestClass &raytest);
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const;
    virtual int Init_Height_Data(
        int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline);
    virtual int Free_Map_Resources();
    virtual void Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights);
    virtual void Adjust_Terrain_LOD(int adj);
    virtual void Do_Partial_Update(
        IRegion2D &partial_range, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) = 0;
    virtual void Static_Lighting_Changed();
    virtual void Oversize_Terrain(int tiles_to_oversize);
    virtual void Reset();
    virtual int Get_Num_Extra_Blend_Tiles(bool b);
    virtual int Update_Block() = 0;

    virtual void Release_Resources();
    virtual void ReAcquire_Resources();

    virtual void CRC_Snapshot(Xfer *xfer);
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process();

    WorldHeightMap *Get_Map() { return m_map; }
    int Get_Static_Diffuse(int x, int y);
    float Get_Max_Cell_Height(float x, float y);
    void Set_Time_Of_Day(TimeOfDayType time);
    void Do_The_Light(VertexFormatXYZDUV2 *vb,
        Vector3 *light,
        Vector3 *normal,
        RefMultiListIterator<RenderObjClass> *lights,
        unsigned char alpha);
    float Get_Height_Map_Height(float x, float y, Coord3D *pos);
    void Notify_Shroud_Changed();

    W3DShroud *Get_Shroud() { return m_shroud; }

    int Get_Num_Water_Blend_Tiles(bool b)
    {
        if (b) {
            return m_shorelineTileBufferSize;
        } else {
            return m_shorelineTilesRendered;
        }
    }

protected:
    int m_x;
    int m_y;
    DX8VertexBufferClass *m_vertexScorch;
    DX8IndexBufferClass *m_indexScorch;
    TextureClass *m_scorchTexture;
    int m_curNumScorchVertices;
    int m_curNumScorchIndices;
    TScorch m_scorches[500];
    int m_numScorches;
    int m_scorchesInBuffer;
    float m_cliffAngle;
    WorldHeightMap *m_map;
    bool m_useDepthFade;
    bool m_updating;
    Vector3 m_depthFade;
    bool m_disableTextures;
    bool m_needFullUpdate;
    bool m_doXNextTime;
    float m_minHeight;
    float m_maxHeight;
    bool m_showImpassableAreas;
    std::vector<bool> m_impassableAreas;
    ShaderClass m_shaderClass;
    VertexMaterialClass *m_vertexMaterialClass;
    TextureClass *m_stageZeroTexture;
    TextureClass *m_stageOneTexture;
    CloudMapTerrainTextureClass *m_stageTwoTexture;
    TextureClass *m_stageThreeTexture;
    Utf8String m_macroTextureName;
    TextureClass *m_destAlphaLUT;
    W3DTreeBuffer *m_treeBuffer;
    int m_propBuffer;
    int m_bibBuffer;
    W3DWaypointBuffer *m_waypointBuffer;
    W3DRoadBuffer *m_roadBuffer;
    W3DBridgeBuffer *m_bridgeBuffer;
    W3DShroud *m_shroud;
    ShorelineTile *m_shorelineTiles;
    int m_shorelineTileCount;
    int m_shorelineTilesRendered;
    int m_shorelineTileBufferSize;
    float m_transparentWaterMinOpacity;
    ShorelineSortInfo *m_shorlineSortInfos;
    int m_shorlineSortInfoCount;
    int m_sortAxis;
    int m_endCell;
    int m_startCell;
};

#ifdef GAME_DLL
extern BaseHeightMapRenderObjClass *&g_theTerrainRenderObject;
#else
extern BaseHeightMapRenderObjClass *g_theTerrainRenderObject;
#endif

void Do_Trees(RenderInfoClass &rinfo);
