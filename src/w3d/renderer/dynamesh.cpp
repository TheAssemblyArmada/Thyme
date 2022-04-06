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
#include "dynamesh.h"
#include "camera.h"
#include "dx8wrapper.h"
#include "rinfo.h"
#include "sortingrenderer.h"
#include "w3d.h"

DynamicMeshModel::DynamicMeshModel(unsigned int max_polys, unsigned int max_verts) :
    m_dynamicMeshPNum(0), m_dynamicMeshVNum(0), m_matDesc(0), m_matInfo(0)
{
    m_matInfo = new MaterialInfoClass();
    m_matDesc = new MeshMatDescClass();
    m_matDesc->Set_Polygon_Count(max_polys);
    m_matDesc->Set_Vertex_Count(max_verts);
    Reset_Geometry(max_polys, max_verts);
}

DynamicMeshModel::DynamicMeshModel(unsigned int max_polys, unsigned int max_verts, MaterialInfoClass *mat_info) :
    m_dynamicMeshPNum(0), m_dynamicMeshVNum(0), m_matDesc(0), m_matInfo(0)
{
    Ref_Ptr_Set(m_matInfo, mat_info);
    m_matDesc = new MeshMatDescClass();
    m_matDesc->Set_Polygon_Count(max_polys);
    m_matDesc->Set_Vertex_Count(max_verts);
    Reset_Geometry(max_polys, max_verts);
}

DynamicMeshModel::DynamicMeshModel(const DynamicMeshModel &src) :
    MeshGeometryClass(src),
    m_dynamicMeshPNum(src.m_dynamicMeshPNum),
    m_dynamicMeshVNum(src.m_dynamicMeshVNum),
    m_matDesc(0),
    m_matInfo(0)
{
    m_matInfo = new MaterialInfoClass(*src.m_matInfo);
    m_matDesc = new MeshMatDescClass();
    MaterialRemapperClass remapper(src.m_matInfo, m_matInfo);
    remapper.Remap_Mesh(src.m_matDesc, m_matDesc);
}

DynamicMeshModel::~DynamicMeshModel()
{
    delete m_matDesc;
    m_matDesc = nullptr;
    Ref_Ptr_Release(m_matInfo);
}

void DynamicMeshModel::Compute_Bounds(Vector3 *verts)
{
    int polycount = m_polyCount;
    int vertcount = m_vertexCount;
    m_polyCount = m_dynamicMeshPNum;
    m_vertexCount = m_dynamicMeshVNum;
    MeshGeometryClass::Compute_Bounds(verts);
    m_polyCount = polycount;
    m_vertexCount = vertcount;
}

void DynamicMeshModel::Compute_Plane_Equations()
{
    Get_Vert_Normals();
    Vector4 *planes = Get_Planes();
    int polycount = m_polyCount;
    int vertcount = m_vertexCount;
    m_polyCount = m_dynamicMeshPNum;
    m_vertexCount = m_dynamicMeshVNum;
    MeshGeometryClass::Compute_Plane_Equations(planes);
    m_polyCount = polycount;
    m_vertexCount = vertcount;
}

void DynamicMeshModel::Compute_Vertex_Normals()
{
    Vector3 *normals = Get_Vert_Normals();
    Get_Planes();
    int polycount = m_polyCount;
    int vertcount = m_vertexCount;
    m_polyCount = m_dynamicMeshPNum;
    m_vertexCount = m_dynamicMeshVNum;
    MeshGeometryClass::Compute_Vertex_Normals(normals);
    m_polyCount = polycount;
    m_vertexCount = vertcount;
}

void DynamicMeshModel::Reset()
{
    m_dynamicMeshPNum = 0;
    m_dynamicMeshVNum = 0;
    int polycount = m_polyCount;
    int vertcount = m_vertexCount;
    Reset_Geometry(m_polyCount, m_vertexCount);
    m_matDesc->Reset(polycount, vertcount, 1);
    Ref_Ptr_Release(m_matInfo);
    m_matInfo = new MaterialInfoClass();
}

