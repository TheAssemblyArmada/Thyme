/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberium Technologies
 *
 * @brief Class controlling 2D rendering.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "render2d.h"
#include "assetmgr.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "matrix4.h"
#include "texture.h"
#include "w3d.h"

#ifndef GAME_DLL
RectClass Render2DClass::s_screenResolution;
#else
#include "hooker.h"
#endif

Render2DClass::Render2DClass(TextureClass *texture) :
    m_coordinateScale(1.0f, 1.0f),
    m_coordinateOffset(0.0f, 0.0f),
    m_texture(nullptr),
    m_shader(),
    m_indices(60, m_indicesPreAlloc),
    m_vertices(60, m_verticesPreAlloc),
    m_uvCoordinates(60, m_uvCoordinatePreAlloc),
    m_colors(60, m_coloursBuffer),
    m_isHidden(false),
    m_greyScale(false),
    m_zValue(0.0f)
{
    Set_Texture(texture);
    m_shader = Get_Default_Shader();
}

Render2DClass::~Render2DClass()
{
    Ref_Ptr_Release(m_texture);
}

/**
 * Resets the render state.
 *
 * 0x008094E0
 */
void Render2DClass::Reset()
{
    m_vertices.Reset_Active();
    m_uvCoordinates.Reset_Active();
    m_colors.Reset_Active();
    m_indices.Reset_Active();
    Update_Bias();
}

/**
 * Renders the information held in the class.
 *
 * 0x0080AAC0
 */
void Render2DClass::Render()
{
#ifdef BUILD_WITH_D3D8
    if (m_indices.Count() && !m_isHidden) {
        Matrix4 proj;
        Matrix4 view;
        Matrix4 identity;
        identity.Make_Identity();
        DX8Wrapper::Get_Transform(D3DTS_PROJECTION, proj);
        DX8Wrapper::Get_Transform(D3DTS_VIEW, view);

        int width;
        int height;
        int bit_depth;
        bool windowed;
        W3D::Get_Device_Resolution(width, height, bit_depth, windowed);

        D3DVIEWPORT8 vp;
        vp.X = 0;
        vp.Y = 0;
        vp.Width = width;
        vp.Height = height;
        vp.MinZ = 0;
        vp.MaxZ = 1.0f;
        DX8Wrapper::Set_Viewport(&vp);

        DX8Wrapper::Set_Texture(0, m_texture);

        VertexMaterialClass *mat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
        DX8Wrapper::Set_Material(mat);
        mat->Release_Ref();

        DX8Wrapper::Set_Transform(D3DTS_WORLD, identity);
        DX8Wrapper::Set_Transform(D3DTS_VIEW, identity);
        DX8Wrapper::Set_Transform(D3DTS_PROJECTION, identity);

        DynamicVBAccessClass vb(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8,
            (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_DIFFUSE),
            m_vertices.Count());

        {
            DynamicVBAccessClass::WriteLockClass lock(&vb);
            VertexFormatXYZNDUV2 *vf = lock.Get_Formatted_Vertex_Array();

            for (int i = 0; i < m_vertices.Count(); i++) {
                vf[i].x = m_vertices[i].X;
                vf[i].y = m_vertices[i].Y;
                vf[i].z = m_zValue;
                vf[i].diffuse = m_colors[i];
                vf[i].u1 = m_uvCoordinates[i].X;
                vf[i].v1 = m_uvCoordinates[i].Y;
            }
        }

        DynamicIBAccessClass ib = DynamicIBAccessClass(2, (unsigned short)m_indices.Count());
        {
            DynamicIBAccessClass::WriteLockClass lock(&ib);
            unsigned short *ind = lock.Get_Index_Array();
            for (int i = 0; i < m_indices.Count(); i++) {
                ind[i] = m_indices[i];
            }
        }

        DX8Wrapper::Set_Vertex_Buffer(vb);
        DX8Wrapper::Set_Index_Buffer(ib, 0);

        if (m_greyScale) {
            DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
            DX8Wrapper::Apply_Render_State_Changes();

            if (DX8Wrapper::Get_Current_Caps()->Supports_Dot3_Blend()) {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(128, 165, 202, 142));
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG0, D3DTA_ALPHAREPLICATE | D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_ALPHAREPLICATE | D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
            } else {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(96, 96, 96, 96));
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
                DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            }
        } else {
            DX8Wrapper::Set_Shader(m_shader);
        }

        DX8Wrapper::Draw_Triangles(0, m_indices.Count() / 3, 0, m_vertices.Count());
        DX8Wrapper::Set_Transform(D3DTS_VIEW, view);
        DX8Wrapper::Set_Transform(D3DTS_PROJECTION, proj);

        if (m_greyScale) {
            ShaderClass::Invalidate();
        }
    }
