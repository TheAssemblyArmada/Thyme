/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Dynamic Mesh
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
#include "matinfo.h"
#include "meshgeometry.h"
#include "meshmatdesc.h"
#include "rendobj.h"

class DynamicMeshModel : public MeshGeometryClass
{
    IMPLEMENT_W3D_POOL(DynamicMeshModel)

private:
    int m_dynamicMeshPNum;
    int m_dynamicMeshVNum;
    MeshMatDescClass *m_matDesc;
    MaterialInfoClass *m_matInfo;

public:
    DynamicMeshModel(unsigned int max_polys, unsigned int max_verts);
    DynamicMeshModel(unsigned int max_polys, unsigned int max_verts, MaterialInfoClass *mat_info);
    DynamicMeshModel(const DynamicMeshModel &src);

    virtual ~DynamicMeshModel() override;
    virtual void Compute_Bounds(Vector3 *verts);
    virtual void Compute_Plane_Equations();
    virtual void Compute_Vertex_Normals();

    void Reset();
    void Initialize_Texture_Array(int pass, int stage, TextureClass *texture);
    void Initialize_Material_Array(int pass, VertexMaterialClass *material);
    void Render(RenderInfoClass &rinfo);

    void Set_Counts(int poly_count, int vert_count)
    {
        m_dynamicMeshPNum = poly_count;
        m_dynamicMeshVNum = vert_count;
    }

    unsigned int *Get_Color_Array(int pass) { return m_matDesc->Get_Color_Array(pass); }
    Vector2 *Get_UV_Array(int pass) { return m_matDesc->Get_UV_Array_By_Index(pass); }
    void Set_Single_Shader(ShaderClass shader, int pass) { m_matDesc->Set_Single_Shader(shader, pass); }
    void Set_Single_Texture(TextureClass *tex, int pass, int stage) { m_matDesc->Set_Single_Texture(tex, pass, stage); }
    void Set_Single_Material(VertexMaterialClass *vmat, int pass) { m_matDesc->Set_Single_Material(vmat, pass); }
    void Set_Material(int vidx, VertexMaterialClass *vmat, int pass) { m_matDesc->Set_Material(vidx, vmat, pass); }
    void Set_Shader(int pidx, ShaderClass shader, int pass) { m_matDesc->Set_Shader(pidx, shader, pass); }
    void Set_Texture(int pidx, TextureClass *tex, int pass, int stage) { m_matDesc->Set_Texture(pidx, tex, pass, stage); }
    void Set_Pass_Count(int passes) { m_matDesc->Set_Pass_Count(passes); }
    int Get_Pass_Count() const { return m_matDesc->Get_Pass_Count(); }
    MaterialInfoClass *Peek_Material_Info() { return m_matInfo; }
    void Set_Material_Info(MaterialInfoClass *mat) { Ref_Ptr_Set(m_matInfo, mat); }
    TriIndex *Get_Non_Const_Polygon_Array() { return Get_Polys(); }

    MaterialInfoClass *Get_Material_Info()
    {
        if (m_matInfo) {
            m_matInfo->Add_Ref();
        }

        return m_matInfo;
    }

    Vector3 *Get_Non_Const_Vertex_Normal_Array()
    {
        if (Get_Flag(MeshGeometryClass::DIRTY_VNORMALS)) {
            Compute_Vertex_Normals();
        }

        return Get_Vert_Normals();
    }
};

class DynamicMeshClass : public RenderObjClass
{
protected:
    DynamicMeshModel *m_model;
    int m_vertexMaterialIdx[4];
    bool m_multiVertexMaterial[4];
    int m_textureIdx[4];
    bool m_multiTexture[4];
    Vector4 m_curVertexColor[2];
    bool m_multiVertexColor[2];
    int m_polyCount;
    int m_vertCount;
    int m_triVertexCount;
    int m_fanVertex;
    int m_triMode;
    char m_sortLevel;

public:
    enum
    {
        TRI_MODE_STRIPS,
        TRI_MODE_FANS,
    };

    DynamicMeshClass(int max_poly, int max_vert);
    DynamicMeshClass(int max_poly, int max_vert, MaterialInfoClass *mat_info);
    DynamicMeshClass(const DynamicMeshClass &src);

