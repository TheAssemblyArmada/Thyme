/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief Streak Line
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "streak.h"
#include "coltest.h"
#include "w3d.h"
#include "w3d_file.h"

StreakLineClass::StreakLineClass() : m_maxSubdivisionLevels(0), m_unk(nullptr), m_normalizedScreenArea(0) {}

StreakLineClass::StreakLineClass(const StreakLineClass &src) :
    m_maxSubdivisionLevels(src.m_maxSubdivisionLevels),
    m_unk(src.m_unk),
    m_normalizedScreenArea(src.m_normalizedScreenArea),
    m_pointLocations(src.m_pointLocations),
    m_pointColors(src.m_pointColors),
    m_pointWidths(src.m_pointWidths),
    m_lineRenderer(src.m_lineRenderer),
    m_streakRenderer(src.m_streakRenderer)
{
}

StreakLineClass &StreakLineClass::operator=(const StreakLineClass &that)
{
    RenderObjClass::operator=(that);

    if (this != &that) {
        m_maxSubdivisionLevels = that.m_maxSubdivisionLevels;
        m_normalizedScreenArea = that.m_normalizedScreenArea;
        m_pointLocations = that.m_pointLocations;
        m_pointColors = that.m_pointColors;
        m_pointWidths = that.m_pointWidths;
        m_lineRenderer = that.m_lineRenderer;
        m_streakRenderer = that.m_streakRenderer;
        m_unk = that.m_unk;
    }

    return *this;
}

StreakLineClass::~StreakLineClass() {}

void StreakLineClass::Reset_Line()
{
    m_lineRenderer.Reset_Line();
}

void StreakLineClass::Set_Locs(unsigned int count, Vector3 *locs)
{
    if (count >= 2 && locs != nullptr) {
        m_pointLocations.Delete_All();

        for (unsigned int i = 0; i < count; ++i) {
            m_pointLocations.Add(locs[i], count);
        }

        Invalidate_Cached_Bounding_Volumes();
    } else {
        captainslog_dbgassert(false, "0");
    }
}

void StreakLineClass::Set_Widths(unsigned int count, float *widths)
{
    if (count >= 0 && widths != nullptr) {
        m_pointWidths.Delete_All();

        for (unsigned int i = 0; i < count; ++i) {
            m_pointWidths.Add(widths[i], count);
        }

        Invalidate_Cached_Bounding_Volumes();
    } else {
        captainslog_dbgassert(false, "0");
    }
}

void StreakLineClass::Set_Colors(unsigned int count, Vector4 *colors)
{
    if (count >= 2 && colors != nullptr) {
        m_pointColors.Delete_All();

        for (unsigned int i = 0; i < count; ++i) {
            m_pointColors.Add(colors[i], count);
        }

        Invalidate_Cached_Bounding_Volumes();
    } else {
        captainslog_dbgassert(false, "0");
    }
}

void StreakLineClass::Set_Locs_Widths_Colors(
    unsigned int count, Vector3 *locs, float *widths, Vector4 *colors, unsigned int *unk)
{
    m_unk = unk;

    Set_Locs(count, locs);

    if (widths != nullptr) {
        Set_Widths(count, widths);
        captainslog_assert(m_pointLocations.Count() == m_pointWidths.Count());
    }

    if (colors != nullptr) {
        Set_Colors(count, colors);
        captainslog_assert(m_pointLocations.Count() == m_pointColors.Count());
    }

    Invalidate_Cached_Bounding_Volumes();
}

int StreakLineClass::Get_Num_Points()
{
    return m_pointLocations.Count();
}

void StreakLineClass::Set_Point_Location(unsigned int point_idx, const Vector3 &location)
{
    if (point_idx < static_cast<unsigned int>(m_pointLocations.Count())) {
        m_pointLocations[point_idx] = location;
    }

    Invalidate_Cached_Bounding_Volumes();
}

void StreakLineClass::Get_Point_Location(unsigned int point_idx, Vector3 &loc)
{
    if (point_idx < static_cast<unsigned int>(m_pointLocations.Count())) {
        loc = m_pointLocations[point_idx];
    } else {
        loc = Vector3(0, 0, 0);
    }
}

void StreakLineClass::Add_Point(const Vector3 &location)
{
    m_pointLocations.Add(location);
}

