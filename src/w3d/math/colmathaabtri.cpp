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
#include "tri.h"
#include "vector3.h"

struct AABTCollisionStruct
{
    bool start_bad;
    float max_frac;
    int axis_id;
    int side;
    int point;
    int test_axis_id;
    int test_side;
    int test_point;
    Vector3 test_axis;
    Vector3 d;
    Vector3 move;
    float ae[3][3];
    float an[3];
    Vector3 axe[3][3];
    Vector3 e[3];
    Vector3 n;
    Vector3 final_d;
    // #BUGFIX Initialize important members
    const AABoxClass *box = nullptr;
    const TriClass *tri = nullptr;
    const Vector3 *box_move = nullptr;
    const Vector3 *tri_move = nullptr;

    void Init(const AABoxClass &b, const Vector3 &move1, const TriClass &t, const Vector3 &move2)
    {
        start_bad = true;
        max_frac = -0.001f;
        axis_id = 0;
        point = 0;
        side = 0;
        box = &b;
        tri = &t;
        box_move = &move1;
        tri_move = &move2;
        d = *(t.V[0]) - b.m_center;
        move = move1 - move2;
        e[0] = *(t.V[1]) - *(t.V[0]);
        e[1] = *(t.V[2]) - *(t.V[0]);
        e[2] = e[1] - e[0];
        n = Vector3::Cross_Product(e[0], e[1]);
    }
};

static bool AABTri_Separation_Test(AABTCollisionStruct &CollisionContext, float lp, float leb0, float leb1)
{
    float f1 = 0;
    float f2 = lp - leb0;

    if (f2 <= 0) {
        f1 = CollisionContext.test_axis.Length() * 0.001f;
    }

    if (-f1 < f2) {
        CollisionContext.start_bad = false;
        float f3 = leb1 - leb0;

        if (f3 <= 0) {
            CollisionContext.axis_id = CollisionContext.test_axis_id;
            CollisionContext.max_frac = 1;
            return true;
        }

        float f4 = f2 / f3;

        if (f4 >= 1) {
            CollisionContext.axis_id = CollisionContext.test_axis_id;
            CollisionContext.max_frac = 1;
            return true;
        }

        if (f4 > CollisionContext.max_frac) {
            CollisionContext.max_frac = f4;
            CollisionContext.point = CollisionContext.test_point;
            CollisionContext.axis_id = CollisionContext.test_axis_id;
            CollisionContext.side = CollisionContext.test_side;
        }
    }

    return false;
}

static bool AABTri_Check_Unk_Axis(AABTCollisionStruct &CollisionContext)
{
    float dp1 = Vector3::Dot_Product(CollisionContext.d, CollisionContext.test_axis);
    float dp2 = Vector3::Dot_Product(CollisionContext.move, CollisionContext.test_axis);

    if (dp1 >= 0) {
        CollisionContext.test_side = 1;
    } else {
        dp1 = -dp1;
        dp2 = -dp2;
        CollisionContext.test_axis = -CollisionContext.test_axis;
        CollisionContext.test_side = -1;
    }

    float f1 = GameMath::Fabs(CollisionContext.an[0]) * CollisionContext.box->m_extent.X
        + GameMath::Fabs(CollisionContext.an[1]) * CollisionContext.box->m_extent.Y
        + GameMath::Fabs(CollisionContext.an[2]) * CollisionContext.box->m_extent.Z;
    float f2 = f1 + dp2;
    CollisionContext.test_point = 0;
    return AABTri_Separation_Test(CollisionContext, dp1, f1, f2);
}

static bool AABTri_Check_Axis(AABTCollisionStruct &CollisionContext)
{
    float dp1 = Vector3::Dot_Product(CollisionContext.d, CollisionContext.test_axis);
    float dp2 = Vector3::Dot_Product(CollisionContext.move, CollisionContext.test_axis);

    if (dp1 >= 0) {
        CollisionContext.test_side = 1;
    } else {
        dp1 = -dp1;
        dp2 = -dp2;
        CollisionContext.test_axis = -CollisionContext.test_axis;
        CollisionContext.test_side = -1;
    }

    float leb0 = GameMath::Fabs(CollisionContext.test_axis.X) * CollisionContext.box->m_extent.X
        + GameMath::Fabs(CollisionContext.test_axis.Y) * CollisionContext.box->m_extent.Y
        + GameMath::Fabs(CollisionContext.test_axis.Z) * CollisionContext.box->m_extent.Z;
    float leb1 = dp2 + leb0;
    float f1 = Vector3::Dot_Product(CollisionContext.e[0], CollisionContext.test_axis);

    if (f1 > 0) {
        f1 = 0;
    }

    float f2 = Vector3::Dot_Product(CollisionContext.e[1], CollisionContext.test_axis);

    if (f1 <= f2) {
        f2 = f1;
    }

    float lp = dp1 + f2;
    return AABTri_Separation_Test(CollisionContext, lp, leb0, leb1);
}