    virtual ~DynamicMeshClass();
    virtual RenderObjClass *Clone() const;
    virtual int Class_ID() const override { return CLASSID_DYNAMESH; }
    virtual int Get_Num_Polys() const override { return m_polyCount; }
    virtual void Render(RenderInfoClass &rinfo) override;

    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override
    {
        if (!Bounding_Volumes_Valid()) {
            m_model->Compute_Bounds(nullptr);
        }

        m_model->Get_Bounding_Sphere(&sphere);
    }

    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override
    {
        if (!Bounding_Volumes_Valid()) {
            m_model->Compute_Bounds(0);
        }

        m_model->Get_Bounding_Box(&box);
    }

    virtual MaterialInfoClass *Get_Material_Info() override { return m_model->Get_Material_Info(); }
    virtual int Get_Sort_Level() const override { return m_sortLevel; }

    virtual void Set_Sort_Level(int sort) override
    {
        m_sortLevel = (char)sort;

        if (sort) {
            m_model->Set_Flag(MeshGeometryClass::SORT, false);
        }
    }

    virtual MaterialInfoClass *Peek_Material_Info() { return m_model->Peek_Material_Info(); }
    virtual void Set_Material_Info(MaterialInfoClass *mat) { m_model->Set_Material_Info(mat); }
    virtual int Get_Num_Vertices() const { return m_vertCount; }

    virtual void Reset()
    {
        Reset_Flags();
        Reset_Native_Mesh();
        Reset_Mesh_Counters();

        for (int i = 0; i < 4; i++) {
            m_textureIdx[i] = -1;
            m_vertexMaterialIdx[i] = -1;
            m_multiVertexMaterial[i] = false;
        }
    }

    virtual void Location(float x, float y, float z);
    virtual void Move_Vertex(int index, float x, float y, float z);
    virtual void Get_Vertex(int index, float &x, float &y, float &z);
    virtual void Change_Vertex_Color(int index, const Vector4 &color, int color_array_index);
    virtual bool Flip_Face() { return (m_triVertexCount & 1) == 0; }

    bool End_Vertex();
    void Resize(int max_polys, int max_verts);
    void Translate_Vertices(const Vector3 &offset);
    void Switch_To_Multi_Vertex_Color(int color_array_index);
    int Set_Texture(int idx, int pass);
    int Set_Vertex_Material(int idx, int pass);
    int Set_Texture(TextureClass *texture, bool dont_search, int pass);
    int Set_Vertex_Material(VertexMaterialClass *texture, bool dont_search, int pass);
    void Color(Vector4 const &color, int pass);
    void Color(float r, float g, float b, float a, int pass);
    // void Set_Polygon_Info(PolygonInfoClass &info, bool b, bool b2, int i); // don't have PoygonInfoClass
    int Set_Vertex_Color(class Vector3 &color, int idx);
    int Set_Vertex_Color(class Vector4 &color, int idx);

    void Set_Pass_Count(int count) { m_model->Set_Pass_Count(count); }
    int Get_Pass_Count() { return m_model->Get_Pass_Count(); }
    int Set_Shader(ShaderClass const &shader, int pass)
    {
        m_model->Set_Single_Shader(shader, pass);
        return 0;
    }

    void Reset_Flags()
    {
        m_model->Set_Flag(MeshGeometryClass::DIRTY_BOUNDS, true);
        m_model->Set_Flag(MeshGeometryClass::DIRTY_PLANES, true);
        m_model->Set_Flag(MeshGeometryClass::DIRTY_VNORMALS, true);
    }

    void Reset_Native_Mesh() { m_model->Reset(); }

    void Reset_Mesh_Counters()
    {
        m_model->Set_Counts(0, 0);
        m_model->Set_Flag(MeshGeometryClass::SORT, false);
        m_polyCount = 0;
        m_vertCount = 0;
    }

    void Begin_Vertex() {}

    void Begin_Tri_Strip()
    {
        m_triVertexCount = 0;
        m_triMode = TRI_MODE_STRIPS;
    }

    void Begin_Tri_Fan()
    {
        m_triVertexCount = 0;
        m_triMode = TRI_MODE_FANS;
        m_fanVertex = m_vertCount;
    }

    void UV(float u, float v, int pass)
    {
        Vector2 *uv = m_model->Get_UV_Array(pass);
        uv[m_vertCount].U = u;
        uv[m_vertCount].V = v;
    }

    bool Vertex(Vector2 v)
    {
        Begin_Vertex();
        Location(v.X, v.Y, 0.0f);
        return End_Vertex();
    }

