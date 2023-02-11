/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Bridge buffer
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
#include "gametype.h"
#include "matrix3d.h"
#include "multilist.h"
#include "sphere.h"
#include "terrainlogic.h"
#include "vector3.h"
class TextureClass;
class MeshClass;
class DX8VertexBufferClass;
class DX8IndexBufferClass;
class VertexMaterialClass;
class CameraClass;
class RenderObjClass;
class W3DTerrainLogic;
class GameAssetManager;
class SimpleSceneClass;

struct VertexFormatXYZNDUV1
{
    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nz;
    unsigned int diffuse;
    float u1;
    float v1;
};

enum TBridgeType
{
    FIXED_BRIDGE,
    SECTIONAL_BRIDGE,
};

class W3DBridge
{
public:
    W3DBridge();
    ~W3DBridge();
    void Clear_Bridge();
    bool Cull_Bridge(CameraClass *camera);
    void Get_Bridge_Info(BridgeInfo *info);
    void Get_Indices_And_Vertices(unsigned short *destination_ib,
        VertexFormatXYZNDUV1 *destination_vb,
        int *cur_index,
        int *cur_vertex,
        RefMultiListIterator<RenderObjClass> *lights);
    int Get_Model_Indices(unsigned short *destination_ib, int cur_index, int vertex_offset, MeshClass *mesh);
    int Get_Model_Vertices(VertexFormatXYZNDUV1 *destination_vb,
        int cur_vertex,
        float x_offset,
        Vector3 &vec,
        Vector3 &vec_normal,
        Vector3 &vec_z,
        Vector3 &offset,
        Matrix3D const &mtx,
        MeshClass *mesh,
        RefMultiListIterator<RenderObjClass> *lights);
    int Get_Model_Vertices_Fixed(VertexFormatXYZNDUV1 *destination_vb,
        int cur_vertex,
        Matrix3D const &mtx,
        MeshClass *mesh,
        RefMultiListIterator<RenderObjClass> *lights);
    void Init(Vector3 from_loc, Vector3 to_loc, Utf8String bridge_template_name);
    bool Load(BodyDamageType cur_damage_state);
    void Render_Bridge(bool wireframe);
    Utf8String Get_Template() { return m_template; }
    Vector3 &Get_Start() { return m_start; }
    Vector3 &Get_End() { return m_end; }
    BodyDamageType Get_Damage_State() { return m_curDamageState; }
    void Set_Damage_State(BodyDamageType state) { m_curDamageState = state; }
    bool Is_Visible() { return m_visible; }
    bool Is_Enabled() { return m_enabled; }
    void Set_Enabled(bool enabled) { m_enabled = enabled; }

private:
    Vector3 m_start;
    Vector3 m_end;
    float m_scale;
    float m_length;
    TBridgeType m_bridgeType;
    SphereClass m_bounds;
    TextureClass *m_bridgeTexture;
    MeshClass *m_leftMesh;
    Matrix3D m_leftMtx;
    float m_minY;
    float m_maxY;
    float m_leftMinX;
    float m_leftMaxX;
    MeshClass *m_sectionMesh;
    Matrix3D m_sectionMtx;
    float m_sectionMinX;
    float m_sectionMaxX;
    MeshClass *m_rightMesh;
    Matrix3D m_rightMtx;
    float m_rightMinX;
    float m_rightMaxX;
    int m_firstIndex;
    int m_numVertex;
    int m_firstVertex;
    int m_numPolygons;
    bool m_visible;
    Utf8String m_template;
    BodyDamageType m_curDamageState;
    bool m_enabled;
};

class W3DBridgeBuffer
{
public:
    enum
    {
        MAX_BRIDGE_VERTEX = 12000,
        MAX_BRIDGE_INDEX = 24000,
        MAX_BRIDGES = 200,
    };

    W3DBridgeBuffer();
    ~W3DBridgeBuffer();
    void Add_Bridge(Vector3 from_loc, Vector3 to_loc, Utf8String name, W3DTerrainLogic *terrain_logic, Dict *props);
    void Allocate_Bridge_Buffers();
    void Clear_All_Bridges();
    void Cull(CameraClass *camera);
    void Draw_Bridges(CameraClass *camera, bool wireframe, TextureClass *cloud_texture);
    void Free_Bridge_Buffers();
    void Load_Bridges(W3DTerrainLogic *terrain_logic, bool unk);
    void Load_Bridges_In_Vertex_And_Index_Buffers(RefMultiListIterator<class RenderObjClass> *lights);
    void Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights);
    void World_Builder_Update_Bridge_Towers(GameAssetManager *assetmgr, SimpleSceneClass *scene);

    void Do_Full_Update() { m_updateVis = true; }

#ifdef GAME_DLL
    W3DBridgeBuffer *Hook_Ctor() { return new (this) W3DBridgeBuffer(); }
    void Hook_Dtor() { W3DBridgeBuffer::~W3DBridgeBuffer(); }
#endif

private:
    DX8VertexBufferClass *m_vertexBridge;
    DX8IndexBufferClass *m_indexBridge;
    VertexMaterialClass *m_vertexMaterial;
    TextureClass *m_bridgeTexture;
    int m_curNumBridgeVertices;
    int m_curNumBridgeIndices;
    W3DBridge m_bridges[MAX_BRIDGES];
    int m_numBridges;
    bool m_initialized;
    bool m_updateVis;
    bool m_anythingChanged;
};
