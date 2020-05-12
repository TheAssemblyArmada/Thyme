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
    LineSegClass() {}
    LineSegClass(const Vector3 &p0, const Vector3 &p1) : m_P0(p0), m_P1(p1) { Recalculate(); }
    LineSegClass(const LineSegClass &that, const Matrix3D &tm) { Set(that, tm); }

    void Set(const Vector3 &p0, const Vector3 &p1)
    {
        m_P0 = p0;
        m_P1 = p1;
        Recalculate();
    }

    void Set(const LineSegClass &that, const Matrix3D &tm);
    void Set_Random(const Vector3 &min, const Vector3 &max);
    const Vector3 &Get_P0() const { return m_P0; }
    const Vector3 &Get_P1() const { return m_P1; }
    const Vector3 &Get_DP() const { return m_DP; }
    const Vector3 &Get_Dir() const { return m_dir; }
    float Get_Length() const { return m_length; }
    void Compute_Point(float t, Vector3 *set) const { Vector3::Add(m_P0, t * m_DP, set); }
    Vector3 Find_Point_Closest_To(const Vector3 &pos) const;
    bool Find_Intersection(
        const LineSegClass &other_line, Vector3 *p1, float *fraction1, Vector3 *p2, float *fraction2) const;

protected:
    void Recalculate()
    {
        m_DP = m_P1 - m_P0;
        m_dir = m_DP;
        m_dir.Normalize();
        m_length = m_DP.Length();
    }

    Vector3 m_P0;
    Vector3 m_P1;
    Vector3 m_DP;
    Vector3 m_dir;
    float m_length;
};