void DynamicMeshModel::Initialize_Texture_Array(int pass, int stage, TextureClass *texture)
{
    TexBufferClass *buf = m_matDesc->Get_Texture_Array(pass, 0);

    for (int i = 0; i < m_polyCount; i++) {
        buf->Set_Element(i, texture);
    }
}

void DynamicMeshModel::Initialize_Material_Array(int pass, VertexMaterialClass *material)
{
    MatBufferClass *buf = m_matDesc->Get_Material_Array(pass, 0);

    for (int i = 0; i < m_polyCount; i++) {
        buf->Set_Element(i, material);
    }
}

void DynamicMeshModel::Render(RenderInfoClass &rinfo)
{
    bool sorted = Get_Flag(MeshGeometryClass::SORT) && W3D::Is_Sorting_Enabled();
    DynamicVBAccessClass dynamic_vb(
        sorted ? VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING : VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8,
        DX8_FVF_XYZNDUV2,
        m_dynamicMeshVNum);
    {
        DynamicVBAccessClass::WriteLockClass vb_lock(&dynamic_vb);
        VertexFormatXYZNDUV2 *buffer = vb_lock.Get_Formatted_Vertex_Array();

        const Vector3 *vertices = Get_Vertex_Array();
        const Vector3 *normals = Get_Vertex_Normal_Array();
        const Vector2 *uv0 = Get_UV_Array(0);
        const Vector2 *uv1 = Get_UV_Array(1);
        const unsigned int *colors = Get_Color_Array(0);
        static Vector3 default_normal(0.0f, 0.0f, 0.0f);
        static Vector2 default_uv(0.0f, 0.0f);

        for (int v = 0; v < m_dynamicMeshVNum; v++) {
            buffer->x = vertices[v].X;
            buffer->y = vertices[v].Y;
            buffer->z = vertices[v].Z;
            buffer->nx = normals[v].X;
            buffer->ny = normals[v].Y;
            buffer->nz = normals[v].Z;

            if (uv0) {
                buffer->u1 = uv0[v].X;
                buffer->v1 = uv0[v].Y;
            } else {
                buffer->u1 = 0;
                buffer->v1 = 0;
            }

            if (uv1) {
                buffer->u2 = uv1[v].X;
                buffer->v2 = uv1[v].Y;
            } else {
                buffer->u2 = 0;
                buffer->v2 = 0;
            }

            if (colors) {
                buffer->diffuse = colors[v];
            } else {
                buffer->diffuse = 0xFFFFFFFF;
            }

            ++buffer;
        }
    }

    const TriIndex *polygons = Get_Polygon_Array();
    int index_count = m_dynamicMeshPNum * 3;
    DynamicIBAccessClass dynamic_ib(
        sorted ? IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING : IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, index_count);
    {
        DynamicIBAccessClass::WriteLockClass lock(&dynamic_ib);
        unsigned short *ib_start = lock.Get_Index_Array();
        memcpy(ib_start, polygons, index_count * sizeof(unsigned short));
    }

    DX8Wrapper::Set_Vertex_Buffer(dynamic_vb);
    DX8Wrapper::Set_Index_Buffer(dynamic_ib, 0);

    int passes = Get_Pass_Count();
    for (int pass = 0; pass < passes; ++pass) {
        unsigned short min_vert_idx = m_dynamicMeshVNum - 1;
        unsigned short i1 = 0;
        unsigned short start_tri_idx = 0;
        unsigned short cur_tri_idx = 0;

        TexBufferClass *tb0 = m_matDesc->Get_Texture_Array(pass, 0, false);
        TextureClass **texture0 = nullptr;

        if (tb0 != nullptr) {
            texture0 = tb0->Get_Array();
        }

        TexBufferClass *tb1 = m_matDesc->Get_Texture_Array(pass, 1, false);
        TextureClass **texture1 = nullptr;

        if (tb1 != nullptr) {
            texture1 = tb1->Get_Array();
        }

        MatBufferClass *mb = m_matDesc->Get_Material_Array(pass, false);
        VertexMaterialClass **material = nullptr;

        if (mb != nullptr) {
            material = mb->Get_Array();
        }

        ShaderClass *shader = m_matDesc->Get_Shader_Array(pass, false);

        if (texture0 != nullptr) {
            DX8Wrapper::Set_Texture(0, texture0[0]);
        } else {
            DX8Wrapper::Set_Texture(0, m_matDesc->Peek_Single_Texture(pass, 0));
        }

        if (texture1 != nullptr) {
            DX8Wrapper::Set_Texture(1, texture1[0]);
        } else {
            DX8Wrapper::Set_Texture(1, m_matDesc->Peek_Single_Texture(pass, 1));
        }

        if (material != nullptr) {
            DX8Wrapper::Set_Material(material[polygons[0].I]);
        } else {
            DX8Wrapper::Set_Material(m_matDesc->Peek_Single_Material(pass));
        }

        if (shader != nullptr) {
            DX8Wrapper::Set_Shader(shader[0]);
        } else {
            DX8Wrapper::Set_Shader(m_matDesc->Get_Single_Shader(pass));
        }

        SphereClass sphere;
        Get_Bounding_Sphere(&sphere);

        if (texture0 != nullptr || texture1 != nullptr || material != nullptr || shader != nullptr) {
            bool done = false;
            do {
                const TriIndex *poly = &polygons[cur_tri_idx];
                unsigned short i2 = poly->J;

                if (i2 > poly->I) {
                    i2 = poly->I;
                }

                unsigned short i3 = poly->K;

                if (i3 > i2) {
                    i3 = i2;
                }

                unsigned short i4 = poly->J;

                if (i4 < poly->I) {
                    i4 = poly->I;
                }

                unsigned short i5 = poly->K;

                if (i5 < i4) {
                    i5 = i4;
                }

                if (i3 > min_vert_idx) {
                    i3 = min_vert_idx;
                }

                min_vert_idx = i3;
                unsigned short max_vert_idx = i5;

                if (i5 < i1) {
                    max_vert_idx = i1;
                }

                i1 = max_vert_idx;
                unsigned short next_tri_idx = cur_tri_idx + 1;
                done = next_tri_idx >= m_dynamicMeshPNum;
                bool texture0_changed = false;
                bool texture1_changed = false;
                bool material_changed = false;
                bool shader_changed = false;

                if (next_tri_idx < m_dynamicMeshPNum) {
                    if (texture0 != nullptr && texture0[cur_tri_idx] != texture0[next_tri_idx]) {
                        texture0_changed = true;
                    }

                    if (texture1 != nullptr && texture1[cur_tri_idx] != texture1[next_tri_idx]) {
                        texture1_changed = true;
                    }

                    if (material != nullptr && material[polygons[cur_tri_idx].I] != material[polygons[next_tri_idx].I]) {
                        material_changed = true;
                    }

                    if (shader != nullptr && shader[cur_tri_idx] != shader[next_tri_idx]) {
                        shader_changed = true;
                    }
                }

                if (next_tri_idx >= m_dynamicMeshPNum || texture0_changed || material_changed || shader_changed) {
                    if (sorted) {
                        SortingRendererClass::Insert_Triangles(sphere,
                            3 * start_tri_idx,
                            cur_tri_idx - start_tri_idx + 1,
                            min_vert_idx,
                            max_vert_idx - min_vert_idx + 1);
                    } else {
                        DX8Wrapper::Draw_Triangles(3 * start_tri_idx,
                            cur_tri_idx - start_tri_idx + 1,
                            min_vert_idx,
                            max_vert_idx - min_vert_idx + 1);
                    }

                    start_tri_idx = cur_tri_idx + 1;
                    min_vert_idx = m_dynamicMeshVNum - 1;
                    i1 = 0;

                    if (texture0_changed) {
                        DX8Wrapper::Set_Texture(0, texture0[next_tri_idx]);
                    }

                    if (texture1_changed) {
                        DX8Wrapper::Set_Texture(1, texture1[next_tri_idx]);
                    }

                    if (material_changed) {
                        DX8Wrapper::Set_Material(material[polygons[next_tri_idx].I]);
                    }

                    if (shader_changed) {
                        DX8Wrapper::Set_Shader(shader[next_tri_idx]);
                    }
                }
                cur_tri_idx++;
            } while (!done);
        } else {
            if (sorted) {
                SortingRendererClass::Insert_Triangles(sphere, 0, m_dynamicMeshPNum, 0, m_dynamicMeshVNum);
            } else {
                DX8Wrapper::Draw_Triangles(0, m_dynamicMeshPNum, 0, m_dynamicMeshVNum);
            }
        }
    }
}

