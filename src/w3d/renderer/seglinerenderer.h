/**
 * @file
 *
 * @author Jonathan Wilson
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
#pragma once

#include "always.h"
#include "matrix3d.h"
#include "shader.h"
#include "texture.h"
#include "vector2.h"

class RenderInfoClass;
class SphereClass;
struct W3dEmitterLinePropertiesStruct;
struct VertexFormatXYZDUV1;

#define MAX_SEGLINE_SUBDIV_LEVELS 7

class SegLineRendererClass
{
public:
    SegLineRendererClass();
    SegLineRendererClass(const SegLineRendererClass &that);
    SegLineRendererClass &operator=(const SegLineRendererClass &that);
    ~SegLineRendererClass();

    enum TextureMapMode
    {
        UNIFORM_WIDTH_TEXTURE_MAP = 0x00000000,
        UNIFORM_LENGTH_TEXTURE_MAP = 0x00000001,
        TILED_TEXTURE_MAP = 0x00000002
    };

    void Init(const W3dEmitterLinePropertiesStruct &props);

    TextureClass *Get_Texture() const;
    TextureClass *Peek_Texture() const { return m_texture; }
    ShaderClass Get_Shader() const { return m_shader; }
    float Get_Width() const { return m_width; }
    const Vector3 &Get_Color() const { return m_color; }
    float Get_Opacity() const { return m_opacity; }
    float Get_Noise_Amplitude() const { return m_noiseAmplitude; }
    float Get_Merge_Abort_Factor() const { return m_mergeAbortFactor; }
    unsigned int Get_Current_Subdivision_Level() const { return m_subdivisionLevel; }
    TextureMapMode Get_Texture_Mapping_Mode() const;
    float Get_Texture_Tile_Factor() const { return m_textureTileFactor; }
    Vector2 Get_UV_Offset_Rate() const;
    int Is_Merge_Intersections() const { return m_bits & MERGE_INTERSECTIONS; }
    int Is_Freeze_Random() const { return m_bits & FREEZE_RANDOM; }
    int Is_Sorting_Disabled() const { return m_bits & DISABLE_SORTING; }
    int Are_End_Caps_Enabled() const { return m_bits & END_CAPS; }

    void Set_Texture(TextureClass *texture);
    void Set_Shader(ShaderClass shader) { m_shader = shader; }
    void Set_Width(float width) { m_width = width; }
    void Set_Color(const Vector3 &color) { m_color = color; }
    void Set_Opacity(float opacity) { m_opacity = opacity; }
    void Set_Noise_Amplitude(float amplitude) { m_noiseAmplitude = amplitude; }
    void Set_Merge_Abort_Factor(float factor) { m_mergeAbortFactor = factor; }
    void Set_Current_Subdivision_Level(unsigned int lv) { m_subdivisionLevel = lv; }
    void Set_Texture_Mapping_Mode(TextureMapMode mode);
    void Set_Texture_Tile_Factor(float factor);
    void Set_Current_UV_Offset(const Vector2 &offset);
    void Set_UV_Offset_Rate(const Vector2 &rate);
    void Set_Merge_Intersections(int onoff)
    {
        if (onoff) {
            m_bits |= MERGE_INTERSECTIONS;
        } else {
            m_bits &= ~MERGE_INTERSECTIONS;
        };
    }
    void Set_Freeze_Random(int onoff)
    {
        if (onoff) {
            m_bits |= FREEZE_RANDOM;
        } else {
            m_bits &= ~FREEZE_RANDOM;
        };
    }
    void Set_Disable_Sorting(int onoff)
    {
        if (onoff) {
            m_bits |= DISABLE_SORTING;
        } else {
            m_bits &= ~DISABLE_SORTING;
        };
    }
    void Set_End_Caps(int onoff)
    {
        if (onoff) {
            m_bits |= END_CAPS;
        } else {
            m_bits &= ~END_CAPS;
        };
    }

    void Render(RenderInfoClass &rinfo,
        const Matrix3D &transform,
        unsigned int point_count,
        Vector3 *points,
        const SphereClass &obj_sphere,
        Vector4 *colors);

    void Reset_Line();
    void Scale(float scale);

private:
    enum BitShiftOffsets
    {
        TEXTURE_MAP_MODE_OFFSET = 24
    };

    enum
    {
        MERGE_INTERSECTIONS = 0x00000001,
        FREEZE_RANDOM = 0x00000002,
        DISABLE_SORTING = 0x00000004,
        END_CAPS = 0x00000008,
        TEXTURE_MAP_MODE_MASK = 0xFF000000,
        DEFAULT_BITS = MERGE_INTERSECTIONS | (UNIFORM_WIDTH_TEXTURE_MAP << TEXTURE_MAP_MODE_OFFSET)
    };

    void Subdivision_Util(unsigned int point_cnt,
        const Vector3 *xformed_pts,
        const float *base_tex_v,
        unsigned int *p_sub_point_cnt,
        Vector3 *xformed_subdiv_pts,
        float *subdiv_tex_v,
        Vector4 *base_color_v,
        Vector4 *subdiv_color_v);
    VertexFormatXYZDUV1 *Get_Vertex_Buffer(unsigned int count);

    TextureClass *m_texture;
    ShaderClass m_shader;
    float m_width;
    Vector3 m_color;
    float m_opacity;
    unsigned int m_subdivisionLevel;
    float m_noiseAmplitude;
    float m_mergeAbortFactor;
    float m_textureTileFactor;
    unsigned int m_lastUsedSyncTime;
    Vector2 m_currentUVOffset;
    Vector2 m_UVOffsetDeltaPerMS;
    unsigned int m_bits;
    unsigned int m_vertexCount;
    VertexFormatXYZDUV1 *m_vertexBuffer;

    friend class SegmentedLineClass;
};

inline SegLineRendererClass::TextureMapMode SegLineRendererClass::Get_Texture_Mapping_Mode() const
{
    return static_cast<TextureMapMode>((m_bits & TEXTURE_MAP_MODE_MASK) >> TEXTURE_MAP_MODE_OFFSET);
}

inline void SegLineRendererClass::Set_Texture_Mapping_Mode(SegLineRendererClass::TextureMapMode mode)
{
    m_bits &= ~TEXTURE_MAP_MODE_MASK;
    m_bits |= ((mode << TEXTURE_MAP_MODE_OFFSET) & TEXTURE_MAP_MODE_MASK);
}

inline Vector2 SegLineRendererClass::Get_UV_Offset_Rate() const
{
    return m_UVOffsetDeltaPerMS * 1000.0f;
}

inline void SegLineRendererClass::Set_UV_Offset_Rate(const Vector2 &rate)
{
    m_UVOffsetDeltaPerMS = rate * 0.001f;
}
