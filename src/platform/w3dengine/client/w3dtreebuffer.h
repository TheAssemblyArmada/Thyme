/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Tree Buffer
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
#include "coord.h"
#include "globaldata.h"
#include "matrix3d.h"
#include "multilist.h"
#include "object.h"
#include "snapshot.h"
#include "sphere.h"
#include "texture.h"
#include "vector3.h"

class MeshClass;
class W3DTreeDrawModuleData;
class DX8VertexBufferClass;
class DX8IndexBufferClass;
class TileData;
class W3DProjectedShadow;
class CameraClass;
struct BreezeInfo;

struct TTreeType
{
    TTreeType() {} // wb 0x00654D60

    MeshClass *mesh; // confirmed
    SphereClass bounds; // confirmed
    const W3DTreeDrawModuleData *module; // confirmed
    int texture_width;
    int texture_height;
    int num_tiles;
    int first_tile;
    int width;
    bool valid_tile;
    Vector3 pos;
    float uv_offset;
    bool do_shadow;
};

struct TTree
{
    enum TreeToppleState
    {
        TOPPLE_UPRIGHT,
        TOPPLE_WAITING_FOR_START,
        TOPPLE_TOPPLING,
        TOPPLE_WAITING_FOR_DONE,
        TOPPLE_DOWN,
    };

    enum TreeToppleOptions
    {
        TOPPLE_OPTIONS_NONE,
        TOPPLE_OPTIONS_NO_BOUNCE,
        TOPPLE_OPTIONS_NO_FX,
    };

    TTree() {} // wb 0x00654D30

    Vector3 location; // confirmed
    float scale; // confirmed
    float sin; // confirmed
    float cos; // confirmed
    int tree_type; // confirmed
    bool visible; // confirmed
    SphereClass bounds; // confirmed
    float sort_key; // confirmed
    DrawableID drawable_id;
    float push_aside_speed;
    float push_aside_move_time;
    Coord2D push_aside_location;
    ObjectID push_aside_obj_id;
    int push_aside_frame;
    int partition_bucket_index;
    int sway_rand;
    int vertex_array_index;
    int vb_index;
    float angular_velocity;
    float angular_acceleration;
    Coord3D topple_direction;
    TreeToppleState topple_state;
    float angular_accumulation;
    unsigned int options;
    Matrix3D topple_transform;
    unsigned int topple_sink_countdown;
};

class W3DTreeBuffer : public SnapShot
{
public:
    class W3DTreeTextureClass : public TextureClass
    {
        IMPLEMENT_W3D_POOL(W3DTreeTextureClass);

    public:
        // WB 0x0064EC71
        W3DTreeTextureClass(unsigned int width, unsigned int height) :
            TextureClass(width, height, WW3D_FORMAT_A8R8G8B8, MIP_LEVELS_ALL, POOL_MANAGED, false, true)
        {
        }

        virtual ~W3DTreeTextureClass() override {} // WB 0x00654AB0
        virtual void Apply(unsigned stage) override { TextureClass::Apply(stage); } // WB 0x0064EF38

        int Update(W3DTreeBuffer *buffer); // WB 0x0064ECA4
        void Set_Lod(int LOD) const; // WB 0x0064EEF9
    };

#ifdef GAME_DLL
    W3DTreeBuffer *Hook_Ctor() { return new (this) W3DTreeBuffer(); }
    void Hook_Dtor() { W3DTreeBuffer::~W3DTreeBuffer(); }
#endif

    W3DTreeBuffer(); // WB 0x00651917
    ~W3DTreeBuffer(); // WB 0x006517FD

    virtual void CRC_Snapshot(Xfer *xfer) override {} // WB 0x006544E0
    virtual void Xfer_Snapshot(Xfer *xfer) override; // WB 0x006544ED
    virtual void Load_Post_Process() override {} // WB 0x006549F6

