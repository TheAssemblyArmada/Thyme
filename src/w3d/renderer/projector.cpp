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
#include "projector.h"
#include "matrixmapper.h"

ProjectorClass::ProjectorClass() :
    m_transform(true),
    m_projection(true),
    m_lcalBoundingVolume(Vector3(0, 0, 0), Vector3(1, 1, 1)),
    m_worldBoundingVolume(Vector3(0, 0, 0), Vector3(1, 1, 1), Matrix3(true))
{
    m_mapper = new MatrixMapperClass(0);
}

ProjectorClass::~ProjectorClass()
{
    Ref_Ptr_Release(m_mapper);
}

void ProjectorClass::Set_Transform(const Matrix3D &tm)
{
    m_transform = tm;
    Update_WS_Bounding_Volume();
}

const Matrix3D &ProjectorClass::Get_Transform() const
{
    return m_transform;
}

void ProjectorClass::Set_Perspective_Projection(float hfov, float vfov, float znear, float zfar)
{
    m_mapper->Set_Type(MatrixMapperClass::PERSPECTIVE_PROJECTION);
    m_projection.Init_Perspective(hfov, vfov, 0.1f, zfar);

    float tan_hfov2 = GameMath::Tan(hfov) * 0.5f;
    float tan_vfov2 = GameMath::Tan(vfov) * 0.5f;

    m_lcalBoundingVolume.m_center.Set(0.0f, 0.0f, -(zfar + znear) * 0.5f);
    m_lcalBoundingVolume.m_extent.X = zfar * tan_hfov2;
    m_lcalBoundingVolume.m_extent.Y = zfar * tan_vfov2;
    m_lcalBoundingVolume.m_extent.Z = (zfar - znear) * 0.5f;

    Update_WS_Bounding_Volume();
}

void ProjectorClass::Set_Ortho_Projection(float xmin, float xmax, float ymin, float ymax, float znear, float zfar)
{
    m_mapper->Set_Type(MatrixMapperClass::ORTHO_PROJECTION);
    m_projection.Init_Ortho(xmin, xmax, ymin, ymax, 0.1f, zfar);

    m_lcalBoundingVolume.m_center.Set((xmax + xmin) * 0.5f, (ymax + ymin) * 0.5f, -(zfar + znear) * 0.5f);
    m_lcalBoundingVolume.m_extent.Set((xmax - xmin) * 0.5f, (ymax - ymin) * 0.5f, (zfar - znear) * 0.5f);

    Update_WS_Bounding_Volume();
}

void ProjectorClass::Compute_Texture_Coordinate(const Vector3 &point, Vector3 *set_stq)
{
    m_mapper->Compute_Texture_Coordinate(point, set_stq);
}

void ProjectorClass::Update_WS_Bounding_Volume()
{
    OBBoxClass localbox(m_lcalBoundingVolume.m_center, m_lcalBoundingVolume.m_extent, Matrix3(true));
    OBBoxClass::Transform(m_transform, localbox, &m_worldBoundingVolume);
}
