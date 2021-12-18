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
#include "colmath.h"
#include "obbox.h"
#include "tri.h"
#include "vector3.h"
#include <algorithm>

struct OBBTCollisionStruct
{
    OBBTCollisionStruct(const OBBoxClass &b, const Vector3 &move1, const TriClass &t, const Vector3 &move2) :
        box(b), tri(t), box_move(move1), tri_move(move2)
    {
        Reset();
    }

    void Reset()
    {
        axis_id = 0;
        point = 0;
        side = 0;
        start_bad = true;
        max_frac = -1.0f;
        d = (*tri.V[0]) - box.m_center;
        move = box_move - tri_move;
        e[0] = (*tri.V[1]) - (*tri.V[0]);
        e[1] = (*tri.V[2]) - (*tri.V[0]);
        e[2] = e[1] - e[0];
        a[0].X = box.m_basis[0].X;
        a[0].Y = box.m_basis[1].X;
        a[0].Z = box.m_basis[2].X;
        a[1].X = box.m_basis[0].Y;
        a[1].Y = box.m_basis[1].Y;
        a[1].Z = box.m_basis[2].Y;
        a[2].X = box.m_basis[0].Z;
        a[2].Y = box.m_basis[1].Z;
        a[2].Z = box.m_basis[2].Z;
        n = Vector3::Cross_Product(e[0], e[1]);
    }

    bool start_bad;
    float max_frac;
    int axis_id;
    int point;
    int side;
    int test_side;
    int test_axis_id;
    int test_point;
    Vector3 test_axis;
    Vector3 d;
    Vector3 move;
    float ae[3][3];
    float an[3];
    Vector3 axe[3][3];
    Vector3 a[3];
    Vector3 e[3];
    Vector3 n;
    Vector3 final_d;
    const OBBoxClass &box;
    const TriClass &tri;
    const Vector3 &box_move;
    const Vector3 &tri_move;
};

static bool OBBTri_Collision_Separation_Test(OBBTCollisionStruct &context, float lp, float leb0, float leb1)
{
    float f1 = 0.0f;
    float f2 = lp - leb0;

    if (f2 <= 0.0f) {
        f1 = context.test_axis.Length() * 0.001f;
    }

    if (-f1 >= f2) {
        return false;
    }

    context.start_bad = false;
    float f3 = leb1 - leb0;

    if (f3 <= 0.0f) {
        context.axis_id = context.test_axis_id;
        context.max_frac = 1.0f;
        return true;
    }

    float f4 = f2 / f3;

    if (f4 >= 1.0f) {
        context.axis_id = context.test_axis_id;
        context.max_frac = 1.0f;
        return true;
    }

    if (f4 > context.max_frac) {
        context.max_frac = f4;
        context.point = context.test_point;
        context.axis_id = context.test_axis_id;
        context.side = context.test_side;
    }

    return false;
}

static bool OBBTri_Check_Collision_Unk_Axis(OBBTCollisionStruct &context)
{
    float f1 = Vector3::Dot_Product(context.d, context.test_axis);
    float f2 = Vector3::Dot_Product(context.move, context.test_axis);

    if (f1 >= 0.0f) {
        context.test_side = 1;
    } else {
        f1 = -f1;
        f2 = -f2;
        context.test_axis = -context.test_axis;
        context.test_side = -1;
    }

    float leb0 = GameMath::Fabs(context.an[0]) * context.box.m_extent.X
        + GameMath::Fabs(context.an[1]) * context.box.m_extent.Y + GameMath::Fabs(context.an[2]) * context.box.m_extent.Z;
    float leb1 = leb0 + f2;
    context.test_point = 0;

    return OBBTri_Collision_Separation_Test(context, f1, leb0, leb1);
}

static bool OBBTri_Check_Collision_Basis_Axis(OBBTCollisionStruct &context, float unk1, float unk2, float unk3)
{
    float f1 = Vector3::Dot_Product(context.d, context.test_axis);
    float f2 = Vector3::Dot_Product(context.move, context.test_axis);

    if (f1 >= 0.0f) {
        context.test_side = 1;
    } else {
        f1 = -f1;
        f2 = -f2;
        unk2 = -unk2;
        unk3 = -unk3;
        context.test_axis = -context.test_axis;
        context.test_side = -1;
    }

    context.test_point = 0;

    if (unk2 >= 0.0f) {
        unk2 = 0.0f;
    } else {
        context.test_point = 1;
    }

    if (unk2 <= unk3) {
        unk3 = unk2;
    } else {
        context.test_point = 2;
    }

    return OBBTri_Collision_Separation_Test(context, f1 + unk3, unk1, f2 + unk1);
}

