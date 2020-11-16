/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
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
#include "castres.h"
#include "vector3.h"

class AAPlaneClass;
class PlaneClass;
class LineSegClass;
class TriClass;
class SphereClass;
class AABoxClass;
class OBBoxClass;
class FrustumClass;

const float COLLISION_EPSILON = 0.001f;

class CollisionMath
{
public:
    // Specifies if the object is in the positive space (front/outside) of the volume
    // the negative space (back/inside) of the volume, or both (overlapping)
    enum OverlapType
    {
        POS = 1,
        NEG = 2,
        ON = 4,
        BOTH = 8,

        OUTSIDE = POS,
        INSIDE = NEG,
        OVERLAPPED = BOTH,

        FRONT = POS,
        BACK = NEG
    };

    // Intresection tests
    static bool Intersection_Test(const AABoxClass &box, const TriClass &tri);
    static bool Intersection_Test(const AABoxClass &box, const AABoxClass &box2);
    static bool Intersection_Test(const AABoxClass &box, const OBBoxClass &box2);
    static bool Intersection_Test(const OBBoxClass &box, const TriClass &tri);
    static bool Intersection_Test(const OBBoxClass &box, const AABoxClass &box2);
    static bool Intersection_Test(const OBBoxClass &box, const OBBoxClass &box2);
    static bool Intersection_Test(const SphereClass &sphere, const AABoxClass &box);
    static bool Intersection_Test(const SphereClass &sphere, const OBBoxClass &box);

    // Overlap tests

    // AAPlane tests
    static OverlapType Overlap_Test(const AAPlaneClass &plane, const Vector3 &point);
    static OverlapType Overlap_Test(const AAPlaneClass &plane, const LineSegClass &line);
    static OverlapType Overlap_Test(const AAPlaneClass &plane, const TriClass &tri);
    static OverlapType Overlap_Test(const AAPlaneClass &plane, const SphereClass &sphere);
    static OverlapType Overlap_Test(const AAPlaneClass &plane, const AABoxClass &box);
    static OverlapType Overlap_Test(const AAPlaneClass &plane, const OBBoxClass &box);

    // Plane tests
    static OverlapType Overlap_Test(const PlaneClass &plane, const Vector3 &point);
    static OverlapType Overlap_Test(const PlaneClass &plane, const LineSegClass &line);
    static OverlapType Overlap_Test(const PlaneClass &plane, const TriClass &tri);
    static OverlapType Overlap_Test(const PlaneClass &plane, const SphereClass &sphere);
    static OverlapType Overlap_Test(const PlaneClass &plane, const AABoxClass &box);
    static OverlapType Overlap_Test(const PlaneClass &plane, const OBBoxClass &obbox);

    // Sphere tests
    static OverlapType Overlap_Test(const SphereClass &sphere, const Vector3 &point);
    static OverlapType Overlap_Test(const SphereClass &sphere, const LineSegClass &line);
    static OverlapType Overlap_Test(const SphereClass &sphere, const TriClass &tri);
    static OverlapType Overlap_Test(const SphereClass &sphere, const SphereClass &sphere2);
    static OverlapType Overlap_Test(const SphereClass &sphere, const AABoxClass &aabox);
    static OverlapType Overlap_Test(const SphereClass &sphere, const OBBoxClass &obbox);

    // AABox tests
    static OverlapType Overlap_Test(const AABoxClass &box, const Vector3 &point);
    static OverlapType Overlap_Test(const AABoxClass &box, const LineSegClass &line);
    static OverlapType Overlap_Test(const AABoxClass &box, const TriClass &tri);
    static OverlapType Overlap_Test(const AABoxClass &box, const AABoxClass &box2);
    static OverlapType Overlap_Test(const AABoxClass &box, const OBBoxClass &obbox);
    static OverlapType Overlap_Test(const AABoxClass &box, const SphereClass &sphere);

    // OBBox tests
    static OverlapType Overlap_Test(const OBBoxClass &box, const Vector3 &point);
    static OverlapType Overlap_Test(const OBBoxClass &box, const LineSegClass &line);
    static OverlapType Overlap_Test(const OBBoxClass &box, const TriClass &tri);
    static OverlapType Overlap_Test(const OBBoxClass &box, const AABoxClass &box2);
    static OverlapType Overlap_Test(const OBBoxClass &box, const OBBoxClass &box2);

    // Frustum tests
    static OverlapType Overlap_Test(const FrustumClass &frustum, const Vector3 &point);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const TriClass &tri);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const SphereClass &sphere);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const AABoxClass &box);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const OBBoxClass &obbox);

    static OverlapType Overlap_Test(const FrustumClass &frustum, const AABoxClass &box, int &planes_passed);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const OBBoxClass &box, int &planes_passed);

    // Misc tests
    static OverlapType Overlap_Test(const Vector3 &min, const Vector3 &max, const LineSegClass &line);

    // collision tests

    // Line tests
    static bool Collide(const LineSegClass &line, const AAPlaneClass &plane, CastResultStruct *result);
    static bool Collide(const LineSegClass &line, const PlaneClass &plane, CastResultStruct *result);
    static bool Collide(const LineSegClass &line, const TriClass &tri, CastResultStruct *result);
    static bool Collide(const LineSegClass &line, const SphereClass &sphere, CastResultStruct *result);
    static bool Collide(const LineSegClass &line, const AABoxClass &box, CastResultStruct *result);
    static bool Collide(const LineSegClass &line, const OBBoxClass &box, CastResultStruct *result);

    // AABox tests
    static bool Collide(const AABoxClass &box, const Vector3 &move, const PlaneClass &plane, CastResultStruct *result);
    static bool Collide(const AABoxClass &box, const Vector3 &move, const TriClass &tri, CastResultStruct *result);
    static bool Collide(const AABoxClass &box, const Vector3 &move, const AABoxClass &box2, CastResultStruct *result);
    static bool Collide(
        const AABoxClass &box, const Vector3 &move, const OBBoxClass &box2, const Vector3 &move2, CastResultStruct *result);

    // OBBox tests
    static bool Collide(const OBBoxClass &box, const Vector3 &move, const PlaneClass &plane, CastResultStruct *result);
    static bool Collide(
        const OBBoxClass &box, const Vector3 &move, const TriClass &tri, const Vector3 &move2, CastResultStruct *result);
    static bool Collide(
        const OBBoxClass &box, const Vector3 &move, const AABoxClass &box2, const Vector3 &move2, CastResultStruct *result);
    static bool Collide(
        const OBBoxClass &box, const Vector3 &move, const OBBoxClass &box2, const Vector3 &move2, CastResultStruct *result);

private:
    static OverlapType Eval_Overlap_Mask(int mask);
    static OverlapType Eval_Overlap_Collision(const CastResultStruct &res);

    static const float COINCIDENCE_EPSILON;
};

inline CollisionMath::OverlapType CollisionMath::Eval_Overlap_Mask(int mask)
{
    if (mask == ON) {
        return ON;
    }

    if ((mask & ~(POS | ON)) == 0) {
        return POS;
    }

    if ((mask & ~(NEG | ON)) == 0) {
        return NEG;
    }

    return BOTH;
}

inline CollisionMath::OverlapType CollisionMath::Eval_Overlap_Collision(const CastResultStruct &res)
{
    if (res.fraction < 1.0f) {
        return BOTH;
    } else {
        if (res.start_bad) {
            return NEG;
        } else {
            return POS;
        }
    }
}
