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
    float oo_intmax = 1.0f / (float)INT_MAX;
    Vector3SolidBoxRandomizer randomizer(Vector3(1, 1, 1));
    Vector3 randvec(0, 0, 0);
    StreakLineSubdivision stack[MAX_STREAK_SUBDIV_LEVELS * 2];
    unsigned int output_index = 0;

    for (unsigned int input_index = 0; input_index < point_cnt - 1; input_index++) {
        int tos = 0;
        stack[0].start_pos = xformed_pts[input_index];
        stack[0].end_pos = xformed_pts[input_index + 1];
        stack[0].start_texv = base_tex_v[input_index];
        stack[0].end_texv = base_tex_v[input_index + 1];

        stack[0].rand = m_noiseAmplitude;
        stack[0].level = 0;

        while (tos >= 0) {
            if (stack[tos].level == m_subdivisionLevel) {
                xformed_subdiv_pts[output_index] = stack[tos].start_pos;
                subdiv_tex_v[output_index] = stack[tos].start_texv;
                output_index++;
                tos--;
            } else {
                if (freeze_random) {
                    randvec.Set(randomize * oo_intmax, randomize * oo_intmax, randomize * oo_intmax);
                } else {
                    randomizer.Get_Vector(randvec);
                }

                stack[tos + 1].start_pos = stack[tos].start_pos;
                stack[tos + 1].end_pos = (stack[tos].start_pos + stack[tos].end_pos) * .5f + stack[tos].rand * randvec;
                stack[tos + 1].start_texv = stack[tos].start_texv;
                stack[tos + 1].end_texv = (stack[tos].start_texv + stack[tos].end_texv) * .5f;
                stack[tos + 1].rand = stack[tos].rand * .5f;
                stack[tos + 1].level = stack[tos].level + 1;
                stack[tos].start_pos = stack[tos + 1].end_pos;
                stack[tos].start_texv = stack[tos + 1].end_texv;
                stack[tos].rand = stack[tos + 1].rand;
                stack[tos].level = stack[tos + 1].level;
                tos++;
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
        unsigned int growth = count + (count >> 1);
        m_vertexCount = growth;
        m_vertexBuffer = new VertexFormatXYZUV1[growth];
    }

    for (unsigned int i = 0; i < m_vertexCount; ++i) {
        m_vertexBuffer[i].x = 3735928600.0f;
        m_vertexBuffer[i].y = 3735928600.0f;
        m_vertexBuffer[i].z = 3735928600.0f;
        m_vertexBuffer[i].u1 = 3735928600.0f;
        m_vertexBuffer[i].v1 = 3735928600.0f;
    }

    return m_vertexBuffer;
}

void StreakRendererClass::Render(RenderInfoClass &rinfo,
    const Matrix3D &transform,
    unsigned int point_count,
    Vector3 *points,
    const SphereClass &obj_sphere)
{
}

void StreakRendererClass::Render_Streak(RenderInfoClass &rinfo,
    Matrix3D const &transform,
    unsigned int point_count,
    Vector3 *points,
    Vector4 *colors,
    float *widths,
    SphereClass const &obj_sphere,
    unsigned int *unk)
{
#ifdef GAME_DLL
    Call_Method<void,
        StreakRendererClass,
        RenderInfoClass &,
        const Matrix3D &,
        unsigned int,
        Vector3 *,
        Vector4 *,
        float *,
        const SphereClass &,
        unsigned int *>(
        PICK_ADDRESS(0x00889F20, 0x00BB6150), this, rinfo, transform, point_count, points, colors, widths, obj_sphere, unk);
#else
#ifdef BUILD_WITH_D3D8
    struct LineSegmentIntersection
    {
        int point_count;
        int next_segment_id;
        Vector3 direction;
        Vector3 point;
        float texv;
        bool fold;
        bool parallel;
    };

    struct LineSegment
    {
        Vector3 start_plane;
        Vector3 edge_plane[2];
    };

    Matrix4 view;
    DX8Wrapper::Get_Transform(D3DTS_VIEW, view);
    Matrix4 m(true);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
    DX8Wrapper::Set_Transform(D3DTS_VIEW, m);

    unsigned int chunk_size = (1 << MAX_STREAK_SUBDIV_LEVELS >> m_subdivisionLevel) + 1;

    if (chunk_size > point_count) {
        chunk_size = point_count;
    }

    for (unsigned int start_point = 0; start_point < point_count - 1; start_point += chunk_size - 1) {
        unsigned int point_cnt = std::min(chunk_size, point_count - start_point);

        Matrix3D modelview(view[0].X,
            view[0].Y,
            view[0].Z,
            view[0].W,
            view[1].X,
            view[1].Y,
            view[1].Z,
            view[1].W,
            view[2].X,
            view[2].Y,
            view[2].Z,
            view[2].W);

        Vector3 xformed_pts[129];
        float base_tex_v[129];

        VectorProcessorClass::Transform(xformed_pts, &points[start_point], modelview * transform, point_cnt);

        unsigned int sub_point_cnt;
        Vector3 xformed_subdiv_pts[129];
        float subdiv_tex_v[129];

        Subdivision_Util(point_cnt, xformed_pts, base_tex_v, &sub_point_cnt, xformed_subdiv_pts, subdiv_tex_v);

        bool switch_edges = false;
        LineSegment segment[130];
        float radius = m_width * 0.5f;
        LineSegmentIntersection intersection[130][2];

        for (unsigned int sub_point = 1; sub_point < sub_point_cnt; sub_point++) {
            radius = widths[sub_point];
            Vector3 prev_point = xformed_subdiv_pts[sub_point - 1];
            Vector3 curr_point = xformed_subdiv_pts[sub_point];

            if (Equal_Within_Epsilon(xformed_subdiv_pts[sub_point - 1], xformed_subdiv_pts[sub_point], GAMEMATH_EPSILON)) {
                curr_point.X = curr_point.X + 0.001f;
            }

            segment[sub_point].start_plane = Normalize(curr_point - prev_point);
            Vector3 offset = Normalize(Vector3::Cross_Product(segment[sub_point].start_plane,
                prev_point
                    + segment[sub_point].start_plane * -Vector3::Dot_Product(segment[sub_point].start_plane, prev_point)));
            Vector3 top_normal =
                Normalize(Vector3::Cross_Product(prev_point + radius * offset, segment[sub_point].start_plane));
            Vector3 bottom_normal =
                Normalize(Vector3::Cross_Product(segment[sub_point].start_plane, prev_point - radius * offset));
            segment[sub_point].edge_plane[0] = top_normal;
            segment[sub_point].edge_plane[1] = bottom_normal;

            if (sub_point > 1) {
                Vector3 cross1 = Normalize(Vector3::Cross_Product(xformed_subdiv_pts[sub_point - 2], prev_point));
                Vector3 cross2 = Normalize(Vector3::Cross_Product(prev_point, curr_point));

                if (Vector3::Dot_Product(cross1, cross2) >= 0.0f) {
                    intersection[sub_point][0].fold = false;
                    intersection[sub_point][1].fold = false;
                } else {
                    switch_edges = !switch_edges;
                    intersection[sub_point][0].fold = true;
                    intersection[sub_point][1].fold = true;
                }
            }

            if (switch_edges) {
                segment[sub_point].edge_plane[0] = -bottom_normal;
                segment[sub_point].edge_plane[1] = -top_normal;
            }
        }

        enum SegmentEdge
        {
            FIRST_EDGE = 0,
            TOP_EDGE = 0,
            BOTTOM_EDGE = 1,
            MAX_EDGE = 1,
            NUM_EDGES = 2,
        };
        unsigned int num_intersections[NUM_EDGES];
        num_intersections[TOP_EDGE] = sub_point_cnt;
        num_intersections[BOTTOM_EDGE] = sub_point_cnt;

        intersection[0][0].point_count = 0;
        intersection[0][0].next_segment_id = 0;
        intersection[0][0].direction = Vector3(1.0f, 0.0f, 0.0f);
        intersection[0][0].point = Vector3(0.0f, 0.0f, 0.0f);
        intersection[0][0].texv = 0.0f;
        intersection[0][0].fold = true;
        intersection[0][0].parallel = false;

        intersection[0][1].point_count = 0;
        intersection[0][1].next_segment_id = 0;
        intersection[0][1].direction = Vector3(1.0f, 0.0f, 0.0f);
        intersection[0][1].point = Vector3(0.0f, 0.0f, 0.0f);
        intersection[0][1].texv = 0.0f;
        intersection[0][1].fold = true;
        intersection[0][1].parallel = false;

        intersection[1][0].point_count = 1;
        intersection[1][0].next_segment_id = 1;
        intersection[1][0].point = xformed_subdiv_pts[0];
        intersection[1][0].texv = subdiv_tex_v[0];
        intersection[1][0].fold = true;
        intersection[1][0].parallel = false;

        intersection[1][1].point_count = 1;
        intersection[1][1].next_segment_id = 1;
        intersection[1][1].point = xformed_subdiv_pts[0];
        intersection[1][1].texv = subdiv_tex_v[0];
        intersection[1][1].fold = true;
        intersection[1][1].parallel = false;

        Vector3 top = Normalize(xformed_subdiv_pts[0]
            - Vector3::Dot_Product(segment[1].edge_plane[0], xformed_subdiv_pts[0]) * segment[1].edge_plane[0]);
        intersection[1][0].direction = top;

        Vector3 bottom = Normalize(xformed_subdiv_pts[0]
            - Vector3::Dot_Product(segment[1].edge_plane[1], xformed_subdiv_pts[0]) * segment[1].edge_plane[1]);
        intersection[1][1].direction = bottom;

        Vector3 segdir = Normalize(xformed_subdiv_pts[1] - xformed_subdiv_pts[0]);
        Vector3 start_pl = Normalize(Vector3::Cross_Product(top, bottom));

        if (Vector3::Dot_Product(segdir, start_pl) <= 0.0f) {
            start_pl = -start_pl;
        }

        segment[0].start_plane = start_pl;
        segment[0].edge_plane[0] = start_pl;
        segment[0].edge_plane[1] = start_pl;
        segment[1].start_plane = start_pl;

        intersection[num_intersections[0]][0].point_count = 1;
        intersection[num_intersections[0]][0].next_segment_id = sub_point_cnt;
        intersection[num_intersections[0]][0].point = xformed_subdiv_pts[sub_point_cnt - 1];
        intersection[num_intersections[0]][0].texv = subdiv_tex_v[sub_point_cnt - 1];
        intersection[num_intersections[0]][0].fold = true;
        intersection[num_intersections[0]][0].parallel = false;

        intersection[num_intersections[0]][1].point_count = 1;
        intersection[num_intersections[0]][1].next_segment_id = sub_point_cnt;
        intersection[num_intersections[0]][1].point = xformed_subdiv_pts[sub_point_cnt - 1];
        intersection[num_intersections[0]][1].texv = subdiv_tex_v[sub_point_cnt - 1];
        intersection[num_intersections[0]][1].fold = true;
        intersection[num_intersections[0]][1].parallel = false;

        top = Normalize(xformed_subdiv_pts[sub_point_cnt - 1]
            - Vector3::Dot_Product(segment[sub_point_cnt - 1].edge_plane[0], xformed_subdiv_pts[sub_point_cnt - 1])
                * segment[sub_point_cnt - 1].edge_plane[0]);
        intersection[num_intersections[0]][0].direction = top;

        bottom = Normalize(xformed_subdiv_pts[sub_point_cnt - 1]
            - Vector3::Dot_Product(segment[sub_point_cnt - 1].edge_plane[1], xformed_subdiv_pts[sub_point_cnt - 1])
                * segment[sub_point_cnt - 1].edge_plane[1]);
        intersection[num_intersections[0]][1].direction = bottom;

        segdir = Normalize(xformed_subdiv_pts[sub_point_cnt - 1] - xformed_subdiv_pts[sub_point_cnt - 2]);
        start_pl = Normalize(
            Vector3::Cross_Product(intersection[sub_point_cnt][0].direction, intersection[sub_point_cnt][1].direction));

        if (Vector3::Dot_Product(segdir, start_pl) <= 0.0f) {
            start_pl = -start_pl;
        }

        segment[sub_point_cnt].start_plane = start_pl;
        segment[sub_point_cnt].edge_plane[0] = start_pl;
        segment[sub_point_cnt].edge_plane[1] = start_pl;

        for (unsigned int sub_point = 2; sub_point < sub_point_cnt; sub_point++) {
            intersection[sub_point][0].point_count = 1;
            intersection[sub_point][0].next_segment_id = sub_point;
            intersection[sub_point][0].point = xformed_subdiv_pts[sub_point - 1];
            intersection[sub_point][0].texv = subdiv_tex_v[sub_point - 1];
            intersection[sub_point][1].point_count = 1;
            intersection[sub_point][1].next_segment_id = sub_point;
            intersection[sub_point][1].point = xformed_subdiv_pts[sub_point - 1];
            intersection[sub_point][1].texv = subdiv_tex_v[sub_point - 1];

            float dot_top = Vector3::Dot_Product(segment[sub_point - 1].edge_plane[0], segment[sub_point].edge_plane[0]);

            if (GameMath::Fabs(dot_top) >= 0.9f) {
                Vector3 average_offset;

                if (dot_top <= 0.0f) {
                    average_offset = Normalize(segment[sub_point - 1].edge_plane[0] - segment[sub_point].edge_plane[0]);
                } else {
                    average_offset = Normalize(segment[sub_point - 1].edge_plane[0] + segment[sub_point].edge_plane[0]);
                }

                intersection[sub_point][0].direction = Normalize(xformed_subdiv_pts[sub_point - 1]
                    - Vector3::Dot_Product(average_offset, xformed_subdiv_pts[sub_point - 1]) * average_offset);
                intersection[sub_point][0].parallel = true;
            } else {
                intersection[sub_point][0].direction = Normalize(
                    Vector3::Cross_Product(segment[sub_point - 1].edge_plane[0], segment[sub_point].edge_plane[0]));

                if (Vector3::Dot_Product(intersection[sub_point][0].direction, xformed_subdiv_pts[sub_point - 1]) < 0.0f) {
                    intersection[sub_point][0].direction = -intersection[sub_point][0].direction;
                }

                intersection[sub_point][0].parallel = false;
            }

            float dot_bottom = Vector3::Dot_Product(segment[sub_point - 1].edge_plane[1], segment[sub_point].edge_plane[1]);

            if (GameMath::Fabs(dot_bottom) >= 0.9f) {
                Vector3 average_offset;

                if (dot_bottom <= 0.0f) {
                    average_offset = Normalize(segment[sub_point - 1].edge_plane[1] - segment[sub_point].edge_plane[1]);
                } else {
                    average_offset = Normalize(segment[sub_point - 1].edge_plane[1] + segment[sub_point].edge_plane[1]);
                }

                intersection[sub_point][1].direction = Normalize(xformed_subdiv_pts[sub_point - 1]
                    - Vector3::Dot_Product(average_offset, xformed_subdiv_pts[sub_point - 1]) * average_offset);
                intersection[sub_point][1].parallel = true;
            } else {
                intersection[sub_point][1].direction = Normalize(
                    Vector3::Cross_Product(segment[sub_point - 1].edge_plane[1], segment[sub_point].edge_plane[1]));

                if (Vector3::Dot_Product(intersection[sub_point][1].direction, xformed_subdiv_pts[sub_point - 1]) < 0.0f) {
                    intersection[sub_point][1].direction = -intersection[sub_point][1].direction;
                }

                intersection[sub_point][1].parallel = false;
            }

            start_pl = Normalize(
                Vector3::Cross_Product(intersection[sub_point][0].direction, intersection[sub_point][1].direction));

            if (Vector3::Dot_Product(segment[sub_point].start_plane, start_pl) <= 0.0f) {
                segment[sub_point].start_plane = -start_pl;
            } else {
                segment[sub_point].start_plane = start_pl;
            }
        }

        if (Is_Merge_Intersections()) {
            bool merged = true;

            while (merged) {
                merged = false;

                for (unsigned int edge_type = 0; edge_type < NUM_EDGES; edge_type++) {
                    unsigned int num_isects = num_intersections[edge_type];
                    unsigned int iidx_r = 1;
                    unsigned int iidx_w = 1;

                    while (iidx_r < num_isects) {
                        LineSegmentIntersection &edge1 = intersection[iidx_r][edge_type];
                        LineSegmentIntersection &edge2 = intersection[iidx_r + 1][edge_type];
                        LineSegmentIntersection &edge3 = intersection[iidx_w][edge_type];
                        LineSegmentIntersection &edge4 = intersection[iidx_w - 1][edge_type];
                        LineSegment &vertex2 = segment[edge2.next_segment_id];
                        LineSegment &vertex1 = segment[edge1.next_segment_id];
                        LineSegment &vertex4 = segment[edge4.next_segment_id];

                        for (;;) {
                            if (edge2.fold || Vector3::Dot_Product(edge1.direction, vertex2.start_plane) <= 0.0f
                                || Vector3::Dot_Product(edge1.direction, vertex2.edge_plane[edge_type]) <= 0.0f) {
                                if (edge1.fold || Vector3::Dot_Product(edge2.direction, -vertex1.start_plane) <= 0.0f
                                    || Vector3::Dot_Product(edge2.direction, vertex4.edge_plane[edge_type]) <= 0.0f) {
                                    break;
                                }
                            }

                            unsigned int new_count = edge2.point_count + edge1.point_count;
                            float curr_factor = 1.0f / new_count * edge1.point_count;
                            Vector3 new_point = (curr_factor * edge2.point) + (curr_factor * edge1.point);
                            float new_tex_v = curr_factor * edge1.texv + curr_factor * edge2.texv;
                            Vector3 new_direction;
                            bool new_parallel;
                            float dot_top =
                                Vector3::Dot_Product(vertex4.edge_plane[edge_type], vertex2.edge_plane[edge_type]);

                            if (GameMath::Fabs(dot_top) >= 0.9f) {
                                Vector3 pl;

                                if (dot_top <= 0.0f) {
                                    pl = Normalize(vertex4.edge_plane[edge_type] - vertex2.edge_plane[edge_type]);
                                } else {
                                    pl = Normalize(vertex4.edge_plane[edge_type] + vertex2.edge_plane[edge_type]);
                                }

                                if (edge1.parallel) {
                                    new_direction =
                                        Normalize(edge1.direction - Vector3::Dot_Product(pl, edge1.direction) * pl);
                                } else {
                                    new_direction = Normalize(Vector3::Cross_Product(vertex1.edge_plane[edge_type], pl));
                                }

                                new_parallel = true;
                            } else {
                                new_direction = Normalize(
                                    Vector3::Cross_Product(vertex4.edge_plane[edge_type], vertex2.edge_plane[edge_type]));

                                if (Vector3::Dot_Product(new_direction, new_point) < 0.0f) {
                                    new_direction = -new_direction;
                                }

                                new_parallel = false;
                            }

                            if (m_mergeAbortFactor > 0.0f) {
                                float threshold = (m_mergeAbortFactor * radius) * (m_mergeAbortFactor * radius);

                                if ((edge1.point - Vector3::Dot_Product(edge1.point, new_direction) * new_direction)
                                            .Length2()
                                        > threshold
                                    || (edge2.point - Vector3::Dot_Product(edge2.point, new_direction) * new_direction)
                                            .Length2()
                                        > threshold) {
                                    break;
                                }
                            }

                            merged = true;
                            edge1.direction = new_direction;
                            edge1.parallel = new_parallel;
                            edge1.point = new_point;
                            edge1.texv = new_tex_v;
                            edge1.point_count = new_count;
                            edge1.next_segment_id = edge2.next_segment_id;
                            edge1.fold = (edge1.fold || edge2.fold);
                            num_intersections[edge_type]--;
                            iidx_r++;

                            if (iidx_r == num_isects) {
                                break;
                            }

                            edge2 = intersection[iidx_r + 1][edge_type];
                            vertex2 = segment[edge2.next_segment_id];
                        }

                        edge3.point_count = edge1.point_count;
                        edge3.next_segment_id = edge1.next_segment_id;
                        edge3.point = edge1.point;
                        edge3.texv = edge1.texv;
                        edge3.direction = edge1.direction;
                        edge3.fold = edge1.fold;
                        iidx_r++;
                        iidx_w++;
                    }

                    if (iidx_r == num_isects) {
                        LineSegmentIntersection &intersection_w = intersection[iidx_w][edge_type];
                        LineSegmentIntersection &intersection_r = intersection[iidx_r][edge_type];
                        intersection_w.point_count = intersection_r.point_count;
                        intersection_w.next_segment_id = intersection_r.next_segment_id;
                        intersection_w.point = intersection_r.point;
                        intersection_w.texv = intersection_r.texv;
                        intersection_w.direction = intersection_r.direction;
                        intersection_w.fold = intersection_r.fold;
                    }
                }
            }
        }

        unsigned int vnum = num_intersections[BOTTOM_EDGE] + num_intersections[TOP_EDGE];
        VertexFormatXYZUV1 *v_vertex_array = Get_Vertex_Buffer(vnum);
        Vector3i v_index_array[256];
        unsigned int vertex_count = 0;
        unsigned int tri_count = 0;

        top = Vector3::Dot_Product(xformed_subdiv_pts[0], intersection[1][0].direction) * intersection[1][0].direction;
        bottom = Vector3::Dot_Product(xformed_subdiv_pts[0], intersection[1][1].direction) * intersection[1][1].direction;

        v_vertex_array[vertex_count].x = top.X;
        v_vertex_array[vertex_count].y = top.Y;
        v_vertex_array[vertex_count].z = top.Z;
        v_vertex_array[vertex_count].u1 = 0.0f;
        v_vertex_array[vertex_count].v1 = intersection[1][0].texv;
        vertex_count++;

        v_vertex_array[vertex_count].x = bottom.X;
        v_vertex_array[vertex_count].y = bottom.Y;
        v_vertex_array[vertex_count].z = bottom.Z;
        v_vertex_array[vertex_count].u1 = 1.0f;
        v_vertex_array[vertex_count].v1 = intersection[1][1].texv;
        vertex_count++;
    }
#endif
#endif
}
