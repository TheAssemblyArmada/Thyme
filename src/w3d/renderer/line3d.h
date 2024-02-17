/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Line 3D Object
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
#include "shader.h"
#include "w3dmpo.h"

class Line3DClass : public W3DMPO, public RenderObjClass
{
    IMPLEMENT_W3D_POOL(Line3DClass);

public:
    virtual ~Line3DClass() override {}
    virtual Line3DClass *Clone() const override { return new Line3DClass(*this); }
    virtual int Class_ID() const override { return CLASSID_LINE3D; }
    virtual int Get_Num_Polys() const override { return 12; }
    virtual void Render(RenderInfoClass &rinfo) override;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    virtual int Get_Sort_Level() const override { return m_sortLevel; }
    virtual void Set_Sort_Level(int level) override { m_sortLevel = level; }
    virtual void Scale(float scale) override;
    virtual void Scale(float scalex, float scaley, float scalez) override;

    Line3DClass(const Vector3 &start, const Vector3 &end, float width, float r, float g, float b, float opacity);
    Line3DClass(const Line3DClass &src);

    void Reset(const Vector3 &new_start, const Vector3 &new_end);
    void Reset(const Vector3 &new_start, const Vector3 &new_end, float new_width);
    void Re_Color(float r, float g, float b);
    void Set_Opacity(float opacity);

    Line3DClass *Hook_Ctor(const Vector3 &start, const Vector3 &end, float width, float r, float g, float b, float opacity)
    {
        return new (this) Line3DClass(start, end, width, r, g, b, opacity);
    }

private:
    float m_length;
    float m_thickness;
    ShaderClass m_shader;
    Vector3 m_vertices[8];
    Vector4 m_color;
    char m_sortLevel;
};
