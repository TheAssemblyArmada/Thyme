/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Line Segment
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
#include "vector3.h"

class LineSegClass
{
public:
    LineSegClass(void) {}
    LineSegClass(const Vector3 &p0, const Vector3 &p1) : P0(p0), P1(p1) { recalculate(); }
    LineSegClass(const LineSegClass &that, const Matrix3D &tm) { Set(that, tm); }

    void Set(const Vector3 &p0, const Vector3 &p1)
    {
        P0 = p0;
        P1 = p1;
        recalculate();
    }

    void Set(const LineSegClass &that, const Matrix3D &tm);
    void Set_Random(const Vector3 &min, const Vector3 &max);
    const Vector3 &Get_P0() const { return P0; }
    const Vector3 &Get_P1() const { return P1; }
    const Vector3 &Get_DP() const { return DP; }
    const Vector3 &Get_Dir() const { return Dir; }
    float Get_Length() const { return Length; }
    void Compute_Point(float t, Vector3 *set) const { Vector3::Add(P0, t * DP, set); }
    Vector3 Find_Point_Closest_To(const Vector3 &pos) const;
    bool Find_Intersection(
        const LineSegClass &other_line, Vector3 *p1, float *fraction1, Vector3 *p2, float *fraction2) const;

protected:
    void recalculate(void)
    {
        DP = P1 - P0;
        Dir = DP;
        Dir.Normalize();
        Length = DP.Length();
    }

    Vector3 P0;
    Vector3 P1;
    Vector3 DP;
    Vector3 Dir;
    float Length;
};
