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
#include "multilist.h"
#include "simplevec.h"
#include "vector.h"
#include "vector3.h"
#include "shader.h"
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

class DX8MeshRendererClass
{
private:
    bool m_enableLighting;
    CameraClass *m_camera;
    SimpleDynVecClass<MultiListClass<DX8FVFCategoryContainer> *> m_textureCategoryContainerListsRigid;
    MultiListClass<DX8FVFCategoryContainer> *m_textureCategoryContainerListSkin;
    DecalMeshClass *m_visibleDecalMeshes;

public:
    void Init();
    void Shutdown();
    void Invalidate();
    void Clear_Pending_Delete_Lists();
};

class DX8TextureCategoryClass : public MultiListObjectClass
{
private:
    int m_pass;
    TextureClass *m_textures[2];
    ShaderClass m_shader;
    VertexMaterialClass *m_material;
    MultiListClass<DX8PolygonRendererClass> m_polygonRendererList;
    DX8FVFCategoryContainer *m_container;
    PolyRenderTaskClass *m_renderTaskHead;

public:
    ~DX8TextureCategoryClass();
};

class DX8FVFCategoryContainer : public MultiListObjectClass
{
private:
    MultiListClass<DX8TextureCategoryClass> m_textureCategoryList[4];
    MultiListClass<DX8TextureCategoryClass> m_visibleTextureCategoryList[4];
    MatPassTaskClass *m_visibleMatpassHead;
    MatPassTaskClass *m_visibleMatpassTail;
    IndexBufferClass *m_indexBuffer;
    int m_usedIndices;
    unsigned int m_FVF;
    unsigned int m_passes;
    unsigned int m_uvCoordinateChannels;
    bool m_sorting;
    bool m_anythingToRender;
    bool m_delayedAnythingToRender;

public:
    ~DX8FVFCategoryContainer();
    virtual void Render() = 0;
    virtual void Add_Mesh(MeshClass *mesh) = 0;
    virtual void Log(bool only_visible) = 0;
    virtual bool Check_If_Mesh_Fits(MeshModelClass *mmc) = 0;
    virtual void Add_Delayed_Visible_Material_Pass(MaterialPassClass *, MeshClass *) = 0;
    virtual void Render_Delayed_Procedural_Material_Passes() = 0;
};

#ifdef GAME_DLL
extern DX8MeshRendererClass &g_theDX8MeshRenderer;
extern DynamicVectorClass<Vector3> &g_tempVertexBuffer;
extern DynamicVectorClass<Vector3> &g_tempNormalBuffer;
extern MultiListClass<DX8TextureCategoryClass> &g_textureCategoryDeleteList;
extern MultiListClass<DX8FVFCategoryContainer> &g_fvfCategoryContainerDeleteList;
#else
extern DX8MeshRendererClass g_theDX8MeshRenderer;
extern DynamicVectorClass<Vector3> g_tempVertexBuffer;
extern DynamicVectorClass<Vector3> g_tempNormalBuffer;
extern MultiListClass<DX8TextureCategoryClass> g_textureCategoryDeleteList;
extern MultiListClass<DX8FVFCategoryContainer> g_fvfCategoryContainerDeleteList;
#endif