    void Cull(const CameraClass *camera); // WB 0x0064EF8D
    int Get_Partition_Bucket(const Coord3D *loc) const; // WB 0x0064F133
    void Update_Sway(const BreezeInfo &info); // WB 0x0064F28C
    void Update_Texture(); // WB 0x0064F4C0
    void Set_Texture_LOD(int LOD); // WB 0x00650283
    int Do_Lighting(const Vector3 *loc,
        const GlobalData::TerrainLighting *light,
        const Vector3 *emissive,
        unsigned int color,
        float factor); // WB 0x006502AE
    void Load_Trees_In_Index_And_Vertex_Buffers(
        RefMultiListIterator<class RenderObjClass> *pDynamicLightsIterator); // WB 0x0065058A
    void Update_Vertex_Buffer(); // WB 0x00651361
    void Free_Tree_Buffers(); // WB 0x00651A8A
    void Unit_Moved(Object *unit); // WB 0x00651B6D
    void Allocate_Tree_Buffers(); // WB 0x0065211C
    void Clear_All_Trees(); // WB 0x006522AE
    void Remove_Tree(DrawableID drawable); // WB 0x006523CA
    void Remove_Trees_For_Construction(const Coord3D *pos, const GeometryInfo &geom, float angle); // WB 0x006524A4
    int Add_Tree_Type(const W3DTreeDrawModuleData *module); // WB 0x006525D3
    void Add_Tree(DrawableID drawable,
        Coord3D loc,
        float scale,
        float angle,
        float random,
        const W3DTreeDrawModuleData *module); // WB 0x006529CA
    bool Update_Tree_Position(DrawableID drawable, Coord3D pos, float angle); // WB 0x00652EA1
    void Push_Aside_Tree(DrawableID drawable, const Coord3D *pos, const Coord3D *dir, ObjectID object); // WB 0x0065303C
    void Draw_Trees(
        CameraClass *camera, RefMultiListIterator<class RenderObjClass> *pDynamicLightsIterator); // WB 0x00653262
    void Apply_Toppling_Force(TTree *tree, const Coord3D *pos, float speed, int options); // WB 0x00654094
    void Update_Toppling_Tree(TTree *tree); // WB 0x006541A0

    void Set_Partition_Region(Region2D *region) { m_partitionRegion = *region; } // WB 0x00604BD0
    void Do_Full_Update() { m_updateAllKeys = true; } // WB 0x00604C00
    void Set_Is_Terrain() { m_isTerrainPass = true; } // WB 0x0060D390
    int Get_Num_Tiles() { return m_numTiles; } // WB 0x00654AD0
    TileData *Get_Tile_Data(int index) { return m_tileData[index]; } // WB 0x00654AF0

private:
    enum
    {
        MAX_TREE_TYPES = 64,
        MAX_TREE_VERTEX = 30000,
        MAX_TREE_INDEX = 60000,
        MAX_TREES = 4000,
        MAX_TILE_SIZE = 512,
        PARTITION_WIDTH_HEIGHT = 100,
        MAX_PARTITON_INDICES = PARTITION_WIDTH_HEIGHT * PARTITION_WIDTH_HEIGHT,
    };

    DX8VertexBufferClass *m_vertexTree[1]; // confirmed
    DX8IndexBufferClass *m_indexTree[1]; // confirmed
    unsigned long m_dwTreePixelShader; // confirmed
    unsigned long m_dwTreeVertexShader; // confirmed
    short m_partitionIndices[MAX_PARTITON_INDICES];
    Region2D m_partitionRegion;
    W3DTreeTextureClass *m_treeTexture; // confirmed
    int m_textureWidth;
    int m_textureHeight;
    int m_curNumTreeVertices[1]; // confirmed
    int m_curNumTreeIndices[1]; // confirmed
    TTree m_trees[MAX_TREES]; // confirmed
    int m_numTrees; // confirmed
    bool m_anythingChanged; // confirmed
    bool m_vertexesDirty;
    bool m_updateAllKeys; // confirmed
    bool m_initialized; // confirmed
    bool m_isTerrainPass; // confirmed
    bool m_textureDirty;
    TTreeType m_treeTypes[MAX_TREE_TYPES]; // confirmed
    int m_numTreeTypes; // confirmed
    int m_numTiles;
    TileData *m_tileData[MAX_TILE_SIZE];
    Vector3 m_cameraLookAtVector; // confirmed
    Vector3 m_swayVector[100];
    unsigned int m_swayVersion;
    float m_swayCurrent[10];
    float m_swayPeriods[10];
    float m_swayLeanAngles[10];
    W3DProjectedShadow *m_decalShadow;
};
