/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Point Group
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "pointgr.h"
#include "aabox.h"
#include "camera.h"
#include "dx8fvf.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "matrix4.h"
#include "rinfo.h"
#include "simplevec.h"
#include "sortingrenderer.h"
#include "texture.h"
#include "vertmaterial.h"
#include "vp.h"
#include "w3d.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

Vector3 PointGroupClass::s_triVertexLocationOrientationTable[256][3];
Vector3 PointGroupClass::s_quadVertexLocationOrientationTable[256][4];
Vector2 *PointGroupClass::s_triVertexUVFrameTable[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
Vector2 *PointGroupClass::s_quadVertexUVFrameTable[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
VertexMaterialClass *PointGroupClass::s_pointMaterial = nullptr;

VectorClass<Vector3> PointGroupClass::s_compressedLoc;
VectorClass<Vector4> PointGroupClass::s_compressedDiffuse;
VectorClass<float> PointGroupClass::s_compressedSize;
VectorClass<unsigned char> PointGroupClass::s_compressedOrient;
VectorClass<unsigned char> PointGroupClass::s_compressedFrame;
VectorClass<Vector3> PointGroupClass::s_transformedLoc;

Vector3 PointGroupClass::g_screenspaceVertexLocationSizeTable[2][3] = { Vector3(0.5f, 0.0f, -1.0f),
    Vector3(1.0f, 1.0f, -1.0f),
    Vector3(0.0f, 1.0f, -1.0f),
    Vector3(1.0f, -0.5f, -1.0f),
    Vector3(2.7f, 2.0f, -1.0f),
    Vector3(-0.7f, 2.0f, -1.0f) };

VectorClass<Vector3> g_vertexLoc;
VectorClass<Vector4> g_vertexDiffuse;
VectorClass<Vector2> g_vertexUV;

enum
{
    MAX_VB_SIZE = 2048,
    MAX_TRI_POINTS = MAX_VB_SIZE / 3,
    MAX_TRI_IB_SIZE = 3 * MAX_TRI_POINTS,
    MAX_QUAD_POINTS = MAX_VB_SIZE / 4,
    MAX_QUAD_IB_SIZE = 6 * MAX_QUAD_POINTS
};

DX8IndexBufferClass *g_tris;
DX8IndexBufferClass *g_quads;
SortingIndexBufferClass *g_sortingTris;
SortingIndexBufferClass *g_sortingQuads;

PointGroupClass::PointGroupClass() :
    m_pointLoc(nullptr),
    m_pointDiffuse(nullptr),
    m_APT(nullptr),
    m_pointSize(nullptr),
    m_pointOrientation(nullptr),
    m_pointFrame(nullptr),
    m_pointCount(0),
    m_frameRowColumnCountLog2(0),
    m_texture(nullptr),
    m_shader(ShaderClass::s_presetAdditiveSpriteShader),
    m_pointMode(TRIS),
    m_flags(0),
    m_defaultPointSize(0.0f),
    m_defaultPointColor(1.0f, 1.0f, 1.0f),
    m_defaultPointAlpha(1.0f),
    m_defaultPointOrientation(0),
    m_defaultPointFrame(0),
    m_VPXMin(0.0f),
    m_VPYMin(0.0f),
    m_VPXMax(0.0f),
    m_VPYMax(0.0f)
{
}

PointGroupClass::~PointGroupClass()
{
    Ref_Ptr_Release(m_pointLoc);
    Ref_Ptr_Release(m_pointDiffuse);
    Ref_Ptr_Release(m_APT);
    Ref_Ptr_Release(m_pointSize);
    Ref_Ptr_Release(m_pointOrientation);
    Ref_Ptr_Release(m_pointFrame);
    Ref_Ptr_Release(m_texture);
}

PointGroupClass &PointGroupClass::operator=(const PointGroupClass &that)
{
    if (this != &that) {
        captainslog_assert(0);
    }

    return *this;
}

void PointGroupClass::Set_Arrays(ShareBufferClass<Vector3> *locs,
    ShareBufferClass<Vector4> *diffuse,
    ShareBufferClass<unsigned int> *apt,
    ShareBufferClass<float> *sizes,
    ShareBufferClass<unsigned char> *orientations,
    ShareBufferClass<unsigned char> *frames,
    int active_point_count,
    float vpxmin,
    float vpymin,
    float vpxmax,
    float vpymax)
{
    captainslog_assert(locs);

    captainslog_assert(!diffuse || locs->Get_Count() == diffuse->Get_Count());
    captainslog_assert(!apt || locs->Get_Count() == apt->Get_Count());
    captainslog_assert(!sizes || locs->Get_Count() == sizes->Get_Count());
    captainslog_assert(!orientations || locs->Get_Count() == orientations->Get_Count());
    captainslog_assert(!frames || locs->Get_Count() == frames->Get_Count());

    Ref_Ptr_Set(m_pointLoc, locs);
    Ref_Ptr_Set(m_pointDiffuse, diffuse);
    Ref_Ptr_Set(m_APT, apt);
    Ref_Ptr_Set(m_pointSize, sizes);
    Ref_Ptr_Set(m_pointOrientation, orientations);
    Ref_Ptr_Set(m_pointFrame, frames);

    if (m_APT != nullptr) {
        m_pointCount = active_point_count;
    } else {
        m_pointCount = (active_point_count >= 0) ? active_point_count : m_pointLoc->Get_Count();
    }

    m_VPXMin = vpxmin;
    m_VPYMin = vpymin;
    m_VPXMax = vpxmax;
    m_VPYMax = vpymax;
}

void PointGroupClass::Set_Point_Size(float size)
{
    m_defaultPointSize = size;
}

float PointGroupClass::Get_Point_Size()
{
    return m_defaultPointSize;
}

void PointGroupClass::Set_Point_Color(Vector3 color)
{
    m_defaultPointColor = color;
}

Vector3 PointGroupClass::Get_Point_Color()
{
    return m_defaultPointColor;
}

void PointGroupClass::Set_Point_Alpha(float alpha)
{
    m_defaultPointAlpha = alpha;
}

float PointGroupClass::Get_Point_Alpha()
{
    return m_defaultPointAlpha;
}

void PointGroupClass::Set_Point_Orientation(unsigned char orientation)
{
    m_defaultPointOrientation = orientation;
}

unsigned char PointGroupClass::Get_Point_Orientation()
{
    return m_defaultPointOrientation;
}

void PointGroupClass::Set_Point_Frame(unsigned char frame)
{
    m_defaultPointFrame = frame;
}

unsigned char PointGroupClass::Get_Point_Frame()
{
    return m_defaultPointFrame;
}

void PointGroupClass::Set_Point_Mode(PointModeEnum mode)
{
    m_pointMode = mode;
}

PointGroupClass::PointModeEnum PointGroupClass::Get_Point_Mode()
{
    return m_pointMode;
}

void PointGroupClass::Set_Flag(FlagsType flag, bool onoff)
{
    if (onoff) {
        m_flags |= 1 << flag;
    } else {
        m_flags &= ~(1 << flag);
    }
}

int PointGroupClass::Get_Flag(FlagsType flag)
{
    return (m_flags >> flag) & 0x1;
}

void PointGroupClass::Set_Texture(TextureClass *texture)
{
    Ref_Ptr_Set(m_texture, texture);
}

TextureClass *PointGroupClass::Get_Texture()
{
    if (m_texture != nullptr) {
        m_texture->Add_Ref();
    }

    return m_texture;
}

TextureClass *PointGroupClass::Peek_Texture()
{
    return m_texture;
}

void PointGroupClass::Set_Billboard(bool onoff)
{
    m_billboard = onoff;
}

bool PointGroupClass::Get_Billboard()
{
    return m_billboard;
}

void PointGroupClass::Set_Shader(ShaderClass shader)
{
    m_shader = shader;
}

ShaderClass PointGroupClass::Get_Shader()
{
    return m_shader;
}

unsigned char PointGroupClass::Get_Frame_Row_Column_Count_Log2()
{
    return m_frameRowColumnCountLog2;
}

void PointGroupClass::Set_Frame_Row_Column_Count_Log2(unsigned char frccl2)
{
    m_frameRowColumnCountLog2 = std::min(frccl2, (unsigned char)4);
}

int PointGroupClass::Get_Polygon_Count()
{
    switch (m_pointMode) {
        case TRIS:
        case SCREENSPACE:
            return m_pointCount;
            break;
        case QUADS:
            return m_pointCount * 2;
            break;
    }

    captainslog_assert(0);
    return 0;
}

static SimpleVecClass<unsigned long> g_remap;
void PointGroupClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    m_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);

    if (m_pointCount == 0) {
        return;
    }

    captainslog_assert(m_pointLoc && m_pointLoc->Get_Array());

    Vector3 *current_loc = nullptr;
    Vector4 *current_diffuse = nullptr;
    float *current_size = nullptr;
    unsigned char *current_orient = nullptr;
    unsigned char *current_frame = nullptr;

    float value_255 = 0.9961f;
    bool default_white_opaque = (m_defaultPointColor.X > value_255 && m_defaultPointColor.Y > value_255
        && m_defaultPointColor.Z > value_255 && m_defaultPointAlpha > value_255);

    if (m_pointDiffuse != nullptr || !default_white_opaque || m_texture == nullptr) {
        m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
    } else {
        m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
    }

    if (m_texture != nullptr) {
        m_shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
    } else {
        m_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
    }

    if (m_APT != nullptr) {
        if (s_compressedLoc.Length() < m_pointCount) {
            s_compressedLoc.Resize(m_pointCount * 2);
        }

        VectorProcessorClass::CopyIndexed(&s_compressedLoc[0], m_pointLoc->Get_Array(), m_APT->Get_Array(), m_pointCount);
        current_loc = &s_compressedLoc[0];

        if (m_pointDiffuse != nullptr) {
            if (s_compressedDiffuse.Length() < m_pointCount) {
                s_compressedDiffuse.Resize(m_pointCount * 2);
            }

            VectorProcessorClass::CopyIndexed(
                &s_compressedDiffuse[0], m_pointDiffuse->Get_Array(), m_APT->Get_Array(), m_pointCount);
            current_diffuse = &s_compressedDiffuse[0];
        }

        if (m_pointSize != nullptr) {
            if (s_compressedSize.Length() < m_pointCount) {
                s_compressedSize.Resize(m_pointCount * 2);
            }

            VectorProcessorClass::CopyIndexed(
                &s_compressedSize[0], m_pointSize->Get_Array(), m_APT->Get_Array(), m_pointCount);
            current_size = &s_compressedSize[0];
        }

        if (m_pointOrientation != nullptr) {
            if (s_compressedOrient.Length() < m_pointCount) {
                s_compressedOrient.Resize(m_pointCount * 2);
            }

            VectorProcessorClass::CopyIndexed(
                &s_compressedOrient[0], m_pointOrientation->Get_Array(), m_APT->Get_Array(), m_pointCount);
            current_orient = &s_compressedOrient[0];
        }

        if (m_pointFrame != nullptr) {
            if (s_compressedFrame.Length() < m_pointCount) {
                s_compressedFrame.Resize(m_pointCount * 2);
            }

            VectorProcessorClass::CopyIndexed(
                &s_compressedFrame[0], m_pointFrame->Get_Array(), m_APT->Get_Array(), m_pointCount);
            current_frame = &s_compressedFrame[0];
        }
    } else {
        current_loc = m_pointLoc->Get_Array();

        if (m_pointDiffuse != nullptr) {
            current_diffuse = m_pointDiffuse->Get_Array();
        }

        if (m_pointSize != nullptr) {
            current_size = m_pointSize->Get_Array();
        }

        if (m_pointOrientation != nullptr) {
            current_orient = m_pointOrientation->Get_Array();
        }

        if (m_pointFrame != nullptr) {
            current_frame = m_pointFrame->Get_Array();
        }
    }

    Matrix4 view;
    DX8Wrapper::Get_Transform(D3DTS_VIEW, view);

    if (Get_Flag(TRANSFORM) && m_billboard) {
        if (s_transformedLoc.Length() < m_pointCount) {
            s_transformedLoc.Resize(m_pointCount * 2);
        }

        for (int i = 0; i < m_pointCount; i++) {
            Vector4 result = view * current_loc[i];
            s_transformedLoc[i].X = result.X;
            s_transformedLoc[i].Y = result.Y;
            s_transformedLoc[i].Z = result.Z;
        }

        current_loc = &s_transformedLoc[0];
    }

    int vnum, pnum;

    Update_Arrays(current_loc,
        current_diffuse,
        current_size,
        current_orient,
        current_frame,
        m_pointCount,
        m_pointLoc->Get_Count(),
        vnum,
        pnum);

    Matrix4 identity(true);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, identity);
    DX8Wrapper::Set_Transform(D3DTS_VIEW, identity);

    DX8Wrapper::Set_Material(s_pointMaterial);
    DX8Wrapper::Set_Shader(m_shader);
    DX8Wrapper::Set_Texture(0, m_texture);

    const bool sort = (m_shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO)
        && (m_shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) && (W3D::Is_Sorting_Enabled());

    IndexBufferClass *index_buffer;
    int vertices_per_primitive;
    int current;
    int delta;

    if (m_pointMode == QUADS) {
        vertices_per_primitive = 2;
        index_buffer = sort ? static_cast<IndexBufferClass *>(g_sortingQuads) : static_cast<IndexBufferClass *>(g_quads);
    } else {
        vertices_per_primitive = 3;
        index_buffer = sort ? static_cast<IndexBufferClass *>(g_sortingTris) : static_cast<IndexBufferClass *>(g_tris);
    }

    current = 0;

    while (current < vnum) {
        delta = std::min(vnum - current, (int)MAX_VB_SIZE);
        DynamicVBAccessClass point_verts(
            sort ? DX8VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING : DX8VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8,
            dynamic_fvf_type,
            delta);
        {
            DynamicVBAccessClass::WriteLockClass Lock(&point_verts);
            unsigned char *vb = reinterpret_cast<unsigned char *>(Lock.Get_Formatted_Vertex_Array());
            const FVFInfoClass &fvfinfo = point_verts.FVF_Info();

            for (int i = current; i < current + delta; i++) {
                *reinterpret_cast<Vector3 *>(vb + fvfinfo.Get_Location_Offset()) = g_vertexLoc[i];

                if (current_diffuse != nullptr) {
                    unsigned color = DX8Wrapper::Convert_Color_Clamp(g_vertexDiffuse[i]);
                    *reinterpret_cast<unsigned int *>(vb + fvfinfo.Get_Diffuse_Offset()) = color;
                } else {
                    *reinterpret_cast<unsigned int *>(vb + fvfinfo.Get_Diffuse_Offset()) =
                        DX8Wrapper::Convert_Color_Clamp(Vector4(
                            m_defaultPointColor[0], m_defaultPointColor[1], m_defaultPointColor[2], m_defaultPointAlpha));
                }

                *reinterpret_cast<Vector2 *>(vb + fvfinfo.Get_Tex_Offset(0)) = g_vertexUV[i];
                vb += fvfinfo.Get_FVF_Size();
            }
        }

        DX8Wrapper::Set_Index_Buffer(index_buffer, 0);
        DX8Wrapper::Set_Vertex_Buffer(point_verts);

        if (sort) {
            SortingRendererClass::Insert_Triangles(0, delta / vertices_per_primitive, 0, delta);
        } else {
            DX8Wrapper::Draw_Triangles(0, delta / vertices_per_primitive, 0, delta);
        }

        current += delta;
    }

    DX8Wrapper::Set_Transform(D3DTS_VIEW, view);
