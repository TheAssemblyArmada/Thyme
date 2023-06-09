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
#include "gameclient.h"
#include "gametype.h"
#include "rendobj.h"
#include "snapshot.h"
#include "worldheightmap.h"
#include <vector>

class CloudMapTerrainTextureClass;
class W3DTreeBuffer;
class W3DWaypointBuffer;
class W3DRoadBuffer;
class W3DBridgeBuffer;
class W3DShroud;
class W3DPropBuffer;
class W3DBibBuffer;
class WorldHeightMap;
class DX8VertexBufferClass;
class DX8IndexBufferClass;
class W3DTreeDrawModuleData;
class W3DTerrainLogic;
class FrustumClass;
class GeometryInfo;
class GameAssetManager;
class SimpleSceneClass;

struct ShorelineTile
{
    int index;
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
    float x3;
    float y3;
    float z3;
    float x4;
    float y4;
    float z4;
    float u1;
    float u2;
    float u3;
    float u4;
};

struct ShorelineSortInfo
{
    int unk1;
    int unk2;
    unsigned short unk3;
    unsigned short unk4;
};

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
    BaseHeightMapRenderObjClass();

    virtual ~BaseHeightMapRenderObjClass() override;
    virtual RenderObjClass *Clone() const override { return nullptr; }
    virtual int Class_ID() const override { return CLASSID_HEIGHTMAP; }
    virtual void On_Frame_Update() override {}
    virtual void Notify_Added(SceneClass *scene) override;
    virtual bool Cast_Ray(RayCollisionTestClass &raytest) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual int Init_Height_Data(
        int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline);
    virtual int Free_Map_Resources();
    virtual void Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights);
    virtual void Adjust_Terrain_LOD(int adj);
    virtual void Do_Partial_Update(
        IRegion2D &partial_range, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) = 0;
    virtual void Static_Lighting_Changed();
    virtual void Oversize_Terrain(int tiles_to_oversize) {}
    virtual void Reset();
    virtual int Get_Num_Extra_Blend_Tiles(bool b) { return 0; }
    virtual int Update_Block(
        int x0, int y0, int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights) = 0;

    virtual void Release_Resources() override;
    virtual void Re_Acquire_Resources() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    int Get_Static_Diffuse(int x, int y);
    float Get_Max_Cell_Height(float x, float y) const;
    void Set_Time_Of_Day(TimeOfDayType time);
    void Do_The_Light(VertexFormatXYZDUV2 *vb,
        Vector3 *light,
        Vector3 *normal,
        RefMultiListIterator<RenderObjClass> *lights,
        unsigned char alpha);
    float Get_Height_Map_Height(float x, float y, Coord3D *pos) const;
    void Notify_Shroud_Changed();
    void Add_Tree(
        DrawableID drawable, Coord3D location, float scale, float angle, float random, W3DTreeDrawModuleData const *module);
    void Add_Prop(int id, Coord3D location, float orientation, float scale, const Utf8String &name);
    void Add_Scorch(Vector3 location, float radius, Scorches type);
    void Add_Terrain_Bib(Vector3 *points, ObjectID id, bool highlighted);
    void Add_Terrain_Bib_Drawable(Vector3 *points, DrawableID id, bool highlighted);
    void Allocate_Scorch_Buffers();
    void Clear_All_Scorches();
    void Draw_Scorches();
    bool Evaluate_As_Visible_Cliff(int x, int y, float height);
    void Free_Scorch_Buffers();
    bool Get_Maximum_Visible_Box(const FrustumClass &frustum, AABoxClass *box, bool ignore_max_height);
    void Init_Dest_Alpha_LUT();
    bool Is_Clear_Line_Of_Sight(const Coord3D &pos1, const Coord3D &pos2) const;
    bool Is_Cliff_Cell(float x, float y);
    void Load_Roads_And_Bridges(W3DTerrainLogic *pTerrainLogic, bool unk);
    void Record_Shore_Line_Sort_Infos();
    void Remove_All_Props();
    void Remove_All_Terrain_Bibs();
    void Remove_All_Trees();
    void Remove_Prop(int id);
    void Remove_Terrain_Bib(ObjectID id);
    void Remove_Terrain_Bib_Drawable(DrawableID id);
    void Remove_Terrain_Bib_Highlighting();
    void Remove_Tree(DrawableID id);
    void Remove_Trees_And_Props_For_Construction(const Coord3D *pos, const GeometryInfo &geom, float angle);
    void Render_Shore_Lines(CameraClass *camera);
    void Render_Shore_Lines_Sorted(CameraClass *camera);
    void Render_Trees(CameraClass *camera);
    void Set_Shore_Line_Detail();
    void Set_Texture_LOD(int LOD);
    bool Show_As_Visible_Cliff(int x, int y) const;
    void Unit_Moved(Object *object);
    void Update_Macro_Texture(Utf8String texture_name);
    void Update_Scorches();
    void Update_Shoreline_Tile(int x, int y, int border_size, WorldHeightMap *map);
    void Update_Shoreline_Tiles(int min_x, int min_y, int max_x, int max_y, WorldHeightMap *map);
    void Update_Tree_Position(DrawableID drawable, Coord3D pos, float angle);
    void Update_View_Impassable_Areas(bool unk, int min_x, int min_y, int max_x, int max_y);
    void World_Builder_Update_Bridge_Towers(GameAssetManager *assetmgr, SimpleSceneClass *scene);

    WorldHeightMap *Get_Map() const { return m_map; }
    W3DShroud *Get_Shroud() const { return m_shroud; }
    bool Get_Show_Impassable_Areas() const { return m_showImpassableAreas; }
    void Set_Show_Impassable_Areas(bool show) { m_showImpassableAreas = show; }
    void Set_Cliff_Angle(float angle) { m_cliffAngle = angle; }
    void Enable_Textures(bool enable) { m_disableTextures = !enable; }
    float Get_Max_Height() const { return m_maxHeight; }
    float Get_Min_Height() const { return m_minHeight; }
    bool Get_Need_Full_Update() const { return m_needFullUpdate; }
    float Get_Cliff_Angle() const { return m_cliffAngle; }

    int Get_Num_Water_Blend_Tiles(bool b)
    {
        if (b) {
            return m_shorelineBlendTileSize;
        } else {
            return m_shorelineBlendTilesRendered;
        }
    }

    unsigned char Get_Clip_Height(int x, int y) const
    {
        int x_extent = m_map->Get_X_Extent() - 1;
        int y_extent = m_map->Get_Y_Extent() - 1;

        if (x < 0) {
            x = 0;
        } else if (x > x_extent) {
            x = x_extent;
        }

        if (y < 0) {
            y = 0;
        } else if (y > y_extent) {
            y = y_extent;
        }

        return m_map->Get_Data_Ptr()[x + m_map->Get_X_Extent() * y];
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
    W3DPropBuffer *m_propBuffer;
    W3DBibBuffer *m_bibBuffer;
    W3DWaypointBuffer *m_waypointBuffer;
    W3DRoadBuffer *m_roadBuffer;
    W3DBridgeBuffer *m_bridgeBuffer;
    W3DShroud *m_shroud;
    ShorelineTile *m_shorelineTiles;
    int m_numShorelineBlendTiles;
    int m_shorelineBlendTilesRendered;
    int m_shorelineBlendTileSize;
    float m_transparentWaterMinOpacity;
    ShorelineSortInfo *m_shorelineSortInfos;
    int m_shorelineSortInfoCount;
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