#endif
}

/**
 * Updates the constraints on where coordinates can be rendered.
 *
 * 0x008096C0
 */
void Render2DClass::Set_Coordinate_Range(const RectClass &range)
{
    m_coordinateScale.X = 2.0f / range.Width();
    m_coordinateScale.Y = -2.0f / range.Height();
    m_coordinateOffset.X = (float)(-(m_coordinateScale.X * range.left)) - 1.0f;
    m_coordinateOffset.Y = 1.0f - (float)(range.top * m_coordinateScale.Y);
    Update_Bias();
}

/**
 * Sets a texture to render to the 2D area.
 *
 * 0x00809590
 */
void Render2DClass::Set_Texture(const char *filename)
{
    auto *tex = W3DAssetManager::Get_Instance()->Get_Texture(filename, MIP_LEVELS_1);
    Set_Texture(tex);
    tex->Release_Ref();
}

/**
 * Sets a texture to render to the 2D area.
 *
 * 0x00809560
 */
void Render2DClass::Set_Texture(TextureClass *tex)
{
    Ref_Ptr_Set(m_texture, tex);
}

/**
 * Sets the additive shading mode.
 *
 * 0x00809640
 */
void Render2DClass::Enable_Additive(bool b)
{
    if (b) {
        m_shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
        m_shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
    } else {
        m_shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
        m_shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ZERO);
    }
}

/**
 * Sets the alpha shading mode.
 *
 * 0x008095F0
 */
void Render2DClass::Enable_Alpha(bool b)
{
    if (b) {
        m_shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);
        m_shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
    } else {
        m_shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
        m_shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ZERO);
    }
}

/**
 * Sets the texturing mode.
 *
 * 0x00809690
 */
void Render2DClass::Enable_Texturing(bool b)
{
    if (b) {
        m_shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
    } else {
        m_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
    }
}

/**
 * Adds a quad to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad(
    const Vector2 &v0, const Vector2 &v1, const Vector2 &v2, const Vector2 &v3, const RectClass &uv, uint32_t color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(v0, v1, v2, v3);
    Internal_Add_Quad_UVs(uv);
    Internal_Add_Quad_Colors(color);
}

/**
 * Adds a backfaced quad to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad_Backfaced(
    const Vector2 &v0, const Vector2 &v1, const Vector2 &v2, const Vector2 &v3, const RectClass &uv, uint32_t color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), true);
    Internal_Add_Quad_Vertices(v0, v1, v2, v3);
    Internal_Add_Quad_UVs(uv);
    Internal_Add_Quad_Colors(color);
}

/**
 * Adds a quad to the pipeline.
 *
 * 0x00809AF0
 */
void Render2DClass::Add_Quad(const RectClass &screen, const RectClass &uv, uint32_t color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(screen);
    Internal_Add_Quad_UVs(uv);
    Internal_Add_Quad_Colors(color);
}

