/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "linegrp.h"

#include "camera.h"
#include "dx8wrapper.h"
#include "matrix4.h"
#include "rinfo.h"
#include "sortingrenderer.h"

LineGroupClass::LineGroupClass() :
    m_startLoc(nullptr),
    m_endLoc(nullptr),
    m_diffuseBuffer(nullptr),
    m_tailDiffuseBuffer(nullptr),
    m_altBuffer(nullptr),
    m_sizeBuffer(nullptr),
    m_UCoordsBuffer(nullptr),
    m_lineCount(0),
    m_texture(nullptr),
    m_flags(0),
    m_defaultSize(0),
    m_defaultColor(1, 1, 1),
    m_defaultAlpha(1),
    m_defaultUCoord(0),
    m_defaultTailDiffuse(0, 0, 0, 0),
    m_mode(TETRAHEDRON)
{
}

LineGroupClass::~LineGroupClass()
{
    Ref_Ptr_Release(m_startLoc);
    Ref_Ptr_Release(m_endLoc);
    Ref_Ptr_Release(m_diffuseBuffer);
    Ref_Ptr_Release(m_tailDiffuseBuffer);
    Ref_Ptr_Release(m_altBuffer);
    Ref_Ptr_Release(m_sizeBuffer);
    Ref_Ptr_Release(m_UCoordsBuffer);
    Ref_Ptr_Release(m_texture);
}

void LineGroupClass::Set_Arrays(ShareBufferClass<Vector3> *start_locs,
    ShareBufferClass<Vector3> *end_locs,
    ShareBufferClass<Vector4> *diffuse,
    ShareBufferClass<Vector4> *tail_diffuse,
    ShareBufferClass<unsigned int> *alt,
    ShareBufferClass<float> *sizes,
    ShareBufferClass<float> *u_coords,
    int count)
{
    captainslog_assert(start_locs != nullptr);
    captainslog_assert(end_locs != nullptr);
    captainslog_assert(start_locs->Get_Count() == end_locs->Get_Count());
    captainslog_assert(diffuse == nullptr || start_locs->Get_Count() == diffuse->Get_Count());
    captainslog_assert(alt == nullptr || start_locs->Get_Count() == alt->Get_Count());
    captainslog_assert(sizes == nullptr || start_locs->Get_Count() == sizes->Get_Count());
    captainslog_assert(u_coords == nullptr || start_locs->Get_Count() == u_coords->Get_Count());
    captainslog_assert(tail_diffuse == nullptr || start_locs->Get_Count() == tail_diffuse->Get_Count());

    Ref_Ptr_Set(m_startLoc, start_locs);
    Ref_Ptr_Set(m_endLoc, end_locs);
    Ref_Ptr_Set(m_diffuseBuffer, diffuse);
    Ref_Ptr_Set(m_tailDiffuseBuffer, tail_diffuse);
    Ref_Ptr_Set(m_altBuffer, alt);
    Ref_Ptr_Set(m_sizeBuffer, sizes);
    Ref_Ptr_Set(m_UCoordsBuffer, u_coords);

    if (m_altBuffer != nullptr) {
        m_lineCount = count;
    } else {

        if (m_startLoc != nullptr && count < 0) {
            count = m_startLoc->Get_Count();
        }

        m_lineCount = count;
    }
}

void LineGroupClass::Set_Texture(TextureClass *texture)
{
    if (texture != nullptr) {
        texture->Add_Ref();
    }
    m_texture = texture;
}

void LineGroupClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (m_lineCount <= 0) {
        return;
    }

    m_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_ENABLE);

    float value_255 = 254.0f / 255.0f;

    bool default_white_opaque = m_defaultColor.X > value_255 && m_defaultColor.Y > value_255 && m_defaultColor.Z > value_255
        && m_defaultAlpha > value_255;

    if (m_diffuseBuffer != nullptr || !default_white_opaque || m_texture == nullptr) {
        m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
    } else {
        m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
    }

    if (m_texture != nullptr) {
        m_shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
    } else {
        m_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
    }

    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);

    DX8Wrapper::Set_Material(material);
    DX8Wrapper::Set_Shader(m_shader);
    DX8Wrapper::Set_Texture(0, m_texture);

    if (material != nullptr) {
        material->Release_Ref();
    }

    captainslog_assert(m_startLoc != nullptr && m_startLoc->Get_Array());
    captainslog_assert(m_endLoc != nullptr && m_endLoc->Get_Array());

    const bool sort = m_shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO
        && m_shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE && W3D::Is_Sorting_Enabled();

    static Vector3 _offset_a(GameMath::Cos(DEG_TO_RADF(90.f)), GameMath::Sin(DEG_TO_RADF(90.f)), 0);
    static Vector3 _offset_b(GameMath::Cos(DEG_TO_RADF(210.f)), GameMath::Sin(DEG_TO_RADF(210.f)), 0);
    static Vector3 _offset_c(GameMath::Cos(DEG_TO_RADF(330.f)), GameMath::Sin(DEG_TO_RADF(330.f)), 0);
    static Vector3 _offset[3];

    _offset[0].Set(_offset_a);
    _offset[1].Set(_offset_b);
    _offset[2].Set(_offset_c);

    Matrix4 old_view;
    DX8Wrapper::Get_Transform(D3DTS_VIEW, old_view);

    Matrix4 identity(true);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, identity);

    if (Get_Flag(TRANSFORM)) {
        Matrix3D model_view;

        model_view = rinfo.m_camera.Get_Transform();
        model_view.Set_Translation(Vector3(0, 0, 0));
        model_view.Get_Orthogonal_Inverse(model_view);

        for (int i = 0; i < 3; ++i) {
            Matrix3D::Transform_Vector(model_view, _offset[i], &_offset[i]);
        }
    } else {
        DX8Wrapper::Set_Transform(D3DTS_VIEW, identity);
    }

    int polygon_count = 0;
    int index_count = 0;
    int vertex_count = 0;

    if (m_mode == TETRAHEDRON) {
        polygon_count = TETRAHEADRON_NUM_POLYGONS * m_lineCount;
        index_count = TETRAHEADRON_NUM_INDEXES * polygon_count;
        vertex_count = TETRAHEADRON_NUM_VERTEXES * m_lineCount;
    } else if (m_mode == PRISM) {
        polygon_count = PRISM_NUM_POLYGONS * m_lineCount;
        index_count = PRISM_NUM_INDEXES * polygon_count;
        vertex_count = PRISM_NUM_VERTEXES * m_lineCount;
    } else {
        captainslog_assert(0);
    }

    DynamicIBAccessClass index_buffer(
        sort ? IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING : IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, index_count);
    { // added to control the lifetime of the WriteLockClass object
        DynamicIBAccessClass::WriteLockClass index_lock(&index_buffer);

        uint16_t *index = index_lock.Get_Index_Array();

        if (m_mode == TETRAHEDRON) {
            for (int i = 0; i < m_lineCount; ++i) {
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 0);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 2);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 1);

                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 0);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 3);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 2);

                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 0);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 1);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 3);

                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 1);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 2);
                *index++ = uint16_t(TETRAHEADRON_NUM_VERTEXES * i + 3);
            }

        } else if (m_mode == PRISM) {
            for (int i = 0; i < m_lineCount; ++i) {
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 0);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 1);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 2);

                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 0);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 3);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 1);

                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 1);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 3);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 4);

                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 1);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 4);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 5);

                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 1);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 5);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 2);

                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 0);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 2);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 5);

                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 0);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 5);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 3);

                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 3);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 5);
                *index++ = uint16_t(PRISM_NUM_VERTEXES * i + 4);
            }
        } else {
            captainslog_assert(0);
        }
    }

    DynamicVBAccessClass vertex_buffer(
        sort ? VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING : VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8,
        D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2,
        vertex_count);
    { // added to control the lifetime of the WriteLockClass object
        DynamicVBAccessClass::WriteLockClass vertex_lock(&vertex_buffer);
        VertexFormatXYZNDUV2 *vertex = vertex_lock.Get_Formatted_Vertex_Array();

        Vector3 vtx;
        Vector3 start_vtx;
        Vector3 end_vtx;

        float size = m_defaultSize;
        Vector4 diffuse(m_defaultColor.X, m_defaultColor.Y, m_defaultColor.Z, m_defaultAlpha);
        float u_coord = m_defaultUCoord;
        Vector4 tail_diffuse(m_defaultTailDiffuse);

        for (int i = 0; i < m_lineCount; ++i) {

            int idx = i;

            if (m_altBuffer != nullptr) {
                idx = m_altBuffer->Get_Element(i);
            }

            if (m_sizeBuffer != nullptr) {
                size = m_sizeBuffer->Get_Element(idx);
            }

            if (m_diffuseBuffer != nullptr) {
                diffuse = m_diffuseBuffer->Get_Element(idx);
            }

            if (m_UCoordsBuffer != nullptr) {
                u_coord = m_UCoordsBuffer->Get_Element(idx);
            }

            if (m_tailDiffuseBuffer != nullptr) {
                tail_diffuse = m_tailDiffuseBuffer->Get_Element(idx);
            }

            end_vtx.Set(m_endLoc->Get_Element(idx));
            start_vtx.Set(m_startLoc->Get_Element(idx));

            if (m_mode == TETRAHEDRON) {

                vertex->x = end_vtx.X;
                vertex->y = end_vtx.Y;
                vertex->z = end_vtx.Z;
                vertex->diffuse = DX8Wrapper::Convert_Color(tail_diffuse);
                vertex->u1 = u_coord;
                vertex->v1 = 1.0f;
                ++vertex;

                for (int k = 0; k < 3; ++k) {
                    vtx.Set(start_vtx + (size * _offset[k]));
                    vertex->x = vtx.X;
                    vertex->y = vtx.Y;
                    vertex->z = vtx.Z;
                    vertex->diffuse = DX8Wrapper::Convert_Color(diffuse);
                    vertex->u1 = u_coord;
                    vertex->v1 = 0.0f;
                    ++vertex;
                }
            } else if (m_mode == PRISM) {

                for (int k = 0; k < 3; ++k) {
                    vtx.Set(start_vtx + (size * _offset[k]));
                    vertex->x = vtx.X;
                    vertex->y = vtx.Y;
                    vertex->z = vtx.Z;
                    vertex->diffuse = DX8Wrapper::Convert_Color(diffuse);
                    vertex->u1 = u_coord;
                    vertex->v1 = 0.0f;
                    ++vertex;
                }

                for (int k = 0; k < 3; ++k) {
                    vtx.Set(end_vtx + (size * _offset[k]));
                    vertex->x = vtx.X;
                    vertex->y = vtx.Y;
                    vertex->z = vtx.Z;
                    vertex->diffuse = DX8Wrapper::Convert_Color(tail_diffuse);
                    vertex->u1 = u_coord;
                    vertex->v1 = 1.0f;
                    ++vertex;
                }
            } else {
                captainslog_assert(0);
            }
        }
    }

    DX8Wrapper::Set_Index_Buffer(index_buffer, 0);
    DX8Wrapper::Set_Vertex_Buffer(vertex_buffer);

    if (sort) {
        SortingRendererClass::Insert_Triangles(0, polygon_count, 0, vertex_count);
    } else {
        DX8Wrapper::Draw_Triangles(0, polygon_count, 0, vertex_count);
    }

    DX8Wrapper::Set_Transform(D3DTS_VIEW, old_view);
#endif
}

int LineGroupClass::Get_Polygon_Count() const
{
    switch (m_mode) {
        case TETRAHEDRON:
            return TETRAHEADRON_NUM_POLYGONS * m_lineCount;
        case PRISM:
            return PRISM_NUM_POLYGONS * m_lineCount;
    }

    captainslog_assert(0);
    return 0;
}
