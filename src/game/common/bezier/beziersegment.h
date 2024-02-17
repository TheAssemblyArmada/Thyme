/**
 * @file
 *
 * @author xezon
 *
 * @brief Bezier Segment class.
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
#include "coord.h"
#include "matrix4.h"
#include <vector>

class BezFwdIterator;

// This code can be tested with Tank Shells. Just shoot at some practice targets with CTRL + Mouse.

class BezierSegment
{
    friend class BezFwdIterator;

public:
    BezierSegment();

    BezierSegment(float p0x,
        float p0y,
        float p0z,
        float p1x,
        float p1y,
        float p1z,
        float p2x,
        float p2y,
        float p2z,
        float p3x,
        float p3y,
        float p3z);

    explicit BezierSegment(float xyz_points[12]);

    BezierSegment(const Coord3D &p0, const Coord3D &p1, const Coord3D &p2, const Coord3D &p3);

    explicit BezierSegment(const Coord3D points[4]);

    void Evaluate_Bez_Segment_At_T(float t, Coord3D *point);

    void Get_Segment_Points(int segment, std::vector<Coord3D> *points);

    float Get_Approximate_Length(float t);

    void Split_Segment_At_T(float t, BezierSegment &segment1, BezierSegment &segment2);

#ifdef GAME_DLL
    BezierSegment *Hook_Ctor1() { return new (this) BezierSegment(); }

    BezierSegment *Hook_Ctor2(float p0x,
        float p0y,
        float p0z,
        float p1x,
        float p1y,
        float p1z,
        float p2x,
        float p2y,
        float p2z,
        float p3x,
        float p3y,
        float p3z)
    {
        return new (this) BezierSegment(p0x, p0y, p0z, p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z);
    }

    BezierSegment *Hook_Ctor3(float xyz_points[12]) { return new (this) BezierSegment(xyz_points); }

    BezierSegment *Hook_Ctor4(const Coord3D &p0, const Coord3D &p1, const Coord3D &p2, const Coord3D &p3)
    {
        return new (this) BezierSegment(p0, p1, p2, p3);
    }

    BezierSegment *Hook_Ctor5(const Coord3D points[4]) { return new (this) BezierSegment(points); }
#endif

private:
    Coord3D m_points[4];

    static const Matrix4 s_bezBasisMatrix;
};
