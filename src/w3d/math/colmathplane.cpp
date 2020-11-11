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
#include "colmath.h"
#include "lineseg.h"
#include "plane.h"
#include "tri.h"

inline void Get_Far_Extent(const Vector3 &normal, const Vector3 &extent, Vector3 *pos_far_pt)
{
    if (GameMath::Fast_Is_Float_Positive(normal.X)) {
        pos_far_pt->X = extent.X;
    } else {
        pos_far_pt->X = -extent.X;
    }

    if (GameMath::Fast_Is_Float_Positive(normal.Y)) {
        pos_far_pt->Y = extent.Y;
    } else {
        pos_far_pt->Y = -extent.Y;
    }

    if (GameMath::Fast_Is_Float_Positive(normal.Z)) {
        pos_far_pt->Z = extent.Z;
    } else {
        pos_far_pt->Z = -extent.Z;
    }
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const PlaneClass &plane, const Vector3 &point)
{
    float delta = Vector3::Dot_Product(point, plane.N) - plane.D;
    if (delta > COINCIDENCE_EPSILON) {
        return POS;
    }
    if (delta < -COINCIDENCE_EPSILON) {
        return NEG;
    }
    return ON;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const PlaneClass &plane, const LineSegClass &line)
{
    int mask = 0;
    mask |= CollisionMath::Overlap_Test(plane, line.Get_P0());
    mask |= CollisionMath::Overlap_Test(plane, line.Get_P1());
    return Eval_Overlap_Mask(mask);
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const PlaneClass &plane, const TriClass &tri)
{
    int mask = 0;
    mask |= CollisionMath::Overlap_Test(plane, *tri.V[0]);
    mask |= CollisionMath::Overlap_Test(plane, *tri.V[1]);
    mask |= CollisionMath::Overlap_Test(plane, *tri.V[2]);
    return Eval_Overlap_Mask(mask);
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const PlaneClass &plane, const SphereClass &sphere)
{
    float dot = Vector3::Dot_Product(sphere.Center, plane.N) - plane.D;
    if (dot > sphere.Radius) {
        return OUTSIDE;
    }

    if (dot < -sphere.Radius) {
        return INSIDE;
    }

    return OVERLAPPED;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const PlaneClass &plane, const AABoxClass &box)
{
    Vector3 pos_far_pt;
    Vector3 neg_far_pt;

    Get_Far_Extent(plane.N, box.m_extent, &pos_far_pt);

    neg_far_pt = -pos_far_pt;
    pos_far_pt += box.m_center;
    neg_far_pt += box.m_center;

    if (Overlap_Test(plane, neg_far_pt) == POS) {
        return POS;
    }
    if (Overlap_Test(plane, pos_far_pt) == NEG) {
        return NEG;
    }

    return BOTH;
}
