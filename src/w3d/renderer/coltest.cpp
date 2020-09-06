/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Classes for testing collision.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "coltest.h"

AABoxCollisionTestClass::AABoxCollisionTestClass(const AABoxCollisionTestClass &that) :
    CollisionTestClass(that),
    m_box(that.m_box),
    m_move(that.m_move),
    m_sweepMin(that.m_sweepMin),
    m_sweepMax(that.m_sweepMax)
{
}

AABoxCollisionTestClass::AABoxCollisionTestClass(
    const AABoxClass &aabox, const Vector3 &move, CastResultStruct *res, int collision_type) :
    CollisionTestClass(res, collision_type), m_box(aabox), m_move(move)
{
    m_sweepMin = m_box.m_center - m_box.m_extent;
    m_sweepMax = m_box.m_center + m_box.m_extent;

    Vector3 endmin = m_box.m_center + move - m_box.m_extent;
    Vector3 endmax = m_box.m_center + move + m_box.m_extent;

    if (endmax.X > m_sweepMax.X) {
        m_sweepMax.X = endmax.X;
    }
    if (endmax.Y > m_sweepMax.Y) {
        m_sweepMax.Y = endmax.Y;
    }
    if (endmax.Z > m_sweepMax.Z) {
        m_sweepMax.Z = endmax.Z;
    }
    if (endmin.X < m_sweepMin.X) {
        m_sweepMin.X = endmin.X;
    }
    if (endmin.Y < m_sweepMin.Y) {
        m_sweepMin.Y = endmin.Y;
    }
    if (endmin.Z < m_sweepMin.Z) {
        m_sweepMin.Z = endmin.Z;
    }
}

bool AABoxCollisionTestClass::Cull(const AABoxClass &box)
{
    Vector3 corner_min = m_box.m_center - m_box.m_extent;
    Vector3 corner_max = m_box.m_center + m_box.m_extent;

    return Cull(corner_min, corner_max);
}

void AABoxCollisionTestClass::Rotate(ROTATION_TYPE rotation)
{
    float tmp, minx, miny, maxx, maxy;

    switch (rotation) {
        case ROTATE_NONE:
            break;

        case ROTATE_Z90:
            // rotate the center point and the move vector
            tmp = m_box.m_center.X;
            m_box.m_center.X = -m_box.m_center.Y;
            m_box.m_center.Y = tmp;
            tmp = m_move.X;
            m_move.X = -m_move.Y;
            m_move.Y = tmp;

            // swap x and y for the extent
            tmp = m_box.m_extent.X;
            m_box.m_extent.X = m_box.m_extent.Y;
            m_box.m_extent.Y = tmp;

            // update sweep bounding box
            minx = m_sweepMin.X;
            miny = m_sweepMin.Y;
            maxx = m_sweepMax.X;
            maxy = m_sweepMax.Y;
            m_sweepMin.X = -maxy;
            m_sweepMin.Y = minx;
            m_sweepMax.X = -miny;
            m_sweepMax.Y = maxx;
            break;

        case ROTATE_Z180:
            // rotate center and move vector
            m_box.m_center.X = -m_box.m_center.X;
            m_box.m_center.Y = -m_box.m_center.Y;
            m_move.X = -m_move.X;
            m_move.Y = -m_move.Y;

            // update min/max boxes
            minx = m_sweepMin.X;
            miny = m_sweepMin.Y;
            maxx = m_sweepMax.X;
            maxy = m_sweepMax.Y;
            m_sweepMin.X = -maxx;
            m_sweepMin.Y = -maxy;
            m_sweepMax.X = -minx;
            m_sweepMax.Y = -miny;
            break;

        case ROTATE_Z270:
            // rotate center and move.
            tmp = m_box.m_center.X;
            m_box.m_center.X = m_box.m_center.Y;
            m_box.m_center.Y = -tmp;
            tmp = m_move.X;
            m_move.X = m_move.Y;
            m_move.Y = -tmp;

            // update extent (x and y axis swap)
            tmp = m_box.m_extent.X;
            m_box.m_extent.X = m_box.m_extent.Y;
            m_box.m_extent.Y = tmp;

            // update min/max boxes
            minx = m_sweepMin.X;
            miny = m_sweepMin.Y;
            maxx = m_sweepMax.X;
            maxy = m_sweepMax.Y;
            m_sweepMin.X = miny;
            m_sweepMin.Y = -maxx;
            m_sweepMax.X = maxy;
            m_sweepMax.Y = -minx;
            break;
    }
}