static bool OBBTri_Check_Collision_Cross_Axis(OBBTCollisionStruct &context, float unk1, int unk2, float unk3)
{
    float f1 = Vector3::Dot_Product(context.d, context.test_axis);
    float f2 = Vector3::Dot_Product(context.move, context.test_axis);

    if (f1 >= 0.0f) {
        context.test_side = 1;
    } else {
        f1 = -f1;
        f2 = -f2;
        unk1 = -unk1;
        context.test_axis = -context.test_axis;
        context.test_side = -1;
    }

    context.test_point = 0;

    if (unk1 >= 0.0f) {
        unk1 = 0.0f;
    } else {
        context.test_point = unk2;
    }

    return OBBTri_Collision_Separation_Test(context, f1 + unk1, unk3, f2 + unk3);
}

static bool OBBTri_Check_Collision_Axis(OBBTCollisionStruct &context)
{
    float f1 = Vector3::Dot_Product(context.d, context.test_axis);
    float f2 = Vector3::Dot_Product(context.move, context.test_axis);

    if (f1 >= 0.0f) {
        context.test_side = 1;
    } else {
        f1 = -f1;
        f2 = -f2;
        context.test_axis = -context.test_axis;
        context.test_side = -1;
    }

    float f3 = GameMath::Fabs(Vector3::Dot_Product(context.test_axis, context.a[0])) * context.box.m_extent.X
        + GameMath::Fabs(Vector3::Dot_Product(context.test_axis, context.a[1])) * context.box.m_extent.Y
        + GameMath::Fabs(Vector3::Dot_Product(context.test_axis, context.a[2])) * context.box.m_extent.Z;

    float f4 = Vector3::Dot_Product(context.e[0], context.test_axis);

    if (f4 >= 0.0f) {
        f4 = 0.0f;
    }

    float f5 = Vector3::Dot_Product(context.e[1], context.test_axis);

    if (f4 <= f5) {
        f5 = f4;
    }

    return OBBTri_Collision_Separation_Test(context, f1 + f5, f3, f2 + f3);
}

static void OBBTri_Compute_Contact_Normal(const OBBTCollisionStruct &context, Vector3 *result)
{
    switch (context.axis_id) {
        case 1:
            result->X = -context.side * context.tri.N->X;
            result->Y = -context.side * context.tri.N->Y;
            result->Z = -context.side * context.tri.N->Z;

            break;
        case 2:
            result->X = -context.side * context.a[0].X;
            result->Y = -context.side * context.a[0].Y;
            result->Z = -context.side * context.a[0].Z;

            break;
        case 3:
            result->X = -context.side * context.a[1].X;
            result->Y = -context.side * context.a[1].Y;
            result->Z = -context.side * context.a[1].Z;

            break;
        case 4:
            result->X = -context.side * context.a[2].X;
            result->Y = -context.side * context.a[2].Y;
            result->Z = -context.side * context.a[2].Z;

            break;
        case 5:
            result->X = -context.side * context.axe[0][0].X;
            result->Y = -context.side * context.axe[0][0].Y;
            result->Z = -context.side * context.axe[0][0].Z;
            result->Normalize();

            break;
        case 6:
            result->X = -context.side * context.axe[1][0].X;
            result->Y = -context.side * context.axe[1][0].Y;
            result->Z = -context.side * context.axe[1][0].Z;
            result->Normalize();

            break;
        case 7:
            result->X = -context.side * context.axe[2][0].X;
            result->Y = -context.side * context.axe[2][0].Y;
            result->Z = -context.side * context.axe[2][0].Z;
            result->Normalize();

            break;
        case 8:
            result->X = -context.side * context.axe[0][1].X;
            result->Y = -context.side * context.axe[0][1].Y;
            result->Z = -context.side * context.axe[0][1].Z;
            result->Normalize();

            break;
        case 9:
            result->X = -context.side * context.axe[1][1].X;
            result->Y = -context.side * context.axe[1][1].Y;
            result->Z = -context.side * context.axe[1][1].Z;
            result->Normalize();

            break;
        case 10:
            result->X = -context.side * context.axe[2][1].X;
            result->Y = -context.side * context.axe[2][1].Y;
            result->Z = -context.side * context.axe[2][1].Z;
            result->Normalize();

            break;
        case 11:
            result->X = -context.side * context.axe[0][2].X;
            result->Y = -context.side * context.axe[0][2].Y;
            result->Z = -context.side * context.axe[0][2].Z;
            result->Normalize();

            break;
        case 12:
            result->X = -context.side * context.axe[1][2].X;
            result->Y = -context.side * context.axe[1][2].Y;
            result->Z = -context.side * context.axe[1][2].Z;
            result->Normalize();

            break;
        case 13:
            result->X = -context.side * context.axe[2][2].X;
            result->Y = -context.side * context.axe[2][2].Y;
            result->Z = -context.side * context.axe[2][2].Z;
            result->Normalize();

            break;
    }

    captainslog_assert(result->Length2() > 0.0f);
}

