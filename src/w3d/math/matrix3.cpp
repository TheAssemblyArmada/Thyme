/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief 3x3 Matrix class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "matrix3.h"
#include "quat.h"

const Matrix3 Matrix3::Identity(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

Matrix3::Matrix3(const Matrix3D &m)
{
    Row[0].Set(m[0][0], m[0][1], m[0][2]);
    Row[1].Set(m[1][0], m[1][1], m[1][2]);
    Row[2].Set(m[2][0], m[2][1], m[2][2]);
}

Matrix3::Matrix3(const Matrix4 &m)
{
    Row[0].Set(m[0][0], m[0][1], m[0][2]);
    Row[1].Set(m[1][0], m[1][1], m[1][2]);
    Row[2].Set(m[2][0], m[2][1], m[2][2]);
}

void Matrix3::Set(const Matrix3D &m)
{
    Row[0].Set(m[0][0], m[0][1], m[0][2]);
    Row[1].Set(m[1][0], m[1][1], m[1][2]);
    Row[2].Set(m[2][0], m[2][1], m[2][2]);
}

void Matrix3::Set(const Matrix4 &m)
{
    Row[0].Set(m[0][0], m[0][1], m[0][2]);
    Row[1].Set(m[1][0], m[1][1], m[1][2]);
    Row[2].Set(m[2][0], m[2][1], m[2][2]);
}

void Matrix3::Set(const Quaternion &q)
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

Matrix3 &Matrix3::operator=(const Matrix3D &m)
{
    Row[0].Set(m[0][0], m[0][1], m[0][2]);
    Row[1].Set(m[1][0], m[1][1], m[1][2]);
    Row[2].Set(m[2][0], m[2][1], m[2][2]);

    return *this;
}

Matrix3 &Matrix3::operator=(const Matrix4 &m)
{
    Row[0].Set(m[0][0], m[0][1], m[0][2]);
    Row[1].Set(m[1][0], m[1][1], m[1][2]);
    Row[2].Set(m[2][0], m[2][1], m[2][2]);

    return *this;
}

void Matrix3::Multiply(const Matrix3D &a, const Matrix3 &b, Matrix3 *res)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]
    (*res)[0][0] = ROWCOL(0, 0);
    (*res)[0][1] = ROWCOL(0, 1);
    (*res)[0][2] = ROWCOL(0, 2);
    (*res)[1][0] = ROWCOL(1, 0);
    (*res)[1][1] = ROWCOL(1, 1);
    (*res)[1][2] = ROWCOL(1, 2);
    (*res)[2][0] = ROWCOL(2, 0);
    (*res)[2][1] = ROWCOL(2, 1);
    (*res)[2][2] = ROWCOL(2, 2);
#undef ROWCOL
}

void Matrix3::Multiply(const Matrix3 &a, const Matrix3D &b, Matrix3 *res)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]
    (*res)[0][0] = ROWCOL(0, 0);
    (*res)[0][1] = ROWCOL(0, 1);
    (*res)[0][2] = ROWCOL(0, 2);
    (*res)[1][0] = ROWCOL(1, 0);
    (*res)[1][1] = ROWCOL(1, 1);
    (*res)[1][2] = ROWCOL(1, 2);
    (*res)[2][0] = ROWCOL(2, 0);
    (*res)[2][1] = ROWCOL(2, 1);
    (*res)[2][2] = ROWCOL(2, 2);
#undef ROWCOL
}

Matrix3 operator*(const Matrix3D &a, const Matrix3 &b)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]
    return Matrix3(Vector3(ROWCOL(0, 0), ROWCOL(0, 1), ROWCOL(0, 2)),
        Vector3(ROWCOL(1, 0), ROWCOL(1, 1), ROWCOL(1, 2)),
        Vector3(ROWCOL(2, 0), ROWCOL(2, 1), ROWCOL(2, 2)));
#undef ROWCOL
}

Matrix3 operator*(const Matrix3 &a, const Matrix3D &b)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]
    return Matrix3(Vector3(ROWCOL(0, 0), ROWCOL(0, 1), ROWCOL(0, 2)),
        Vector3(ROWCOL(1, 0), ROWCOL(1, 1), ROWCOL(1, 2)),
        Vector3(ROWCOL(2, 0), ROWCOL(2, 1), ROWCOL(2, 2)));
#undef ROWCOL
}

void Matrix3::Multiply(const Matrix3 &A, const Matrix3 &B, Matrix3 *set_res)
{
    Matrix3 tmp;
    const Matrix3 *Aptr;
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
}

int Matrix3::Is_Orthogonal() const
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

    if (GameMath::Fabs(x.Length() - 1.0f) > GAMEMATH_EPSILON) {
        return 0;
    }

    if (GameMath::Fabs(y.Length() - 1.0f) > GAMEMATH_EPSILON) {
        return 0;
    }

    if (GameMath::Fabs(z.Length() - 1.0f) > GAMEMATH_EPSILON) {
        return 0;
    }

    return 1;
}

void Matrix3::Re_Orthogonalize()
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
        x /= len;
    }

    len = y.Length();

    if (len < GAMEMATH_EPSILON) {
        Make_Identity();
        return;
    } else {
        y /= len;
    }

    len = z.Length();

    if (len < GAMEMATH_EPSILON) {
        Make_Identity();
        return;
    } else {
        z /= len;
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