DynamicMeshClass::DynamicMeshClass(int max_poly, int max_vert) :
    m_model(0),
    m_curVertexColor{},
    m_polyCount(0),
    m_vertCount(0),
    m_triVertexCount(0),
    m_fanVertex(0),
    m_triMode(TRI_MODE_STRIPS),
    m_sortLevel(0)
{
    for (int i = 0; i < 4; i++) {
        m_multiTexture[i] = false;
        m_textureIdx[i] = -1;
        m_multiVertexMaterial[i] = false;
        m_vertexMaterialIdx[i] = -1;
    }

    for (int i = 0; i < 2; i++) {
        m_multiVertexColor[i] = false;
        m_curVertexColor[i].X = 1.0f;
        m_curVertexColor[i].Y = 1.0f;
        m_curVertexColor[i].Z = 1.0f;
        m_curVertexColor[i].W = 1.0f;
    }

    m_model = new DynamicMeshModel(max_poly, max_vert);
}

DynamicMeshClass::DynamicMeshClass(int max_poly, int max_vert, MaterialInfoClass *mat_info) :
    m_model(0),
    m_curVertexColor{},
    m_polyCount(0),
    m_vertCount(0),
    m_triVertexCount(0),
    m_fanVertex(0),
    m_triMode(TRI_MODE_STRIPS),
    m_sortLevel(0)
{
    for (int i = 0; i < 4; i++) {
        m_multiTexture[i] = false;
        m_textureIdx[i] = -1;
        m_multiVertexMaterial[i] = false;
        m_vertexMaterialIdx[i] = -1;
    }

    for (int i = 0; i < 2; i++) {
        m_multiVertexColor[i] = false;
        m_curVertexColor[i].X = 1.0f;
        m_curVertexColor[i].Y = 1.0f;
        m_curVertexColor[i].Z = 1.0f;
        m_curVertexColor[i].W = 1.0f;
    }

    m_model = new DynamicMeshModel(max_poly, max_vert, mat_info);
}