#endif
}

void PointGroupClass::Render_Volume_Particle(RenderInfoClass &rinfo, int unk)
{
#ifdef BUILD_WITH_D3D8
    if (unk <= 1) {
        Render(rinfo);
        return;
    }

    if (unk > 16) {
        unk = 16;
    }

    m_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);

    if (m_pointCount != 0) {

        captainslog_assert(m_pointLoc && m_pointLoc->Get_Array());

        float value_255 = 0.9961f;
        bool default_white_opaque = (m_defaultPointColor.X > value_255 && m_defaultPointColor.Y > value_255
            && m_defaultPointColor.Z > value_255 && m_defaultPointAlpha > value_255);

        if (m_pointDiffuse != nullptr || !default_white_opaque || m_texture == nullptr) {
            m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
        } else {
            m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
        }

        if (m_texture != nullptr) {
            m_shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
        } else {
            m_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
        }

        Matrix4 view;
        DX8Wrapper::Get_Transform(D3DTS_VIEW, view);

        Vector3 *current_loc;
        Vector4 *current_diffuse = nullptr;
        float *current_size = nullptr;
        unsigned char *current_orient = nullptr;
        unsigned char *current_frame = nullptr;

        for (int i = 0; i < unk; i++) {
            if (m_APT != nullptr) {
                if (s_compressedLoc.Length() < m_pointCount) {
                    s_compressedLoc.Resize(m_pointCount * 2);
                }

                VectorProcessorClass::CopyIndexed(
                    &s_compressedLoc[0], m_pointLoc->Get_Array(), m_APT->Get_Array(), m_pointCount);
                current_loc = &s_compressedLoc[0];

                if (m_pointDiffuse != nullptr) {
                    if (s_compressedDiffuse.Length() < m_pointCount) {
                        s_compressedDiffuse.Resize(m_pointCount * 2);
                    }

                    VectorProcessorClass::CopyIndexed(
                        &s_compressedDiffuse[0], m_pointDiffuse->Get_Array(), m_APT->Get_Array(), m_pointCount);
                    current_diffuse = &s_compressedDiffuse[0];
                }

                if (m_pointSize != nullptr) {
                    if (s_compressedSize.Length() < m_pointCount) {
                        s_compressedSize.Resize(m_pointCount * 2);
                    }

                    VectorProcessorClass::CopyIndexed(
                        &s_compressedSize[0], m_pointSize->Get_Array(), m_APT->Get_Array(), m_pointCount);
                    current_size = &s_compressedSize[0];
                }

                if (m_pointOrientation != nullptr) {
                    if (s_compressedOrient.Length() < m_pointCount) {
                        s_compressedOrient.Resize(m_pointCount * 2);
                    }

                    VectorProcessorClass::CopyIndexed(
                        &s_compressedOrient[0], m_pointOrientation->Get_Array(), m_APT->Get_Array(), m_pointCount);
                    current_orient = &s_compressedOrient[0];
                }

                if (m_pointFrame != nullptr) {
                    if (s_compressedFrame.Length() < m_pointCount) {
                        s_compressedFrame.Resize(m_pointCount * 2);
                    }

                    VectorProcessorClass::CopyIndexed(
                        &s_compressedFrame[0], m_pointFrame->Get_Array(), m_APT->Get_Array(), m_pointCount);
                    current_frame = &s_compressedFrame[0];
                }
            } else {
                current_loc = m_pointLoc->Get_Array();

                if (m_pointDiffuse != nullptr) {
                    current_diffuse = m_pointDiffuse->Get_Array();
                }

                if (m_pointSize != nullptr) {
                    current_size = m_pointSize->Get_Array();
                }

                if (m_pointOrientation != nullptr) {
                    current_orient = m_pointOrientation->Get_Array();
                }

                if (m_pointFrame != nullptr) {
                    current_frame = m_pointFrame->Get_Array();
                }
            }

            if (Get_Flag(TRANSFORM) && m_billboard) {
                if (s_transformedLoc.Length() < m_pointCount) {
                    s_transformedLoc.Resize(m_pointCount * 2);
                }

                float f1 = (i * current_size[0]) * (0.1f / unk);
                Vector3 camera_pos = rinfo.m_camera.Get_Position();

                for (int j = 0; j < m_pointCount; j++) {
                    Vector3 position = camera_pos - current_loc[j];
                    position.Normalize();

                    Vector4 result = view * ((position * f1) + current_loc[j]);
                    s_transformedLoc[j].X = result.X;
                    s_transformedLoc[j].Y = result.Y;
                    s_transformedLoc[j].Z = result.Z;
                }

                current_loc = &s_transformedLoc[0];
            }

            int vnum, pnum;

            Update_Arrays(current_loc,
                current_diffuse,
                current_size,
                current_orient,
                current_frame,
                m_pointCount,
                m_pointLoc->Get_Count(),
                vnum,
                pnum);

            Matrix4 identity(true);
            DX8Wrapper::Set_Transform(D3DTS_WORLD, identity);
            DX8Wrapper::Set_Transform(D3DTS_VIEW, identity);

            DX8Wrapper::Set_Material(s_pointMaterial);
            DX8Wrapper::Set_Shader(m_shader);
            DX8Wrapper::Set_Texture(0, m_texture);

            const bool sort = (m_shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO)
                && (m_shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) && (W3D::Is_Sorting_Enabled());

            IndexBufferClass *index_buffer;
            int vertices_per_primitive;
            int current;
            int delta;

            if (m_pointMode == QUADS) {
                vertices_per_primitive = 2;
                index_buffer =
                    sort ? static_cast<IndexBufferClass *>(g_sortingQuads) : static_cast<IndexBufferClass *>(g_quads);
            } else {
                vertices_per_primitive = 3;
                index_buffer =
                    sort ? static_cast<IndexBufferClass *>(g_sortingTris) : static_cast<IndexBufferClass *>(g_tris);
            }

            current = 0;

            while (current < vnum) {
                delta = std::min(vnum - current, (int)MAX_VB_SIZE);
                DynamicVBAccessClass point_verts(
                    sort ? DX8VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING : DX8VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8,
                    dynamic_fvf_type,
                    delta);
                {
                    DynamicVBAccessClass::WriteLockClass Lock(&point_verts);
                    unsigned char *vb = reinterpret_cast<unsigned char *>(Lock.Get_Formatted_Vertex_Array());
                    const FVFInfoClass &fvfinfo = point_verts.FVF_Info();

                    for (int j = current; j < current + delta; j++) {
                        *reinterpret_cast<Vector3 *>(vb + fvfinfo.Get_Location_Offset()) = g_vertexLoc[j];

                        if (current_diffuse != nullptr) {
                            unsigned color = DX8Wrapper::Convert_Color_Clamp(g_vertexDiffuse[j]);
                            *reinterpret_cast<unsigned int *>(vb + fvfinfo.Get_Diffuse_Offset()) = color;
                        } else {
                            *reinterpret_cast<unsigned int *>(vb + fvfinfo.Get_Diffuse_Offset()) =
                                DX8Wrapper::Convert_Color_Clamp(Vector4(m_defaultPointColor[0],
                                    m_defaultPointColor[1],
                                    m_defaultPointColor[2],
                                    m_defaultPointAlpha));
                        }

                        *reinterpret_cast<Vector2 *>(vb + fvfinfo.Get_Tex_Offset(0)) = g_vertexUV[j];
                        vb += fvfinfo.Get_FVF_Size();
                    }
                }

                DX8Wrapper::Set_Index_Buffer(index_buffer, 0);
                DX8Wrapper::Set_Vertex_Buffer(point_verts);

                if (sort) {
                    SortingRendererClass::Insert_Triangles(0, delta / vertices_per_primitive, 0, delta);
                } else {
                    DX8Wrapper::Draw_Triangles(0, delta / vertices_per_primitive, 0, delta);
                }

                current += delta;
            }
        }

        DX8Wrapper::Set_Transform(D3DTS_VIEW, view);
    }
