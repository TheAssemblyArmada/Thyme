/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Frustum
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "plane.h"
#include "vector3.h"

class FrustumClass
{
public:
    void Init(const Matrix3D &camera, const Vector2 &viewport_min, const Vector2 &viewport_max, float znear, float zfar);

    const Vector3 &Get_Bound_Min(void) const { return m_boundMin; }
    const Vector3 &Get_Bound_Max(void) const { return m_boundMax; }

public:
    Matrix3D m_cameraTransform;
    PlaneClass m_planes[6];
    Vector3 m_corners[8];
    Vector3 m_boundMin;
    Vector3 m_boundMax;
};