DynamicMeshClass::DynamicMeshClass(const DynamicMeshClass &src) :
    m_model(0),
    m_curVertexColor{},
    m_polyCount(src.m_polyCount),
    m_vertCount(src.m_vertCount),
    m_triVertexCount(src.m_triVertexCount),
    m_fanVertex(src.m_fanVertex),
    m_triMode(src.m_triMode),
    m_sortLevel(src.m_sortLevel)
{
    for (int i = 0; i < 4; i++) {
        m_multiTexture[i] = src.m_multiTexture[i];
        m_textureIdx[i] = src.m_textureIdx[i];
        m_multiVertexMaterial[i] = src.m_multiVertexMaterial[i];
        m_vertexMaterialIdx[i] = src.m_vertexMaterialIdx[i];
    }

    for (int i = 0; i < 2; i++) {
        m_multiVertexColor[i] = src.m_multiVertexColor[i];
        m_curVertexColor[i].X = src.m_curVertexColor[i].X;
        m_curVertexColor[i].Y = src.m_curVertexColor[i].Y;
        m_curVertexColor[i].Z = src.m_curVertexColor[i].Z;
        m_curVertexColor[i].W = src.m_curVertexColor[i].W;
    }

    m_model = new DynamicMeshModel(*src.m_model);
}