static float Eval_Side(float val, int side)
{
    if (val > 0.0f) {
        return (float)(side);
    } else if (val < 0.0f) {
        return (float)(-side);
    } else {
        return 0.0f;
    }
}

static void Eval_A0_Point(const OBBTCollisionStruct &context, float *x, int edge)
{
    float f1 = Eval_Side(context.ae[2][edge], context.side);

    x[1] = -f1 * context.box.m_extent.Y;

    float f2 = Eval_Side(context.ae[1][edge], context.side);

    x[2] = f2 * context.box.m_extent.Z;

    float f3;

    if (context.point) {
        f3 = 1.0f;
    } else {
        f3 = 0.0f;
    }

    float f4 = Vector3::Dot_Product(context.n, context.axe[0][edge]);

    if (GameMath::Fabs(f4) <= 0.0f) {
        x[0] = 0.0f;
    } else {
        x[0] = Vector3::Dot_Product(context.n, Vector3::Cross_Product(context.final_d, context.e[edge]));

        if (edge) {
            x[0] += f3 * context.n.Length2();
        } else {
            x[0] -= f3 * context.n.Length2();
        }

        x[0] -= (context.n * context.axe[1][edge]) * x[1];
        x[0] -= (context.n * context.axe[2][edge]) * x[2];
        x[0] /= f4;
    }
}

static void Eval_A1_Point(const OBBTCollisionStruct &context, float *x, int edge)
{
    float f1 = Eval_Side(context.ae[2][edge], context.side);

    x[0] = f1 * context.box.m_extent.X;

    float f2 = Eval_Side(context.ae[0][edge], context.side);

    x[2] = -f2 * context.box.m_extent.Z;

    float f3;

    if (context.point) {
        f3 = 1.0f;
    } else {
        f3 = 0.0f;
    }

    float f4 = Vector3::Dot_Product(context.n, context.axe[1][edge]);

    if (GameMath::Fabs(f4) <= 0.0f) {
        x[1] = 0.0f;
    } else {
        x[1] = Vector3::Dot_Product(context.n, Vector3::Cross_Product(context.final_d, context.e[edge]));

        if (edge) {
            x[1] += f3 * context.n.Length2();
        } else {
            x[1] -= f3 * context.n.Length2();
        }

        x[1] -= Vector3::Dot_Product(context.n, context.axe[0][edge]) * x[0];
        x[1] -= Vector3::Dot_Product(context.n, context.axe[2][edge]) * x[2];
        x[1] /= f4;
    }
}

