/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief 4x3 Matrix class with an implicit 4th row for 4x4 matrix math.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "matrix3d.h"
#include "gamemath.h"
#include "matrix3.h"
#include "plane.h"
#include "quat.h"

using GameMath::Atan2;
using GameMath::Fabs;
using GameMath::Inv_Sqrt;
using GameMath::Sqrt;

const Matrix3D Matrix3D::Identity(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

int Matrix3D::Is_Orthogonal() const
{
    Vector3 x(Row[0].X, Row[0].Y, Row[0].Z);
    Vector3 y(Row[1].X, Row[1].Y, Row[1].Z);
    Vector3 z(Row[2].X, Row[2].Y, Row[2].Z);

    if (Vector3::Dot_Product(x, y) > GAMEMATH_EPSILON) {
        return 0;
    }

    if (Vector3::Dot_Product(y, z) > GAMEMATH_EPSILON) {
        return 0;
    }

    if (Vector3::Dot_Product(z, x) > GAMEMATH_EPSILON) {
        return 0;
    }

    if (Fabs(x.Length2() - 1.0f) > GAMEMATH_EPSILON) {
        return 0;
    }

    if (Fabs(y.Length2() - 1.0f) > GAMEMATH_EPSILON) {
        return 0;
    }

    if (Fabs(z.Length2() - 1.0f) > GAMEMATH_EPSILON) {
        return 0;
    }

    return 1;
}

void Matrix3D::Get_Orthogonal_Inverse(Matrix3D &inv) const
{
    inv.Row[0][0] = Row[0][0];
    inv.Row[0][1] = Row[1][0];
    inv.Row[0][2] = Row[2][0];
    inv.Row[1][0] = Row[0][1];
    inv.Row[1][1] = Row[1][1];
    inv.Row[1][2] = Row[2][1];
    inv.Row[2][0] = Row[0][2];
    inv.Row[2][1] = Row[1][2];
    inv.Row[2][2] = Row[2][2];
    Vector3 trans = Get_Translation();
    trans = inv.Rotate_Vector(trans);
    trans = -trans;
    inv.Row[0][3] = trans[0];
    inv.Row[1][3] = trans[1];
    inv.Row[2][3] = trans[2];
}

float Matrix3D::Get_X_Rotation() const
{
    return Atan2(Row[2][1], Row[1][1]);
}

float Matrix3D::Get_Y_Rotation() const
{
    return Atan2(Row[0][2], Row[2][2]);
}

float Matrix3D::Get_Z_Rotation() const
{
    return Atan2(Row[1][0], Row[0][0]);
}

void Matrix3D::Look_At(const Vector3 &p, const Vector3 &t, float roll)
{
    float dx, dy, dz;
    float sinp, cosp;
    float siny, cosy;
    dx = (t[0] - p[0]);
    dy = (t[1] - p[1]);
    dz = (t[2] - p[2]);
    float rad2 = dx * dx + dy * dy;
    float len = Sqrt(rad2);

    if (rad2 != 0.0f) {
        float inv_len = 1.0f / len;
        siny = dy * inv_len;
        cosy = dx * inv_len;
    } else {
        siny = 0.0f;
        cosy = 1.0f;
    }

    rad2 += dz * dz;

    if (rad2 != 0.0f) {
        float inv_len2 = Inv_Sqrt(rad2);
        sinp = dz * inv_len2;
        cosp = len * inv_len2;
    } else {
        sinp = 0.0f;
        cosp = 1.0f;
    }

    Row[0].X = 0.0f;
    Row[0].Y = 0.0f;
    Row[0].Z = -1.0f;
    Row[1].X = -1.0f;
    Row[1].Y = 0.0f;
    Row[1].Z = 0.0f;
    Row[2].X = 0.0f;
    Row[2].Y = 1.0f;
    Row[2].Z = 0.0f;
    Row[0].W = p.X;
    Row[1].W = p.Y;
    Row[2].W = p.Z;

    Rotate_Y(siny, cosy);
    Rotate_X(sinp, cosp);
    Rotate_Z(-roll);
}

void Matrix3D::Obj_Look_At(const Vector3 &p, const Vector3 &t, float roll)
{
    float dx, dy, dz;
    float len1, len2;
    float sinp, cosp;
    float siny, cosy;
    dx = (t[0] - p[0]);
    dy = (t[1] - p[1]);
    dz = (t[2] - p[2]);
    len1 = Sqrt(dx * dx + dy * dy + dz * dz);
    len2 = Sqrt(dx * dx + dy * dy);

    if (len1 != 0.0f) {
        sinp = dz / len1;
        cosp = len2 / len1;
    } else {
        sinp = 0.0f;
        cosp = 1.0f;
    }

    if (len2 != 0.0f) {
        siny = dy / len2;
        cosy = dx / len2;
    } else {
        siny = 0.0f;
        cosy = 1.0f;
    }

    Make_Identity();
    Translate(p);
    Rotate_Z(siny, cosy);
    Rotate_Y(-sinp, cosp);
    Rotate_X(roll);
}

Vector3 Matrix3D::Rotate_Vector(const Vector3 &vect) const
{
    return Vector3((Row[0][0] * vect[0] + Row[0][1] * vect[1] + Row[0][2] * vect[2]),
        (Row[1][0] * vect[0] + Row[1][1] * vect[1] + Row[1][2] * vect[2]),
        (Row[2][0] * vect[0] + Row[2][1] * vect[1] + Row[2][2] * vect[2]));
}

Vector3 Matrix3D::Inverse_Rotate_Vector(const Vector3 &vect) const
{
    return Vector3((Row[0][0] * vect[0] + Row[1][0] * vect[1] + Row[2][0] * vect[2]),
        (Row[0][1] * vect[0] + Row[1][1] * vect[1] + Row[2][1] * vect[2]),
        (Row[0][2] * vect[0] + Row[1][2] * vect[1] + Row[2][2] * vect[2]));
}

void Matrix3D::Set_Rotation(const Quaternion &q)
{
    Row[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
    Row[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
    Row[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));
    Row[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
    Row[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
    Row[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));
    Row[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
    Row[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
    Row[2][2] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));
}

void Matrix3D::Set_Rotation(const Matrix3 &m)
{
    Row[0][0] = m[0][0];
    Row[0][1] = m[0][1];
    Row[0][2] = m[0][2];
    Row[1][0] = m[1][0];
    Row[1][1] = m[1][1];
    Row[1][2] = m[1][2];
    Row[2][0] = m[2][0];
    Row[2][1] = m[2][1];
    Row[2][2] = m[2][2];
}

void Matrix3D::Get_Inverse(Matrix3D &inv) const
{
    Get_Orthogonal_Inverse(inv);
}

void Matrix3D::Set(const Matrix3 &rot, const Vector3 &pos)
{
    Row[0].Set(rot[0][0], rot[0][1], rot[0][2], pos[0]);
    Row[1].Set(rot[1][0], rot[1][1], rot[1][2], pos[1]);
    Row[2].Set(rot[2][0], rot[2][1], rot[2][2], pos[2]);
}

void Matrix3D::Set(const Quaternion &rot, const Vector3 &pos)
{
    Set_Rotation(rot);
    Set_Translation(pos);
}

void Matrix3D::Copy_3x3_Matrix(float matrix[3][3])
{
    Row[0][0] = matrix[0][0];
    Row[0][1] = matrix[0][1];
    Row[0][2] = matrix[0][2];
    Row[0][3] = 0;
    Row[1][0] = matrix[1][0];
    Row[1][1] = matrix[1][1];
    Row[1][2] = matrix[1][2];
    Row[1][3] = 0;
    Row[2][0] = matrix[2][0];
    Row[2][1] = matrix[2][1];
    Row[2][2] = matrix[2][2];
    Row[2][3] = 0;
}

void Matrix3D::Multiply(const Matrix3D &A, const Matrix3D &B, Matrix3D *set_res)
{
    captainslog_assert(set_res != nullptr);
    Matrix3D tmp;
    const Matrix3D *Aptr;
    float tmp1, tmp2, tmp3;

    if (set_res == &A) {
        tmp = A;
        Aptr = &tmp;
    } else {
        Aptr = &A;
    }

    tmp1 = B[0][0];
    tmp2 = B[1][0];
    tmp3 = B[2][0];
    (*set_res)[0][0] = (*Aptr)[0][0] * tmp1 + (*Aptr)[0][1] * tmp2 + (*Aptr)[0][2] * tmp3;
    (*set_res)[1][0] = (*Aptr)[1][0] * tmp1 + (*Aptr)[1][1] * tmp2 + (*Aptr)[1][2] * tmp3;
    (*set_res)[2][0] = (*Aptr)[2][0] * tmp1 + (*Aptr)[2][1] * tmp2 + (*Aptr)[2][2] * tmp3;
    tmp1 = B[0][1];
    tmp2 = B[1][1];
    tmp3 = B[2][1];
    (*set_res)[0][1] = (*Aptr)[0][0] * tmp1 + (*Aptr)[0][1] * tmp2 + (*Aptr)[0][2] * tmp3;
    (*set_res)[1][1] = (*Aptr)[1][0] * tmp1 + (*Aptr)[1][1] * tmp2 + (*Aptr)[1][2] * tmp3;
    (*set_res)[2][1] = (*Aptr)[2][0] * tmp1 + (*Aptr)[2][1] * tmp2 + (*Aptr)[2][2] * tmp3;
    tmp1 = B[0][2];
    tmp2 = B[1][2];
    tmp3 = B[2][2];
    (*set_res)[0][2] = (*Aptr)[0][0] * tmp1 + (*Aptr)[0][1] * tmp2 + (*Aptr)[0][2] * tmp3;
    (*set_res)[1][2] = (*Aptr)[1][0] * tmp1 + (*Aptr)[1][1] * tmp2 + (*Aptr)[1][2] * tmp3;
    (*set_res)[2][2] = (*Aptr)[2][0] * tmp1 + (*Aptr)[2][1] * tmp2 + (*Aptr)[2][2] * tmp3;
    tmp1 = B[0][3];
    tmp2 = B[1][3];
    tmp3 = B[2][3];
    (*set_res)[0][3] = (*Aptr)[0][0] * tmp1 + (*Aptr)[0][1] * tmp2 + (*Aptr)[0][2] * tmp3 + (*Aptr)[0][3];
    (*set_res)[1][3] = (*Aptr)[1][0] * tmp1 + (*Aptr)[1][1] * tmp2 + (*Aptr)[1][2] * tmp3 + (*Aptr)[1][3];
    (*set_res)[2][3] = (*Aptr)[2][0] * tmp1 + (*Aptr)[2][1] * tmp2 + (*Aptr)[2][2] * tmp3 + (*Aptr)[2][3];
}

void Matrix3D::Transform_Min_Max_AABox(const Vector3 &min, const Vector3 &max, Vector3 *set_min, Vector3 *set_max) const
{
    captainslog_assert(set_min != &min);
    captainslog_assert(set_max != &max);
    float tmp0, tmp1;
    set_min->X = set_max->X = Row[0][3];
    set_min->Y = set_max->Y = Row[1][3];
    set_min->Z = set_max->Z = Row[2][3];

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tmp0 = Row[i][j] * min[j];
            tmp1 = Row[i][j] * max[j];

            if (tmp0 < tmp1) {
                (*set_min)[i] += tmp0;
                (*set_max)[i] += tmp1;
            } else {
                (*set_min)[i] += tmp1;
                (*set_max)[i] += tmp0;
            }
        }
    }
}

void Matrix3D::Transform_Center_Extent_AABox(
    const Vector3 &center, const Vector3 &extent, Vector3 *set_center, Vector3 *set_extent) const
{
    captainslog_assert(set_center != &center);
    captainslog_assert(set_extent != &extent);

    for (int i = 0; i < 3; i++) {
        (*set_center)[i] = Row[i][3];
        (*set_extent)[i] = 0.0f;

        for (int j = 0; j < 3; j++) {
            (*set_center)[i] += Row[i][j] * center[j];
            (*set_extent)[i] += Fabs(Row[i][j] * extent[j]);
        }
    }
}

bool Matrix3D::Solve_Linear_System(Matrix3D &system)
{
    if (system[0][0] == 0.0f) {
        return false;
    }

    system[0] *= 1.0f / system[0][0];
    system[1] -= system[1][0] * system[0];
    system[2] -= system[2][0] * system[0];

    if (system[1][1] == 0.0f) {
        return false;
    }

    system[1] *= 1.0f / system[1][1];
    system[2] -= system[2][1] * system[1];

    if (system[2][2] == 0.0f) {
        return false;
    }

    system[2] *= 1.0f / system[2][2];
    system[1] -= system[1][2] * system[2];
    system[0] -= system[0][2] * system[2];
    system[0] -= system[0][1] * system[1];

    return true;
}

void Matrix3D::Re_Orthogonalize()
{
    Vector3 x(Row[0][0], Row[0][1], Row[0][2]);
    Vector3 y(Row[1][0], Row[1][1], Row[1][2]);
    Vector3 z;
    Vector3::Cross_Product(x, y, &z);
    Vector3::Cross_Product(z, x, &y);
    float len = x.Length();

    if (len < GAMEMATH_EPSILON) {
        Make_Identity();

        return;
    } else {
        x *= 1.0f / len;
    }

    len = y.Length();

    if (len < GAMEMATH_EPSILON) {
        Make_Identity();

        return;
    } else {
        y *= 1.0f / len;
    }

    len = z.Length();

    if (len < GAMEMATH_EPSILON) {
        Make_Identity();

        return;
    } else {
        z *= 1.0f / len;
    }

    Row[0][0] = x.X;
    Row[0][1] = x.Y;
    Row[0][2] = x.Z;
    Row[1][0] = y.X;
    Row[1][1] = y.Y;
    Row[1][2] = y.Z;
    Row[2][0] = z.X;
    Row[2][1] = z.Y;
    Row[2][2] = z.Z;
}

void Matrix3D::Build_Transform_Matrix(Vector3 &p, Vector3 &dir)
{
    float siny, cosy;
    float tmp1, tmp2;

    float len = GameMath::Sqrt(dir.X * dir.X + dir.Y * dir.Y);
    float z = dir.Z;

    if (len != 0.0f) {
        float inv_len = 1.0f / len;
        siny = dir.Y * inv_len;
        cosy = dir.X * inv_len;
    } else {
        siny = 0.0f;
        cosy = 1.0f;
    }

    Make_Identity();
    Translate(p);

    tmp1 = Row[0][0];
    tmp2 = Row[0][1];
    Row[0][0] = cosy * tmp1 + siny * tmp2;
    Row[0][1] = cosy * tmp2 - siny * tmp1;

    tmp1 = Row[1][0];
    tmp2 = Row[1][1];
    Row[1][0] = cosy * tmp1 + siny * tmp2;
    Row[1][1] = cosy * tmp2 - siny * tmp1;

    tmp1 = Row[2][0];
    tmp2 = Row[2][1];
    Row[2][0] = cosy * tmp1 + siny * tmp2;
    Row[2][1] = cosy * tmp2 - siny * tmp1;

    float inz = -z;

    tmp1 = Row[0][0];
    tmp2 = Row[0][2];
    Row[0][0] = len * tmp1 - inz * tmp2;
    Row[0][2] = inz * tmp1 + len * tmp2;

    tmp1 = Row[1][0];
    tmp2 = Row[1][2];
    Row[1][0] = len * tmp1 - inz * tmp2;
    Row[1][2] = inz * tmp1 + len * tmp2;

    tmp1 = Row[2][0];
    tmp2 = Row[2][2];
    Row[2][0] = len * tmp1 - inz * tmp2;
    Row[2][2] = inz * tmp1 + len * tmp2;
}
Matrix3D Lerp(const Matrix3D &A, const Matrix3D &B, float factor)
{
    captainslog_assert(factor >= 0.0f);
    captainslog_assert(factor <= 1.0f);
    Vector3 pos = Lerp(A.Get_Translation(), B.Get_Translation(), factor);
    Quaternion rot;
    Slerp(rot, Build_Quaternion(A), Build_Quaternion(B), factor);
    return Matrix3D(rot, pos);
}
