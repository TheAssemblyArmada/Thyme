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
#include "segline.h"

#include "coltest.h"
#include "w3d.h"
#include "w3d_file.h"

SegmentedLineClass::SegmentedLineClass() : m_maxSubdivisionLevels(0), m_normalizedScreenArea(0) {}

SegmentedLineClass::SegmentedLineClass(const SegmentedLineClass &src) :
    m_maxSubdivisionLevels(src.m_maxSubdivisionLevels),
    m_normalizedScreenArea(src.m_normalizedScreenArea),
    m_pointLocations(src.m_pointLocations),
    m_lineRenderer(src.m_lineRenderer)
{
}

SegmentedLineClass &SegmentedLineClass::operator=(const SegmentedLineClass &that)
{
    RenderObjClass::operator=(that);

    if (this != &that) {
        m_maxSubdivisionLevels = that.m_maxSubdivisionLevels;
        m_normalizedScreenArea = that.m_normalizedScreenArea;
        m_pointLocations = that.m_pointLocations;
        m_lineRenderer = that.m_lineRenderer;
    }

    return *this;
}

SegmentedLineClass::~SegmentedLineClass() {}

void SegmentedLineClass::Reset_Line()
{
    m_lineRenderer.Reset_Line();
}

void SegmentedLineClass::Set_Points(unsigned int num_points, Vector3 *locs)
{
    if (num_points >= 2 && locs != nullptr) {
        m_pointLocations.Delete_All();

        for (unsigned int i = 0; i < num_points; ++i) {
            m_pointLocations.Add(locs[i], num_points);
        }

        Invalidate_Cached_Bounding_Volumes();
    } else {
        captainslog_dbgassert(false, "0");
    }
}

int SegmentedLineClass::Get_Num_Points()
{
    return m_pointLocations.Count();
}

void SegmentedLineClass::Set_Point_Location(unsigned int point_idx, const Vector3 &location)
{
    if (point_idx < static_cast<unsigned int>(m_pointLocations.Count())) {
        m_pointLocations[point_idx] = location;
    }

    Invalidate_Cached_Bounding_Volumes();
}

void SegmentedLineClass::Get_Point_Location(unsigned int point_idx, Vector3 &loc)
{
    if (point_idx < static_cast<unsigned int>(m_pointLocations.Count())) {
        loc = m_pointLocations[point_idx];
    } else {
        loc = Vector3(0, 0, 0);
    }
}

void SegmentedLineClass::Add_Point(const Vector3 &location)
{
    m_pointLocations.Add(location);
}

void SegmentedLineClass::Delete_Point(unsigned int point_idx)
{
    if (point_idx < static_cast<unsigned int>(m_pointLocations.Count())) {
        m_pointLocations.Delete(point_idx);
    }
}

TextureClass *SegmentedLineClass::Get_Texture()
{
    return m_lineRenderer.Get_Texture();
}

ShaderClass SegmentedLineClass::Get_Shader() const
{
    return m_lineRenderer.Get_Shader();
}

float SegmentedLineClass::Get_Width() const
{
    return m_lineRenderer.Get_Width();
}

void SegmentedLineClass::Get_Color(Vector3 &color) const
{
    color = m_lineRenderer.Get_Color();
}

float SegmentedLineClass::Get_Opacity() const
{
    return m_lineRenderer.Get_Opacity();
}

float SegmentedLineClass::Get_Noise_Amplitude() const
{
    return m_lineRenderer.Get_Noise_Amplitude();
}

float SegmentedLineClass::Get_Merge_Abort_Factor() const
{
    return m_lineRenderer.Get_Merge_Abort_Factor();
}

unsigned SegmentedLineClass::Get_Subdivision_Levels() const
{
    return m_maxSubdivisionLevels;
}

SegLineRendererClass::TextureMapMode SegmentedLineClass::Get_Texture_Mapping_Mode() const
{
    return m_lineRenderer.Get_Texture_Mapping_Mode();
}

float SegmentedLineClass::Get_Texture_Tile_Factor() const
{
    return m_lineRenderer.Get_Texture_Tile_Factor();
}