DynamicMeshClass::~DynamicMeshClass()
{
    Ref_Ptr_Release(m_model);
}

RenderObjClass *DynamicMeshClass::Clone() const
{
    return new DynamicMeshClass(*this);
}

int DynamicMeshClass::Set_Texture(int idx, int pass)
{
    captainslog_assert(idx < Peek_Material_Info()->Texture_Count());
    m_textureIdx[pass] = idx;

    if (!m_multiTexture[pass]) {
        TextureClass *texture = Peek_Material_Info()->Get_Texture(idx);
        m_model->Set_Single_Texture(texture, pass, 0);
        texture->Release_Ref();
    }

    return m_textureIdx[pass];
}

int DynamicMeshClass::Set_Vertex_Material(int idx, int pass)
{
    m_vertexMaterialIdx[pass] = idx;

    if (!m_multiVertexMaterial[pass]) {
        VertexMaterialClass *material = Peek_Material_Info()->Get_Vertex_Material(idx);
        m_model->Set_Single_Material(material, pass);
        material->Release_Ref();
    }

    return m_vertexMaterialIdx[pass];
}

int DynamicMeshClass::Set_Texture(TextureClass *texture, bool dont_search, int pass)
{
    if (Peek_Material_Info()->Texture_Count() != 0) {
        if (m_textureIdx[pass] != -1) {
            if (Peek_Material_Info()->Peek_Texture(m_textureIdx[pass]) == texture) {
                return m_textureIdx[pass];
            }
        }
    }

    if (!m_multiTexture[pass]) {
        if (Peek_Material_Info()->Texture_Count() != 0) {
            if (m_textureIdx[pass] != -1) {
                if (Peek_Material_Info()->Peek_Texture(m_textureIdx[pass]) != texture) {
                    TextureClass *tex = Peek_Material_Info()->Get_Texture(m_textureIdx[pass]);
                    m_model->Initialize_Texture_Array(pass, 0, tex);
                    tex->Release_Ref();
                    m_multiTexture[pass] = true;
                }
            }
        }
    }

    if (!dont_search) {
        for (int i = 0; i < Peek_Material_Info()->Texture_Count(); i++) {
            TextureClass *tex = Peek_Material_Info()->Get_Texture(i);

            if (texture == tex) {
                tex->Release_Ref();
                return Set_Texture(i, pass);
            }

            tex->Release_Ref();
        }
    }

    Peek_Material_Info()->Add_Texture(texture);
    return Set_Texture(Peek_Material_Info()->Texture_Count() - 1, pass);
}

int DynamicMeshClass::Set_Vertex_Material(VertexMaterialClass *material, bool dont_search, int pass)
{
    if (Peek_Material_Info()->Vertex_Material_Count() != 0) {
        if (m_vertexMaterialIdx[pass] != -1) {
            if (Peek_Material_Info()->Peek_Vertex_Material(m_vertexMaterialIdx[pass]) == material) {
                return m_vertexMaterialIdx[pass];
            }
        }
    }

    if (!m_multiVertexMaterial[pass]) {
        if (Peek_Material_Info()->Vertex_Material_Count() != 0) {
            if (m_vertexMaterialIdx[pass] != -1) {
                if (Peek_Material_Info()->Peek_Vertex_Material(m_vertexMaterialIdx[pass]) != material) {
                    VertexMaterialClass *mat = Peek_Material_Info()->Get_Vertex_Material(m_vertexMaterialIdx[pass]);
                    m_model->Initialize_Material_Array(pass, mat);
                    mat->Release_Ref();
                    m_multiVertexMaterial[pass] = true;
                }
            }
        }
    }

    if (!dont_search) {
        for (int i = 0; i < Peek_Material_Info()->Vertex_Material_Count(); i++) {
            VertexMaterialClass *mat = Peek_Material_Info()->Get_Vertex_Material(i);

            if (material == mat) {
                mat->Release_Ref();
                return Set_Vertex_Material(i, pass);
            }

            mat->Release_Ref();
        }
    }

    Peek_Material_Info()->Add_Vertex_Material(material);
    return Set_Vertex_Material(Peek_Material_Info()->Vertex_Material_Count() - 1, pass);
}

void DynamicMeshClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (Is_Not_Hidden_At_All() && m_polyCount != 0) {
        if (W3D::Are_Static_Sort_Lists_Enabled() && m_sortLevel != 0) {
            W3D::Add_To_Static_Sort_List(this, m_sortLevel);
        } else {
            if (!rinfo.m_camera.Cull_Box(Get_Bounding_Box())) {
                DX8Wrapper::Set_Transform(D3DTS_WORLD, Get_Transform());
                m_model->Render(rinfo);
            }
        }
    }
#endif
}

void DynamicMeshClass::Move_Vertex(int index, float x, float y, float z)
{
    Vector3 *loc = m_model->Get_Vertex_Array();
    loc[index].X = x;
    loc[index].Y = y;
    loc[index].Z = z;
}

void DynamicMeshClass::Location(float x, float y, float z)
{
    Vector3 *loc = m_model->Get_Vertex_Array();
    loc[m_vertCount].X = x;
    loc[m_vertCount].Y = y;
    loc[m_vertCount].Z = z;
}

void DynamicMeshClass::Get_Vertex(int index, float &x, float &y, float &z)
{
    Vector3 *loc = m_model->Get_Vertex_Array();
    x = loc[index].X;
    y = loc[index].Y;
    z = loc[index].Z;
}

void DynamicMeshClass::Switch_To_Multi_Vertex_Color(int color_array_index)
{
#ifdef BUILD_WITH_D3D8
    unsigned int *colors = m_model->Get_Color_Array(color_array_index);
    unsigned int col = DX8Wrapper::Convert_Color_Clamp(m_curVertexColor[color_array_index]);

    for (int i = 0; i < m_vertCount; i++) {
        colors[i] = col;
    }

    m_multiVertexColor[color_array_index] = true;
#endif
}

void DynamicMeshClass::Change_Vertex_Color(int index, const Vector4 &color, int color_array_index)
{
#ifdef BUILD_WITH_D3D8
    if (!m_multiVertexColor[color_array_index]) {
        Switch_To_Multi_Vertex_Color(color_array_index);
    }

    m_curVertexColor[color_array_index] = color;
    unsigned int *colors = m_model->Get_Color_Array(color_array_index);
    unsigned int col = DX8Wrapper::Convert_Color_Clamp(m_curVertexColor[color_array_index]);
    colors[index] = col;
#endif
}

bool DynamicMeshClass::End_Vertex()
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(m_vertCount < m_model->Get_Vertex_Count());

    for (int pass = m_model->Get_Pass_Count() - 1; pass != 0; pass--) {
        if (m_multiVertexMaterial[pass]) {
            VertexMaterialClass *material = Peek_Material_Info()->Get_Vertex_Material(m_vertexMaterialIdx[pass]);
            m_model->Set_Material(m_vertCount, material, pass);
            material->Release_Ref();
        }
    }

    for (int pass = 0; pass < 2; pass++) {
        unsigned int *colors = m_model->Get_Color_Array(pass);
        colors[m_vertCount] = DX8Wrapper::Convert_Color_Clamp(m_curVertexColor[pass]);
    }

    m_vertCount++;
    m_triVertexCount++;

    if (m_triVertexCount >= 3) {
        captainslog_assert(m_polyCount < m_model->Get_Polygon_Count());
        TriIndex *triangle = &m_model->Get_Non_Const_Polygon_Array()[m_polyCount];

        if (m_triMode == TRI_MODE_STRIPS) {
            triangle->I = m_vertCount - 3;
            triangle->J = m_vertCount - 2;
            triangle->K = m_vertCount - 1;
            if (Flip_Face()) {
                triangle->J = m_vertCount - 1;
                triangle->K = m_vertCount - 2;
            }
        } else {
            triangle->I = m_fanVertex;
            triangle->J = m_vertCount - 2;
            triangle->K = m_vertCount - 1;
        }

        for (int pass = m_model->Get_Pass_Count() - 1; pass != 0; pass--) {
            if (m_multiTexture[pass]) {
                TextureClass *texture = Get_Material_Info()->Get_Texture(m_textureIdx[pass]);
                m_model->Set_Texture(m_polyCount, texture, pass, 0);
                texture->Release_Ref();
            }
        }

        m_polyCount++;
        m_model->Set_Counts(m_polyCount, m_vertCount);
    }
