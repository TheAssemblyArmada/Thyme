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
#include "frustum.h"

void FrustumClass::Init(const Matrix3D &camera, const Vector2 &vpmin, const Vector2 &vpmax, float znear, float zfar)
{
    // Store the camera transform
    m_cameraTransform = camera;

    if ((znear > 0.0f) && (zfar > 0.0f)) {
        znear = -znear;
        zfar = -zfar;
    }

    m_corners[0].Set(vpmin.X, vpmax.Y, 1.0f);
    m_corners[4] = m_corners[0];
    m_corners[0] *= znear;
    m_corners[4] *= zfar;
    m_corners[1].Set(vpmax.X, vpmax.Y, 1.0f);
    m_corners[5] = m_corners[1];
    m_corners[1] *= znear;
    m_corners[5] *= zfar;
    m_corners[2].Set(vpmin.X, vpmin.Y, 1.0f);
    m_corners[6] = m_corners[2];
    m_corners[2] *= znear;
    m_corners[6] *= zfar;
    m_corners[3].Set(vpmax.X, vpmin.Y, 1.0f);
    m_corners[7] = m_corners[3];
    m_corners[3] *= znear;
    m_corners[7] *= zfar;

    for (int i = 0; i < 8; i++) {
        Matrix3D::Transform_Vector(m_cameraTransform, m_corners[i], &(m_corners[i]));
    }

    PlaneClass frustum_planes[6];
    m_planes[0].Set(m_corners[0], m_corners[3], m_corners[1]); // near
    m_planes[1].Set(m_corners[0], m_corners[5], m_corners[4]); // bottom
    m_planes[2].Set(m_corners[0], m_corners[6], m_corners[2]); // right
    m_planes[3].Set(m_corners[2], m_corners[7], m_corners[3]); // top
    m_planes[4].Set(m_corners[1], m_corners[7], m_corners[5]); // left
    m_planes[5].Set(m_corners[4], m_corners[7], m_corners[6]); // far

    m_boundMin = m_boundMax = m_corners[0];

    for (int i = 1; i < 8; i++) {
        if (m_corners[i].X < m_boundMin.X) {
            m_boundMin.X = m_corners[i].X;
        }

        if (m_corners[i].X > m_boundMax.X) {
            m_boundMax.X = m_corners[i].X;
        }

        if (m_corners[i].Y < m_boundMin.Y) {
            m_boundMin.Y = m_corners[i].Y;
        }

        if (m_corners[i].Y > m_boundMax.Y) {
            m_boundMax.Y = m_corners[i].Y;
        }

        if (m_corners[i].Z < m_boundMin.Z) {
            m_boundMin.Z = m_corners[i].Z;
        }

        if (m_corners[i].Z > m_boundMax.Z) {
            m_boundMax.Z = m_corners[i].Z;
        }
    }
}
