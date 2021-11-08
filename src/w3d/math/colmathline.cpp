/**
 * @file
 *
 * @author Jonathan Wilson
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
#include "aabox.h"
#include "colmath.h"
#include "lineseg.h"
#include "obbox.h"
#include "tri.h"

Vector3 g_boxNormal[6] = {
    Vector3(-1, 0, 0), Vector3(1, 0, 0), Vector3(0, -1, 0), Vector3(0, 1, 0), Vector3(0, 0, -1), Vector3(0, 0, 1)
};

CollisionMath::OverlapType CollisionMath::Overlap_Test(const Vector3 &min, const Vector3 &max, const LineSegClass &line)
{
    AABoxClass box;
    box.Init_Min_Max(min, max);
    return CollisionMath::Overlap_Test(box, line);
}

bool CollisionMath::Collide(const LineSegClass &line, const TriClass &tri, CastResultStruct *result)
{
    float den = Vector3::Dot_Product(*tri.N, line.Get_DP()) + COINCIDENCE_EPSILON;
    float s = -(Vector3::Dot_Product(*tri.N, line.Get_P0()) - Vector3::Dot_Product(*tri.N, *tri.V[0])) / den;

    if (!(s >= 0.0f && s <= 1.0f && s <= result->fraction)) {
        return false;
    }

    Vector3 contact_point = (s * line.Get_DP()) + line.Get_P0();
    if (!tri.Contains_Point(contact_point)) {
        return false;
    }

    result->fraction = s;
    result->normal = den > 0 ? -*tri.N : *tri.N;

    if (result->compute_contact_point) {
        result->contact_point = contact_point;
    }

    return true;
}

struct BoxTestStruct
{
    enum
    {
        BOX_SIDE_BOTTOM = 0,
        BOX_SIDE_TOP,
        BOX_SIDE_MIDDLE
    };

    Vector3 min;
    Vector3 max;
    Vector3 p0;
    Vector3 dp;
    float fraction;
    bool inside;
    int axis;
    int side;
};

static bool Test_Aligned_Box(BoxTestStruct *test)
{
    int collisionSides[3];
    float closestCorners[3];
    float fractions[3];

    bool startedBad = true;

    for (int x = 0; x < 3; x++) {
        if (test->p0[x] >= test->min[x]) {
            if (test->p0[x] <= test->max[x]) {
                collisionSides[x] = BoxTestStruct::BOX_SIDE_MIDDLE;
            } else {
                collisionSides[x] = BoxTestStruct::BOX_SIDE_TOP;
                closestCorners[x] = test->max[x];
                startedBad = false;
            }
        } else {
            collisionSides[x] = BoxTestStruct::BOX_SIDE_BOTTOM;
            closestCorners[x] = test->min[x];
            startedBad = false;
        }
    }

    if (startedBad) {
        test->fraction = 0.0f;
        test->inside = true;
        return true;
    }

    for (int x = 0; x < 3; x++) {
        if (collisionSides[x] == BoxTestStruct::BOX_SIDE_MIDDLE || test->dp[x] == 0.0f) {
            fractions[x] = -1.0f;
        } else {
            fractions[x] = ((closestCorners[x] - test->p0[x]) / test->dp[x]);
        }
    }

    int biggestFractionIndex = 0;

    for (int x = 1; x < 3; x++) {
        if (fractions[x] > fractions[biggestFractionIndex]) {
            biggestFractionIndex = x;
        }
    }

    if (fractions[biggestFractionIndex] < 0.0f) {
        return false;
    } else {
        Vector3 contactPoint;

        for (int x = 0; x < 3; x++) {
            if (biggestFractionIndex == x) {
                contactPoint[x] = closestCorners[x];
            } else {
                contactPoint[x] = (fractions[biggestFractionIndex] * test->dp[x] + test->p0[x]);
                if (test->min[x] > contactPoint[x] || contactPoint[x] > test->max[x]) {
                    return false;
                }
            }
        }

        test->fraction = fractions[biggestFractionIndex];
        test->inside = false;
        test->axis = biggestFractionIndex;
        test->side = collisionSides[biggestFractionIndex];

        return true;
    }
}

bool CollisionMath::Collide(const LineSegClass &line, const AABoxClass &box, CastResultStruct *result)
{
    BoxTestStruct test;
    test.min = box.m_center - box.m_extent;
    test.max = box.m_center + box.m_extent;
    test.p0 = line.Get_P0();
    test.dp = line.Get_DP();

    if (!Test_Aligned_Box(&test)) {
        return false;
    }

    if (test.inside) {
        result->start_bad = true;
        return true;
    }

    if (result->fraction <= test.fraction) {
        return false;
    }

    result->fraction = test.fraction;

    captainslog_assert(test.side != BoxTestStruct::BOX_SIDE_MIDDLE);

    result->normal = g_boxNormal[test.side + 2 * test.axis];

    if (result->compute_contact_point) {
        result->contact_point.X = (result->fraction * line.Get_DP().X + line.Get_P0().X);
        result->contact_point.Y = (result->fraction * line.Get_DP().Y + line.Get_P0().Y);
        result->contact_point.Z = (result->fraction * line.Get_DP().Z + line.Get_P0().Z);
    }

    return true;
}

bool CollisionMath::Collide(const LineSegClass &line, const OBBoxClass &box, CastResultStruct *result)
{
    BoxTestStruct test;
    test.min = box.m_center - box.m_extent;
    test.max = box.m_center + box.m_extent;

    Vector3 lineStartBoxOffset = line.Get_P0() - box.m_center;

    test.p0.X = (lineStartBoxOffset.X * box.m_basis[0].X + box.m_basis[1].X * lineStartBoxOffset.Y
        + box.m_basis[2].X * lineStartBoxOffset.Z + box.m_center.X);
    test.p0.Y = (lineStartBoxOffset.Y * box.m_basis[1].Y + box.m_basis[0].Y * lineStartBoxOffset.X
        + box.m_basis[2].Y * lineStartBoxOffset.Z + box.m_center.Y);
    test.p0.Z = (lineStartBoxOffset.X * box.m_basis[0].Z + box.m_basis[1].Z * lineStartBoxOffset.Y
        + box.m_basis[2].Z * lineStartBoxOffset.Z + box.m_center.Z);

    test.dp.X =
        (box.m_basis[0].X * line.Get_DP().X + box.m_basis[1].X * line.Get_DP().Y + box.m_basis[2].X * line.Get_DP().Z);
    test.dp.Y =
        (box.m_basis[0].Y * line.Get_DP().X + box.m_basis[1].Y * line.Get_DP().Y + box.m_basis[2].Y * line.Get_DP().Z);
    test.dp.Z =
        (box.m_basis[0].Z * line.Get_DP().X + box.m_basis[1].Z * line.Get_DP().Y + box.m_basis[2].Z * line.Get_DP().Z);

    if (!Test_Aligned_Box(&test)) {
        return false;
    }

    if (test.inside) {
        result->start_bad = true;

        return true;
    }

    if (result->fraction <= test.fraction) {
        return false;
    }

    result->fraction = test.fraction;

    captainslog_assert(test.side != BoxTestStruct::BOX_SIDE_MIDDLE);

    if (test.axis == 0) {
        result->normal.X = box.m_basis[0].X;
        result->normal.Y = box.m_basis[1].X;
        result->normal.Z = box.m_basis[2].X;
    } else if (test.axis == 1) {
        result->normal.X = box.m_basis[0].Y;
        result->normal.Y = box.m_basis[1].Y;
        result->normal.Z = box.m_basis[2].Y;
    } else if (test.axis == 2) {
        result->normal.X = box.m_basis[0].Z;
        result->normal.Y = box.m_basis[1].Z;
        result->normal.Z = box.m_basis[2].Z;
    }

    if (test.side == BoxTestStruct::BOX_SIDE_BOTTOM) {
        result->normal = -result->normal;
    }

    if (result->compute_contact_point) {
        result->contact_point.X = (result->fraction * line.Get_DP().X + line.Get_P0().X);
        result->contact_point.Y = (result->fraction * line.Get_DP().Y + line.Get_P0().Y);
        result->contact_point.Z = (result->fraction * line.Get_DP().Z + line.Get_P0().Z);
    }

    return true;
}