/**
 * Adds a quad to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2, const Vector2 &v3, uint32_t color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(v0, v1, v2, v3);
    Internal_Add_Quad_UVs(RectClass(0.0f, 0.0f, 1.0f, 1.0f));
    Internal_Add_Quad_Colors(color);
}

/**
 * Adds a quad to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad(const RectClass &screen, uint32_t color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(screen);
    Internal_Add_Quad_UVs(RectClass(0.0f, 0.0f, 1.0f, 1.0f));
    Internal_Add_Quad_Colors(color);
}

/**
 * Adds a quad with a gradient to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad_VGradient(const RectClass &screen, uint32_t top_color, uint32_t bottom_color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(screen);
    Internal_Add_Quad_UVs(RectClass(0.0f, 0.0f, 1.0f, 1.0f));
    Internal_Add_Quad_VColors(top_color, bottom_color);
}

/**
 * Adds a quad with a gradient to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad_HGradient(const RectClass &screen, uint32_t left_color, uint32_t right_color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(screen);
    Internal_Add_Quad_UVs(RectClass(0.0f, 0.0f, 1.0f, 1.0f));
    Internal_Add_Quad_HColors(left_color, right_color);
}

/**
 * Adds a quad with a gradient to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad_VGradient(const Vector2 &v0,
    const Vector2 &v1,
    const Vector2 &v2,
    const Vector2 &v3,
    const RectClass &uv,
    uint32_t top_color,
    uint32_t bottom_color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(v0, v1, v2, v3);
    Internal_Add_Quad_UVs(uv);
    Internal_Add_Quad_VColors(top_color, bottom_color);
}

/**
 * Adds a quad with a gradient to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Quad_HGradient(const Vector2 &v0,
    const Vector2 &v1,
    const Vector2 &v2,
    const Vector2 &v3,
    const RectClass &uv,
    uint32_t left_color,
    uint32_t right_color)
{
    Internal_Add_Quad_Indicies(m_vertices.Count(), false);
    Internal_Add_Quad_Vertices(v0, v1, v2, v3);
    Internal_Add_Quad_UVs(uv);
    Internal_Add_Quad_HColors(left_color, right_color);
}

/**
 * Adds a tri to the pipeline.
 *
 * 0x00809CA0
 */
void Render2DClass::Add_Tri(const Vector2 &v0,
    const Vector2 &v1,
    const Vector2 &v2,
    const Vector2 &uv0,
    const Vector2 &uv1,
    const Vector2 &uv2,
    uint32_t color)
{
    int startindex = m_vertices.Count();
    Convert_Vert(*m_vertices.Uninitialized_Add(), v0);
    Convert_Vert(*m_vertices.Uninitialized_Add(), v1);
    Convert_Vert(*m_vertices.Uninitialized_Add(), v2);
    *m_uvCoordinates.Uninitialized_Add() = uv0;
    *m_uvCoordinates.Uninitialized_Add() = uv1;
    *m_uvCoordinates.Uninitialized_Add() = uv2;
    *m_colors.Uninitialized_Add() = color;
    *m_colors.Uninitialized_Add() = color;
    *m_colors.Uninitialized_Add() = color;
    *m_indices.Uninitialized_Add() = (uint16_t)startindex;
    *m_indices.Uninitialized_Add() = (uint16_t)startindex + 1;
    *m_indices.Uninitialized_Add() = (uint16_t)startindex + 2;
}

/**
 * Adds a line to the pipeline.
 *
 * 0x00809EC0
 */
void Render2DClass::Add_Line(const Vector2 &a, const Vector2 &b, float width, uint32_t color)
{
    Add_Line(a, b, width, RectClass(0.0f, 0.0f, 1.0f, 1.0f), color);
}

/**
 * Adds a line to the pipeline.
 *
 * 0x0080A0F0
 */
void Render2DClass::Add_Line(const Vector2 &a, const Vector2 &b, float width, const RectClass &uv, uint32_t color)
{
    Vector2 tmp(a.Y - b.Y, -(a.X - b.X));
    tmp.Normalize();
    tmp *= width * 0.5f;
    Add_Quad(a - tmp, a + tmp, b - tmp, b + tmp, uv, color);
}

/**
 * Adds a line with a gradient to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Line(const Vector2 &a, const Vector2 &b, float width, uint32_t color1, uint32_t color2)
{
    Add_Line(a, b, width, RectClass(0.0f, 0.0f, 1.0f, 1.0f), color1, color2);
}

/**
 * Adds a line with a gradient to the pipeline.
 *
 * Optimised out
 */
void Render2DClass::Add_Line(
    const Vector2 &a, const Vector2 &b, float width, const RectClass &uv, uint32_t color1, uint32_t color2)
{
    Vector2 tmp(a.Y - b.Y, -(a.X - b.X));
    tmp.Normalize();
    tmp *= width * 0.5f;
    Add_Quad_HGradient(a - tmp, a + tmp, b - tmp, b + tmp, uv, color1, color2);
}

/**
 * Adds an outline to the pipeline.
 *
 * 0x0080A790
 */
void Render2DClass::Add_Outline(const RectClass &rect, float width, uint32_t color)
{
    Add_Outline(rect, width, RectClass(0.0f, 0.0f, 1.0f, 1.0f), color);
}

