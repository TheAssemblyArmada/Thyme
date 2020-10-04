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
#include "lineseg.h"

void LineSegClass::Set(const LineSegClass &object, const Matrix3D &transform)
{
    Matrix3D::Transform_Vector(transform, object.m_P0, &m_P0);
    Matrix3D::Transform_Vector(transform, object.m_P1, &m_P1);
    m_DP = m_P1 - m_P0;
    Matrix3D::Rotate_Vector(transform, object.m_dir, &m_dir);
    m_length = object.m_length;
}

void LineSegClass::Set_Random(const Vector3 &min, const Vector3 &max)
{
    m_P0.X = (max.X - min.X) * GameMath::Random_Float() + min.X;
    m_P0.Y = (max.Y - min.Y) * GameMath::Random_Float() + min.Y;
    m_P0.Z = (max.Z - min.Z) * GameMath::Random_Float() + min.Z;
    m_P1.X = (max.X - min.X) * GameMath::Random_Float() + min.X;
    m_P1.Y = (max.Y - min.Y) * GameMath::Random_Float() + min.Y;
    m_P1.Z = (max.Z - min.Z) * GameMath::Random_Float() + min.Z;
    m_DP = m_P1 - m_P0;
    m_dir = m_DP;
    m_dir.Normalize();
    m_length = m_DP.Length();
}

Vector3 LineSegClass::Find_Point_Closest_To(const Vector3 &pos) const
{
    float val1 = Vector3::Dot_Product(m_dir, pos - m_P0);

    if (val1 <= 0.0f) {
        return m_P0;
    }

    if (val1 >= m_length) {
        return m_P1;
    }

    return m_P0 + (m_dir * val1);
}

bool LineSegClass::Find_Intersection(
    const LineSegClass &other_line, Vector3 *p1, float *fraction1, Vector3 *p2, float *fraction2) const
{
    Vector3 cross1 = Vector3::Cross_Product(m_dir, other_line.m_dir);
    Vector3 cross2 = Vector3::Cross_Product(other_line.m_P0 - m_P0, other_line.m_dir);
    float f1 = cross2 * cross1;
    float f2 = cross1 * cross1;
    Vector3 cross3 = Vector3::Cross_Product(other_line.m_dir, m_dir);
    Vector3 cross4 = Vector3::Cross_Product(m_P0 - other_line.m_P0, m_dir);
    float f3 = cross4 * cross3;
    float f4 = cross3 * cross3;

    if (f2 == 0 || f4 == 0) {
        return false;
    }

    float f5 = f1 / f2;
    float f6 = f3 / f4;
    *p1 = m_P0 + (m_dir * f5);
    *p2 = other_line.m_P0 + (other_line.m_dir * f6);

    if (fraction1) {
        *fraction1 = f5 / m_length;
    }

    if (fraction2) {
        *fraction2 = f6 / m_length;
    }

    return true;
}
