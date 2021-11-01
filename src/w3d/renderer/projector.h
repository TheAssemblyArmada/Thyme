/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Projector Class
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
#include "aabox.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "obbox.h"

class MatrixMapperClass;

class ProjectorClass
{
public:
    ProjectorClass();
    virtual ~ProjectorClass();
    virtual void Set_Transform(const Matrix3D &tm);
    virtual const Matrix3D &Get_Transform() const;
    virtual void Set_Perspective_Projection(float hfov, float vfov, float znear, float zfar);
    virtual void Set_Ortho_Projection(float xmin, float xmax, float ymin, float ymax, float znear, float zfar);
    const OBBoxClass &Get_Bounding_Volume() const { return m_worldBoundingVolume; }
    void Compute_Texture_Coordinate(const Vector3 &point, Vector3 *set_stq);

protected:
    virtual void Update_WS_Bounding_Volume();

    Matrix3D m_transform;
    Matrix4 m_projection;
    AABoxClass m_lcalBoundingVolume;
    OBBoxClass m_worldBoundingVolume;
    MatrixMapperClass *m_mapper;
};
