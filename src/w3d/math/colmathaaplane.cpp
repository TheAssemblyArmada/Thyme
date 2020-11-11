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
#include "aabox.h"
#include "aaplane.h"
#include "colmath.h"
#include "sphere.h"
#include "tri.h"

CollisionMath::OverlapType CollisionMath::Overlap_Test(const AAPlaneClass &plane, const Vector3 &point)
{
    float delta = point[plane.m_normal] - plane.m_dist;
    if (delta > COINCIDENCE_EPSILON) {
        return POS;
    }
    if (delta < -COINCIDENCE_EPSILON) {
        return NEG;
    }
    return ON;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const AAPlaneClass &plane, const LineSegClass &line)
{
    int mask = 0;
    mask |= CollisionMath::Overlap_Test(plane, line.Get_P0());
    mask |= CollisionMath::Overlap_Test(plane, line.Get_P1());
    return Eval_Overlap_Mask(mask);
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const AAPlaneClass &plane, const TriClass &tri)
{
    int mask = 0;
    mask |= CollisionMath::Overlap_Test(plane, *tri.V[0]);
    mask |= CollisionMath::Overlap_Test(plane, *tri.V[1]);
    mask |= CollisionMath::Overlap_Test(plane, *tri.V[2]);
    return Eval_Overlap_Mask(mask);
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const AAPlaneClass &plane, const SphereClass &sphere)
{
    float dot = sphere.Center[plane.m_normal] - plane.m_dist;
    if (dot > sphere.Radius) {
        return OUTSIDE;
    }

    if (dot < -sphere.Radius) {
        return INSIDE;
    }

    return OVERLAPPED;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const AAPlaneClass &plane, const AABoxClass &box)
{
    int mask = 0;

    float delta = box.m_center[plane.m_normal] - box.m_extent[plane.m_normal] - plane.m_dist;

    if (delta > GAMEMATH_EPSILON) {
        mask |= POS;
    } else if (delta < -GAMEMATH_EPSILON) {
        mask |= NEG;
    } else {
        mask |= ON;
    }

    delta = box.m_center[plane.m_normal] + box.m_extent[plane.m_normal] - plane.m_dist;

    if (delta > GAMEMATH_EPSILON) {
        mask |= POS;
    } else if (delta < -GAMEMATH_EPSILON) {
        mask |= NEG;
    } else {
        mask |= ON;
    }
    return Eval_Overlap_Mask(mask);
}
