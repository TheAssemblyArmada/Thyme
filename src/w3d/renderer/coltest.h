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
#pragma once
#include "always.h"
#include "aabox.h"
#include "castres.h"
#include "colmath.h"
#include "coltype.h"
#include "obbox.h"
#include "vector3.h"

class TriClass;
class RenderObjClass;

class CollisionTestClass
{
public:
    CollisionTestClass(CastResultStruct *res, int collision_type);
    CollisionTestClass(const CollisionTestClass &that);

public:
    CastResultStruct *m_result;
    int m_collisionType;
    RenderObjClass *m_collidedRenderObj;
};

inline CollisionTestClass::CollisionTestClass(CastResultStruct *res, int collision_type) :
    m_result(res), m_collisionType(collision_type), m_collidedRenderObj(nullptr)
{
}

inline CollisionTestClass::CollisionTestClass(const CollisionTestClass &that) :
    m_result(that.m_result), m_collisionType(that.m_collisionType), m_collidedRenderObj(that.m_collidedRenderObj)
{
}

class RayCollisionTestClass : public CollisionTestClass
{
public:
    RayCollisionTestClass(const LineSegClass &ray,
        CastResultStruct *res,
        int collision_type = COLLISION_TYPE_0,
        bool check_alpha = false,
        bool allow_hidden = false);
    RayCollisionTestClass(const RayCollisionTestClass &raytest, const Matrix3D &tm);

    bool Cull(const Vector3 &min, const Vector3 &max);
    bool Cull(const AABoxClass &box);
    bool Cast_To_Triangle(const TriClass &tri);

public:
    LineSegClass m_ray;
    bool m_checkAlpha;
    bool m_allowHidden;

private:
    // not implemented
    RayCollisionTestClass(const RayCollisionTestClass &);
    RayCollisionTestClass &operator=(const RayCollisionTestClass &);
};

inline RayCollisionTestClass::RayCollisionTestClass(
    const LineSegClass &ray, CastResultStruct *res, int collision_type, bool check_alpha, bool allow_hidden) :
    CollisionTestClass(res, collision_type), m_ray(ray), m_checkAlpha(check_alpha), m_allowHidden(allow_hidden)
{
}

inline RayCollisionTestClass::RayCollisionTestClass(const RayCollisionTestClass &raytest, const Matrix3D &tm) :
    CollisionTestClass(raytest), m_ray(raytest.m_ray, tm), m_checkAlpha(raytest.m_checkAlpha)
{
    // #BUGFIX Initialize all members
    m_allowHidden = false;
}

inline bool RayCollisionTestClass::Cull(const Vector3 &min, const Vector3 &max)
{
    return (CollisionMath::Overlap_Test(min, max, m_ray) == CollisionMath::POS);
}

inline bool RayCollisionTestClass::Cull(const AABoxClass &box)
{
    return (CollisionMath::Overlap_Test(box, m_ray) == CollisionMath::POS);
}

inline bool RayCollisionTestClass::Cast_To_Triangle(const TriClass &tri)
{
    return CollisionMath::Collide(m_ray, tri, m_result);
}

class AABoxCollisionTestClass : public CollisionTestClass
{
public:
    AABoxCollisionTestClass(
        const AABoxClass &aabox, const Vector3 &move, CastResultStruct *res, int collision_type = COLLISION_TYPE_0);
    AABoxCollisionTestClass(const AABoxCollisionTestClass &that);

    enum ROTATION_TYPE
    {
        ROTATE_NONE = 0,
        ROTATE_Z90,
        ROTATE_Z180,
        ROTATE_Z270
    };

    bool Cull(const AABoxClass &box);
    bool Cull(const Vector3 &min, const Vector3 &max);
    bool Cast_To_Triangle(const TriClass &tri);

    void Translate(const Vector3 &translation);
    void Rotate(ROTATION_TYPE rotation);
    void Transform(const Matrix3D &tm);

public:
    AABoxClass m_box;
    Vector3 m_move;
    Vector3 m_sweepMin;
    Vector3 m_sweepMax;

private:
    // not implemented
    AABoxCollisionTestClass &operator=(const AABoxCollisionTestClass &);
};

inline void AABoxCollisionTestClass::Translate(const Vector3 &translation)
{
    m_box.m_center += translation;
    m_sweepMin += translation;
    m_sweepMax += translation;
}

inline bool AABoxCollisionTestClass::Cull(const Vector3 &min, const Vector3 &max)
{
    if ((m_sweepMin.X > max.X) || (m_sweepMax.X < min.X)) {
        return true;
    }

    if ((m_sweepMin.Y > max.Y) || (m_sweepMax.Y < min.Y)) {
        return true;
    }

    if ((m_sweepMin.Z > max.Z) || (m_sweepMax.Z < min.Z)) {
        return true;
    }
    return false;
}

inline bool AABoxCollisionTestClass::Cast_To_Triangle(const TriClass &tri)
{
    return CollisionMath::Collide(m_box, m_move, tri, m_result);
}

class OBBoxCollisionTestClass : public CollisionTestClass
{
public:
    OBBoxCollisionTestClass(
        const OBBoxClass &obbox, const Vector3 &move, CastResultStruct *res, int type = COLLISION_TYPE_0);
    OBBoxCollisionTestClass(const OBBoxCollisionTestClass &that);
    OBBoxCollisionTestClass(const OBBoxCollisionTestClass &that, const Matrix3D &tm);
    OBBoxCollisionTestClass(const AABoxCollisionTestClass &that, const Matrix3D &tm);

    bool Cull(const AABoxClass &box);
    bool Cull(const Vector3 &min, const Vector3 &max);
    bool Cast_To_Triangle(const TriClass &tri);

public:
    OBBoxClass m_box;
    Vector3 m_move;
    Vector3 m_sweepMin;
    Vector3 m_sweepMax;

private:
    // not implemented
    OBBoxCollisionTestClass &operator=(const OBBoxCollisionTestClass &);
};

inline bool OBBoxCollisionTestClass::Cull(const Vector3 &min, const Vector3 &max)
{
    if ((m_sweepMin.X > max.X) || (m_sweepMax.X < min.X)) {
        return true;
    }

    if ((m_sweepMin.Y > max.Y) || (m_sweepMax.Y < min.Y)) {
        return true;
    }

    if ((m_sweepMin.Z > max.Z) || (m_sweepMax.Z < min.Z)) {
        return true;
    }
    return false;
}

inline bool OBBoxCollisionTestClass::Cast_To_Triangle(const TriClass &tri)
{
    return CollisionMath::Collide(m_box, m_move, tri, Vector3(0, 0, 0), m_result);
}