void AABoxCollisionTestClass::Transform(const Matrix3D &tm)
{
    Vector3 tmp_center = m_box.m_center;
    Vector3 tmp_extent = m_box.m_extent;

    tm.Transform_Center_Extent_AABox(tmp_center, tmp_extent, &m_box.m_center, &m_box.m_extent);

    m_move = tm.Rotate_Vector(m_move);

    Vector3 pts[8];

    Vector3 &min = m_sweepMin;
    Vector3 &max = m_sweepMax;

    pts[0].Set(min.X, min.Y, min.Z);
    pts[1].Set(min.X, max.Y, min.Z);
    pts[2].Set(max.X, max.Y, min.Z);
    pts[3].Set(max.X, min.Y, min.Z);

    pts[4].Set(min.X, min.Y, max.Z);
    pts[5].Set(min.X, max.Y, max.Z);
    pts[6].Set(max.X, max.Y, max.Z);
    pts[7].Set(max.X, min.Y, max.Z);

    for (int i = 0; i < ARRAY_SIZE(pts); ++i) {
        pts[i] = tm * pts[i];
    }

    Vector3 real_min = pts[0];
    Vector3 real_max = pts[0];

    for (int i = 1; i < ARRAY_SIZE(pts); ++i) {
        if (real_min.X >= pts[i].X)
            real_min.X = pts[i].X;
        if (real_min.Y >= pts[i].Y)
            real_min.Y = pts[i].Y;
        if (real_min.Z >= pts[i].Z)
            real_min.Z = pts[i].Z;

        if (real_max.X <= pts[i].X)
            real_max.X = pts[i].X;
        if (real_max.Y <= pts[i].Y)
            real_max.Y = pts[i].Y;
        if (real_max.Z <= pts[i].Z)
            real_max.Z = pts[i].Z;
    }

    m_sweepMin = real_min;
    m_sweepMax = real_max;
}

OBBoxCollisionTestClass::OBBoxCollisionTestClass(
    const OBBoxClass &obbox, const Vector3 &move, CastResultStruct *res, int type) :
    CollisionTestClass(res, type), m_box(obbox), m_move(move)
{
    Vector3 extent;
    extent.X = GameMath::Fabs(m_box.m_basis[0][0] * m_box.m_extent.X)
        + GameMath::Fabs(m_box.m_basis[0][1] * m_box.m_extent.Y) + GameMath::Fabs(m_box.m_basis[0][2] * m_box.m_extent.Z)
        + 0.01f;

    extent.Y = GameMath::Fabs(m_box.m_basis[1][0] * m_box.m_extent.X)
        + GameMath::Fabs(m_box.m_basis[1][1] * m_box.m_extent.Y) + GameMath::Fabs(m_box.m_basis[1][2] * m_box.m_extent.Z)
        + 0.01f;

    extent.Z = GameMath::Fabs(m_box.m_basis[2][0] * m_box.m_extent.X)
        + GameMath::Fabs(m_box.m_basis[2][1] * m_box.m_extent.Y) + GameMath::Fabs(m_box.m_basis[2][2] * m_box.m_extent.Z)
        + 0.01f;

    m_sweepMin = m_box.m_center - extent;
    m_sweepMax = m_box.m_center + extent;

    Vector3 end_min = m_box.m_center + move - extent;
    Vector3 end_max = m_box.m_center + move + extent;

    if (end_max.X > m_sweepMax.X)
        m_sweepMax.X = end_max.X;
    if (end_max.Y > m_sweepMax.Y)
        m_sweepMax.Y = end_max.Y;
    if (end_max.Z > m_sweepMax.Z)
        m_sweepMax.Z = end_max.Z;

    if (end_min.X < m_sweepMin.X)
        m_sweepMin.X = end_min.X;
    if (end_min.Y < m_sweepMin.Y)
        m_sweepMin.Y = end_min.Y;
    if (end_min.Z < m_sweepMin.Z)
        m_sweepMin.Z = end_min.Z;
}

OBBoxCollisionTestClass::OBBoxCollisionTestClass(const OBBoxCollisionTestClass &that) :
    CollisionTestClass(that),
    m_box(that.m_box),
    m_move(that.m_move),
    m_sweepMin(that.m_sweepMin),
    m_sweepMax(that.m_sweepMax)
{
}

OBBoxCollisionTestClass::OBBoxCollisionTestClass(const OBBoxCollisionTestClass &that, const Matrix3D &tm) :
    CollisionTestClass(that)
{
    tm.Transform_Min_Max_AABox(that.m_sweepMin, that.m_sweepMax, &m_sweepMin, &m_sweepMax);
    Matrix3D::Rotate_Vector(tm, that.m_move, &m_move);
    OBBoxClass::Transform(tm, that.m_box, &m_box);
}

OBBoxCollisionTestClass::OBBoxCollisionTestClass(const AABoxCollisionTestClass &that, const Matrix3D &tm) :
    CollisionTestClass(that)
{
    tm.Transform_Min_Max_AABox(that.m_sweepMin, that.m_sweepMax, &m_sweepMin, &m_sweepMax);
    Matrix3D::Rotate_Vector(tm, that.m_move, &m_move);
    Matrix3D::Transform_Vector(tm, that.m_box.m_center, &(m_box.m_center));
    m_box.m_extent = that.m_box.m_extent;
    m_box.m_basis = tm;
}

bool OBBoxCollisionTestClass::Cull(const AABoxClass &box)
{
    Vector3 corner_min = m_box.m_center - m_box.m_extent;
    Vector3 corner_max = m_box.m_center + m_box.m_extent;

    return Cull(corner_min, corner_max);
}
