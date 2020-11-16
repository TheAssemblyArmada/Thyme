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
#include "castres.h"
#include "colmath.h"
#include "obbox.h"
#include "vector3.h"

struct ObbIntersectionStruct
{
    Vector3 c;
    Vector3 a[3];
    Vector3 b[3];
    float ab[3][3];
    const OBBoxClass &box0;
    const OBBoxClass &box1;
    ObbIntersectionStruct(const OBBoxClass &b0, const OBBoxClass &b1) : ab{}, box0(b0), box1(b1)
    {
        c = b1.m_center - b0.m_center;
        a[0].X = b0.m_basis[0].X;
        a[0].Y = b0.m_basis[1].X;
        a[0].Z = b0.m_basis[2].X;
        a[1].X = b0.m_basis[0].Y;
        a[1].Y = b0.m_basis[1].Y;
        a[1].Z = b0.m_basis[2].Y;
        a[2].X = b0.m_basis[0].Z;
        a[2].Y = b0.m_basis[1].Z;
        a[2].Z = b0.m_basis[2].Z;
        b[0].X = b1.m_basis[0].X;
        b[0].Y = b1.m_basis[1].X;
        b[0].Z = b1.m_basis[2].X;
        b[1].X = b1.m_basis[0].Y;
        b[1].Y = b1.m_basis[1].Y;
        b[1].Z = b1.m_basis[2].Y;
        b[2].X = b1.m_basis[0].Z;
        b[2].Y = b1.m_basis[1].Z;
        b[2].Z = b1.m_basis[2].Z;
    }
};

static bool OBB_Intersect_Box0_Basis(ObbIntersectionStruct &context, int axis)
{
    float f1 = GameMath::Fabs(context.box1.m_extent.X * context.ab[axis][0])
        + GameMath::Fabs(context.box1.m_extent.Y * context.ab[axis][1])
        + GameMath::Fabs(context.box1.m_extent.Z * context.ab[axis][2]) + context.box0.m_extent[axis];

    float f2 = Vector3::Dot_Product(context.c, context.a[axis]);

    return f2 > f1 || -f1 > f2;
}

static bool OBB_Intersect_Box1_Basis(ObbIntersectionStruct &context, int axis)
{
    float f1 = GameMath::Fabs(context.box0.m_extent.X * context.ab[0][axis])
        + GameMath::Fabs(context.box0.m_extent.Y * context.ab[1][axis])
        + GameMath::Fabs(context.box0.m_extent.Z * context.ab[2][axis]) + context.box1.m_extent[axis];

    float f2 = Vector3::Dot_Product(context.c, context.b[axis]);

    return f2 > f1 || -f1 > f2;
}

static bool OBB_Intersect_Unk(Vector3 &axis, Vector3 &c, float f1, float f2)
{
    float f3 = f1 + f2;
    float f4 = Vector3::Dot_Product(c, axis);
    return f4 > f3 || f4 < -f3;
}