/**
 * Adds an outline to the pipeline.
 *
 * 0x0080A7E0
 */
void Render2DClass::Add_Outline(const RectClass &rect, float width, const RectClass &uv, uint32_t color)
{
    Add_Line(Vector2(rect.left + 1.0f, rect.bottom), Vector2(rect.left + 1.0f, rect.top + 1.0f), width, uv, color);
    Add_Line(Vector2(rect.left, rect.top + 1.0f), Vector2(rect.right - 1.0f, rect.top + 1.0f), width, uv, color);
    Add_Line(Vector2(rect.right, rect.top), Vector2(rect.right, rect.bottom - 1.0f), width, uv, color);
    Add_Line(Vector2(rect.right, rect.bottom), Vector2(rect.left + 1.0f, rect.bottom), width, uv, color);
}

/**
 * Adds a rect to the pipeline.
 *
 * 0x0080A580
 */
void Render2DClass::Add_Rect(const RectClass &rect, float border_width, uint32_t border_color, uint32_t fill_color)
{
    // Renegade does not do these ">" checks.
    if (border_width > 0.0f) {
        Add_Outline(rect, border_width, border_color);
    }

    RectClass r = rect;

    if (border_width > 0.0f) {
        r.left = (float)(r.left + border_width) + 1.0f;
        r.top = (float)(r.top + border_width) + 1.0f;
        r.right = (float)(r.right - border_width) - 1.0f;
        r.bottom = (float)(r.bottom - border_width) - 1.0f;
    }

    Add_Quad(r, fill_color);
}

/**
 * Moves the vertices held for rendering to a new position.
 *
 * Optimised out
 */
void Render2DClass::Move(const Vector2 &a)
{
    Vector2 scaled_move = a;
    scaled_move.Scale(m_coordinateScale);

    for (int i = 0; i < m_vertices.Count(); i++) {
        m_vertices[i] += scaled_move;
    }
}

/**
 * Forces alpha to a given value.
 *
 * Optimised out
 */
void Render2DClass::Force_Alpha(float alpha)
{
    if (alpha < 0.0f) {
        alpha = 0.0f;
    }

    if (alpha > 1.0f) {
        alpha = 1.0f;
    }

    alpha *= 255;
    int alphaval = (int)alpha;

    // Renegade does some bitwise operations with this value.
    for (int i = 0; i < m_colors.Count(); i++) {
        m_colors[i] = alphaval;
    }
}

/**
 * Forces color to a given value.
 *
 * Optimised out
 */
void Render2DClass::Force_Color(int color)
{
    for (int i = 0; i < m_colors.Count(); i++) {
        m_colors[i] = color;
    }
}

/**
 * Gets the default shader configuration.
 *
 * 0x008094B0
 */
ShaderClass Render2DClass::Get_Default_Shader()
{
    return Shader_Const(ShaderClass::PASS_ALWAYS,
        ShaderClass::DEPTH_WRITE_DISABLE,
        ShaderClass::COLOR_WRITE_ENABLE,
        ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA,
        ShaderClass::FOG_DISABLE,
        ShaderClass::GRADIENT_MODULATE,
        ShaderClass::SECONDARY_GRADIENT_DISABLE,
        ShaderClass::SRCBLEND_SRC_ALPHA,
        ShaderClass::TEXTURING_ENABLE,
        ShaderClass::NPATCH_DISABLE,
        ShaderClass::ALPHATEST_DISABLE,
        ShaderClass::CULL_MODE_ENABLE,
        ShaderClass::DETAILCOLOR_DISABLE,
        ShaderClass::DETAILCOLOR_DISABLE);
}

/**
 * Converts coordinates to conform with classes scaling and bias.
 *
 * 0x00809770
 */
void Render2DClass::Convert_Vert(Vector2 &vert_out, const Vector2 &vert_in)
{
    vert_out.X = (float)(vert_in.X * m_coordinateScale.X) + m_biasedCoordinateOffset.X;
    vert_out.Y = (float)(vert_in.Y * m_coordinateScale.Y) + m_biasedCoordinateOffset.Y;
}

/**
 * Converts coordinates to conform with classes scaling and bias.
 *
 * 0x008097A0
 */
