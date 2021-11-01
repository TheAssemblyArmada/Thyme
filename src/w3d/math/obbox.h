/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Oriented Bounding Box
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
#include "gamemath.h"
#include "matrix3.h"
#include "matrix3d.h"
#include "vector3.h"

class OBBoxClass
{
public:
    OBBoxClass() {}

    OBBoxClass(const OBBoxClass &that) : m_basis(that.m_basis), m_center(that.m_center), m_extent(that.m_extent) {}

    OBBoxClass(const Vector3 &center, const Vector3 &extent) : m_basis(1), m_center(center), m_extent(extent) {}

    OBBoxClass(const Vector3 &center, const Vector3 &extent, const Matrix3 &basis) :
        m_basis(basis), m_center(center), m_extent(extent)
    {
    }

    OBBoxClass(const Vector3 *points, int num_points);

    bool operator==(const OBBoxClass &src)
    {
        return (m_center == src.m_center) && (m_extent == src.m_extent) && (m_basis == src.m_basis);
    }

    bool operator!=(const OBBoxClass &src)
    {
        return (m_center != src.m_center) || (m_extent != src.m_extent) && (m_basis == src.m_basis);
    }

    void Init_From_Box_Points(Vector3 *points, int num_points);
    void Init_Random(float min_extent = 0.5f, float max_extent = 1.0f);

    float Project_To_Axis(const Vector3 &axis) const
    {
        float x = m_extent[0] * Vector3::Dot_Product(axis, Vector3(m_basis[0][0], m_basis[1][0], m_basis[2][0]));
        float y = m_extent[1] * Vector3::Dot_Product(axis, Vector3(m_basis[0][1], m_basis[1][1], m_basis[2][1]));
        float z = m_extent[2] * Vector3::Dot_Product(axis, Vector3(m_basis[0][2], m_basis[1][2], m_basis[2][2]));

        return GameMath::Fabs(x) + GameMath::Fabs(y) + GameMath::Fabs(z);
    }

    float Volume(void) const { return 2.0 * m_extent.X * 2.0 * m_extent.Y * 2.0 * m_extent.Z; }

    void Compute_Point(float params[3], Vector3 *set_point) const
    {
        Vector3 point = m_extent;
        point.X *= params[0];
        point.Y *= params[1];
        point.Z *= params[2];
        Matrix3::Rotate_Vector(m_basis, point, set_point);
        Vector3::Add(m_center, *set_point, set_point);
    }

    void Compute_Axis_Aligned_m_extent(Vector3 *set_extent) const
    {
        captainslog_assert(set_extent != nullptr);
        set_extent->X = GameMath::Fabs(m_extent[0] * m_basis[0][0]) + GameMath::Fabs(m_extent[1] * m_basis[0][1])
            + GameMath::Fabs(m_extent[2] * m_basis[0][2]);
        set_extent->Y = GameMath::Fabs(m_extent[0] * m_basis[1][0]) + GameMath::Fabs(m_extent[1] * m_basis[1][1])
            + GameMath::Fabs(m_extent[2] * m_basis[1][2]);
        set_extent->Z = GameMath::Fabs(m_extent[0] * m_basis[2][0]) + GameMath::Fabs(m_extent[1] * m_basis[2][1])
            + GameMath::Fabs(m_extent[2] * m_basis[2][2]);
    }

    static void Transform(const Matrix3D &tm, const OBBoxClass &in, OBBoxClass *out)
    {
        captainslog_assert(out);
        captainslog_assert(out != &in);
        out->m_extent = in.m_extent;
        Matrix3D::Transform_Vector(tm, in.m_center, &(out->m_center));
        Matrix3::Multiply(tm, in.m_basis, &(out->m_basis));
    }

    void Compute_Axis_Aligned_Extent(Vector3 *set_extent) const;

    Matrix3 m_basis;
    Vector3 m_center;
    Vector3 m_extent;
};

inline void OBBoxClass::Compute_Axis_Aligned_Extent(Vector3 *set_extent) const
{
    captainslog_assert(set_extent != nullptr);

    set_extent->X = GameMath::Fabs(m_extent[0] * m_basis[0][0]) + GameMath::Fabs(m_extent[1] * m_basis[0][1])
        + GameMath::Fabs(m_extent[2] * m_basis[0][2]);

    set_extent->Y = GameMath::Fabs(m_extent[0] * m_basis[1][0]) + GameMath::Fabs(m_extent[1] * m_basis[1][1])
        + GameMath::Fabs(m_extent[2] * m_basis[1][2]);

    set_extent->Z = GameMath::Fabs(m_extent[0] * m_basis[2][0]) + GameMath::Fabs(m_extent[1] * m_basis[2][1])
        + GameMath::Fabs(m_extent[2] * m_basis[2][2]);
}
