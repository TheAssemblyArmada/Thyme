/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Renderer
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
#include "autopool.h"
#include "meshmatdesc.h"
#include "meshmdl.h"
#include "multilist.h"
#include "shader.h"
#include "simplevec.h"
#include "vector.h"
#include "vector3.h"

class CameraClass;
class DX8FVFCategoryContainer;
class DecalMeshClass;
class DX8TextureCategoryClass;
class TextureClass;
class VertexMaterialClass;
class DX8PolygonRendererClass;
class PolyRenderTaskClass;
class MatPassTaskClass;
class IndexBufferClass;
class MeshClass;
class MaterialPassClass;
class MeshModelClass;
class Vertex_Split_Table;
class VertexBufferClass;

class DX8MeshRendererClass
{
public:
    DX8MeshRendererClass();
    ~DX8MeshRendererClass();

    void Init();
    void Shutdown();

    void Flush();
    void Clear_Pending_Delete_Lists();

    void Log_Statistics_String(bool only_visible);

    void Register_Mesh_Type(MeshModelClass *mmc);
    void Unregister_Mesh_Type(MeshModelClass *mmc);

    void Set_Camera(CameraClass *camera) { m_camera = camera; }
    CameraClass *Peek_Camera() { return m_camera; }

    void Add_To_Render_List(DecalMeshClass *decalmesh);

    void Enable_Lighting(bool enable) { m_enableLighting = enable; }

    void Invalidate(bool shutdown);

    static void Request_Log_Statistics();

protected:
    void Render_Decal_Meshes();

    bool m_enableLighting;
    CameraClass *m_camera;
    SimpleDynVecClass<MultiListClass<DX8FVFCategoryContainer> *> m_textureCategoryContainerListsRigid;
    MultiListClass<DX8FVFCategoryContainer> *m_textureCategoryContainerListSkin;
    DecalMeshClass *m_visibleDecalMeshes;
};

class DX8TextureCategoryClass : public MultiListObjectClass
{
public:
    DX8TextureCategoryClass(
        DX8FVFCategoryContainer *container, TextureClass **texs, ShaderClass shd, VertexMaterialClass *mat, int pass);
    ~DX8TextureCategoryClass() override;

    void Add_Render_Task(DX8PolygonRendererClass *p_renderer, MeshClass *p_mesh);
    void Render();
    bool Anything_To_Render() { return m_renderTaskHead != nullptr; }
    void Clear_Render_List() { m_renderTaskHead = nullptr; }

    TextureClass *Peek_Texture(int stage) { return m_textures[stage]; }
    VertexMaterialClass *Peek_Material() { return m_material; }

    ShaderClass Get_Shader() { return m_shader; }
    DX8FVFCategoryContainer *Get_Container() { return m_container; }
    MultiListClass<DX8PolygonRendererClass> &Get_Polygon_Renderer_List() { return m_polygonRendererList; }

    unsigned int Add_Mesh(Vertex_Split_Table &split_table,
        unsigned int vertex_offset,
        unsigned int index_offset,
        IndexBufferClass *index_buffer,
        unsigned int pass);

    void Log(bool only_visible);

    void Remove_Polygon_Renderer(DX8PolygonRendererClass *p_renderer);
    void Add_Polygon_Renderer(DX8PolygonRendererClass *p_renderer, DX8PolygonRendererClass *add_after_this);

    static void Set_Force_Multiply(bool multiply) { s_forceMultiply = multiply; }

private:
    int m_pass;
    TextureClass *m_textures[2];
    ShaderClass m_shader;
    VertexMaterialClass *m_material;
    MultiListClass<DX8PolygonRendererClass> m_polygonRendererList;
    DX8FVFCategoryContainer *m_container;
    PolyRenderTaskClass *m_renderTaskHead;
#ifdef GAME_DLL
    static bool &s_forceMultiply;
#else
    static bool s_forceMultiply;
#endif

public:
    static constexpr int TextureSize = ARRAY_SIZE(m_textures); // Thyme specific
};

class DX8FVFCategoryContainer : public MultiListObjectClass
{
public:
    enum
    {
        MAX_PASSES = 4
    };

    DX8FVFCategoryContainer(unsigned int FVF, bool sorting);
    ~DX8FVFCategoryContainer() override;

    virtual void Render() = 0;

    virtual void Add_Mesh(MeshModelClass *mmc) = 0;

    virtual void Log(bool only_visible) = 0;

    virtual bool Check_If_Mesh_Fits(MeshModelClass *mmc) = 0;

    virtual void Add_Delayed_Visible_Material_Pass(MaterialPassClass *pass, MeshClass *mesh) = 0;
    virtual void Render_Delayed_Procedural_Material_Passes() = 0;

    bool Is_Sorting() { return m_sorting; }

    void Change_Polygon_Renderer_Texture(MultiListClass<DX8PolygonRendererClass> &polygon_renderer_list,
        TextureClass *texture,
        TextureClass *new_texture,
        unsigned int pass,
        unsigned int stage);
    void Change_Polygon_Renderer_Material(MultiListClass<DX8PolygonRendererClass> &polygon_renderer_list,
        VertexMaterialClass *vmat,
        VertexMaterialClass *new_vmat,
        unsigned int pass);