    bool Vertex(float x, float y, float z, float u, float v)
    {
        Begin_Vertex();
        Location(x, y, z);
        UV(u, v, 0);
        return End_Vertex();
    }

    void Color(unsigned int col, int pass)
    {
        unsigned int *color = m_model->Get_Color_Array(pass);
        color[m_vertCount] = col;
    }

    void Location_Inline(float x, float y, float z)
    {
        Vector3 *verts = m_model->Get_Vertex_Array();
        verts[m_vertCount].X = x;
        verts[m_vertCount].Y = y;
        verts[m_vertCount].Z = z;
    }

    void Normal(float x, float y, float z)
    {
        Vector3 *norms = m_model->Get_Non_Const_Vertex_Normal_Array();
        norms[m_vertCount].X = x;
        norms[m_vertCount].Y = y;
        norms[m_vertCount].Z = z;
    }

    void End_Tri_Strip() { m_triVertexCount = 0; }
    void End_Tri_Fan() { m_triVertexCount = 0; }
    void Set_Dirty_Bounds() { m_model->Set_Flag(MeshGeometryClass::DIRTY_BOUNDS, true); }
    void Clear_Dirty_Bounds() { m_model->Set_Flag(MeshGeometryClass::DIRTY_BOUNDS, false); }
    void Set_Dirty_Vertex_Normals() { m_model->Set_Flag(MeshGeometryClass::DIRTY_VNORMALS, true); }
    void Clear_Dirty_Vertex_Normals() { m_model->Set_Flag(MeshGeometryClass::DIRTY_VNORMALS, false); }
    void Set_Dirty_Planes() { m_model->Set_Flag(MeshGeometryClass::DIRTY_PLANES, true); }
    void Clear_Dirty_Planes() { m_model->Set_Flag(MeshGeometryClass::DIRTY_PLANES, false); }
    void Disable_Sort() { m_model->Set_Flag(MeshGeometryClass::SORT, false); }
    void Enable_Sort() { m_model->Set_Flag(MeshGeometryClass::SORT, true); }
    void Set_Dirty() { Reset_Flags(); }
    void Disable_Bounding_Box() { m_model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_BOX, false); }
    void Enable_Bounding_Box() { m_model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_BOX, true); }
    void Disable_Bounding_Sphere() { m_model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_SPHERE, false); }
    void Enable_Bounding_Sphere() { m_model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_SPHERE, true); }
    unsigned int Get_Color(int idx, int pass) { return m_model->Get_Color_Array(pass)[idx]; }
    Vector3 &Get_Location(int idx) { return m_model->Get_Vertex_Array()[idx]; }
    Vector3 &Get_Normal(int idx) { return m_model->Get_Non_Const_Vertex_Normal_Array()[idx]; }
    Vector2 &Get_UV(int idx, int pass) { return m_model->Get_UV_Array(pass)[idx]; }
    void Location_Inline(Vector3 &loc) { Location_Inline(loc.X, loc.Y, loc.Z); }
    void Normal(Vector3 &norm) { Normal(norm.X, norm.Y, norm.Z); }
    DynamicMeshModel *Peek_Model() { return m_model; }
    bool Sort_Enabled() { return m_model->Get_Flag(MeshGeometryClass::SORT) != 0; }
    bool Test_Bounding_Box() { return m_model->Get_Flag(MeshGeometryClass::DISABLE_BOUNDING_BOX) != 0; }
    bool Test_Bounding_Sphere() { return m_model->Get_Flag(MeshGeometryClass::DISABLE_BOUNDING_SPHERE) != 0; }
    void UV(Vector2 &uv, int pass) { UV(uv.U, uv.V, pass); }
};

class DynamicScreenMeshClass : public DynamicMeshClass
{
private:
    float m_aspect;

public:
    DynamicScreenMeshClass(const DynamicScreenMeshClass &src);
    DynamicScreenMeshClass(int max_poly, int max_vert, float aspect);

    virtual ~DynamicScreenMeshClass() override {}
    virtual RenderObjClass *Clone() const override;
    virtual int Class_ID() const override { return CLASSID_DYNASCREENMESH; }
    virtual void Set_Position(const Vector3 &v) override;
    virtual void Reset() override;
    virtual void Location(float x, float y, float z) override;
    virtual void Move_Vertex(int index, float x, float y, float z) override;
    virtual bool Flip_Face() override { return m_triVertexCount & 1; }
    virtual void Set_Aspect(float aspect) { m_aspect = aspect; }
};