Vector2 SegmentedLineClass::Get_UV_Offset_Rate() const
{
    return m_lineRenderer.Get_UV_Offset_Rate();
}

int SegmentedLineClass::Is_Merge_Intersections() const
{
    return m_lineRenderer.Is_Merge_Intersections();
}

int SegmentedLineClass::Is_Freeze_Random() const
{
    return m_lineRenderer.Is_Freeze_Random();
}

int SegmentedLineClass::Is_Sorting_Disabled() const
{
    return m_lineRenderer.Is_Sorting_Disabled();
}

int SegmentedLineClass::Are_End_Caps_Enabled() const
{
    return m_lineRenderer.Are_End_Caps_Enabled();
}

void SegmentedLineClass::Set_Texture(TextureClass *texture)
{
    m_lineRenderer.Set_Texture(texture);
}

void SegmentedLineClass::Set_Shader(ShaderClass shader)
{
    m_lineRenderer.Set_Shader(shader);
}

void SegmentedLineClass::Set_Width(float width)
{
    if (width > 0.0f) {
        m_lineRenderer.Set_Width(width);
    } else {
        m_lineRenderer.Set_Width(0.0f);
    }

    Invalidate_Cached_Bounding_Volumes();
}

void SegmentedLineClass::Set_Color(const Vector3 &color)
{
    m_lineRenderer.Set_Color(color);
}

void SegmentedLineClass::Set_Opacity(float opacity)
{
    m_lineRenderer.Set_Opacity(opacity);
}

void SegmentedLineClass::Set_Noise_Amplitude(float amplitude)
{
    m_lineRenderer.Set_Noise_Amplitude(GameMath::Fabs(amplitude));
    Invalidate_Cached_Bounding_Volumes();
}

void SegmentedLineClass::Set_Merge_Abort_Factor(float factor)
{
    m_lineRenderer.Set_Merge_Abort_Factor(factor);
}

void SegmentedLineClass::Set_Subdivision_Levels(unsigned int levels)
{
    m_maxSubdivisionLevels = std::min<unsigned int>(levels, 7);
    Invalidate_Cached_Bounding_Volumes();
}

void SegmentedLineClass::Set_Texture_Mapping_Mode(SegLineRendererClass::TextureMapMode mode)
{
    m_lineRenderer.Set_Texture_Mapping_Mode(mode);
}

void SegmentedLineClass::Set_Texture_Tile_Factor(float factor)
{
    m_lineRenderer.Set_Texture_Tile_Factor(factor);
}

void SegmentedLineClass::Set_UV_Offset_Rate(const Vector2 &rate)
{
    m_lineRenderer.Set_UV_Offset_Rate(rate);
}

void SegmentedLineClass::Set_Merge_Intersections(int onoff)
{
    m_lineRenderer.Set_Merge_Intersections(onoff);
}

void SegmentedLineClass::Set_Freeze_Random(int onoff)
{
    m_lineRenderer.Set_Freeze_Random(onoff);
}

void SegmentedLineClass::Set_Disable_Sorting(int onoff)
{
    m_lineRenderer.Set_Disable_Sorting(onoff);
}

void SegmentedLineClass::Set_End_Caps(int onoff)
{
    m_lineRenderer.Set_End_Caps(onoff);
}

RenderObjClass *SegmentedLineClass::Clone() const
{
    return new SegmentedLineClass(*this);
}

int SegmentedLineClass::Get_Num_Polys() const
{
    return 2 * (m_pointLocations.Count() - 1) * (2 * m_lineRenderer.Get_Current_Subdivision_Level());
}

void SegmentedLineClass::Render(RenderInfoClass &rinfo)
{
    if (Is_Not_Hidden_At_All()) {
        int sort = 0;
        if (!W3D::Is_Sorting_Enabled()) {
            sort = Get_Shader().Guess_Sort_Level();
        }
        if (W3D::Are_Static_Sort_Lists_Enabled() && sort) {
            W3D::Add_To_Static_Sort_List(this, sort);
        } else {
            Render_Seg_Line(rinfo);
        }
    }
}

void SegmentedLineClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    AABoxClass aabox;
    Get_Obj_Space_Bounding_Box(aabox);
    sphere.Init(aabox.m_center, aabox.m_extent.Length());
}

void SegmentedLineClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
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

        Vector3 scale_factor(bounds_width, bounds_width, bounds_width);
        max_pos += scale_factor;
        min_pos -= scale_factor;

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
            scale_factor.Set(bounds_width, bounds_width, bounds_width);
            max_pos_2 += scale_factor;
            min_pos_2 -= scale_factor;

            max_pos.Update_Max(max_pos_2);
            min_pos.Update_Min(min_pos_2);
        }

        box.Init_Min_Max(min_pos, max_pos);

    } else {
        box.Init(Vector3(0, 0, 0), Vector3(1, 1, 1));
    }
}

void SegmentedLineClass::Prepare_LOD(CameraClass &camera)
{
    m_normalizedScreenArea = Get_Screen_Size(camera);
    unsigned int level = m_lineRenderer.Get_Current_Subdivision_Level();

    if (level >= m_maxSubdivisionLevels) {
        level = m_maxSubdivisionLevels;
    }

    m_lineRenderer.Set_Current_Subdivision_Level(level);
}

void SegmentedLineClass::Increment_LOD()
{
    unsigned int level = m_lineRenderer.Get_Current_Subdivision_Level();

    level = std::min(level + 1, m_maxSubdivisionLevels);

    m_lineRenderer.Set_Current_Subdivision_Level(level);
}

void SegmentedLineClass::Decrement_LOD()
{
    unsigned int level = m_lineRenderer.Get_Current_Subdivision_Level();
    if (level) {
        m_lineRenderer.Set_Current_Subdivision_Level(level - 1);
    }
}

float SegmentedLineClass::Get_Cost() const
{
    return (float)Get_Num_Polys();
}

float SegmentedLineClass::Get_Value() const
{
    if (m_lineRenderer.Get_Current_Subdivision_Level() == 0) {
        return AT_MIN_LOD;
    }

    float poly = (float)Get_Num_Polys();
    float p = (1.0f - 0.5f / (poly * poly)) * m_normalizedScreenArea;
    return p / Get_Cost();
}

float SegmentedLineClass::Get_Post_Increment_Value() const
{
    if (m_lineRenderer.Get_Current_Subdivision_Level() == m_maxSubdivisionLevels) {
        return AT_MAX_LOD;
    }

    float poly = (float)Get_Num_Polys();
    float poly2 = poly + poly;
    float p = (1.0f - 0.5f / (poly2 * poly2));
    return p * m_normalizedScreenArea / poly2;
}

void SegmentedLineClass::Set_LOD_Level(int lod)
{
    lod = lod < 0 ? 0 : lod;
    if (lod >= (int)m_maxSubdivisionLevels) {
        lod = (int)m_maxSubdivisionLevels;
    }
    m_lineRenderer.Set_Current_Subdivision_Level(lod);
}

int SegmentedLineClass::Get_LOD_Level() const
{
    return m_lineRenderer.Get_Current_Subdivision_Level();
}

int SegmentedLineClass::Get_LOD_Count() const
{
    return m_maxSubdivisionLevels;
}

bool SegmentedLineClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    if (Get_Collision_Type() & raytest.m_collisionType) {

        bool retval = false;
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
                    retval = true;
                    break;
                }
            }
        }
        if (retval) {
            raytest.m_result->fraction = fraction;
            raytest.m_result->surface_type = SURFACE_TYPE_DEFAULT;
            raytest.m_collidedRenderObj = this;
            return true;
        }
    }

    return false;
}

void SegmentedLineClass::Render_Seg_Line(RenderInfoClass &rinfo)
{
    if (m_pointLocations.Count() >= 2) {
        SphereClass obj_sphere;
        Get_Obj_Space_Bounding_Sphere(obj_sphere);
        m_lineRenderer.Render(rinfo, m_transform, m_pointLocations.Count(), &m_pointLocations[0], obj_sphere, nullptr);
    }
}
