/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief intersection test
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
#include "colmath.h"
#include "coltype.h"
#include "obbox.h"
#include "tri.h"

class IntersectionTestClass
{
public:
    IntersectionTestClass(int collision_type) : m_collisionType(collision_type) {}
    IntersectionTestClass(const IntersectionTestClass &that) : m_collisionType(that.m_collisionType) {}

public:
    int m_collisionType;
};

class AABoxIntersectionTestClass : public IntersectionTestClass
{
public:
    AABoxIntersectionTestClass(const AABoxClass &box, int collision_type) : IntersectionTestClass(collision_type), m_box(box)
    {
    }

    AABoxIntersectionTestClass(const AABoxIntersectionTestClass &that) : IntersectionTestClass(that), m_box(that.m_box) {}

    bool Cull(const Vector3 &cull_min, const Vector3 &cull_max);
    bool Cull(const AABoxClass &cull_box);
    bool Intersect_Triangle(const TriClass &tri);

public:
    AABoxClass m_box;
};

inline bool AABoxIntersectionTestClass::Cull(const Vector3 &cull_min, const Vector3 &cull_max)
{
    Vector3 box_min;
    Vector3::Subtract(m_box.m_center, m_box.m_extent, &box_min);

    Vector3 box_max;
    Vector3::Add(m_box.m_center, m_box.m_extent, &box_max);

    if ((box_min.X > cull_max.X) || (box_max.X < cull_min.X)) {
        return true;
    }

    if ((box_min.Y > cull_max.Y) || (box_max.Y < cull_min.Y)) {
        return true;
    }

    if ((box_min.Z > cull_max.Z) || (box_max.Z < cull_min.Z)) {
        return true;
    }

    // #BUGFIX Return a value.
    return false;
}

inline bool AABoxIntersectionTestClass::Cull(const AABoxClass &cull_box)
{
    Vector3 dc;
    Vector3 r;
    Vector3::Subtract(cull_box.m_center, m_box.m_center, &dc);
    Vector3::Add(cull_box.m_extent, m_box.m_extent, &r);

    if (GameMath::Fabs(dc.X) > r.X) {
        return true;
    }

    if (GameMath::Fabs(dc.Y) > r.Y) {
        return true;
    }

    if (GameMath::Fabs(dc.Z) > r.Z) {
        return true;
    }

    // #BUGFIX Return a value.
    return false;
}

inline bool AABoxIntersectionTestClass::Intersect_Triangle(const TriClass &tri)
{
    return CollisionMath::Intersection_Test(m_box, tri);
}

class OBBoxIntersectionTestClass : public IntersectionTestClass
{
public:
    OBBoxIntersectionTestClass(const OBBoxClass &box, int collision_type);
    OBBoxIntersectionTestClass(const OBBoxIntersectionTestClass &that);
    OBBoxIntersectionTestClass(const OBBoxIntersectionTestClass &that, const Matrix3D &tm);
    OBBoxIntersectionTestClass(const AABoxIntersectionTestClass &that, const Matrix3D &tm);

    bool Cull(const Vector3 &min, const Vector3 &max);
    bool Cull(const AABoxClass &box);
    bool Intersect_Triangle(const TriClass &tri);

protected:
    void update_bounding_box(void);

public:
    OBBoxClass m_box;
    AABoxClass m_boundingBox;
};

inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass(const OBBoxClass &box, int collision_type) :
    IntersectionTestClass(collision_type), m_box(box)
{
    update_bounding_box();
}

inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass(const OBBoxIntersectionTestClass &that) :
    IntersectionTestClass(that), m_box(that.m_box)
{
    update_bounding_box();
}

inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass(const OBBoxIntersectionTestClass &that, const Matrix3D &tm) :
    IntersectionTestClass(that)
{
    OBBoxClass::Transform(tm, that.m_box, &m_box);
    update_bounding_box();
}

inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass(const AABoxIntersectionTestClass &that, const Matrix3D &tm) :
    IntersectionTestClass(that)
{
    Matrix3D::Transform_Vector(tm, that.m_box.m_center, &(m_box.m_center));
    m_box.m_extent = that.m_box.m_extent;
    m_box.m_basis = tm;
    update_bounding_box();
}

inline bool OBBoxIntersectionTestClass::Cull(const Vector3 &cull_min, const Vector3 &cull_max)
{
    Vector3 box_min;
    Vector3::Subtract(m_boundingBox.m_center, m_boundingBox.m_extent, &box_min);

    Vector3 box_max;
    Vector3::Add(m_boundingBox.m_center, m_boundingBox.m_extent, &box_max);

    if ((box_min.X > cull_max.X) || (box_max.X < cull_min.X)) {
        return true;
    }

    if ((box_min.Y > cull_max.Y) || (box_max.Y < cull_min.Y)) {
        return true;
    }

    if ((box_min.Z > cull_max.Z) || (box_max.Z < cull_min.Z)) {
        return true;
    }

    return false;
}

inline bool OBBoxIntersectionTestClass::Cull(const AABoxClass &cull_box)
{
    Vector3 dc;
    Vector3 r;
    Vector3::Subtract(cull_box.m_center, m_boundingBox.m_center, &dc);
    Vector3::Add(cull_box.m_extent, m_boundingBox.m_extent, &r);

    if (GameMath::Fabs(dc.X) > r.X) {
        return true;
    }

    if (GameMath::Fabs(dc.Y) > r.Y) {
        return true;
    }

    if (GameMath::Fabs(dc.Z) > r.Z) {
        return true;
    }

    return false;
}

inline bool OBBoxIntersectionTestClass::Intersect_Triangle(const TriClass &tri)
{
    return CollisionMath::Intersection_Test(m_box, tri);
}

inline void OBBoxIntersectionTestClass::update_bounding_box(void)
{
    m_boundingBox.m_center = m_box.m_center;
    m_box.m_basis.Rotate_AABox_Extent(m_box.m_extent, &m_boundingBox.m_extent);
}