void StreakLineClass::Delete_Point(unsigned int point_idx)
{
    if (point_idx < static_cast<unsigned int>(m_pointLocations.Count())) {
        m_pointLocations.Delete(point_idx);
    }
}

TextureClass *StreakLineClass::Get_Texture()
{
    return m_lineRenderer.Get_Texture();
}

ShaderClass StreakLineClass::Get_Shader() const
{
    return m_lineRenderer.Get_Shader();
}

float StreakLineClass::Get_Width() const
{
    return m_lineRenderer.Get_Width();
}

void StreakLineClass::Get_Color(Vector3 &color) const
{
    color = m_lineRenderer.Get_Color();
}

float StreakLineClass::Get_Opacity() const
{
    return m_lineRenderer.Get_Opacity();
}

float StreakLineClass::Get_Noise_Amplitude() const
{
    return m_lineRenderer.Get_Noise_Amplitude();
}

float StreakLineClass::Get_Merge_Abort_Factor() const
{
    return m_lineRenderer.Get_Merge_Abort_Factor();
}

unsigned StreakLineClass::Get_Subdivision_Levels() const
{
    return m_maxSubdivisionLevels;
}

SegLineRendererClass::TextureMapMode StreakLineClass::Get_Texture_Mapping_Mode() const
{
    return m_lineRenderer.Get_Texture_Mapping_Mode();
}

float StreakLineClass::Get_Texture_Tile_Factor() const
{
    return m_lineRenderer.Get_Texture_Tile_Factor();
}

Vector2 StreakLineClass::Get_UV_Offset_Rate() const
{
    return m_lineRenderer.Get_UV_Offset_Rate();
}

int StreakLineClass::Is_Merge_Intersections() const
{
    return m_lineRenderer.Is_Merge_Intersections();
}

int StreakLineClass::Is_Freeze_Random() const
{
    return m_lineRenderer.Is_Freeze_Random();
}

int StreakLineClass::Is_Sorting_Disabled() const
{
    return m_lineRenderer.Is_Sorting_Disabled();
}

int StreakLineClass::Are_End_Caps_Enabled() const
{
    return m_lineRenderer.Are_End_Caps_Enabled();
}

void StreakLineClass::Set_Texture(TextureClass *texture)
{
    m_lineRenderer.Set_Texture(texture);
    m_streakRenderer.Set_Texture(texture);
}

void StreakLineClass::Set_Shader(ShaderClass shader)
{
    m_lineRenderer.Set_Shader(shader);
    m_streakRenderer.Set_Shader(shader);
}

void StreakLineClass::Set_Width(float width)
{
    if (width > 0.0f) {
        m_lineRenderer.Set_Width(width);
    } else {
        m_lineRenderer.Set_Width(0.0f);
    }

    if (width > 0.0f) {
        m_streakRenderer.Set_Width(width);
    } else {
        m_streakRenderer.Set_Width(0.0f);
    }

    Invalidate_Cached_Bounding_Volumes();
}

void StreakLineClass::Set_Color(const Vector3 &color)
{
    m_lineRenderer.Set_Color(color);
    m_streakRenderer.Set_Color(color);
}

void StreakLineClass::Set_Opacity(float opacity)
{
    m_lineRenderer.Set_Opacity(opacity);
    m_streakRenderer.Set_Opacity(opacity);
}

void StreakLineClass::Set_Noise_Amplitude(float amplitude)
{
    m_lineRenderer.Set_Noise_Amplitude(GameMath::Fabs(amplitude));
    Invalidate_Cached_Bounding_Volumes();
}

void StreakLineClass::Set_Merge_Abort_Factor(float factor)
{
    m_lineRenderer.Set_Merge_Abort_Factor(factor);
}

void StreakLineClass::Set_Subdivision_Levels(unsigned int levels)
{
    m_maxSubdivisionLevels = std::min<unsigned int>(levels, 7);
    Invalidate_Cached_Bounding_Volumes();
}

void StreakLineClass::Set_Texture_Mapping_Mode(SegLineRendererClass::TextureMapMode mode)
{
    m_lineRenderer.Set_Texture_Mapping_Mode(mode);
}