void Render2DClass::Convert_Vert(Vector2 &vert_out, float x_in, float y_in)
{
    vert_out.X = (float)(x_in * m_coordinateScale.X) + m_biasedCoordinateOffset.X;
    vert_out.Y = (float)(y_in * m_coordinateScale.Y) + m_biasedCoordinateOffset.Y;
}

/**
 * Updates the screen bias.
 *
 * Optimised out.
 */
void Render2DClass::Update_Bias()
{
    m_biasedCoordinateOffset = m_coordinateOffset;

    if (W3D::Is_Screen_UVBiased()) {
        m_biasedCoordinateOffset.X += (float)(-0.5f / (float)(s_screenResolution.Width() * 0.5f));
        m_biasedCoordinateOffset.Y += (float)(-0.5f / (float)(s_screenResolution.Height() * -0.5f));
    }
}

/**
 * Internal function to add vertices.
 *
 * Optimised out.
 */
void Render2DClass::Internal_Add_Quad_Vertices(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2, const Vector2 &v3)
{
    Convert_Vert(*m_vertices.Uninitialized_Add(), v0);
    Convert_Vert(*m_vertices.Uninitialized_Add(), v1);
    Convert_Vert(*m_vertices.Uninitialized_Add(), v2);
    Convert_Vert(*m_vertices.Uninitialized_Add(), v3);
}

/**
 * Internal function to add vertices.
 *
 * 0x008097C0
 */
void Render2DClass::Internal_Add_Quad_Vertices(const RectClass &screen)
{
    Convert_Vert(*m_vertices.Uninitialized_Add(), screen.left, screen.top);
    Convert_Vert(*m_vertices.Uninitialized_Add(), screen.left, screen.bottom);
    Convert_Vert(*m_vertices.Uninitialized_Add(), screen.right, screen.top);
    Convert_Vert(*m_vertices.Uninitialized_Add(), screen.right, screen.bottom);
}

/**
 * Internal function to add UVs.
 *
 * Optimised out.
 */
void Render2DClass::Internal_Add_Quad_UVs(const RectClass &uv)
{
    Vector2 *v = m_uvCoordinates.Uninitialized_Add();
    v->X = uv.left;
    v->Y = uv.top;
    v = m_uvCoordinates.Uninitialized_Add();
    v->X = uv.left;
    v->Y = uv.bottom;
    v = m_uvCoordinates.Uninitialized_Add();
    v->X = uv.right;
    v->Y = uv.top;
    v = m_uvCoordinates.Uninitialized_Add();
    v->X = uv.right;
    v->Y = uv.bottom;
}

/**
 * Internal function to add colors.
 *
 * Optimised out.
 */
void Render2DClass::Internal_Add_Quad_Colors(uint32_t color)
{
    *m_colors.Uninitialized_Add() = color;
    *m_colors.Uninitialized_Add() = color;
    *m_colors.Uninitialized_Add() = color;
    *m_colors.Uninitialized_Add() = color;
}

/**
 * Internal function to add gradient colors.
 *
 * Optimised out.
 */
void Render2DClass::Internal_Add_Quad_VColors(uint32_t color1, uint32_t color2)
{
    *m_colors.Uninitialized_Add() = color1;
    *m_colors.Uninitialized_Add() = color2;
    *m_colors.Uninitialized_Add() = color1;
    *m_colors.Uninitialized_Add() = color2;
}

/**
 * Internal function to add gradient colors.
 *
 * Optimised out.
 */
void Render2DClass::Internal_Add_Quad_HColors(uint32_t color1, uint32_t color2)
{
    *m_colors.Uninitialized_Add() = color1;
    *m_colors.Uninitialized_Add() = color1;
    *m_colors.Uninitialized_Add() = color2;
    *m_colors.Uninitialized_Add() = color2;
}

/**
 * Internal function to add indices.
 *
 * 0x008098B0
 */
void Render2DClass::Internal_Add_Quad_Indicies(int start_vert_index, bool backfaced)
{
    if (m_coordinateScale.X * m_coordinateScale.Y <= 0.0f) {
        backfaced = !backfaced;
    }

    if (backfaced) {
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 1;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 2;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 2;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 1;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 3;
    } else {
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 1;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 2;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 1;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 2;
        *m_indices.Uninitialized_Add() = (uint16_t)start_vert_index + 3;
    }
}
