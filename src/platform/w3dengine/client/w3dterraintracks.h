/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Terrain Tracks
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
#include "aabox.h"
#include "rendobj.h"
#include "shader.h"
#include "sphere.h"
#include "vector3.h"
#include "w3dmpo.h"

class Drawable;
class DX8VertexBufferClass;
class DX8IndexBufferClass;
class VertexMaterialClass;
class TextureClass;

class TerrainTracksRenderObjClass : public W3DMPO, public RenderObjClass
{
    IMPLEMENT_W3D_POOL(TerrainTracksRenderObjClass);

public:
    TerrainTracksRenderObjClass();
    virtual ~TerrainTracksRenderObjClass() override {}
    virtual RenderObjClass *Clone() const override { return nullptr; }
    virtual int Class_ID() const override { return CLASSID_TERRAINTRACKS; }
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    int Free_Terrain_Tracks_Resources();
    void Add_Cap_Edge_To_Track(float x, float y);
    void Add_Edge_To_Track(float x, float y);
    void Init(float width, float length, char const *texture_name);

    void Set_Above_Terrain() { m_aboveTerrain = true; }
    void Set_Bound_Drawable(Drawable *d) { m_boundDrawable = d; }

private:
    struct EdgeInfo
    {
        Vector3 end_point_pos[2];
        Vector2 end_point_uv[2];
        int time_added;
        float alpha;
    };

    TextureClass *m_stageZeroTexture;
    SphereClass m_boundingSphere;
    AABoxClass m_boundingBox;
    int m_activeEdgeCount;
    int m_totalEdgesAdded;
    Drawable *m_boundDrawable;
    EdgeInfo m_edges[100];
    Vector3 m_lastAnchor;
    int m_bottomIndex;
    int m_topIndex;
    bool m_haveAnchor;
    bool m_bound;
    float m_width;
    float m_length;
    bool m_aboveTerrain;
    bool m_unk;
    TerrainTracksRenderObjClass *m_nextSystem;
    TerrainTracksRenderObjClass *m_prevSystem;
    friend class TerrainTracksRenderObjClassSystem;
};

class TerrainTracksRenderObjClassSystem
{
public:
    TerrainTracksRenderObjClassSystem();
    ~TerrainTracksRenderObjClassSystem();
    void Release_Resources();
    void Re_Acquire_Resources();
    void Flush();
    void Update();
    void Init(SceneClass *terrain_tracks_scene);
    void Shutdown();
    void Reset();
    TerrainTracksRenderObjClass *Bind_Track(RenderObjClass *render_object, float length, char const *texture_name);
    void Clear_Tracks();
    void Release_Track(TerrainTracksRenderObjClass *mod);
    void Set_Detail();
    void Unbind_Track(TerrainTracksRenderObjClass *mod);

#ifdef GAME_DLL
    TerrainTracksRenderObjClassSystem *Hook_Ctor() { return new (this) TerrainTracksRenderObjClassSystem(); }
    void Hook_Dtor() { TerrainTracksRenderObjClassSystem::~TerrainTracksRenderObjClassSystem(); }
#endif

private:
    DX8VertexBufferClass *m_vertexBuffer;
    DX8IndexBufferClass *m_indexBuffer;
    VertexMaterialClass *m_vertexMaterialClass;
    ShaderClass m_shaderClass;
    TerrainTracksRenderObjClass *m_usedModules;
    TerrainTracksRenderObjClass *m_freeModules;
    SceneClass *m_TerrainTracksScene;
    int m_edgesToFlush;
    int m_maxTankTrackEdges;
    int m_maxTankTrackOpaqueEdges;
    int m_maxTankTrackFadeDelay;
    friend class TerrainTracksRenderObjClass;
};

#ifdef GAME_DLL
extern TerrainTracksRenderObjClassSystem *&g_theTerrainTracksRenderObjClassSystem;
#else
extern TerrainTracksRenderObjClassSystem *g_theTerrainTracksRenderObjClassSystem;
#endif
