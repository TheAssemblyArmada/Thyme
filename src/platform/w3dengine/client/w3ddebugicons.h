/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Class for drawing small debug icons on screen.
 * Commonly used for debugging things like pathfinding, trajectories, flight paths.
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
#include "color.h"
#include "coord.h"
#include "rendobj.h"

class VertexMaterialClass;

struct DebugIcon
{
    Coord3D position;
    float width;
    RGBColor color;
    int endFrame;
};

class W3DDebugIcons : public RenderObjClass
{
public:
    W3DDebugIcons();

    W3DDebugIcons(const W3DDebugIcons &src) = delete;

    W3DDebugIcons &operator=(const W3DDebugIcons &that) = delete;

    virtual ~W3DDebugIcons();

    int Class_ID() const override { return -1; }

    bool Cast_Ray(RayCollisionTestClass &raytest) override { return false; }

    void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

    RenderObjClass *Clone() const override;
    void Render(RenderInfoClass &rinfo) override;

    static void Add_Icon(Coord3D *pos, float width, int num_frames_duration, RGBColor color);

    enum
    {
        MAX_ICONS = 100000,
    };

protected:
    void Allocate_Icons_Array();
    void Compress_Icons_Array();

private:
    VertexMaterialClass *m_vertexMaterial;

protected:
    static DebugIcon *s_debugIcons;
    static int s_numDebugIcons;
    static int s_numUnknown;
};

void Add_Icon(Coord3D *pos, float width, int num_frames_duration, RGBColor color);