#endif
}

void PointGroupClass::Update_Arrays(Vector3 *point_loc,
    Vector4 *point_diffuse,
    float *point_size,
    unsigned char *point_orientation,
    unsigned char *point_frame,
    int active_points,
    int total_points,
    int &vnum,
    int &pnum)
{
    Vector3 g_groundMultiplierX{ 1.0f, 0.0f, 0.0f };
    Vector3 g_groundMultiplierY{ 0.0f, 1.0f, 0.0f };

#ifdef BUILD_WITH_D3D8
    int verts_per_point = (m_pointMode == QUADS) ? 4 : 3;
    int polys_per_point = (m_pointMode == QUADS) ? 2 : 1;

    int total_vnum = verts_per_point * total_points;
    vnum = verts_per_point * active_points;
    pnum = polys_per_point * active_points;

    if (g_vertexLoc.Length() < total_vnum) {
        g_vertexLoc.Resize(total_vnum * 2, false);
        g_vertexUV.Resize(total_vnum * 2, false);
        g_vertexDiffuse.Resize(total_vnum * 2, false);
    }

    int vert;

    enum LoopSelectionEnum
    {
        TRIS_NOSIZE_NOORIENT = ((int)TRIS << 2) + 0,
        TRIS_SIZE_NOORIENT = ((int)TRIS << 2) + 1,
        TRIS_NOSIZE_ORIENT = ((int)TRIS << 2) + 2,
        TRIS_SIZE_ORIENT = ((int)TRIS << 2) + 3,
        QUADS_NOSIZE_NOORIENT = ((int)QUADS << 2) + 0,
        QUADS_SIZE_NOORIENT = ((int)QUADS << 2) + 1,
        QUADS_NOSIZE_ORIENT = ((int)QUADS << 2) + 2,
        QUADS_SIZE_ORIENT = ((int)QUADS << 2) + 3,
        SCREEN_NOSIZE_NOORIENT = ((int)SCREENSPACE << 2) + 0,
        SCREEN_SIZE_NOORIENT = ((int)SCREENSPACE << 2) + 1,
        SCREEN_NOSIZE_ORIENT = ((int)SCREENSPACE << 2) + 2,
        SCREEN_SIZE_ORIENT = ((int)SCREENSPACE << 2) + 3,
    };

    LoopSelectionEnum loop_sel =
        (LoopSelectionEnum)(((int)m_pointMode << 2) + (point_orientation ? 2 : 0) + (point_size ? 1 : 0));

    vert = 0;
    Vector3 *vertex_loc = &g_vertexLoc[0];

    switch (loop_sel) {
        case TRIS_NOSIZE_NOORIENT: {
            Vector3 scaled_offset[3];
            scaled_offset[0] = s_triVertexLocationOrientationTable[m_defaultPointOrientation][0] * m_defaultPointSize;
            scaled_offset[1] = s_triVertexLocationOrientationTable[m_defaultPointOrientation][1] * m_defaultPointSize;
            scaled_offset[2] = s_triVertexLocationOrientationTable[m_defaultPointOrientation][2] * m_defaultPointSize;

            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] = point_loc[i] + scaled_offset[0];
                vertex_loc[vert + 1] = point_loc[i] + scaled_offset[1];
                vertex_loc[vert + 2] = point_loc[i] + scaled_offset[2];
                vert += 3;
            }
        } break;
        case TRIS_SIZE_NOORIENT: {
            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] =
                    point_loc[i] + s_triVertexLocationOrientationTable[m_defaultPointOrientation][0] * point_size[i];
                vertex_loc[vert + 1] =
                    point_loc[i] + s_triVertexLocationOrientationTable[m_defaultPointOrientation][1] * point_size[i];
                vertex_loc[vert + 2] =
                    point_loc[i] + s_triVertexLocationOrientationTable[m_defaultPointOrientation][2] * point_size[i];
                vert += 3;
            }
        } break;
        case TRIS_NOSIZE_ORIENT: {
            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] =
                    point_loc[i] + s_triVertexLocationOrientationTable[point_orientation[i]][0] * m_defaultPointSize;
                vertex_loc[vert + 1] =
                    point_loc[i] + s_triVertexLocationOrientationTable[point_orientation[i]][1] * m_defaultPointSize;
                vertex_loc[vert + 2] =
                    point_loc[i] + s_triVertexLocationOrientationTable[point_orientation[i]][2] * m_defaultPointSize;
                vert += 3;
            }
        } break;
        case TRIS_SIZE_ORIENT: {
            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] =
                    point_loc[i] + s_triVertexLocationOrientationTable[point_orientation[i]][0] * point_size[i];
                vertex_loc[vert + 1] =
                    point_loc[i] + s_triVertexLocationOrientationTable[point_orientation[i]][1] * point_size[i];
                vertex_loc[vert + 2] =
                    point_loc[i] + s_triVertexLocationOrientationTable[point_orientation[i]][2] * point_size[i];
                vert += 3;
            }
        } break;
        case QUADS_NOSIZE_NOORIENT: {
            Vector3 scaled_offset[4];
            scaled_offset[0] = s_quadVertexLocationOrientationTable[m_defaultPointOrientation][0] * m_defaultPointSize;
            scaled_offset[1] = s_quadVertexLocationOrientationTable[m_defaultPointOrientation][1] * m_defaultPointSize;
            scaled_offset[2] = s_quadVertexLocationOrientationTable[m_defaultPointOrientation][2] * m_defaultPointSize;
            scaled_offset[3] = s_quadVertexLocationOrientationTable[m_defaultPointOrientation][3] * m_defaultPointSize;

            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] = point_loc[i] + scaled_offset[0];
                vertex_loc[vert + 1] = point_loc[i] + scaled_offset[1];
                vertex_loc[vert + 2] = point_loc[i] + scaled_offset[2];
                vertex_loc[vert + 3] = point_loc[i] + scaled_offset[3];
                vert += 4;
            }
        } break;
        case QUADS_SIZE_NOORIENT: {
            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[m_defaultPointOrientation][0] * point_size[i];
                vertex_loc[vert + 1] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[m_defaultPointOrientation][1] * point_size[i];
                vertex_loc[vert + 2] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[m_defaultPointOrientation][2] * point_size[i];
                vertex_loc[vert + 3] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[m_defaultPointOrientation][3] * point_size[i];
                vert += 4;
            }
        } break;
        case QUADS_NOSIZE_ORIENT: {
            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][0] * m_defaultPointSize;
                vertex_loc[vert + 1] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][1] * m_defaultPointSize;
                vertex_loc[vert + 2] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][2] * m_defaultPointSize;
                vertex_loc[vert + 3] =
                    point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][3] * m_defaultPointSize;
                vert += 4;
            }
        } break;
        case QUADS_SIZE_ORIENT: {
            Matrix4 view;
            if (!m_billboard) {
                DX8Wrapper::Get_Transform(D3DTS_VIEW, view);
            }

            for (int i = 0; i < active_points; i++) {
                if (m_billboard) {
                    vertex_loc[vert + 0] =
                        point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][0] * point_size[i];
                    vertex_loc[vert + 1] =
                        point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][1] * point_size[i];
                    vertex_loc[vert + 2] =
                        point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][2] * point_size[i];
                    vertex_loc[vert + 3] =
                        point_loc[i] + s_quadVertexLocationOrientationTable[point_orientation[i]][3] * point_size[i];
                    vert += 4;
                } else {
                    D3DXMATRIX mat;
                    D3DXMatrixRotationZ(&mat, point_orientation[i] * 0.024639944f);
                    float f1 = mat._13 * g_groundMultiplierX.Z + mat._12 * g_groundMultiplierX.Y
                        + mat._11 * g_groundMultiplierX.X + mat._14;
                    float f2 = mat._23 * g_groundMultiplierX.Z + mat._22 * g_groundMultiplierX.Y
                        + mat._21 * g_groundMultiplierX.X + mat._24;
                    float f3 = mat._13 * g_groundMultiplierY.Z + mat._12 * g_groundMultiplierY.Y
                        + mat._11 * g_groundMultiplierY.X + mat._14;
                    float f4 = mat._23 * g_groundMultiplierY.Z + mat._22 * g_groundMultiplierY.Y
                        + mat._21 * g_groundMultiplierY.X + mat._24;

                    vertex_loc[vert + 0].X = (f3 + f1) * point_size[i] + point_loc[i].X;
                    vertex_loc[vert + 0].Y = (f4 + f2) * point_size[i] + point_loc[i].Y;
                    vertex_loc[vert + 0].Z = point_loc[i].Z;
                    vertex_loc[vert + 1].X = (f1 - f3) * point_size[i] + point_loc[i].X;
                    vertex_loc[vert + 1].Y = (f2 - f4) * point_size[i] + point_loc[i].Y;
                    vertex_loc[vert + 1].Z = point_loc[i].Z;
                    vertex_loc[vert + 2].X = point_loc[i].X - (f3 + f1) * point_size[i];
                    vertex_loc[vert + 2].Y = point_loc[i].Y - (f4 + f2) * point_size[i];
                    vertex_loc[vert + 2].Z = point_loc[i].Z;
                    vertex_loc[vert + 3].X = (f3 - f1) * point_size[i] + point_loc[i].X;
                    vertex_loc[vert + 3].Y = (f4 - f2) * point_size[i] + point_loc[i].Y;
                    vertex_loc[vert + 3].Z = point_loc[i].Z;

                    float f5 = view[2].Y * vertex_loc[vert + 0].Y + view[2].X * vertex_loc[vert + 0].X
                        + view[2].Z * vertex_loc[vert + 0].Z + view[2].W;
                    float f6 = view[1].Y * vertex_loc[vert + 0].Y + view[1].X * vertex_loc[vert + 0].X
                        + view[1].Z * vertex_loc[vert + 0].Z + view[1].W;
                    float f7 = view[0].Y * vertex_loc[vert + 0].Y + view[0].Z * vertex_loc[vert + 0].Z
                        + view[0].X * vertex_loc[vert + 0].X + view[0].W;

                    vertex_loc[vert + 0].X = f7;
                    vertex_loc[vert + 0].Y = f6;
                    vertex_loc[vert + 0].Z = f5;

                    f5 = view[2].Y * vertex_loc[vert + 1].Y + view[2].X * vertex_loc[vert + 1].X
                        + view[2].Z * vertex_loc[vert + 1].Z + view[2].W;
                    f6 = view[1].Y * vertex_loc[vert + 1].Y + view[1].X * vertex_loc[vert + 1].X
                        + view[1].Z * vertex_loc[vert + 1].Z + view[1].W;
                    f7 = view[0].X * vertex_loc[vert + 1].X + view[0].Y * vertex_loc[vert + 1].Y
                        + view[0].Z * vertex_loc[vert + 1].Z + view[0].W;

                    vertex_loc[vert + 1].X = f7;
                    vertex_loc[vert + 1].Y = f6;
                    vertex_loc[vert + 1].Z = f5;

                    f5 = view[2].X * vertex_loc[vert + 2].X + view[2].Z * vertex_loc[vert + 2].Z
                        + view[2].Y * vertex_loc[vert + 2].Y + view[2].W;
                    f6 = view[1].X * vertex_loc[vert + 2].X + view[1].Z * vertex_loc[vert + 2].Z
                        + view[1].Y * vertex_loc[vert + 2].Y + view[1].W;
                    f7 = view[0].Z * vertex_loc[vert + 2].Z + view[0].X * vertex_loc[vert + 2].X
                        + view[0].Y * vertex_loc[vert + 2].Y + view[0].W;

                    vertex_loc[vert + 2].X = f7;
                    vertex_loc[vert + 2].Y = f6;
                    vertex_loc[vert + 2].Z = f5;

                    f5 = view[2].X * vertex_loc[vert + 3].X + view[2].Z * vertex_loc[vert + 3].Z
                        + view[2].Y * vertex_loc[vert + 3].Y + view[2].W;
                    f6 = view[1].X * vertex_loc[vert + 3].X + view[1].Z * vertex_loc[vert + 3].Z
                        + view[1].Y * vertex_loc[vert + 3].Y + view[1].W;
                    f7 = view[0].X * vertex_loc[vert + 3].X + view[0].Z * vertex_loc[vert + 3].Z
                        + view[0].Y * vertex_loc[vert + 3].Y + view[0].W;

                    vertex_loc[vert + 3].X = f7;
                    vertex_loc[vert + 3].Y = f6;
                    vertex_loc[vert + 3].Z = f5;
                    vert += 4;
                }
            }
        } break;
        case SCREEN_NOSIZE_NOORIENT:
        case SCREEN_NOSIZE_ORIENT: {
            int xres, yres, bitdepth;
            bool windowed;
            W3D::Get_Render_Target_Resolution(xres, yres, bitdepth, windowed);

            float x_scale = (m_VPXMax - m_VPXMin) / xres;
            float y_scale = (m_VPYMax - m_VPYMin) / yres;

            Vector3 scaled_locs[2][3];

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    scaled_locs[i][j].X = g_screenspaceVertexLocationSizeTable[i][j].X * x_scale;
                    scaled_locs[i][j].Y = g_screenspaceVertexLocationSizeTable[i][j].Y * y_scale;
                    scaled_locs[i][j].Z = g_screenspaceVertexLocationSizeTable[i][j].Z;
                }
            }

            int size_idx = (m_defaultPointSize <= 1.0f) ? 0 : 1;

            for (int i = 0; i < active_points; i++) {
                vertex_loc[vert + 0] = point_loc[i] + scaled_locs[size_idx][0];
                vertex_loc[vert + 1] = point_loc[i] + scaled_locs[size_idx][1];
                vertex_loc[vert + 2] = point_loc[i] + scaled_locs[size_idx][2];
                vert += 3;
            }
        } break;
        case SCREEN_SIZE_NOORIENT:
        case SCREEN_SIZE_ORIENT: {
            int xres, yres, bitdepth;
            bool windowed;
            W3D::Get_Render_Target_Resolution(xres, yres, bitdepth, windowed);

            float x_scale = (m_VPXMax - m_VPXMin) / xres;
            float y_scale = (m_VPYMax - m_VPYMin) / yres;

            Vector3 scaled_locs[2][3];

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    scaled_locs[i][j].X = g_screenspaceVertexLocationSizeTable[i][j].X * x_scale;
                    scaled_locs[i][j].Y = g_screenspaceVertexLocationSizeTable[i][j].Y * y_scale;
                    scaled_locs[i][j].Z = g_screenspaceVertexLocationSizeTable[i][j].Z;
                }
            }

            for (int i = 0; i < active_points; i++) {
                int size_idx = (point_size[i] <= 1.0f) ? 0 : 1;
                vertex_loc[vert + 0] = point_loc[i] + scaled_locs[size_idx][0];
                vertex_loc[vert + 1] = point_loc[i] + scaled_locs[size_idx][1];
                vertex_loc[vert + 2] = point_loc[i] + scaled_locs[size_idx][2];
                vert += 3;
            }
        } break;
        default:
            captainslog_assert(0);
            break;
    }

    unsigned int frame_mask = ~(0xFFFFFFFF << (m_frameRowColumnCountLog2 + m_frameRowColumnCountLog2));
    if (point_frame != nullptr) {
        Vector2 *vertex_uv = &g_vertexUV[0];

        if (m_pointMode != QUADS) {
            Vector2 *uv_ptr = s_triVertexUVFrameTable[m_frameRowColumnCountLog2];
            int vert_idx = 0;
            for (int i = 0; i < active_points; i++) {
                int uv_idx = (point_frame[i] & frame_mask) * 3;
                vertex_uv[vert_idx++] = uv_ptr[uv_idx + 0];
                vertex_uv[vert_idx++] = uv_ptr[uv_idx + 1];
                vertex_uv[vert_idx++] = uv_ptr[uv_idx + 2];
            }
        } else {
            Vector2 *uv_ptr = s_quadVertexUVFrameTable[m_frameRowColumnCountLog2];
            int vert_idx = 0;
            for (int i = 0; i < active_points; i++) {
                int uv_idx = (point_frame[i] & frame_mask) * 4;
                vertex_uv[vert_idx++] = uv_ptr[uv_idx + 0];
                vertex_uv[vert_idx++] = uv_ptr[uv_idx + 1];
                vertex_uv[vert_idx++] = uv_ptr[uv_idx + 2];
                vertex_uv[vert_idx++] = uv_ptr[uv_idx + 3];
            }
        }
    } else {
        Vector2 *vertex_uv = &g_vertexUV[0];

        if (m_pointMode != QUADS) {
            Vector2 *uv_ptr = s_triVertexUVFrameTable[m_frameRowColumnCountLog2] + ((m_defaultPointFrame & frame_mask) * 3);
            int vert_idx = 0;
            for (int i = 0; i < active_points; i++) {
                vertex_uv[vert_idx++] = uv_ptr[0];
                vertex_uv[vert_idx++] = uv_ptr[1];
                vertex_uv[vert_idx++] = uv_ptr[2];
            }
        } else {
            Vector2 *uv_ptr = s_quadVertexUVFrameTable[m_frameRowColumnCountLog2] + ((m_defaultPointFrame & frame_mask) * 4);
            int vert_idx = 0;
            for (int i = 0; i < active_points; i++) {
                vertex_uv[vert_idx++] = uv_ptr[0];
                vertex_uv[vert_idx++] = uv_ptr[1];
                vertex_uv[vert_idx++] = uv_ptr[2];
                vertex_uv[vert_idx++] = uv_ptr[3];
            }
        }
    }

    vert = 0;

    if (point_diffuse != nullptr) {
        Vector4 *vertex_color = &g_vertexDiffuse[0];
        for (int i = 0; i < active_points; i++) {
            for (int j = 0; j < verts_per_point; j++) {
                vertex_color[vert + j] = point_diffuse[i];
            }
            vert += verts_per_point;
        }
    }