static bool Intersect_OBB_OBB(ObbIntersectionStruct &context)
{
    context.ab[0][0] = Vector3::Dot_Product(context.a[0], context.b[0]);
    context.ab[0][1] = Vector3::Dot_Product(context.a[0], context.b[1]);
    context.ab[0][2] = Vector3::Dot_Product(context.a[0], context.b[2]);

    if (context.box0.m_extent.X > 0.0f && OBB_Intersect_Box0_Basis(context, 0)) {
        return false;
    }

    context.ab[1][0] = Vector3::Dot_Product(context.a[1], context.b[0]);
    context.ab[1][1] = Vector3::Dot_Product(context.a[1], context.b[1]);
    context.ab[1][2] = Vector3::Dot_Product(context.a[1], context.b[2]);

    if (context.box0.m_extent.Y > 0.0f && OBB_Intersect_Box0_Basis(context, 1)) {
        return false;
    }

    context.ab[2][0] = Vector3::Dot_Product(context.a[2], context.b[0]);
    context.ab[2][1] = Vector3::Dot_Product(context.a[2], context.b[1]);
    context.ab[2][2] = Vector3::Dot_Product(context.a[2], context.b[2]);

    if (context.box0.m_extent.Z > 0.0f && OBB_Intersect_Box0_Basis(context, 2)) {
        return false;
    }

    if (context.box1.m_extent.X > 0.0f && OBB_Intersect_Box1_Basis(context, 0)) {
        return false;
    }

    if (context.box1.m_extent.Y > 0.0f && OBB_Intersect_Box1_Basis(context, 1)) {
        return false;
    }

    if (context.box1.m_extent.Z > 0.0f && OBB_Intersect_Box1_Basis(context, 2)) {
        return false;
    }

    Vector3 axis = Vector3::Cross_Product(context.a[0], context.b[0]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.Y * context.ab[0][2])
            + GameMath::Fabs(context.box1.m_extent.Z * context.ab[0][1]);
        float f1 = GameMath::Fabs(context.box0.m_extent.Y * context.ab[2][0])
            + GameMath::Fabs(context.box0.m_extent.Z * context.ab[1][0]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[0], context.b[1]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.X * context.ab[0][2])
            + GameMath::Fabs(context.box1.m_extent.Z * context.ab[0][0]);
        float f1 = GameMath::Fabs(context.box0.m_extent.Y * context.ab[2][1])
            + GameMath::Fabs(context.box0.m_extent.Z * context.ab[1][1]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[0], context.b[2]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.X * context.ab[0][1])
            + GameMath::Fabs(context.box1.m_extent.Y * context.ab[0][0]);
        float f1 = GameMath::Fabs(context.box0.m_extent.Y * context.ab[2][2])
            + GameMath::Fabs(context.box0.m_extent.Z * context.ab[1][2]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[1], context.b[0]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.Y * context.ab[1][2])
            + GameMath::Fabs(context.box1.m_extent.Z * context.ab[1][1]);
        float f1 = GameMath::Fabs(context.box0.m_extent.X * context.ab[2][0])
            + GameMath::Fabs(context.box0.m_extent.Z * context.ab[0][0]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[1], context.b[1]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.X * context.ab[1][2])
            + GameMath::Fabs(context.box1.m_extent.Z * context.ab[1][0]);
        float f1 = GameMath::Fabs(context.box0.m_extent.X * context.ab[2][1])
            + GameMath::Fabs(context.box0.m_extent.Z * context.ab[0][1]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[1], context.b[2]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.X * context.ab[1][1])
            + GameMath::Fabs(context.box1.m_extent.Y * context.ab[1][0]);
        float f1 = GameMath::Fabs(context.box0.m_extent.X * context.ab[2][2])
            + GameMath::Fabs(context.box0.m_extent.Z * context.ab[0][2]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[2], context.b[0]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.Y * context.ab[2][2])
            + GameMath::Fabs(context.box1.m_extent.Z * context.ab[2][1]);
        float f1 = GameMath::Fabs(context.box0.m_extent.X * context.ab[1][0])
            + GameMath::Fabs(context.box0.m_extent.Y * context.ab[0][0]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[2], context.b[1]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.X * context.ab[2][2])
            + GameMath::Fabs(context.box1.m_extent.Z * context.ab[2][0]);
        float f1 = GameMath::Fabs(context.box0.m_extent.X * context.ab[1][1])
            + GameMath::Fabs(context.box0.m_extent.Y * context.ab[0][1]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    axis = Vector3::Cross_Product(context.a[2], context.b[2]);

    if (axis.Length2() > GAMEMATH_EPSILON2) {
        float f2 = GameMath::Fabs(context.box1.m_extent.X * context.ab[2][1])
            + GameMath::Fabs(context.box1.m_extent.Y * context.ab[2][0]);
        float f1 = GameMath::Fabs(context.box0.m_extent.X * context.ab[1][2])
            + GameMath::Fabs(context.box0.m_extent.Y * context.ab[0][2]);

        if (OBB_Intersect_Unk(axis, context.c, f1, f2)) {
            return false;
        }
    }

    return true;
}

bool CollisionMath::Intersection_Test(const OBBoxClass &box, const AABoxClass &box2)
{
    const OBBoxClass obbox(box2.m_center, box2.m_extent);
    ObbIntersectionStruct context(box, obbox);
    return Intersect_OBB_OBB(context);
}

bool CollisionMath::Intersection_Test(const OBBoxClass &box, const OBBoxClass &box2)
{
    ObbIntersectionStruct context(box, box2);
    return Intersect_OBB_OBB(context);
}

bool CollisionMath::Intersection_Test(const AABoxClass &box, const OBBoxClass &box2)
{
    const OBBoxClass obbox(box.m_center, box.m_extent);
    ObbIntersectionStruct context(obbox, box2);
    return Intersect_OBB_OBB(context);
}

struct ObbCollisionStruct
{
    bool start_bad;
    float max_frac;
    int axis_id;
    int side;
    int test_axis_id;
    Vector3 test_axis;
    Vector3 c;
    Vector3 m;
    Vector3 a[3];
    Vector3 b[3];
    float ab[3][3];
    const OBBoxClass &box0;
    const Vector3 &move0;
    const OBBoxClass &box1;
    const Vector3 &move1;
    ObbCollisionStruct(const OBBoxClass &box, const Vector3 &move, const OBBoxClass &box2, const Vector3 &move2) :
        start_bad(true),
        max_frac(0),
        axis_id(0),
        side(0),
        test_axis_id(0),
        ab{},
        box0(box),
        move0(move),
        box1(box2),
        move1(move2)
    {
        c = box2.m_center - box.m_center;
        m = move2 - move;
        a[0].X = box.m_basis[0].X;
        a[0].Y = box.m_basis[1].X;
        a[0].Z = box.m_basis[2].X;
        a[1].X = box.m_basis[0].Y;
        a[1].Y = box.m_basis[1].Y;
        a[1].Z = box.m_basis[2].Y;
        a[2].X = box.m_basis[0].Z;
        a[2].Y = box.m_basis[1].Z;
        a[2].Z = box.m_basis[2].Z;
        b[0].X = box2.m_basis[0].X;
        b[0].Y = box2.m_basis[1].X;
        b[0].Z = box2.m_basis[2].X;
        b[1].X = box2.m_basis[0].Y;
        b[1].Y = box2.m_basis[1].Y;
        b[1].Z = box2.m_basis[2].Y;
        b[2].X = box2.m_basis[0].Z;
        b[2].Y = box2.m_basis[1].Z;
        b[2].Z = box2.m_basis[2].Z;
    }
};

static bool OBB_Separation_Test(ObbCollisionStruct &context, float ra, float rb, float u0, float u1)
{
    float f1 = rb + ra;

    if (u0 + GAMEMATH_EPSILON > f1) {
        context.start_bad = false;

        if (u1 <= f1) {
            float f2 = u1 - u0;

            if (GameMath::Fabs(f2) > 0.0f) {
                float f3 = ((f1 - u0) / f2);

                if (f3 > context.max_frac) {
                    context.max_frac = f3;
                    context.axis_id = context.test_axis_id;
                    context.side = 1;
                }
            }

            return false;
        } else {
            context.max_frac = 1.0f;
            return true;
        }
    } else if (u0 - GAMEMATH_EPSILON < -f1) {
        context.start_bad = false;

        if (u1 < -f1) {
            context.max_frac = 1.0f;
            return true;
        } else {
            float f2 = u1 - u0;

            if (GameMath::Fabs(f2) > 0.0f) {
                float f3 = ((-f1 - u0) / f2);
                if (f3 > context.max_frac) {
                    context.max_frac = f3;
                    context.axis_id = context.test_axis_id;
                    context.side = -1;
                }
            }

            return false;
        }
    } else {
        return false;
    }
}

static bool OBB_Check_Box0_Basis(ObbCollisionStruct &context, int axis)
{
    float rb = GameMath::Fabs(context.box1.m_extent.X * context.ab[axis][0])
        + GameMath::Fabs(context.box1.m_extent.Y * context.ab[axis][1])
        + GameMath::Fabs(context.box1.m_extent.Z * context.ab[axis][2]);

    float u0 = Vector3::Dot_Product(context.c, context.a[axis]);
    float u1 = Vector3::Dot_Product(context.m, context.a[axis]) + u0;

    return OBB_Separation_Test(context, context.box0.m_extent[axis], rb, u0, u1);
}

static bool OBB_Check_Box1_Basis(ObbCollisionStruct &context, int axis)
{
    float ra = GameMath::Fabs(context.box0.m_extent.X * context.ab[0][axis])
        + GameMath::Fabs(context.box0.m_extent.Y * context.ab[1][axis])
        + GameMath::Fabs(context.box0.m_extent.Z * context.ab[2][axis]);

    float u0 = Vector3::Dot_Product(context.c, context.b[axis]);
    float u1 = Vector3::Dot_Product(context.m, context.b[axis]) + u0;

    return OBB_Separation_Test(context, ra, context.box1.m_extent[axis], u0, u1);
}

static bool OBB_Check_Unk(ObbCollisionStruct &context, float ra, float rb)
{
    float u0 = Vector3::Dot_Product(context.c, context.test_axis);
    float u1 = Vector3::Dot_Product(context.m, context.test_axis) + u0;
    return OBB_Separation_Test(context, ra, rb, u0, u1);
}

static void OBB_Collide_Get_Data(ObbCollisionStruct &context, float *ra, float *rb)
{
    *ra = GameMath::Fabs(Vector3::Dot_Product(context.a[0], context.test_axis)) * context.box0.m_extent.X
        + GameMath::Fabs(Vector3::Dot_Product(context.a[1], context.test_axis)) * context.box0.m_extent.Y
        + GameMath::Fabs(Vector3::Dot_Product(context.a[2], context.test_axis)) * context.box0.m_extent.Z;
    *rb = GameMath::Fabs(Vector3::Dot_Product(context.b[0], context.test_axis)) * context.box1.m_extent.X
        + GameMath::Fabs(Vector3::Dot_Product(context.b[1], context.test_axis)) * context.box1.m_extent.Y
        + GameMath::Fabs(Vector3::Dot_Product(context.b[2], context.test_axis)) * context.box1.m_extent.Z;
}

static void Compute_Contact_Normal(ObbCollisionStruct &context, CastResultStruct *result)
{
    switch (context.axis_id) {
        case 0:
        case 1:
            result->normal = context.a[0];

            break;
        case 2:
            result->normal = context.a[1];

            break;
        case 3:
            result->normal = context.a[2];

            break;
        case 4:
            result->normal = context.b[0];

            break;
        case 5:
            result->normal = context.b[1];

            break;
        case 6:
            result->normal = context.b[2];

            break;
        case 7:
            result->normal = Vector3::Cross_Product(context.a[0], context.b[0]);
            result->normal.Normalize();

            break;
        case 8:
            result->normal = Vector3::Cross_Product(context.a[0], context.b[1]);
            result->normal.Normalize();

            break;
        case 9:
            result->normal = Vector3::Cross_Product(context.a[0], context.b[2]);
            result->normal.Normalize();

            break;
        case 10:
            result->normal = Vector3::Cross_Product(context.a[1], context.b[0]);
            result->normal.Normalize();

            break;
        case 11:
            result->normal = Vector3::Cross_Product(context.a[1], context.b[1]);
            result->normal.Normalize();

            break;
        case 12:
            result->normal = Vector3::Cross_Product(context.a[1], context.b[2]);
            result->normal.Normalize();

            break;
        case 13:
            result->normal = Vector3::Cross_Product(context.a[2], context.b[0]);
            result->normal.Normalize();

            break;
        case 14:
            result->normal = Vector3::Cross_Product(context.a[2], context.b[1]);
            result->normal.Normalize();

            break;
        case 15:
            result->normal = Vector3::Cross_Product(context.a[2], context.b[2]);
            result->normal.Normalize();

            break;
    }
    result->normal *= (float)(-context.side);
}

static float Eval_Side(float val, float side)
{
    if (val > 0.0f) {
        return side;
    } else if (val < 0.0f) {
        return -side;
    } else {
        return 0.0f;
    }
}

static void Compute_Contact_Point(ObbCollisionStruct &context, CastResultStruct *result)
{
    Vector3 vec1(0, 0, 0);
    if (context.axis_id >= 7) {
        Vector3 v1;
        Vector3::Add(context.max_frac * context.move0, context.box0.m_center, &v1);
        Vector3 v2;
        Vector3::Add(context.max_frac * context.move1, context.box1.m_center, &v2);
        vec1 = v2 - v1;
    }

    Vector3 vec(0, 0, 0);

    switch (context.axis_id) {
        case 1:
        case 2:
        case 3: {
            float y[3];
            for (int i = 0; i < 3; i++) {
                y[i] = Eval_Side(context.ab[context.axis_id - 1][i], (float)context.side);
            }
            context.box1.Compute_Point(y, &result->contact_point);
            result->contact_point += result->fraction * context.move1;

            return;
        }
        case 4:
        case 5:
        case 6: {
            float x[3];
            for (int i = 0; i < 3; i++) {
                x[i] = Eval_Side(context.ab[context.axis_id - 4][i], (float)context.side);
            }

            context.box0.Compute_Point(x, &result->contact_point);
            result->contact_point += result->fraction * context.move0;

            return;
        }
        case 7: {
            float f4 = Eval_Side(context.ab[2][0], (float)context.side);
            float f5 = Eval_Side(context.ab[1][0], (float)context.side);
            float f6 = Eval_Side(context.ab[0][2], (float)context.side);
            float f7 = Eval_Side(context.ab[0][1], (float)context.side);

            vec.Y = (-f4 * context.box0.m_extent.Y);
            vec.Z = (f5 * context.box0.m_extent.Z);

            float f8 = (1.0f - context.ab[0][0] * context.ab[0][0]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.X = 0.0f;
            } else {
                vec.X = Vector3::Dot_Product(context.a[0], vec1);
                vec.X += (Vector3::Dot_Product(-context.b[0], vec1) + context.ab[1][0] * vec.Y + context.ab[2][0] * vec.Z)
                    * context.ab[0][0];
                vec.X += context.ab[0][1] * (-f6 * context.box1.m_extent.Y);
                vec.X += context.ab[0][2] * (f7 * context.box1.m_extent.Z);
                vec.X /= f8;
            }

            break;
        }
        case 8: {
            float f4 = Eval_Side(context.ab[2][1], (float)context.side);
            float f5 = Eval_Side(context.ab[1][1], (float)context.side);
            float f6 = Eval_Side(context.ab[0][2], (float)context.side);
            float f7 = Eval_Side(context.ab[0][0], (float)context.side);

            vec.Y = (-f4 * context.box0.m_extent.Y);
            vec.Z = (f5 * context.box0.m_extent.Z);

            float f8 = (1.0f - context.ab[0][1] * context.ab[0][1]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.X = 0.0f;
            } else {
                vec.X = Vector3::Dot_Product(context.a[0], vec1);
                vec.X += (Vector3::Dot_Product(-context.b[1], vec1) + context.ab[1][1] * vec.Y + context.ab[2][1] * vec.Z)
                    * context.ab[0][1];
                vec.X += context.ab[0][0] * (f6 * context.box1.m_extent.X);
                vec.X += context.ab[0][2] * (-f7 * context.box1.m_extent.Z);
                vec.X /= f8;
            }

            break;
        }
        case 9: {
            float f4 = Eval_Side(context.ab[2][2], (float)context.side);
            float f5 = Eval_Side(context.ab[1][2], (float)context.side);
            float f6 = Eval_Side(context.ab[0][1], (float)context.side);
            float f7 = Eval_Side(context.ab[0][0], (float)context.side);

            vec.Y = (-f4 * context.box0.m_extent.Y);
            vec.Z = (f5 * context.box0.m_extent.Z);

            float f8 = (1.0f - context.ab[0][2] * context.ab[0][2]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.X = 0.0f;
            } else {
                vec.X = Vector3::Dot_Product(context.a[0], vec1);
                vec.X += (Vector3::Dot_Product(-context.b[2], vec1) + context.ab[1][2] * vec.Y + context.ab[2][2] * vec.Z)
                    * context.ab[0][2];
                vec.X += context.ab[0][0] * (-f6 * context.box1.m_extent.X);
                vec.X += context.ab[0][1] * (f7 * context.box1.m_extent.Y);
                vec.X /= f8;
            }

            break;
        }
        case 10: {
            float f4 = Eval_Side(context.ab[2][0], (float)context.side);
            float f5 = Eval_Side(context.ab[0][0], (float)context.side);
            float f6 = Eval_Side(context.ab[1][2], (float)context.side);
            float f7 = Eval_Side(context.ab[1][1], (float)context.side);

            vec.X = (f4 * context.box0.m_extent.X);
            vec.Z = (-f5 * context.box0.m_extent.Z);

            float f8 = (1.0f - context.ab[1][0] * context.ab[1][0]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.Y = 0.0f;
            } else {
                vec.Y = Vector3::Dot_Product(context.a[1], vec1);
                vec.Y += (Vector3::Dot_Product(-context.b[0], vec1) + context.ab[0][0] * vec.X + context.ab[2][0] * vec.Z)
                    * context.ab[1][0];
                vec.Y += context.ab[1][1] * (-f6 * context.box1.m_extent.Y);
                vec.Y += context.ab[1][2] * (f7 * context.box1.m_extent.Z);
                vec.Y /= f8;
            }

            break;
        }
        case 11: {
            float f4 = Eval_Side(context.ab[2][1], (float)context.side);
            float f5 = Eval_Side(context.ab[0][1], (float)context.side);
            float f6 = Eval_Side(context.ab[1][2], (float)context.side);
            float f7 = Eval_Side(context.ab[1][0], (float)context.side);

            vec.X = (f4 * context.box0.m_extent.X);
            vec.Z = (-f5 * context.box0.m_extent.Z);

            float f8 = (1.0f / (1.0f - context.ab[1][1] * context.ab[1][1]));
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.Y = 0.0f;
            } else {
                vec.Y = Vector3::Dot_Product(context.a[1], vec1);
                vec.Y += (Vector3::Dot_Product(-context.b[1], vec1) + context.ab[0][1] * vec.X + context.ab[2][1] * vec.Z)
                    * context.ab[1][1];
                vec.Y += context.ab[1][0] * (f6 * context.box1.m_extent.X);
                vec.Y += context.ab[1][2] * (-f7 * context.box1.m_extent.Z);
                vec.Y /= f8;
            }

            break;
        }
        case 12: {
            float f4 = Eval_Side(context.ab[2][2], (float)context.side);
            float f5 = Eval_Side(context.ab[0][2], (float)context.side);
            float f6 = Eval_Side(context.ab[1][1], (float)context.side);
            float f7 = Eval_Side(context.ab[1][0], (float)context.side);

            vec.X = (f4 * context.box0.m_extent.X);
            vec.Z = (-f5 * context.box0.m_extent.Z);

            float f8 = (1.0f - context.ab[1][2] * context.ab[1][2]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.Y = 0.0f;
            } else {
                vec.Y = Vector3::Dot_Product(context.a[1], vec1);
                vec.Y += (Vector3::Dot_Product(-context.b[2], vec1) + context.ab[0][2] * vec.X + context.ab[2][2] * vec.Z)
                    * context.ab[1][2];
                vec.Y += context.ab[1][0] * (-f6 * context.box1.m_extent.X);
                vec.Y += context.ab[1][1] * (f7 * context.box1.m_extent.Y);
                vec.Y /= f8;
            }

            break;
        }
        case 13: {
            float f4 = Eval_Side(context.ab[1][0], (float)context.side);
            float f5 = Eval_Side(context.ab[0][0], (float)context.side);
            float f6 = Eval_Side(context.ab[2][2], (float)context.side);
            float f7 = Eval_Side(context.ab[2][1], (float)context.side);

            vec.X = (-f4 * context.box0.m_extent.X);
            vec.Y = (f5 * context.box0.m_extent.Y);

            float f8 = (1.0f - context.ab[2][0] * context.ab[2][0]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.Z = 0.0f;
            } else {
                vec.Z = Vector3::Dot_Product(context.a[2], vec1);
                vec.Z += (Vector3::Dot_Product(-context.b[0], vec1) + context.ab[0][0] * vec.X + context.ab[1][0] * vec.Y)
                    * context.ab[2][0];
                vec.Z += context.ab[2][1] * (-f6 * context.box1.m_extent.Y);
                vec.Z += context.ab[2][2] * (f7 * context.box1.m_extent.Z);
                vec.Z /= f8;
            }

            break;
        }
        case 14: {
            float f4 = Eval_Side(context.ab[1][1], (float)context.side);
            float f5 = Eval_Side(context.ab[0][1], (float)context.side);
            float f6 = Eval_Side(context.ab[2][2], (float)context.side);
            float f7 = Eval_Side(context.ab[2][0], (float)context.side);

            vec.X = (-f4 * context.box0.m_extent.X);
            vec.Y = (f5 * context.box0.m_extent.Y);

            float f8 = (1.0f - context.ab[2][1] * context.ab[2][1]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.Z = 0.0f;
            } else {
                vec.Z = Vector3::Dot_Product(context.a[2], vec1);
                vec.Z += (Vector3::Dot_Product(-context.b[1], vec1) + context.ab[0][1] * vec.X + context.ab[1][1] * vec.Y)
                    * context.ab[2][1];
                vec.Z += context.ab[2][0] * (f6 * context.box1.m_extent.X);
                vec.Z += context.ab[2][2] * (-f7 * context.box1.m_extent.Z);
                vec.Z /= f8;
            }

            break;
        }
        case 15: {
            float f4 = Eval_Side(context.ab[1][2], (float)context.side);
            float f5 = Eval_Side(context.ab[0][2], (float)context.side);
            float f6 = Eval_Side(context.ab[2][1], (float)context.side);
            float f7 = Eval_Side(context.ab[2][0], (float)context.side);

            vec.X = (-f4 * context.box0.m_extent.X);
            vec.Y = (f5 * context.box0.m_extent.Y);

            float f8 = (1.0f - context.ab[2][2] * context.ab[2][2]);
            if (GameMath::Fabs(f8) <= 0.0f) {
                vec.Z = 0.0f;
            } else {
                vec.Z = Vector3::Dot_Product(context.a[2], vec1);
                vec.Z += (Vector3::Dot_Product(-context.b[2], vec1) + context.ab[0][2] * vec.X + context.ab[1][2] * vec.Y)
                    * context.ab[2][2];
                vec.Z += context.ab[2][0] * (-f6 * context.box1.m_extent.X);
                vec.Z += context.ab[2][1] * (f7 * context.box1.m_extent.Y);
                vec.Z /= f8;
            }

            break;
        }
    }

    result->contact_point.X = vec.X * context.a[0].X + vec.Y * context.a[1].X + vec.Z * context.a[2].X;
    result->contact_point.Y = vec.X * context.a[0].Y + vec.Y * context.a[1].Y + vec.Z * context.a[2].Y;
    result->contact_point.Z = vec.X * context.a[0].Z + vec.Y * context.a[1].Z + vec.Z * context.a[2].Z;
    result->contact_point += context.box0.m_center;
    Vector3::Add(result->contact_point, result->fraction * context.move0, &result->contact_point);
}