static void Eval_A2_Point(const OBBTCollisionStruct &context, float *x, int edge)
{
    float f1 = Eval_Side(context.ae[1][edge], context.side);

    x[0] = -f1 * context.box.m_extent.X;

    float f2 = Eval_Side(context.ae[0][edge], context.side);

    x[1] = f2 * context.box.m_extent.Y;

    float f3;

    if (context.point) {
        f3 = 1.0f;
    } else {
        f3 = 0.0f;
    }

    float f4 = Vector3::Dot_Product(context.n, context.axe[2][edge]);

    if (GameMath::Fabs(f4) <= 0.0f) {
        x[2] = 0.0f;
    } else {
        x[2] = Vector3::Dot_Product(context.n, Vector3::Cross_Product(context.final_d, context.e[edge]));

        if (edge) {
            x[2] += f3 * context.n.Length2();
        } else {
            x[2] -= f3 * context.n.Length2();
        }

        x[2] -= (context.n * context.axe[0][edge]) * x[0];
        x[2] -= (context.n * context.axe[1][edge]) * x[1];
        x[2] /= f4;
    }
}

static void OBBTri_Compute_Contact_Point(OBBTCollisionStruct &context, CastResultStruct *result)
{
    if (context.axis_id > 4) {
        context.final_d = (result->fraction * context.tri_move + (*context.tri.V[0]))
            - (result->fraction * context.box_move + context.box.m_center);
    }

    float x[3];

    switch (context.axis_id) {
        case 0:
            captainslog_assert(0);
            return;
        case 1:
            for (int i = 0; i < 3; i++) {
                x[i] = Eval_Side(context.an[i], context.side) * context.box.m_extent[i];
            }

            break;

        case 2:
        case 3:
        case 4:
            result->contact_point = (*context.tri.V[context.point]);
            Vector3::Add(result->contact_point, result->fraction * context.tri_move, &result->contact_point);

            return;
        case 5:
            Eval_A0_Point(context, x, 0);

            break;
        case 6:
            Eval_A1_Point(context, x, 0);

            break;
        case 7:
            Eval_A2_Point(context, x, 0);

            break;
        case 8:
            Eval_A0_Point(context, x, 1);

            break;
        case 9:
            Eval_A1_Point(context, x, 1);

            break;
        case 10:
            Eval_A2_Point(context, x, 1);

            break;
        case 11:
            Eval_A0_Point(context, x, 2);

            break;
        case 12:
            Eval_A1_Point(context, x, 2);

            break;
        case 13:
            Eval_A2_Point(context, x, 2);

            break;
    }

    x[0] = std::clamp(x[0], -context.box.m_extent.X, context.box.m_extent.X);
    x[1] = std::clamp(x[1], -context.box.m_extent.Y, context.box.m_extent.Y);
    x[2] = std::clamp(x[2], -context.box.m_extent.Z, context.box.m_extent.Z);

    result->contact_point.X = x[0] * context.a[0].X + x[1] * context.a[1].X + x[2] * context.a[2].X + context.box.m_center.X;
    result->contact_point.Y = x[0] * context.a[0].Y + x[1] * context.a[1].Y + x[2] * context.a[2].Y + context.box.m_center.Y;
    result->contact_point.Z = x[0] * context.a[0].Z + x[1] * context.a[1].Z + x[2] * context.a[2].Z + context.box.m_center.Z;

    Vector3::Add(result->contact_point, result->fraction * context.box_move, &result->contact_point);
}