#endif
}

void PointGroupClass::Init()
{
    Vector3 tri_locs[3] = { Vector3(0.0f, -2.0f, 0.0f), Vector3(-1.732f, 1.0f, 0.0f), Vector3(1.732f, 1.0f, 0.0f) };
    Vector3 quad_locs[4] = {
        Vector3(-0.5f, 0.5f, 0.0f), Vector3(-0.5f, -0.5f, 0.0f), Vector3(0.5f, -0.5f, 0.0f), Vector3(0.5f, 0.5f, 0.0f)
    };

    float angle = 0.0f;
    float angle_step = (GAMEMATH_PI * 2.0f) / 256.0f;

    for (int i = 0; i < 256; i++) {
        float c = GameMath::Fast_Cos(angle);
        float s = GameMath::Fast_Sin(angle);

        for (int j = 0; j < 3; j++) {
            s_triVertexLocationOrientationTable[i][j].X = tri_locs[j].X * c - tri_locs[j].Y * s;
            s_triVertexLocationOrientationTable[i][j].Y = tri_locs[j].X * s + tri_locs[j].Y * c;
            s_triVertexLocationOrientationTable[i][j].Z = tri_locs[j].Z;
        }

        for (int j = 0; j < 4; j++) {
            s_quadVertexLocationOrientationTable[i][j].X = quad_locs[j].X * c - quad_locs[j].Y * s;
            s_quadVertexLocationOrientationTable[i][j].Y = quad_locs[j].X * s + quad_locs[j].Y * c;
            s_quadVertexLocationOrientationTable[i][j].Z = quad_locs[j].Z;
        }

        angle += angle_step;
    }

    Vector2 tri_uvs[3] = { Vector2(0.5f, 0.0f), Vector2(0.0f, 0.866f), Vector2(1.0f, 0.866f) };
    Vector2 quad_uvs[4] = { Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 0.0f) };

    for (int i = 0; i < 5; i++) {
        unsigned int rows = 1 << i;
        unsigned int count = rows * rows;

        Vector2 *tri_table = s_triVertexUVFrameTable[i] = new Vector2[count * 3];
        Vector2 *quad_table = s_quadVertexUVFrameTable[i] = new Vector2[count * 4];

        Vector2 corner(0.0f, 0.0f);
        float scale = 1.0f / (float)rows;

        int tri_idx = 0;
        int quad_idx = 0;

        for (unsigned int v = 0; v < rows; v++) {
            for (unsigned int u = 0; u < rows; u++) {
                tri_table[tri_idx++] = corner + (tri_uvs[0] * scale);
                tri_table[tri_idx++] = corner + (tri_uvs[1] * scale);
                tri_table[tri_idx++] = corner + (tri_uvs[2] * scale);

                quad_table[quad_idx++] = corner + (quad_uvs[0] * scale);
                quad_table[quad_idx++] = corner + (quad_uvs[1] * scale);
                quad_table[quad_idx++] = corner + (quad_uvs[2] * scale);
                quad_table[quad_idx++] = corner + (quad_uvs[3] * scale);

                corner.X += scale;
            }

            corner.Y += scale;
            corner.X = 0.0f;
        }
    }

    g_tris = new DX8IndexBufferClass(MAX_TRI_IB_SIZE, DX8IndexBufferClass::USAGE_DEFAULT);
    g_quads = new DX8IndexBufferClass(MAX_QUAD_IB_SIZE, DX8IndexBufferClass::USAGE_DEFAULT);
    g_sortingTris = new SortingIndexBufferClass(MAX_TRI_IB_SIZE);
    g_sortingQuads = new SortingIndexBufferClass(MAX_QUAD_IB_SIZE);

    {
        DX8IndexBufferClass::WriteLockClass locktris(g_tris, 0);
        unsigned short *ib = locktris.Get_Index_Array();

        for (int i = 0; i < MAX_TRI_IB_SIZE; i++) {
            ib[i] = (unsigned short)i;
        }
    }

    {
        unsigned short vert = 0;
        DX8IndexBufferClass::WriteLockClass lockquads(g_quads, 0);
        unsigned short *ib = lockquads.Get_Index_Array();
        vert = 0;

        for (int i = 0; i < MAX_QUAD_IB_SIZE; i += 6) {
            ib[i] = vert;
            ib[i + 1] = vert + 1;
            ib[i + 2] = vert + 2;

            ib[i + 3] = vert + 2;
            ib[i + 4] = vert + 3;
            ib[i + 5] = vert;
            vert += 4;
        }
    }

    {
        SortingIndexBufferClass::WriteLockClass locktris(g_sortingTris, 0);
        unsigned short *ib = locktris.Get_Index_Array();

        for (int i = 0; i < MAX_TRI_IB_SIZE; i++) {
            ib[i] = (unsigned short)i;
        }
    }

    {
        unsigned short vert = 0;
        SortingIndexBufferClass::WriteLockClass lockquads(g_sortingQuads, 0);
        unsigned short *ib = lockquads.Get_Index_Array();
        vert = 0;

        for (int i = 0; i < MAX_QUAD_IB_SIZE; i += 6) {
            ib[i] = vert;
            ib[i + 1] = vert + 1;
            ib[i + 2] = vert + 2;

            ib[i + 3] = vert + 2;
            ib[i + 4] = vert + 3;
            ib[i + 5] = vert;
            vert += 4;
        }
    }

    s_pointMaterial = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
}

void PointGroupClass::Shutdown()
{
    for (int i = 0; i < 5; i++) {
        delete[] s_triVertexUVFrameTable[i];
        delete[] s_quadVertexUVFrameTable[i];
    }

    Ref_Ptr_Release(s_pointMaterial);
    Ref_Ptr_Release(g_sortingQuads);
    Ref_Ptr_Release(g_sortingTris);
    Ref_Ptr_Release(g_quads);
    Ref_Ptr_Release(g_tris);

    s_transformedLoc.Clear();
    g_vertexLoc.Clear();
    g_vertexDiffuse.Clear();
    g_vertexUV.Clear();
}
