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
#include "always.h"
#include "castres.h"

class Vector3;
class AAPlaneClass;
class PlaneClass;
class LineSegClass;
class TriClass;
class SphereClass;
class AABoxClass;
class OBBoxClass;
class FrustumClass;

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

    // Frustum tests
    static OverlapType Overlap_Test(const FrustumClass &frustum, const Vector3 &point);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const TriClass &tri);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const SphereClass &sphere);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const AABoxClass &box);
    static OverlapType Overlap_Test(const FrustumClass &frustum, const OBBoxClass &obbox);

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