static bool AABTri_Check_Cross_Axis(AABTCollisionStruct &CollisionContext, float unk1, int testpoint, float unk2)
{
    float dp1 = Vector3::Dot_Product(CollisionContext.d, CollisionContext.test_axis);
    float dp2 = Vector3::Dot_Product(CollisionContext.move, CollisionContext.test_axis);

    if (dp1 >= 0) {
        CollisionContext.test_side = 1;
    } else {
        dp1 = -dp1;
        dp2 = -dp2;
        unk1 = -unk1;
        CollisionContext.test_axis = -CollisionContext.test_axis;
        CollisionContext.test_side = -1;
    }

    float leb1 = dp2 + unk2;
    CollisionContext.test_point = 0;

    if (unk1 >= 0) {
        unk1 = 0.0f;
    } else {
        CollisionContext.test_point = testpoint;
    }

    float lp = dp1 + unk1;
    return AABTri_Separation_Test(CollisionContext, lp, unk2, leb1);
}

static bool AABTri_Check_Basis_Axis(AABTCollisionStruct &CollisionContext, float unk1, float unk2, float unk3)
{
    float dp1 = Vector3::Dot_Product(CollisionContext.d, CollisionContext.test_axis);
    float dp2 = Vector3::Dot_Product(CollisionContext.move, CollisionContext.test_axis);

    if (dp1 >= 0) {
        CollisionContext.test_side = 1;
    } else {
        dp1 = -dp1;
        dp2 = -dp2;
        unk2 = -unk2;
        unk3 = -unk3;
        CollisionContext.test_axis = -CollisionContext.test_axis;
        CollisionContext.test_side = -1;
    }

    float leb1 = dp2 + unk1;
    CollisionContext.test_point = 0;

    if (unk2 >= 0.0f) {
        unk2 = 0.0f;
    } else {
        CollisionContext.test_point = 1;
    }

    if (unk2 <= unk3) {
        unk3 = unk2;
    } else {
        CollisionContext.test_point = 2;
    }

    float lp = unk3 + dp1;
    return AABTri_Separation_Test(CollisionContext, lp, unk1, leb1);
}

static void AABTri_Compute_Contact_Normal(AABTCollisionStruct &CollisionContext, Vector3 &set_norm)
{
    switch (CollisionContext.axis_id) {
        case 0:
            set_norm = CollisionContext.n;
            set_norm.Normalize();
            break;
        case 1:
            set_norm = (float)-CollisionContext.side * CollisionContext.n;
            set_norm.Normalize();
            break;
        case 2:
            set_norm = (float)-CollisionContext.side * Vector3(1, 0, 0);
            break;
        case 3:
            set_norm = (float)-CollisionContext.side * Vector3(0, 1, 0);
            break;
        case 4:
            set_norm = (float)-CollisionContext.side * Vector3(0, 0, 1);
            break;
        case 5:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[0][0];
            set_norm.Normalize();
            break;
        case 6:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[1][0];
            set_norm.Normalize();
            break;
        case 7:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[2][0];
            set_norm.Normalize();
            break;
        case 8:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[0][1];
            set_norm.Normalize();
            break;
        case 9:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[1][1];
            set_norm.Normalize();
            break;
        case 10:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[2][1];
            set_norm.Normalize();
            break;
        case 11:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[0][2];
            set_norm.Normalize();
            break;
        case 12:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[1][2];
            set_norm.Normalize();
            break;
        case 13:
            set_norm = (float)-CollisionContext.side * CollisionContext.axe[2][2];
            set_norm.Normalize();
            break;
    }

    captainslog_assert(set_norm.Length2() > 0.0f);
}

