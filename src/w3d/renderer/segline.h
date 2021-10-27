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
#pragma once
#include "always.h"
#include "rendobj.h"
#include "seglinerenderer.h"
#include "shader.h"
#include "simplevec.h"

class TextureClass;

class SegmentedLineClass : public RenderObjClass
{
public:
    SegmentedLineClass();
    SegmentedLineClass(const SegmentedLineClass &src);
    SegmentedLineClass &operator=(const SegmentedLineClass &that);
    virtual ~SegmentedLineClass();

    void Reset_Line();

    void Set_Points(unsigned int num_points, Vector3 *locs);
    int Get_Num_Points();
    void Set_Point_Location(unsigned int point_idx, const Vector3 &location);
    void Get_Point_Location(unsigned int point_idx, Vector3 &loc);
    void Add_Point(const Vector3 &location);
    void Delete_Point(unsigned int point_idx);

    TextureClass *Get_Texture();
    ShaderClass Get_Shader() const;
    float Get_Width() const;
    void Get_Color(Vector3 &color) const;
    float Get_Opacity() const;
    float Get_Noise_Amplitude() const;
    float Get_Merge_Abort_Factor() const;
    unsigned int Get_Subdivision_Levels() const;
    SegLineRendererClass::TextureMapMode Get_Texture_Mapping_Mode() const;
    float Get_Texture_Tile_Factor() const;
    Vector2 Get_UV_Offset_Rate() const;
    int Is_Merge_Intersections() const;
    int Is_Freeze_Random() const;
    int Is_Sorting_Disabled() const;
    int Are_End_Caps_Enabled() const;

    void Set_Texture(TextureClass *texture);
    void Set_Shader(ShaderClass shader);
    void Set_Width(float width);
    void Set_Color(const Vector3 &color);
    void Set_Opacity(float opacity);
    void Set_Noise_Amplitude(float amplitude);
    void Set_Merge_Abort_Factor(float factor);
    void Set_Subdivision_Levels(unsigned int levels);
    void Set_Texture_Mapping_Mode(SegLineRendererClass::TextureMapMode mode);
    void Set_Texture_Tile_Factor(float factor);
    void Set_UV_Offset_Rate(const Vector2 &rate);
    void Set_Merge_Intersections(int onoff);
    void Set_Freeze_Random(int onoff);
    void Set_Disable_Sorting(int onoff);
    void Set_End_Caps(int onoff);

    virtual RenderObjClass *Clone() const override;
    virtual int Class_ID() const override { return CLASSID_SEGLINE; }
    virtual int Get_Num_Polys() const override;

    virtual void Render(RenderInfoClass &rinfo) override;

    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

    virtual void Prepare_LOD(CameraClass &camera) override;
    virtual void Increment_LOD() override;
    virtual void Decrement_LOD() override;
    virtual float Get_Cost() const override;
    virtual float Get_Value() const override;
    virtual float Get_Post_Increment_Value() const override;
    virtual void Set_LOD_Level(int lod) override;
    virtual int Get_LOD_Level() const override;
    virtual int Get_LOD_Count() const override;

    virtual bool Cast_Ray(RayCollisionTestClass &raytest) override;

#if GAME_DLL
    SegmentedLineClass *Hook_Ctor() { return new (this) SegmentedLineClass; }

#endif

protected:
    void Render_Seg_Line(RenderInfoClass &rinfo);

private:
    unsigned int m_maxSubdivisionLevels;
    float m_normalizedScreenArea;
    SimpleDynVecClass<Vector3> m_pointLocations;
    SegLineRendererClass m_lineRenderer;
};
