/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Class
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
#include "dx8polygonrenderer.h"
#include "lightenv.h"
#include "proto.h"
#include "rendobj.h"
#include "w3derr.h"
#include "w3dmpo.h"

class MeshBuilderClass;
class MaterialPassClass;
class IndexBufferClass;
class MeshModelClass;
class DecalMeshClass;
class VertexMaterialClass;
class TextureClass;

class MeshClass : public W3DMPO, public RenderObjClass
{
    IMPLEMENT_W3D_POOL(MeshClass);

public:
    MeshClass();
    MeshClass(const MeshClass &src);
    ~MeshClass() override;

    RenderObjClass *Clone() const override;

    int Class_ID() const override { return CLASSID_MESH; }

    const char *Get_Name() const override;
    void Set_Name(const char *name) override;

    int Get_Num_Polys() const override;

    void Render(RenderInfoClass &rinfo) override;
    void Special_Render(SpecialRenderInfoClass &rinfo) override;

    bool Cast_Ray(RayCollisionTestClass &raytest) override;
    bool Cast_AABox(AABoxCollisionTestClass &boxtest) override;
    bool Cast_OBBox(OBBoxCollisionTestClass &boxtest) override;

    bool Intersect_AABox(AABoxIntersectionTestClass &boxtest) override;
    bool Intersect_OBBox(OBBoxIntersectionTestClass &boxtest) override;
    void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

    void Scale(float scale) override;
    void Scale(float scalex, float scaley, float scalez) override;

    MaterialInfoClass *Get_Material_Info() override;

    int Get_Sort_Level() const override;
    void Set_Sort_Level(int level) override;

    void Create_Decal(DecalGeneratorClass *generator) override;
    void Delete_Decal(unsigned long decal_id) override;

    MeshClass &operator=(const MeshClass &);

    W3DErrorType Load_W3D(ChunkLoadClass &cload);

    void Render_Material_Pass(MaterialPassClass *pass, IndexBufferClass *ib);

    void Generate_Culling_Tree();
    bool Contains(const Vector3 &point);

    void Replace_Texture(TextureClass *texture, TextureClass *new_texture);
    void Replace_VertexMaterial(VertexMaterialClass *vmat, VertexMaterialClass *new_vmat);
    void Make_Unique(bool force);

    MeshModelClass *Get_Model();
    uint32_t Get_W3D_Flags();
    const char *Get_User_Text() const;
    void Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm);
    void Get_Deformed_Vertices(Vector3 *dst_vert);
    LightEnvironmentClass *Get_Lighting_Environment() { return m_lightEnvironment; }
    float Get_Alpha_Override() { return m_alphaOverride; }
    int Get_Base_Vertex_Offset() { return m_baseVertexOffset; }

    MeshModelClass *Peek_Model() { return m_model; }
    MeshClass *Peek_Next_Visible_Skin() { return m_nextVisibleSkin; }

    void Set_Lighting_Environment(LightEnvironmentClass *light_env)
    {
        if (!light_env) {
            m_lightEnvironment = nullptr;
        } else {
            m_localLightEnv = *light_env;
            m_lightEnvironment = &m_localLightEnv;
        }
    }
    int Get_Draw_Call_Count();
    void Set_Next_Visible_Skin(MeshClass *next_visible) { m_nextVisibleSkin = next_visible; }
    void Set_Base_Vertex_Offset(int base) { m_baseVertexOffset = base; }

protected:
    unsigned Get_Debug_Id() const { return m_meshDebugId; }
    void Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list, bool textures_only = false) override;
    void Update_Cached_Bounding_Volumes() const override;
    void Free();
    int Get_Draw_Call_Count() const;
    void Set_Debugger_Disable(bool b) { m_isDisabledByDebugger = b; }
    bool Is_Disabled_By_Debugger() const { return m_isDisabledByDebugger; }

    MeshModelClass *m_model;
    DecalMeshClass *m_decalMesh;
    LightEnvironmentClass *m_lightEnvironment;
    LightEnvironmentClass m_localLightEnv;
    float m_alphaOverride;
    float m_emissiveScale;
    float m_opacityOverride;
    int m_baseVertexOffset;
    MeshClass *m_nextVisibleSkin;
    unsigned int m_meshDebugId;
    bool m_isDisabledByDebugger;
};

class PrimitivePrototypeClass : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(PrimitivePrototypeClass);

public:
    PrimitivePrototypeClass(RenderObjClass *proto)
    {
        m_proto = proto;
        proto->Add_Ref();
    }

    ~PrimitivePrototypeClass() override { m_proto->Release_Ref(); }
    const char *Get_Name() const override { return m_proto->Get_Name(); }
    int Get_Class_ID() const override { return m_proto->Class_ID(); }
    RenderObjClass *Create() override { return m_proto->Clone(); }
    void Delete_Self() override { delete this; };

private:
    RenderObjClass *m_proto;
};

class MeshLoaderClass : public PrototypeLoaderClass
{
public:
    int Chunk_Type() override { return W3D_CHUNK_MESH; }
    PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};