#endif
    return true;
}

void DynamicMeshClass::Resize(int max_polys, int max_verts)
{
    Reset();
    Ref_Ptr_Release(m_model);
    m_model = new DynamicMeshModel(max_polys, max_verts);
}

void DynamicMeshClass::Translate_Vertices(const Vector3 &offset)
{
    Vector3 *loc = m_model->Get_Vertex_Array();
    for (int i = 0; i < Get_Num_Vertices(); i++) {
        loc[i].X += offset.X;
        loc[i].Y += offset.Y;
        loc[i].Z += offset.Z;
    }
    m_model->Set_Flag(MeshGeometryClass::DIRTY_BOUNDS, true);
}

void DynamicMeshClass::Color(Vector4 const &color, int pass)
{
#ifdef BUILD_WITH_D3D8
    unsigned int *colors = m_model->Get_Color_Array(pass);
    unsigned int col = DX8Wrapper::Convert_Color_Clamp(color);

    for (int i = 0; i < m_vertCount; i++) {
        colors[i] = col;
    }
#endif
}

void DynamicMeshClass::Color(float r, float g, float b, float a, int pass)
{
#ifdef BUILD_WITH_D3D8
    Vector4 color(r, g, b, a);
    unsigned int *colors = m_model->Get_Color_Array(pass);
    unsigned int col = DX8Wrapper::Convert_Color_Clamp(color);

    for (int i = 0; i < m_vertCount; i++) {
        colors[i] = col;
    }
#endif
}

int DynamicMeshClass::Set_Vertex_Color(class Vector3 &color, int idx)
{
    if (!m_multiVertexColor[idx]) {
        Switch_To_Multi_Vertex_Color(idx);
    }

    m_curVertexColor[idx] = Vector4(color.X, color.Y, color.Z, 1.0f);
    return 0;
}

int DynamicMeshClass::Set_Vertex_Color(class Vector4 &color, int idx)
{
    if (!m_multiVertexColor[idx]) {
        Switch_To_Multi_Vertex_Color(idx);
    }

    m_curVertexColor[idx] = color;
    return 0;
}

DynamicScreenMeshClass::DynamicScreenMeshClass(const DynamicScreenMeshClass &src) :
    DynamicMeshClass(src), m_aspect(src.m_aspect)
{
}

DynamicScreenMeshClass::DynamicScreenMeshClass(int max_poly, int max_vert, float aspect) :
    DynamicMeshClass(max_poly, max_vert), m_aspect(aspect)
{
}

RenderObjClass *DynamicScreenMeshClass::Clone() const
{
    return new DynamicScreenMeshClass(*this);
}

void DynamicScreenMeshClass::Set_Position(const Vector3 &v)
{
    Vector3 v2(v.X * 2.0f, 0.0f - v.Y * m_aspect * 2.0f, 0.0f);
    RenderObjClass::Set_Position(v2);
}

void DynamicScreenMeshClass::Reset()
{
    Reset_Flags();
    Reset_Mesh_Counters();
}

void DynamicScreenMeshClass::Location(float x, float y, float z)
{
    DynamicMeshClass::Location((x * 2.0f) - 1.0f, m_aspect - ((y * m_aspect) * 2.0f), 0.0f);
}

void DynamicScreenMeshClass::Move_Vertex(int index, float x, float y, float z)
{
    DynamicMeshClass::Move_Vertex(index, (x * 2.0f) - 1.0f, m_aspect - ((y * m_aspect) * 2.0f), 0.0f);
}