    void Remove_Texture_Category(DX8TextureCategoryClass *tex_category);

    unsigned int Get_FVF() { return m_FVF; }

    void Add_Visible_Texture_Category(DX8TextureCategoryClass *tex_category, int pass)
    {
        captainslog_assert(pass >= 0 && pass < MAX_PASSES);
        captainslog_assert(tex_category != nullptr);
        captainslog_assert(m_textureCategoryList[pass].Contains(tex_category));
        m_visibleTextureCategoryList[pass].Add(tex_category);
        m_anythingToRender = true;
    }

    void Add_Visible_Material_Pass(MaterialPassClass *pass, MeshClass *mesh);

    static unsigned int Define_FVF(MeshModelClass *mmc, bool enable_lighting);

protected:
    bool Anything_To_Render() { return m_anythingToRender; }
    bool Any_Delayed_Passes_To_Render() { return m_anyDelayedPassesToRender; }

    void Render_Procedural_Material_Passes();

    DX8TextureCategoryClass *Find_Matching_Texture_Category(
        VertexMaterialClass *vmat, unsigned int pass, DX8TextureCategoryClass *ref_category);
    DX8TextureCategoryClass *Find_Matching_Texture_Category(
        TextureClass *texture, unsigned int pass, unsigned int stage, DX8TextureCategoryClass *ref_category);

    void Insert_To_Texture_Category(Vertex_Split_Table &split_table,
        TextureClass **texs,
        VertexMaterialClass *mat,
        ShaderClass shader,
        int pass,
        unsigned int vertex_offset);

    void Generate_Texture_Categories(Vertex_Split_Table &split_table, unsigned int vertex_offset);

    MultiListClass<DX8TextureCategoryClass> m_textureCategoryList[MAX_PASSES];
    MultiListClass<DX8TextureCategoryClass> m_visibleTextureCategoryList[MAX_PASSES];
    MatPassTaskClass *m_visibleMatpassHead;
    MatPassTaskClass *m_visibleMatpassTail;
    IndexBufferClass *m_indexBuffer;
    int m_usedIndices;
    unsigned int m_FVF;
    unsigned int m_passes;
    unsigned int m_uvCoordinateChannels;
    bool m_sorting;
    bool m_anythingToRender;
    bool m_anyDelayedPassesToRender;
};

class DX8RigidFVFCategoryContainer : public DX8FVFCategoryContainer
{
public:
    DX8RigidFVFCategoryContainer(unsigned int FVF, bool sorting);
    ~DX8RigidFVFCategoryContainer() override;

    void Add_Mesh(MeshModelClass *mmc) override;

    void Log(bool only_visible) override;

    bool Check_If_Mesh_Fits(MeshModelClass *mmc) override;

    void Render() override;

    void Add_Delayed_Visible_Material_Pass(MaterialPassClass *pass, MeshClass *mesh) override;
    void Render_Delayed_Procedural_Material_Passes() override;

protected:
    VertexBufferClass *m_vertexBuffer;
    int m_usedVertices;
    MatPassTaskClass *m_delayedVisibleMatpassHead;
    MatPassTaskClass *m_delayedVisibleMatpassTail;
};

class DX8SkinFVFCategoryContainer : public DX8FVFCategoryContainer
{
public:
    DX8SkinFVFCategoryContainer(bool sorting);
    ~DX8SkinFVFCategoryContainer() override;

    void Add_Mesh(MeshModelClass *mmc) override;

    void Log(bool only_visible) override;

    bool Check_If_Mesh_Fits(MeshModelClass *mmc) override;

    void Render() override;

    void Add_Delayed_Visible_Material_Pass(MaterialPassClass *pass, MeshClass *mesh) override;
    void Render_Delayed_Procedural_Material_Passes() override;

    void Add_Visible_Skin(MeshClass *mesh);

private:
    void Reset();
    void Clear_Visible_Skin_List();

    unsigned int m_visibleVertexCount;
    MeshClass *m_visibleSkinHead;
    MeshClass *m_visibleSkinTail;
};

#ifdef GAME_DLL
extern DX8MeshRendererClass &g_theDX8MeshRenderer;
extern DynamicVectorClass<Vector3> &g_tempVertexBuffer;
extern DynamicVectorClass<Vector3> &g_tempNormalBuffer;
extern MultiListClass<MeshModelClass> &g_registeredMeshList;
extern MultiListClass<DX8TextureCategoryClass> &g_textureCategoryDeleteList;
extern MultiListClass<DX8FVFCategoryContainer> &g_fvfCategoryContainerDeleteList;
#else
extern DX8MeshRendererClass g_theDX8MeshRenderer;
extern DynamicVectorClass<Vector3> g_tempVertexBuffer;
extern DynamicVectorClass<Vector3> g_tempNormalBuffer;
extern MultiListClass<MeshModelClass> g_registeredMeshList;
extern MultiListClass<DX8TextureCategoryClass> g_textureCategoryDeleteList;
extern MultiListClass<DX8FVFCategoryContainer> g_fvfCategoryContainerDeleteList;
#endif
