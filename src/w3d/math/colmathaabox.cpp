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
#include <algorithm>
CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass &box, const Vector3 &point)
{
    if (GameMath::Fabs(point.X - box.m_center.X) > box.m_extent.X) {
        return POS;
    }

    if (GameMath::Fabs(point.Y - box.m_center.Y) > box.m_extent.Y) {
        return POS;
    }

    if (GameMath::Fabs(point.Z - box.m_center.Z) > box.m_extent.Z) {
        return POS;
    }

    return NEG;
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass &box, const AABoxClass &box2)
{
    Vector3 dc;
    Vector3::Subtract(box2.m_center, box.m_center, &dc);

    if (box.m_extent.X + box2.m_extent.X < GameMath::Fabs(dc.X)) {
        return POS;
    }

    if (box.m_extent.Y + box2.m_extent.Y < GameMath::Fabs(dc.Y)) {
        return POS;
    }

    if (box.m_extent.Z + box2.m_extent.Z < GameMath::Fabs(dc.Z)) {
        return POS;
    }

    if ((dc.X + box2.m_extent.X <= box.m_extent.X) && (dc.Y + box2.m_extent.Y <= box.m_extent.Y)
        && (dc.Z + box2.m_extent.Z <= box.m_extent.Z) && (dc.X - box2.m_extent.X >= -box.m_extent.X)
        && (dc.Y - box2.m_extent.Y >= -box.m_extent.Y) && (dc.Z - box2.m_extent.Z >= -box.m_extent.Z)) {
        return NEG;
    }

    return BOTH;
}

bool CollisionMath::Intersection_Test(const AABoxClass &box, const AABoxClass &box2)
{
    Vector3 dc = box2.m_center - box.m_center;
    return ((box.m_extent.X + box2.m_extent.X) >= GameMath::Fabs(dc.X))
        && ((box.m_extent.Y + box2.m_extent.Y) >= GameMath::Fabs(dc.Y))
        && ((box.m_extent.Z + box2.m_extent.Z) >= GameMath::Fabs(dc.Z));
}

CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass &box, const LineSegClass &line)
{
    int count = 0;

    if (Overlap_Test(box, line.Get_P0()) == NEG) {
        count++;
    }

    if (Overlap_Test(box, line.Get_P1()) == NEG) {
        count++;
    }

    if (count == 2) {
        return INSIDE;
    }

    if (count == 1) {
        return OVERLAPPED;
    }

    Vector3 v1;
    Vector3::Subtract(line.Get_P0(), box.m_center, &v1);
    float f1 = box.m_extent.X;
    float f2 = v1.X;
    float f3 = line.Get_DP().X;

    if (f2 <= 0.0f) {
        float f4 = -f2;
        float f5 = std::max(f3, 0.0f);

        if (f5 + f1 < f4) {
            return OUTSIDE;
        }
    } else {
        float f5 = std::min(f3, 0.0f);

        if (f1 - f5 < f2) {
            return OUTSIDE;
        }
    }

    f1 = box.m_extent.Y;
    f2 = v1.Y;
    f3 = line.Get_DP().Y;

    if (f2 <= 0.0f) {
        float f4 = -f2;
        float f5 = std::max(f3, 0.0f);

        if (f5 + f1 < f4) {
            return OUTSIDE;
        }
    } else {
        float f5 = std::min(f3, 0.0f);

        if (f1 - f5 < f2) {
            return OUTSIDE;
        }
    }

    f1 = box.m_extent.Z;
    f2 = v1.Z;
    f3 = line.Get_DP().Z;

    if (f2 <= 0.0f) {
        float f4 = -f2;
        float f5 = std::max(f3, 0.0f);

        if (f5 + f1 < f4) {
            return OUTSIDE;
        }
    } else {
        float f5 = std::min(f3, 0.0f);

        if (f1 - f5 < f2) {
            return OUTSIDE;
        }
    }

    Vector3 v2;
    v2.Set(0.0f, -line.Get_Dir().Z, line.Get_Dir().Y);
    f1 = GameMath::Fabs(v2.Z * box.m_extent.Z) + GameMath::Fabs(v2.Y * box.m_extent.Y);
    f2 = Vector3::Dot_Product(v2, v1);

    if (GameMath::Fabs(f2) > f1) {
        return OUTSIDE;
    }

    v2.Set(line.Get_Dir().Z, 0.0f, -line.Get_Dir().X);
    f1 = GameMath::Fabs(v2.X * box.m_extent.X) + GameMath::Fabs(v2.Z * box.m_extent.Z);
    f2 = Vector3::Dot_Product(v2, v1);

    if (GameMath::Fabs(f2) > f1) {
        return OUTSIDE;
    }

    v2.Set(-line.Get_Dir().Y, line.Get_Dir().X, 0.0f);
    f1 = GameMath::Fabs(v2.X * box.m_extent.X) + GameMath::Fabs(v2.Y * box.m_extent.Y);
    f2 = Vector3::Dot_Product(v2, v1);

    if (GameMath::Fabs(f2) > f1) {
        return OUTSIDE;
    }

    return OVERLAPPED;
}

struct AABCollisionStruct
{
    AABCollisionStruct(const AABoxClass &box0, const Vector3 &move, const AABoxClass &box1, const Vector3 &move2) :
        start_bad(true), max_frac(0.0f), axis_id(-1), side(0), box0(box0), box1(box1)
    {
        c = box1.m_center - box0.m_center;
        m = move2 - move;
    }

    bool start_bad;
    float max_frac;
    int axis_id;
    int side;
    Vector3 c;
    Vector3 m;
    const AABoxClass &box0;
    const AABoxClass &box1;
};

static bool AAB_Separation_Test(AABCollisionStruct &context, int axis)
{
    float u1 = context.c[axis];
    float u0 = u1 + context.m[axis];
    float extent = context.box1.m_extent[axis] + context.box0.m_extent[axis];

    if (u1 + GAMEMATH_EPSILON > extent) {
        context.start_bad = false;

        if (u0 > extent) {
            context.max_frac = 1.0f;
            return true;
        } else {
            float frac = ((extent - u1) / (u0 - u1));

            if (frac > context.max_frac) {
                context.max_frac = frac;
                context.axis_id = axis;
                context.side = 1;
            }

            return false;
        }
    } else {
        if (u1 - GAMEMATH_EPSILON < -extent) {
            context.start_bad = false;

            if (u0 < -extent) {
                context.max_frac = 1.0f;
                return true;
            } else {
                float frac = ((-extent - u1) / (u0 - u1));

                if (frac > context.max_frac) {
                    context.max_frac = frac;
                    context.axis_id = axis;
                    context.side = -1;
                }

                return false;
            }
        } else {
            return false;
        }
    }
}

bool CollisionMath::Collide(const AABoxClass &box, const Vector3 &move, const AABoxClass &box2, CastResultStruct *result)
{
    AABCollisionStruct context(box, move, box2, Vector3(0, 0, 0));

    if (!AAB_Separation_Test(context, 0) && !AAB_Separation_Test(context, 1)) {
        AAB_Separation_Test(context, 2);
    }

    if (context.start_bad) {
        result->start_bad = true;
        result->fraction = 0.0f;

        return true;
    }

    if (result->fraction <= context.max_frac) {
        return false;
    }

    result->fraction = context.max_frac;
    result->normal.X = 0.0f;
    result->normal.Y = 0.0f;
    result->normal.Z = 0.0f;
    result->normal[context.axis_id] = (float)(-context.side);

    if (result->compute_contact_point) {
        captainslog_assert(0);
    }

    return true;
}