void StreakLineClass::Set_Texture_Tile_Factor(float factor)
{
    m_lineRenderer.Set_Texture_Tile_Factor(factor);
}

void StreakLineClass::Set_UV_Offset_Rate(const Vector2 &rate)
{
    m_lineRenderer.Set_UV_Offset_Rate(rate);
}

void StreakLineClass::Set_Merge_Intersections(int onoff)
{
    m_lineRenderer.Set_Merge_Intersections(onoff);
}

void StreakLineClass::Set_Freeze_Random(int onoff)
{
    m_lineRenderer.Set_Freeze_Random(onoff);
}

void StreakLineClass::Set_Disable_Sorting(int onoff)
{
    m_lineRenderer.Set_Disable_Sorting(onoff);
}

void StreakLineClass::Set_End_Caps(int onoff)
{
    m_lineRenderer.Set_End_Caps(onoff);
}

RenderObjClass *StreakLineClass::Clone() const
{
    return new StreakLineClass(*this);
}

int StreakLineClass::Get_Num_Polys() const
{
    return 2 * (m_pointLocations.Count() - 1) * (2 * m_lineRenderer.Get_Current_Subdivision_Level());
}

void StreakLineClass::Render(RenderInfoClass &rinfo)
{
    if (Is_Not_Hidden_At_All()) {
        int sort = 0;

        if (!W3D::Is_Sorting_Enabled()) {
            sort = Get_Shader().Guess_Sort_Level();
        }

        if (W3D::Are_Static_Sort_Lists_Enabled() && sort) {
            W3D::Add_To_Static_Sort_List(this, sort);
        } else if (m_pointColors.Count() && m_pointWidths.Count()) {
            Render_Streak_Line(rinfo);
        } else {
            Render_Seg_Line(rinfo);
        }
    }
}

void StreakLineClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    AABoxClass aabox;
    Get_Obj_Space_Bounding_Box(aabox);
    sphere.Init(aabox.m_center, aabox.m_extent.Length());
}

void StreakLineClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    int count = m_pointLocations.Count();
    if (count >= 2) {
        Vector3 max_pos = m_pointLocations[0];
        Vector3 min_pos = m_pointLocations[0];

        for (int i = 1; i < count; ++i) {
            max_pos.Update_Max(m_pointLocations[i]);
            min_pos.Update_Min(m_pointLocations[i]);
        }

        float bounds_width = m_lineRenderer.Get_Width() * 0.5f;

        Vector3 bounds(bounds_width, bounds_width, bounds_width);
        max_pos += bounds;
        min_pos -= bounds;

        if (m_maxSubdivisionLevels) {
            Vector3 midpoint = (m_pointLocations[0] + m_pointLocations[1]) * 0.5f;
            Vector3 max_pos_2 = midpoint;
            Vector3 min_pos_2 = midpoint;

            for (int i = 1; i < count - 1; ++i) {
                midpoint = (m_pointLocations[i] + m_pointLocations[i + 1]) * 0.5f;
                max_pos_2.Update_Max(midpoint);
                min_pos_2.Update_Min(midpoint);
            }

            float amplitude = m_lineRenderer.Get_Noise_Amplitude();
            bounds_width += amplitude + amplitude;
            bounds.Set(bounds_width, bounds_width, bounds_width);
            max_pos_2 += bounds;
            min_pos_2 -= bounds;

            max_pos.Update_Max(max_pos_2);
            min_pos.Update_Min(min_pos_2);
        }

        box.Init_Min_Max(min_pos, max_pos);

    } else {
        box.Init(Vector3(0, 0, 0), Vector3(1, 1, 1));
    }
}

void StreakLineClass::Prepare_LOD(CameraClass &camera)
{
    m_normalizedScreenArea = Get_Screen_Size(camera);
    unsigned int level = m_lineRenderer.Get_Current_Subdivision_Level();

    if (level >= static_cast<unsigned int>(m_maxSubdivisionLevels)) {
        level = m_maxSubdivisionLevels;
    }

    m_lineRenderer.Set_Current_Subdivision_Level(level);
}

void StreakLineClass::Increment_LOD()
{
    unsigned int level = m_lineRenderer.Get_Current_Subdivision_Level();

    level = std::min(level + 1, m_maxSubdivisionLevels);

    m_lineRenderer.Set_Current_Subdivision_Level(level);
}