bool CollisionMath::Collide(const AABoxClass &box, const Vector3 &move, const TriClass &tri, CastResultStruct *result)
{
    AABTCollisionStruct CollisionContext;
    Vector3 move2(0, 0, 0);
    CollisionContext.Init(box, move, tri, move2);
    CollisionContext.test_axis = CollisionContext.n;
    CollisionContext.test_axis_id = 1;
    CollisionContext.an[0] = CollisionContext.n.X;
    CollisionContext.an[1] = CollisionContext.n.Y;
    CollisionContext.an[2] = CollisionContext.n.Z;

    if (!AABTri_Check_Unk_Axis(CollisionContext)) {
        CollisionContext.test_axis = Vector3(1, 0, 0);
        CollisionContext.test_axis_id = 2;
        CollisionContext.ae[0][0] = CollisionContext.e[0].X;
        CollisionContext.ae[0][1] = CollisionContext.e[1].X;

        if (!AABTri_Check_Basis_Axis(CollisionContext, box.m_extent.X, CollisionContext.e[0].X, CollisionContext.e[1].X)) {
            CollisionContext.test_axis = Vector3(0, 1, 0);
            CollisionContext.test_axis_id = 3;
            CollisionContext.ae[1][0] = CollisionContext.e[0].Y;
            CollisionContext.ae[1][1] = CollisionContext.e[1].Y;

            if (!AABTri_Check_Basis_Axis(
                    CollisionContext, box.m_extent.Y, CollisionContext.e[0].Y, CollisionContext.e[1].Y)) {
                CollisionContext.test_axis = Vector3(0, 0, 1);
                CollisionContext.test_axis_id = 4;
                CollisionContext.ae[2][0] = CollisionContext.e[0].Z;
                CollisionContext.ae[2][1] = CollisionContext.e[1].Z;

                if (!AABTri_Check_Basis_Axis(
                        CollisionContext, box.m_extent.Z, CollisionContext.e[0].Z, CollisionContext.e[1].Z)) {
                    CollisionContext.axe[0][0] = Vector3(0, -CollisionContext.e[0].Z, CollisionContext.e[0].Y);
                    CollisionContext.test_axis = CollisionContext.axe[0][0];
                    CollisionContext.test_axis_id = 5;

                    if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                        || !AABTri_Check_Cross_Axis(CollisionContext,
                            CollisionContext.an[0],
                            2,
                            (GameMath::Fabs(CollisionContext.ae[2][0]) * box.m_extent.Y
                                + GameMath::Fabs(CollisionContext.ae[1][0]) * box.m_extent.Z))) {
                        CollisionContext.axe[0][1] = Vector3(0, -CollisionContext.e[1].Z, CollisionContext.e[1].Y);
                        CollisionContext.test_axis = CollisionContext.axe[0][1];
                        CollisionContext.test_axis_id = 8;

                        if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                            || !AABTri_Check_Cross_Axis(CollisionContext,
                                -CollisionContext.an[0],
                                1,
                                (GameMath::Fabs(CollisionContext.ae[2][1]) * box.m_extent.Y
                                    + GameMath::Fabs(CollisionContext.ae[1][1]) * box.m_extent.Z))) {
                            CollisionContext.ae[0][2] = CollisionContext.e[2].X;
                            CollisionContext.ae[1][2] = CollisionContext.e[2].Y;
                            CollisionContext.ae[2][2] = CollisionContext.e[2].Z;
                            CollisionContext.axe[0][2] = Vector3(0, -CollisionContext.e[2].Z, CollisionContext.e[2].Y);
                            CollisionContext.test_axis = CollisionContext.axe[0][2];
                            CollisionContext.test_axis_id = 11;

                            if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                || !AABTri_Check_Cross_Axis(CollisionContext,
                                    -CollisionContext.an[0],
                                    1,
                                    (GameMath::Fabs(CollisionContext.ae[2][2]) * box.m_extent.Y
                                        + GameMath::Fabs(CollisionContext.ae[1][2]) * box.m_extent.Z))) {
                                CollisionContext.axe[1][0] = Vector3(CollisionContext.e[0].Z, 0, -CollisionContext.e[0].X);
                                CollisionContext.test_axis = CollisionContext.axe[1][0];
                                CollisionContext.test_axis_id = 6;

                                if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                    || !AABTri_Check_Cross_Axis(CollisionContext,
                                        CollisionContext.an[1],
                                        2,
                                        (GameMath::Fabs(CollisionContext.ae[2][0]) * box.m_extent.X
                                            + GameMath::Fabs(CollisionContext.ae[0][0]) * box.m_extent.Z))) {
                                    CollisionContext.axe[1][1] =
                                        Vector3(CollisionContext.e[1].Z, 0, -CollisionContext.e[1].X);
                                    CollisionContext.test_axis = CollisionContext.axe[1][1];
                                    CollisionContext.test_axis_id = 9;

                                    if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                        || !AABTri_Check_Cross_Axis(CollisionContext,
                                            -CollisionContext.an[1],
                                            1,
                                            (GameMath::Fabs(CollisionContext.ae[2][1]) * box.m_extent.X
                                                + GameMath::Fabs(CollisionContext.ae[0][1]) * box.m_extent.Z))) {
                                        CollisionContext.axe[1][2] =
                                            Vector3(CollisionContext.e[2].Z, 0, -CollisionContext.e[2].X);
                                        CollisionContext.test_axis = CollisionContext.axe[1][2];
                                        CollisionContext.test_axis_id = 12;

                                        if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                            || !AABTri_Check_Cross_Axis(CollisionContext,
                                                -CollisionContext.an[1],
                                                1,
                                                (GameMath::Fabs(CollisionContext.ae[2][2]) * box.m_extent.X
                                                    + GameMath::Fabs(CollisionContext.ae[0][2]) * box.m_extent.Z))) {
                                            CollisionContext.axe[2][0] =
                                                Vector3(-CollisionContext.e[0].Y, CollisionContext.e[0].X, 0);
                                            CollisionContext.test_axis = CollisionContext.axe[2][0];
                                            CollisionContext.test_axis_id = 7;

                                            if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                || !AABTri_Check_Cross_Axis(CollisionContext,
                                                    CollisionContext.an[2],
                                                    2,
                                                    (GameMath::Fabs(CollisionContext.ae[1][0]) * box.m_extent.X
                                                        + GameMath::Fabs(CollisionContext.ae[0][0]) * box.m_extent.Y))) {
                                                CollisionContext.axe[2][1] =
                                                    Vector3(-CollisionContext.e[1].Y, CollisionContext.e[1].X, 0);
                                                CollisionContext.test_axis = CollisionContext.axe[2][1];
                                                CollisionContext.test_axis_id = 10;

                                                if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                    || !AABTri_Check_Cross_Axis(CollisionContext,
                                                        -CollisionContext.an[2],
                                                        1,
                                                        (GameMath::Fabs(CollisionContext.ae[1][1]) * box.m_extent.X
                                                            + GameMath::Fabs(CollisionContext.ae[0][1]) * box.m_extent.Y))) {
                                                    CollisionContext.axe[2][2] =
                                                        Vector3(-CollisionContext.e[2].Y, CollisionContext.e[2].X, 0);
                                                    CollisionContext.test_axis = CollisionContext.axe[2][2];
                                                    CollisionContext.test_axis_id = 13;

                                                    if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                        || !AABTri_Check_Cross_Axis(CollisionContext,
                                                            -CollisionContext.an[2],
                                                            1,
                                                            (GameMath::Fabs(CollisionContext.ae[1][2]) * box.m_extent.X
                                                                + GameMath::Fabs(CollisionContext.ae[0][2])
                                                                    * box.m_extent.Y))) {

                                                        if (!CollisionContext.start_bad) {
                                                            CollisionContext.test_point = CollisionContext.point;
                                                            CollisionContext.test_axis_id = CollisionContext.axis_id;
                                                            CollisionContext.test_axis = Vector3(
                                                                0, -CollisionContext.move.Z, CollisionContext.move.Y);

                                                            if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                                || !AABTri_Check_Axis(CollisionContext)) {
                                                                CollisionContext.test_axis = Vector3(
                                                                    CollisionContext.move.Z, 0, -CollisionContext.move.X);

                                                                if (CollisionContext.test_axis.Length2() <= GAMEMATH_EPSILON2
                                                                    || !AABTri_Check_Axis(CollisionContext)) {
                                                                    CollisionContext.test_axis =
                                                                        Vector3(-CollisionContext.move.Y,
                                                                            CollisionContext.move.X,
                                                                            0);

                                                                    if (CollisionContext.test_axis.Length2()
                                                                        > GAMEMATH_EPSILON2) {
                                                                        AABTri_Check_Axis(CollisionContext);
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

    if (CollisionContext.max_frac < 0.0f) {
        CollisionContext.max_frac = 0.0f;
    }
    if (CollisionContext.start_bad) {
        result->start_bad = true;
        result->fraction = 0;
        result->normal = CollisionContext.n;
        return true;
    }
    if (result->fraction >= CollisionContext.max_frac && CollisionContext.max_frac < 1.0f) {
        move2 = Vector3(0, 0, 0);
        AABTri_Compute_Contact_Normal(CollisionContext, move2);

        if (GameMath::Fabs(CollisionContext.max_frac - result->fraction) > GAMEMATH_EPSILON
            || Vector3::Dot_Product(result->normal, move) > Vector3::Dot_Product(move2, move)) {
            result->normal = move2;
            captainslog_assert(GameMath::Fabs(result->normal.Length() - 1.0f) < GAMEMATH_EPSILON);
        }

        result->fraction = CollisionContext.max_frac;
        return true;
    }
    return false;
}