bool CollisionMath::Collide(
    const OBBoxClass &box, const Vector3 &move, const TriClass &tri, const Vector3 &move2, CastResultStruct *result)
{
    OBBTCollisionStruct context(box, move, tri, move2);
    context.test_axis = context.n;
    context.test_axis_id = 1;
    context.an[0] = Vector3::Dot_Product(context.a[0], context.n);
    context.an[1] = Vector3::Dot_Product(context.a[1], context.n);
    context.an[2] = Vector3::Dot_Product(context.a[2], context.n);

    if (!OBBTri_Check_Collision_Unk_Axis(context)) {
        context.test_axis = context.a[0];
        context.test_axis_id = 2;
        context.ae[0][0] = Vector3::Dot_Product(context.a[0], context.e[0]);
        context.ae[0][1] = Vector3::Dot_Product(context.a[0], context.e[1]);

        if (!OBBTri_Check_Collision_Basis_Axis(context, box.m_extent.X, context.ae[0][0], context.ae[0][1])) {
            context.test_axis = context.a[1];
            context.test_axis_id = 3;
            context.ae[1][0] = Vector3::Dot_Product(context.a[1], context.e[0]);
            context.ae[1][1] = Vector3::Dot_Product(context.a[1], context.e[1]);

            if (!OBBTri_Check_Collision_Basis_Axis(context, box.m_extent.Y, context.ae[1][0], context.ae[1][1])) {
                context.test_axis = context.a[2];
                context.test_axis_id = 4;
                context.ae[2][0] = Vector3::Dot_Product(context.a[2], context.e[0]);
                context.ae[2][1] = Vector3::Dot_Product(context.a[2], context.e[1]);

                if (!OBBTri_Check_Collision_Basis_Axis(context, box.m_extent.Z, context.ae[2][0], context.ae[2][1])) {
                    context.axe[0][0] = Vector3::Cross_Product(context.a[0], context.e[0]);
                    context.test_axis = context.axe[0][0];
                    context.test_axis_id = 5;

                    float f1 = (GameMath::Fabs(context.ae[2][0]) * box.m_extent.Y
                        + GameMath::Fabs(context.ae[1][0]) * box.m_extent.Z);

                    if (context.axe[0][0].Length2() <= GAMEMATH_EPSILON2
                        || !OBBTri_Check_Collision_Cross_Axis(context, context.an[0], 2, f1)) {
                        context.axe[0][1] = Vector3::Cross_Product(context.a[0], context.e[1]);
                        context.test_axis = context.axe[0][1];
                        context.test_axis_id = 8;

                        f1 = (GameMath::Fabs(context.ae[2][1]) * box.m_extent.Y
                            + GameMath::Fabs(context.ae[1][1]) * box.m_extent.Z);

                        if (context.axe[0][1].Length2() <= GAMEMATH_EPSILON2
                            || !OBBTri_Check_Collision_Cross_Axis(context, -context.an[0], 1, f1)) {
                            context.axe[0][2] = Vector3::Cross_Product(context.a[0], context.e[2]);
                            context.test_axis = context.axe[0][2];
                            context.test_axis_id = 11;
                            context.ae[1][2] = Vector3::Dot_Product(context.a[1], context.e[2]);
                            context.ae[2][2] = Vector3::Dot_Product(context.a[2], context.e[2]);

                            f1 = (GameMath::Fabs(context.ae[2][2]) * box.m_extent.Y
                                + GameMath::Fabs(context.ae[1][2]) * box.m_extent.Z);

                            if (context.axe[0][2].Length2() <= GAMEMATH_EPSILON2
                                || !OBBTri_Check_Collision_Cross_Axis(context, -context.an[0], 1, f1)) {
                                context.axe[1][0] = Vector3::Cross_Product(context.a[1], context.e[0]);
                                context.test_axis = context.axe[1][0];
                                context.test_axis_id = 6;

                                f1 = (GameMath::Fabs(context.ae[2][0]) * box.m_extent.X
                                    + GameMath::Fabs(context.ae[0][0]) * box.m_extent.Z);

                                if (context.axe[1][0].Length2() <= GAMEMATH_EPSILON2
                                    || !OBBTri_Check_Collision_Cross_Axis(context, context.an[1], 2, f1)) {
                                    context.axe[1][1] = Vector3::Cross_Product(context.a[1], context.e[1]);
                                    context.test_axis = context.axe[1][1];
                                    context.test_axis_id = 9;

                                    f1 = (GameMath::Fabs(context.ae[2][1]) * box.m_extent.X
                                        + GameMath::Fabs(context.ae[0][1]) * box.m_extent.Z);

                                    if (context.axe[1][1].Length2() <= GAMEMATH_EPSILON2
                                        || !OBBTri_Check_Collision_Cross_Axis(context, -context.an[1], 1, f1)) {
                                        context.axe[1][2] = Vector3::Cross_Product(context.a[1], context.e[2]);
                                        context.test_axis = context.axe[1][2];
                                        context.test_axis_id = 12;
                                        context.ae[0][2] = Vector3::Dot_Product(context.a[0], context.e[2]);

                                        f1 = (GameMath::Fabs(context.ae[2][2]) * box.m_extent.X
                                            + GameMath::Fabs(context.ae[0][2]) * box.m_extent.Z);

                                        if (context.axe[1][2].Length2() <= GAMEMATH_EPSILON2
                                            || !OBBTri_Check_Collision_Cross_Axis(context, -context.an[1], 1, f1)) {
                                            context.axe[2][0] = Vector3::Cross_Product(context.a[2], context.e[0]);
                                            context.test_axis = context.axe[2][0];
                                            context.test_axis_id = 7;

                                            f1 = (GameMath::Fabs(context.ae[1][0]) * box.m_extent.X
                                                + GameMath::Fabs(context.ae[0][0]) * box.m_extent.Y);

                                            if (context.axe[2][0].Length2() <= GAMEMATH_EPSILON2
                                                || !OBBTri_Check_Collision_Cross_Axis(context, context.an[2], 2, f1)) {
                                                context.axe[2][1] = Vector3::Cross_Product(context.a[2], context.e[1]);
                                                context.test_axis = context.axe[2][1];
                                                context.test_axis_id = 10;

                                                f1 = (GameMath::Fabs(context.ae[1][1]) * box.m_extent.X
                                                    + GameMath::Fabs(context.ae[0][1]) * box.m_extent.Y);

                                                if (context.axe[2][1].Length2() <= GAMEMATH_EPSILON2
                                                    || !OBBTri_Check_Collision_Cross_Axis(context, -context.an[2], 1, f1)) {
                                                    context.axe[2][2] = Vector3::Cross_Product(context.a[2], context.e[2]);
                                                    context.test_axis = context.axe[2][2];
                                                    context.test_axis_id = 13;

                                                    f1 = (GameMath::Fabs(context.ae[1][2]) * box.m_extent.X
                                                        + GameMath::Fabs(context.ae[0][2]) * box.m_extent.Y);

                                                    if (context.axe[2][2].Length2() <= GAMEMATH_EPSILON2
                                                        || !OBBTri_Check_Collision_Cross_Axis(
                                                            context, -context.an[2], 1, f1)) {

                                                        if (!context.start_bad) {
                                                            context.test_point = context.point;
                                                            context.test_axis_id = context.axis_id;
                                                            context.test_axis =
                                                                Vector3::Cross_Product(context.move, context.a[0]);

                                                            if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                                || !OBBTri_Check_Collision_Axis(context)) {
                                                                context.test_axis =
                                                                    Vector3::Cross_Product(context.move, context.a[1]);

                                                                if (context.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                                    || !OBBTri_Check_Collision_Axis(context)) {
                                                                    context.test_axis =
                                                                        Vector3::Cross_Product(context.move, context.a[2]);

                                                                    if (context.test_axis.Length2() > GAMEMATH_EPSILON2) {
                                                                        OBBTri_Check_Collision_Axis(context);
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

    if (!context.axis_id || context.start_bad) {
        result->start_bad = true;
        result->fraction = 0.0f;
        result->normal = (*tri.N);

        return true;
    }

    if (context.max_frac < 0.0f) {
        context.max_frac = 0.0f;
    }

    if (context.max_frac >= 1.0f || result->fraction < context.max_frac) {
        return false;
    }

    Vector3 contactNormal;
    OBBTri_Compute_Contact_Normal(context, &contactNormal);

    if (GameMath::Fabs(context.max_frac - result->fraction) > GAMEMATH_EPSILON
        || Vector3::Dot_Product(result->normal, move) > Vector3::Dot_Product(contactNormal, move)) {
        result->normal = contactNormal;
    }

    result->fraction = context.max_frac;

    if (result->compute_contact_point) {
        OBBTri_Compute_Contact_Point(context, result);
    }

    return true;
}

struct OOBTIntersectStruct
{
    OOBTIntersectStruct(const OBBoxClass &b, const TriClass &t) : ae{}, an{}, box(&b), tri(&t)
    {
        d = (*t.V[0]) - b.m_center;
        e[0] = (*t.V[1]) - (*t.V[0]);
        e[1] = (*t.V[2]) - (*t.V[0]);
        e[2] = e[1] - e[0];
        a[0].X = b.m_basis[0].X;
        a[0].Y = b.m_basis[1].X;
        a[0].Z = b.m_basis[2].X;
        a[1].X = b.m_basis[0].Y;
        a[1].Y = b.m_basis[1].Y;
        a[1].Z = b.m_basis[2].Y;
        a[2].X = b.m_basis[0].Z;
        a[2].Y = b.m_basis[1].Z;
        a[2].Z = b.m_basis[2].Z;
        n = Vector3::Cross_Product(e[0], e[1]);
    }

    Vector3 d;
    float ae[3][3];
    float an[3];
    Vector3 axe[3][3];
    Vector3 a[3];
    Vector3 e[3];
    Vector3 n;
    Vector3 test_axis;
    const OBBoxClass *box;
    const TriClass *tri;
};

static bool OBBTri_Intersection_Separation_Test(OOBTIntersectStruct &context, float lp, float leb0)
{
    float f1 = lp - leb0;
    float f2 = 0.0f;

    if (f1 <= 0.0f) {
        f2 = context.test_axis.Length() * 0.001f;
    }

    return (-f2 < f1);
}

static bool OBBTri_Check_Intersection_Unk_Axis(OOBTIntersectStruct &context)
{
    float f1 = Vector3::Dot_Product(context.d, context.test_axis);

    if (f1 < 0.0f) {
        context.test_axis = -context.test_axis;
        f1 = -f1;
    }

    return OBBTri_Intersection_Separation_Test(context,
        f1,
        GameMath::Fabs(context.an[0]) * context.box->m_extent.X + GameMath::Fabs(context.an[1]) * context.box->m_extent.Y
            + GameMath::Fabs(context.an[2]) * context.box->m_extent.Z);
}

static bool OBBTri_Check_Intersection_Basis_Axis(OOBTIntersectStruct &context, float unk1, float unk2, float unk3)
{
    float f1 = Vector3::Dot_Product(context.d, context.test_axis);

    if (f1 < 0.0f) {
        context.test_axis = -context.test_axis;
        f1 = -f1;
        unk2 = -unk2;
        unk3 = -unk3;
    }

    float f2 = 0.0f;

    if (unk2 < 0.0f) {
        f2 = unk2;
    }

    if (unk3 < f2) {
        f2 = unk3;
    }

    return OBBTri_Intersection_Separation_Test(context, f1 + f2, unk1);
}

static bool OBBTri_Check_Intersection_Cross_Axis(OOBTIntersectStruct &context, float unk1, float unk2)
{
    float f1 = Vector3::Dot_Product(context.d, context.test_axis);

    if (f1 < 0.0f) {
        context.test_axis = -context.test_axis;
        f1 = -f1;
        unk1 = -unk1;
    }

    float f2 = 0.0f;

    if (unk1 < 0.0f) {
        f2 = unk1;
    }

    return OBBTri_Intersection_Separation_Test(context, f1 + f2, unk2);
}

bool CollisionMath::Intersection_Test(const OBBoxClass &box, const TriClass &tri)
{
    OOBTIntersectStruct context(box, tri);
    context.test_axis = context.n;
    context.an[0] = Vector3::Dot_Product(context.a[0], context.n);
    context.an[1] = Vector3::Dot_Product(context.a[1], context.n);
    context.an[2] = Vector3::Dot_Product(context.a[2], context.n);

    if (OBBTri_Check_Intersection_Unk_Axis(context)) {
        return false;
    }

    context.test_axis = context.a[0];
    context.ae[0][0] = Vector3::Dot_Product(context.a[0], context.e[0]);
    context.ae[0][1] = Vector3::Dot_Product(context.a[0], context.e[1]);

    if (OBBTri_Check_Intersection_Basis_Axis(context, box.m_extent.X, context.ae[0][0], context.ae[0][1])) {
        return false;
    }

    context.test_axis = context.a[1];
    context.ae[1][0] = Vector3::Dot_Product(context.a[1], context.e[0]);
    context.ae[1][1] = Vector3::Dot_Product(context.a[1], context.e[1]);

    if (OBBTri_Check_Intersection_Basis_Axis(context, box.m_extent.Y, context.ae[1][0], context.ae[1][1])) {
        return false;
    }

    context.test_axis = context.a[2];
    context.ae[2][0] = Vector3::Dot_Product(context.a[2], context.e[0]);
    context.ae[2][1] = Vector3::Dot_Product(context.a[2], context.e[1]);

    if (OBBTri_Check_Intersection_Basis_Axis(context, box.m_extent.Z, context.ae[2][0], context.ae[2][1])) {
        return false;
    }

    context.axe[0][0] = Vector3::Cross_Product(context.a[0], context.e[0]);
    context.test_axis = context.axe[0][0];

    if (context.axe[0][0].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[1][0]) * box.m_extent.Z + GameMath::Fabs(context.ae[2][0]) * box.m_extent.Y;

        if (OBBTri_Check_Intersection_Cross_Axis(context, context.an[0], leb0)) {
            return false;
        }
    }

    context.axe[0][1] = Vector3::Cross_Product(context.a[0], context.e[1]);
    context.test_axis = context.axe[0][1];

    if (context.axe[0][1].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[1][1]) * box.m_extent.Z + GameMath::Fabs(context.ae[2][1]) * box.m_extent.Y;

        if (OBBTri_Check_Intersection_Cross_Axis(context, -context.an[0], leb0)) {
            return false;
        }
    }

    context.axe[0][2] = Vector3::Cross_Product(context.a[0], context.e[2]);
    context.test_axis = context.axe[0][2];
    context.ae[1][2] = Vector3::Dot_Product(context.a[1], context.e[2]);
    context.ae[2][2] = Vector3::Dot_Product(context.a[2], context.e[2]);

    if (context.axe[0][2].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[1][2]) * box.m_extent.Z + GameMath::Fabs(context.ae[2][2]) * box.m_extent.Y;

        if (OBBTri_Check_Intersection_Cross_Axis(context, -context.an[0], leb0)) {
            return false;
        }
    }

    context.axe[1][0] = Vector3::Cross_Product(context.a[1], context.e[0]);
    context.test_axis = context.axe[1][0];

    if (context.axe[1][0].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[0][0]) * box.m_extent.Z + GameMath::Fabs(context.ae[2][0]) * box.m_extent.X;

        if (OBBTri_Check_Intersection_Cross_Axis(context, context.an[1], leb0)) {
            return false;
        }
    }

    context.axe[1][1] = Vector3::Cross_Product(context.a[1], context.e[1]);
    context.test_axis = context.axe[1][1];

    if (context.axe[1][1].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[0][1]) * box.m_extent.Z + GameMath::Fabs(context.ae[2][1]) * box.m_extent.X;

        if (OBBTri_Check_Intersection_Cross_Axis(context, -context.an[1], leb0)) {
            return false;
        }
    }

    context.axe[1][2] = Vector3::Cross_Product(context.a[1], context.e[2]);
    context.test_axis = context.axe[1][2];
    context.ae[0][2] = Vector3::Dot_Product(context.a[0], context.e[2]);

    if (context.axe[1][2].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[0][2]) * box.m_extent.Z + GameMath::Fabs(context.ae[2][2]) * box.m_extent.X;

        if (OBBTri_Check_Intersection_Cross_Axis(context, -context.an[1], leb0)) {
            return false;
        }
    }

    context.axe[2][0] = Vector3::Cross_Product(context.a[2], context.e[0]);
    context.test_axis = context.axe[2][0];

    if (context.axe[2][0].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[0][0]) * box.m_extent.Y + GameMath::Fabs(context.ae[1][0]) * box.m_extent.X;

        if (OBBTri_Check_Intersection_Cross_Axis(context, context.an[2], leb0)) {
            return false;
        }
    }

    context.axe[2][1] = Vector3::Cross_Product(context.a[2], context.e[1]);
    context.test_axis = context.axe[2][1];

    if (context.axe[2][1].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[0][1]) * box.m_extent.Y + GameMath::Fabs(context.ae[1][1]) * box.m_extent.X;

        if (OBBTri_Check_Intersection_Cross_Axis(context, -context.an[2], leb0)) {
            return false;
        }
    }

    context.axe[2][2] = Vector3::Cross_Product(context.a[2], context.e[2]);
    context.test_axis = context.axe[2][2];

    if (context.axe[2][2].Length2() > GAMEMATH_EPSILON2) {
        float leb0 = GameMath::Fabs(context.ae[0][2]) * box.m_extent.Y + GameMath::Fabs(context.ae[1][2]) * box.m_extent.X;

        if (OBBTri_Check_Intersection_Cross_Axis(context, -context.an[2], leb0)) {
            return false;
        }
    }

    return true;
}