void StreakLineClass::Decrement_LOD()
{
    unsigned int level = m_lineRenderer.Get_Current_Subdivision_Level();
    if (level != 0) {
        m_lineRenderer.Set_Current_Subdivision_Level(level - 1);
    }
}

float StreakLineClass::Get_Cost() const
{
    return (float)Get_Num_Polys();
}

float StreakLineClass::Get_Value() const
{
    if (m_lineRenderer.Get_Current_Subdivision_Level() == 0) {
        return AT_MIN_LOD;
    }

    float poly = (float)Get_Num_Polys();
    float p = (1.0f - 0.5f / (poly * poly)) * m_normalizedScreenArea;
    return p / Get_Cost();
}

float StreakLineClass::Get_Post_Increment_Value() const
{
    if (m_lineRenderer.Get_Current_Subdivision_Level() == m_maxSubdivisionLevels) {
        return AT_MAX_LOD;
    }

    float poly = (float)Get_Num_Polys();
    float poly2 = poly + poly;
    float p = (1.0f - 0.5f / (poly2 * poly2));
    return p * m_normalizedScreenArea / poly2;
}

void StreakLineClass::Set_LOD_Level(int lod)
{
    lod = lod < 0 ? 0 : lod;
    if (lod >= (int)m_maxSubdivisionLevels) {
        lod = (int)m_maxSubdivisionLevels;
    }
    m_lineRenderer.Set_Current_Subdivision_Level(lod);
}

int StreakLineClass::Get_LOD_Level() const
{
    return m_lineRenderer.Get_Current_Subdivision_Level();
}

int StreakLineClass::Get_LOD_Count() const
{
    return m_maxSubdivisionLevels;
}

bool StreakLineClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    if (Get_Collision_Type() & raytest.m_collisionType) {

        bool collides = false;
        float fraction = 1.0f;

        for (int i = 1; i < m_pointLocations.Count(); ++i) {
            Vector3 curr[2];
            Matrix3D::Transform_Vector(m_transform, m_pointLocations[i - 1], &curr[0]);
            Matrix3D::Transform_Vector(m_transform, m_pointLocations[i], &curr[1]);
            LineSegClass line_seg(curr[0], curr[1]);
            Vector3 p1;
            Vector3 p2;
            if (raytest.m_ray.Find_Intersection(line_seg, &p1, &fraction, &p2, nullptr)) {
                if (m_lineRenderer.Get_Width() >= (p1 - p2).Length() && fraction >= 0.0f
                    && fraction < raytest.m_result->fraction) {
                    collides = true;
                    break;
                }
            }
        }
        if (collides) {
            raytest.m_result->fraction = fraction;
            raytest.m_result->surface_type = SURFACE_TYPE_DEFAULT;
            raytest.m_collidedRenderObj = this;
            return true;
        }
    }

    return false;
}

void StreakLineClass::Render_Seg_Line(RenderInfoClass &rinfo)
{
    if (m_pointLocations.Count() >= 2) {
        SphereClass obj_sphere;
        Get_Obj_Space_Bounding_Sphere(obj_sphere);
        m_lineRenderer.Render(rinfo, m_transform, m_pointLocations.Count(), &m_pointLocations[0], obj_sphere, nullptr);
    }
}

void StreakLineClass::Render_Streak_Line(RenderInfoClass &rinfo)
{
    captainslog_assert(m_pointLocations.Count() == m_pointColors.Count());
    captainslog_assert(m_pointLocations.Count() == m_pointWidths.Count());

    if (m_pointLocations.Count() >= 2 && m_pointColors.Count() >= 2 && m_pointWidths.Count() >= 2) {
        if (m_pointLocations.Count() == m_pointColors.Count() && m_pointLocations.Count() == m_pointWidths.Count()) {
            SphereClass obj_sphere;
            Get_Obj_Space_Bounding_Sphere(obj_sphere);
            m_streakRenderer.Render_Streak(rinfo,
                m_transform,
                m_pointLocations.Count(),
                &m_pointLocations[0],
                &m_pointColors[0],
                &m_pointWidths[0],
                obj_sphere,
                m_unk);
        }
    }
}
