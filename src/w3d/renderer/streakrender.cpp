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
#include "streakrender.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "random.h"
#include "sortingrenderer.h"
#include "v3_rnd.h"
#include "vector3i.h"
#include "vp.h"
#include "w3d.h"
#include <algorithm>

#define MAX_STREAK_SUBDIV_LEVELS 7

StreakRendererClass::StreakRendererClass() :
    m_texture(nullptr),
    m_shader(ShaderClass::s_presetAdditiveSpriteShader),
    m_width(0),
    m_color(1, 1, 1),
    m_opacity(1),
    m_subdivisionLevel(0),
    m_noiseAmplitude(0),
    m_mergeAbortFactor(1.5f),
    m_bits(DEFAULT_BITS),
    m_vertexCount(0),
    m_vertexBuffer(nullptr)
{
}

StreakRendererClass::StreakRendererClass(const StreakRendererClass &that) :
    m_texture(nullptr),
    m_shader(ShaderClass::s_presetAdditiveSpriteShader),
    m_width(0),
    m_color(1, 1, 1),
    m_opacity(1),
    m_subdivisionLevel(0),
    m_noiseAmplitude(0),
    m_mergeAbortFactor(1.5f),
    m_bits(DEFAULT_BITS),
    m_vertexCount(0),
    m_vertexBuffer(nullptr)
{
    *this = that;
}

StreakRendererClass &StreakRendererClass::operator=(const StreakRendererClass &that)
{
    if (this != &that) {
        TextureClass *tex = const_cast<TextureClass *>(that.m_texture);
        Ref_Ptr_Set(m_texture, tex);
        m_shader = that.m_shader;
        m_width = that.m_width;
        m_color = that.m_color;
        m_opacity = that.m_opacity;
        m_subdivisionLevel = that.m_subdivisionLevel;
        m_noiseAmplitude = that.m_noiseAmplitude;
        m_mergeAbortFactor = that.m_mergeAbortFactor;
        m_bits = that.m_bits;
    }
    return *this;
}

StreakRendererClass::~StreakRendererClass()
{
    Ref_Ptr_Release(m_texture);
    delete[] m_vertexBuffer;
}

void StreakRendererClass::Init(const W3dEmitterLinePropertiesStruct &props)
{
    Set_Merge_Intersections(props.Flags & W3D_ELINE_MERGE_INTERSECTIONS);
    Set_Freeze_Random(props.Flags & W3D_ELINE_FREEZE_RANDOM);
    Set_Disable_Sorting(props.Flags & W3D_ELINE_DISABLE_SORTING);
    Set_End_Caps(props.Flags & W3D_ELINE_END_CAPS);

    switch ((props.Flags & W3D_ELINE_TEXTURE_MAP_MODE_MASK) >> W3D_ELINE_TEXTURE_MAP_MODE_OFFSET) {
        case W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP:
            Set_Texture_Mapping_Mode(UNIFORM_WIDTH_TEXTURE_MAP);
            break;
        case W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP:
            Set_Texture_Mapping_Mode(UNIFORM_LENGTH_TEXTURE_MAP);
            break;
        case W3D_ELINE_TILED_TEXTURE_MAP:
            Set_Texture_Mapping_Mode(TILED_TEXTURE_MAP);
            break;
    }

    Set_Current_Subdivision_Level(props.SubdivisionLevel);
    Set_Noise_Amplitude(props.NoiseAmplitude);
    Set_Merge_Abort_Factor(props.MergeAbortFactor);
}

void StreakRendererClass::Set_Texture(TextureClass *texture)
{
    Ref_Ptr_Set(m_texture, texture);
}

TextureClass *StreakRendererClass::Get_Texture() const
{
    if (m_texture) {
        m_texture->Add_Ref();
    }
    return m_texture;
}

void StreakRendererClass::Subdivision_Util(unsigned int point_cnt,
    const Vector3 *xformed_pts,
    const float *base_tex_v,
    unsigned int *p_sub_point_cnt,
    Vector3 *xformed_subdiv_pts,
    float *subdiv_tex_v)
{
    struct StreakLineSubdivision
    {
        Vector3 start_pos;
        Vector3 end_pos;
        float start_texv;
        float end_texv;
        float rand;
        unsigned int level;
    };

    int freeze_random = Is_Freeze_Random();
    Random3Class randomize;
    Vector3SolidBoxRandomizer randomizer(Vector3(1, 1, 1));
    Vector3 randvec(0, 0, 0);
    StreakLineSubdivision stack[MAX_STREAK_SUBDIV_LEVELS * 2];
    unsigned int output_index = 0;

    for (unsigned int input_index = 0; input_index < point_cnt - 1; input_index++) {
        stack[0].start_pos = xformed_pts[input_index];
        stack[0].end_pos = xformed_pts[input_index + 1];
        stack[0].start_texv = base_tex_v[input_index];
        stack[0].end_texv = base_tex_v[input_index + 1];
        stack[0].rand = m_noiseAmplitude;
        stack[0].level = 0;

        for (int i = 0; i >= 0;) {
            if (stack[i].level == m_subdivisionLevel) {
                xformed_subdiv_pts[output_index] = stack[i].start_pos;
                subdiv_tex_v[output_index] = stack[i].start_texv;
                output_index++;
                i--;
            } else {
                if (freeze_random) {
                    randvec.Set(4.6566129e-10f * randomize(), 4.6566129e-10f * randomize(), 4.6566129e-10f * randomize());
                } else {
                    randomizer.Get_Vector(randvec);
                }

                stack[i + 1].start_pos = stack[i].start_pos;
                stack[i + 1].end_pos = (stack[i].start_pos + stack[i].end_pos) * .5f + stack[i].rand * randvec;
                stack[i + 1].start_texv = stack[i].start_texv;
                stack[i + 1].end_texv = (stack[i].start_texv + stack[i].end_texv) * .5f;
                stack[i + 1].rand = stack[i].rand * .5f;
                stack[i + 1].level = stack[i].level + 1;
                stack[i].start_pos = stack[i + 1].end_pos;
                stack[i].start_texv = stack[i + 1].end_texv;
                stack[i].rand = stack[i + 1].rand;
                stack[i].level = stack[i + 1].level;
                i++;
            }
        }
    }
    xformed_subdiv_pts[output_index] = xformed_pts[point_cnt - 1];
    subdiv_tex_v[output_index] = base_tex_v[point_cnt - 1];
    *p_sub_point_cnt = output_index + 1;
}

VertexFormatXYZUV1 *StreakRendererClass::Get_Vertex_Buffer(unsigned int count)
{
    if (count > m_vertexCount) {
        delete[] m_vertexBuffer;
        int c = count + (count >> 1);
        m_vertexCount = c;
        m_vertexBuffer = new VertexFormatXYZUV1[c];
    }

    for (int i = 0; i < m_vertexCount; ++i) {
        m_vertexBuffer[i].x = (float)0xdeadbeef;
        m_vertexBuffer[i].y = (float)0xdeadbeef;
        m_vertexBuffer[i].z = (float)0xdeadbeef;
        m_vertexBuffer[i].u1 = (float)0xdeadbeef;
        m_vertexBuffer[i].v1 = (float)0xdeadbeef;
    }

    return m_vertexBuffer;
}
