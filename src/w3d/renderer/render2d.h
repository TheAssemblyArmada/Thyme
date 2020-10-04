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
#pragma once

#include "always.h"
#include "rect.h"
#include "shader.h"
#include "vector.h"
#include "vector2.h"
#include "w3dmpo.h"
#include <new>

class TextureClass;

class Render2DClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(Render2DClass)
public:
    Render2DClass(TextureClass *texture = nullptr);
    virtual ~Render2DClass();
    virtual void Reset();

    void Render();
    void Set_Coordinate_Range(const RectClass &range);
    void Set_Texture(const char *filename);
    void Set_Texture(TextureClass *tex);
    TextureClass *Peek_Texture() { return m_texture; }
    void Enable_Additive(bool b);
    void Enable_Alpha(bool b);
    void Enable_Texturing(bool b);
    void Enable_GreyScale(bool b) { m_greyScale = b; }
    ShaderClass *Get_Shader() { return &m_shader; }
    void Add_Quad(const Vector2 &v0,
        const Vector2 &v1,
        const Vector2 &v2,
        const Vector2 &v3,
        const RectClass &uv,
        uint32_t color = 0xFFFFFFFF);
    void Add_Quad_Backfaced(const Vector2 &v0,
        const Vector2 &v1,
        const Vector2 &v2,
        const Vector2 &v3,
        const RectClass &uv,
        uint32_t color = 0xFFFFFFFF);
    void Add_Quad(const RectClass &screen, const RectClass &uv, uint32_t color = 0xFFFFFFFF);
    void Add_Quad(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2, const Vector2 &v3, uint32_t color = 0xFFFFFFFF);
    void Add_Quad(const RectClass &screen, uint32_t color = 0xFFFFFFFF);
    void Add_Quad_VGradient(const RectClass &screen, uint32_t top_color, uint32_t bottom_color);
    void Add_Quad_HGradient(const RectClass &screen, uint32_t left_color, uint32_t right_color);
    void Add_Quad_VGradient(const Vector2 &v0,
        const Vector2 &v1,
        const Vector2 &v2,
        const Vector2 &v3,
        const RectClass &screen,
        uint32_t top_color,
        uint32_t bottom_color);
    void Add_Quad_HGradient(const Vector2 &v0,
        const Vector2 &v1,
        const Vector2 &v2,
        const Vector2 &v3,
        const RectClass &screen,
        uint32_t left_color,
        uint32_t right_color);
    void Add_Tri(const Vector2 &v0,
        const Vector2 &v1,
        const Vector2 &v2,
        const Vector2 &uv0,
        const Vector2 &uv1,
        const Vector2 &uv2,
        uint32_t color = 0xFFFFFFFF);
    void Add_Line(const Vector2 &a, const Vector2 &b, float width, uint32_t color = 0xFFFFFFFF);
    void Add_Line(const Vector2 &a, const Vector2 &b, float width, const RectClass &uv, uint32_t color = 0xFFFFFFFF);
    void Add_Line(const Vector2 &a, const Vector2 &b, float width, uint32_t color1, uint32_t color2);
    void Add_Line(const Vector2 &a, const Vector2 &b, float width, const RectClass &uv, uint32_t color1, uint32_t color2);
    void Add_Outline(const RectClass &rect, float width = 1.0F, uint32_t color = 0xFFFFFFFF);
    void Add_Outline(const RectClass &rect, float width, const RectClass &uv, uint32_t color = 0xFFFFFFFF);
    void Add_Rect(const RectClass &rect,
        float border_width = 1.0F,
        uint32_t border_color = 0xFF000000,
        uint32_t fill_color = 0xFFFFFFFF);
    void Set_Hidden(bool hide) { m_isHidden = hide; }
    void Set_Z_Value(float z_value) { m_zValue = z_value; }
    void Move(const Vector2 &a);
    DynamicVectorClass<uint32_t> &Get_Color_Array() { return m_colors; }
    void Force_Alpha(float alpha);
    void Force_Color(int color);

    static void Set_Screen_Resolution(const RectClass &rect) { s_screenResolution = rect; }
    static RectClass Get_Screen_Resolution() { return s_screenResolution; }
    static ShaderClass Get_Default_Shader();

#ifdef GAME_DLL
    Render2DClass *Hook_Ctor(TextureClass *texture) { return new (this) Render2DClass(texture); }
#endif

protected:
    void Convert_Vert(Vector2 &vert_out, const Vector2 &vert_in);
    void Convert_Vert(Vector2 &vert_out, float x_in, float y_in);
    void Update_Bias();
    void Internal_Add_Quad_Vertices(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2, const Vector2 &v3);
    void Internal_Add_Quad_Vertices(const RectClass &screen);
    void Internal_Add_Quad_UVs(const RectClass &uv);
    void Internal_Add_Quad_Colors(uint32_t color);
    void Internal_Add_Quad_VColors(uint32_t color1, uint32_t color2);
    void Internal_Add_Quad_HColors(uint32_t color1, uint32_t color2);
    void Internal_Add_Quad_Indicies(int start_vert_index, bool backfaced = false);

protected:
    Vector2 m_coordinateScale;
    Vector2 m_coordinateOffset;
    Vector2 m_biasedCoordinateOffset;
    TextureClass *m_texture;
    ShaderClass m_shader;
    DynamicVectorClass<uint16_t> m_indices;
    uint16_t m_indicesPreAlloc[60];
    DynamicVectorClass<Vector2> m_vertices;
    Vector2 m_verticesPreAlloc[60];
    DynamicVectorClass<Vector2> m_uvCoordinates;
    Vector2 m_uvCoordinatePreAlloc[60];
    DynamicVectorClass<uint32_t> m_colors;
    uint32_t m_coloursBuffer[60];
    bool m_isHidden;
    bool m_greyScale;
    float m_zValue;

#ifdef GAME_DLL
    static RectClass &s_screenResolution;
#else
    static RectClass s_screenResolution;
#endif
};