static bool Collide_OBB_OBB(ObbCollisionStruct &context, CastResultStruct *result)
{
    context.test_axis_id = 1;
    context.ab[0][0] = context.a[0] * context.b[0];
    context.ab[0][1] = context.a[0] * context.b[1];
    context.ab[0][2] = context.a[0] * context.b[2];

    if (!OBB_Check_Box0_Basis(context, 0)) {
        context.test_axis_id = 2;
        context.ab[1][0] = context.a[1] * context.b[0];
        context.ab[1][1] = context.a[1] * context.b[1];
        context.ab[1][2] = context.a[1] * context.b[2];

        if (!OBB_Check_Box0_Basis(context, 1)) {
            context.test_axis_id = 3;
            context.ab[2][0] = context.a[2] * context.b[0];
            context.ab[2][1] = context.a[2] * context.b[1];
            context.ab[2][2] = context.a[2] * context.b[2];

            if (!OBB_Check_Box0_Basis(context, 2)) {
                context.test_axis_id = 4;

                if (!OBB_Check_Box1_Basis(context, 0)) {
                    context.test_axis_id = 5;

                    if (!OBB_Check_Box1_Basis(context, 1)) {
                        context.test_axis_id = 6;

                        if (!OBB_Check_Box1_Basis(context, 2)) {
                            context.test_axis = Vector3::Cross_Product(context.a[0], context.b[0]);
                            context.test_axis_id = 7;

                            float ra = GameMath::Fabs(context.box0.m_extent.Z * context.ab[1][0])
                                + GameMath::Fabs(context.box0.m_extent.Y * context.ab[2][0]);
                            float rb = GameMath::Fabs(context.box1.m_extent.Z * context.ab[0][1])
                                + GameMath::Fabs(context.box1.m_extent.Y * context.ab[0][2]);

                            if (context.test_axis.Length2() <= GAMEMATH_EPSILON2 || !OBB_Check_Unk(context, ra, rb)) {
                                context.test_axis = Vector3::Cross_Product(context.a[0], context.b[1]);
                                context.test_axis_id = 8;

                                ra = GameMath::Fabs(context.box0.m_extent.Z * context.ab[1][1])
                                    + GameMath::Fabs(context.box0.m_extent.Y * context.ab[2][1]);
                                rb = GameMath::Fabs(context.box1.m_extent.Z * context.ab[0][0])
                                    + GameMath::Fabs(context.box1.m_extent.X * context.ab[0][2]);

                                if (context.test_axis.Length2() <= GAMEMATH_EPSILON2 || !OBB_Check_Unk(context, ra, rb)) {
                                    context.test_axis = Vector3::Cross_Product(context.a[0], context.b[2]);
                                    context.test_axis_id = 9;

                                    ra = GameMath::Fabs(context.box0.m_extent.Z * context.ab[1][2])
                                        + GameMath::Fabs(context.box0.m_extent.Y * context.ab[2][2]);
                                    rb = GameMath::Fabs(context.box1.m_extent.Y * context.ab[0][0])
                                        + GameMath::Fabs(context.box1.m_extent.X * context.ab[0][1]);

                                    if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                        || !OBB_Check_Unk(context, ra, rb)) {
                                        context.test_axis = Vector3::Cross_Product(context.a[1], context.b[0]);
                                        context.test_axis_id = 10;

                                        ra = GameMath::Fabs(context.box0.m_extent.Z * context.ab[0][0])
                                            + GameMath::Fabs(context.box0.m_extent.X * context.ab[2][0]);
                                        rb = GameMath::Fabs(context.box1.m_extent.Z * context.ab[1][1])
                                            + GameMath::Fabs(context.box1.m_extent.Y * context.ab[1][2]);

                                        if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                            || !OBB_Check_Unk(context, ra, rb)) {
                                            context.test_axis = Vector3::Cross_Product(context.a[1], context.b[1]);
                                            context.test_axis_id = 11;

                                            ra = GameMath::Fabs(context.box0.m_extent.Z * context.ab[0][1])
                                                + GameMath::Fabs(context.box0.m_extent.X * context.ab[2][1]);
                                            rb = GameMath::Fabs(context.box1.m_extent.Z * context.ab[1][0])
                                                + GameMath::Fabs(context.box1.m_extent.X * context.ab[1][2]);

                                            if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                || !OBB_Check_Unk(context, ra, rb)) {
                                                context.test_axis = Vector3::Cross_Product(context.a[1], context.b[2]);
                                                context.test_axis_id = 12;

                                                ra = GameMath::Fabs(context.box0.m_extent.Z * context.ab[0][2])
                                                    + GameMath::Fabs(context.box0.m_extent.X * context.ab[2][2]);
                                                rb = GameMath::Fabs(context.box1.m_extent.Y * context.ab[1][0])
                                                    + GameMath::Fabs(context.box1.m_extent.X * context.ab[1][1]);

                                                if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                    || !OBB_Check_Unk(context, ra, rb)) {
                                                    context.test_axis = Vector3::Cross_Product(context.a[2], context.b[0]);
                                                    context.test_axis_id = 13;

                                                    ra = GameMath::Fabs(context.box0.m_extent.Y * context.ab[0][0])
                                                        + GameMath::Fabs(context.box0.m_extent.X * context.ab[1][0]);
                                                    rb = GameMath::Fabs(context.box1.m_extent.Z * context.ab[2][1])
                                                        + GameMath::Fabs(context.box1.m_extent.Y * context.ab[2][2]);

                                                    if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                        || !OBB_Check_Unk(context, ra, rb)) {
                                                        context.test_axis =
                                                            Vector3::Cross_Product(context.a[2], context.b[1]);
                                                        context.test_axis_id = 14;

                                                        ra = GameMath::Fabs(context.box0.m_extent.Y * context.ab[0][1])
                                                            + GameMath::Fabs(context.box0.m_extent.X * context.ab[1][1]);
                                                        rb = GameMath::Fabs(context.box1.m_extent.Z * context.ab[2][0])
                                                            + GameMath::Fabs(context.box1.m_extent.X * context.ab[2][2]);

                                                        if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                            || !OBB_Check_Unk(context, ra, rb)) {
                                                            context.test_axis =
                                                                Vector3::Cross_Product(context.a[2], context.b[2]);
                                                            context.test_axis_id = 15;

                                                            ra = GameMath::Fabs(context.box0.m_extent.Y * context.ab[0][2])
                                                                + GameMath::Fabs(context.box0.m_extent.X * context.ab[1][2]);
                                                            rb = GameMath::Fabs(context.box1.m_extent.Y * context.ab[2][0])
                                                                + GameMath::Fabs(context.box1.m_extent.X * context.ab[2][1]);

                                                            if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                                || !OBB_Check_Unk(context, ra, rb)) {
                                                                if (!result->start_bad) {
                                                                    context.test_axis_id = context.axis_id;
                                                                    context.test_axis =
                                                                        Vector3::Cross_Product(context.a[0], context.move0);

                                                                    if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                                        || (OBB_Collide_Get_Data(context, &ra, &rb),
                                                                            !OBB_Check_Unk(context, ra, rb))) {
                                                                        context.test_axis = Vector3::Cross_Product(
                                                                            context.a[1], context.move0);

                                                                        if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                                            || (OBB_Collide_Get_Data(context, &ra, &rb),
                                                                                !OBB_Check_Unk(context, ra, rb))) {
                                                                            context.test_axis = Vector3::Cross_Product(
                                                                                context.a[2], context.move0);

                                                                            if (context.test_axis.Length2()
                                                                                > GAMEMATH_EPSILON2) {
                                                                                OBB_Collide_Get_Data(context, &ra, &rb);
                                                                                OBB_Check_Unk(context, ra, rb);
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (context.start_bad) {
        result->start_bad = true;
        result->fraction = 0.0f;

        return true;
    }

    if (context.max_frac >= result->fraction) {
        return false;
    }

    result->fraction = context.max_frac;

    Compute_Contact_Normal(context, result);

    if (result->compute_contact_point) {
        Compute_Contact_Point(context, result);
    }

    return true;
}

bool CollisionMath::Collide(
    const AABoxClass &box, const Vector3 &move, const OBBoxClass &box2, const Vector3 &move2, CastResultStruct *result)
{
    OBBoxClass obbox(box.m_center, box.m_extent);
    ObbCollisionStruct context(obbox, move, box2, move2);
    return Collide_OBB_OBB(context, result);
}

bool CollisionMath::Collide(
    const OBBoxClass &box, const Vector3 &move, const OBBoxClass &box2, const Vector3 &move2, CastResultStruct *result)
{
    ObbCollisionStruct context(box, move, box2, move2);
    return Collide_OBB_OBB(context, result);
}

bool CollisionMath::Collide(
    const OBBoxClass &box, const Vector3 &move, const AABoxClass &box2, const Vector3 &move2, CastResultStruct *result)
{
    OBBoxClass obbox(box2.m_center, box2.m_extent);
    ObbCollisionStruct context(box, move, obbox, move2);
    return Collide_OBB_OBB(context, result);
}
