/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief Streak Renderer
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
#include "sphere.h"
#include "vector3.h"

class TextureClass;
struct VertexFormatXYZUV1;
struct W3dEmitterLinePropertiesStruct;
class RenderInfoClass;

#define MAX_STREAK_SUBDIV_LEVELS 7

class StreakRendererClass
{
public:
    StreakRendererClass();
    StreakRendererClass(const StreakRendererClass &src);
    StreakRendererClass &operator=(const StreakRendererClass &src);
    ~StreakRendererClass();

    enum TextureMapMode
    {
        UNIFORM_WIDTH_TEXTURE_MAP = 0x00000000,
        UNIFORM_LENGTH_TEXTURE_MAP = 0x00000001,
        TILED_TEXTURE_MAP = 0x00000002
    };

    void Init(const W3dEmitterLinePropertiesStruct &props);

    void Set_Texture(TextureClass *texture);
    TextureClass *Get_Texture() const;

    void Render(RenderInfoClass &rinfo,
        const Matrix3D &transform,
        unsigned int point_count,
        Vector3 *points,
        const SphereClass &obj_sphere);

    void Subdivision_Util(unsigned int point_cnt,
        const Vector3 *xformed_pts,
        const float *base_tex_v,
        unsigned int *p_sub_point_cnt,
        Vector3 *xformed_subdiv_pts,
        float *subdiv_tex_v);

    void Render_Streak(RenderInfoClass &rinfo,
        Matrix3D const &transform,
        unsigned int point_count,
        Vector3 *points,
        Vector4 *colors,
        float *widths,
        SphereClass const &obj_sphere,
        unsigned int *unk);

    VertexFormatXYZUV1 *Get_Vertex_Buffer(unsigned int vertex_count);

    TextureMapMode Get_Texture_Mapping_Mode() const;
    void Set_Texture_Mapping_Mode(TextureMapMode mode);

    int Is_Freeze_Random() const { return m_bits & FREEZE_RANDOM; }
    int Is_Merge_Intersections() const { return m_bits & MERGE_INTERSECTIONS; }
    int Is_Sorting_Disabled() const { return m_bits & DISABLE_SORTING; }

    void Set_Color(const Vector3 &color) { m_color = color; }
    void Set_Merge_Abort_Factor(float factor) { m_mergeAbortFactor = factor; }
    void Set_Noise_Amplitude(float amplitude) { m_noiseAmplitude = amplitude; }
    void Set_Opacity(float opacity) { m_opacity = opacity; }
    void Set_Shader(ShaderClass shader) { m_shader = shader; }
    void Set_Width(float width) { m_width = width; }

    void Set_Current_Subdivision_Level(unsigned int lv)
    {
        captainslog_dbgassert(lv == 0, "Streak renderer does not work for non-zero subdivisions");
        m_subdivisionLevel = 0;
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

    void Set_Freeze_Random(int onoff)
    {
        if (onoff) {
            m_bits |= FREEZE_RANDOM;
        } else {
            m_bits &= ~FREEZE_RANDOM;
        };
    }

    void Set_Merge_Intersections(int onoff)
    {
        if (onoff) {
            m_bits |= MERGE_INTERSECTIONS;
        } else {
            m_bits &= ~MERGE_INTERSECTIONS;
        };
    }

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

    TextureClass *m_texture;
    ShaderClass m_shader;
    float m_width;
    Vector3 m_color;
    float m_opacity;
    unsigned int m_subdivisionLevel;
    float m_noiseAmplitude;
    float m_mergeAbortFactor;
    unsigned int m_bits;
    unsigned int m_vertexCount;
    VertexFormatXYZUV1 *m_vertexBuffer;
};

inline StreakRendererClass::TextureMapMode StreakRendererClass::Get_Texture_Mapping_Mode() const
{
    return static_cast<TextureMapMode>((m_bits & TEXTURE_MAP_MODE_MASK) >> TEXTURE_MAP_MODE_OFFSET);
}

inline void StreakRendererClass::Set_Texture_Mapping_Mode(StreakRendererClass::TextureMapMode mode)
{
    m_bits &= ~TEXTURE_MAP_MODE_MASK;
    m_bits |= ((mode << TEXTURE_MAP_MODE_OFFSET) & TEXTURE_MAP_MODE_MASK);
}
