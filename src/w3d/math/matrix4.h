/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief 4x4 Matrix class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "matrix3.h"
#include "matrix3d.h"
#include "vector3.h"
#include "vector4.h"

class Matrix4
{
public:
    __forceinline Matrix4() {}

    __forceinline Matrix4(const Matrix4 &m)
    {
        Row[0] = m.Row[0];
        Row[1] = m.Row[1];
        Row[2] = m.Row[2];
        Row[3] = m.Row[3];
    }

    __forceinline explicit Matrix4(bool identity)
    {
        if (identity) {
            Make_Identity();
        }
    }

    __forceinline explicit Matrix4(const Matrix3D &m) { Init(m); }

    __forceinline explicit Matrix4(const Vector4 &v0, const Vector4 &v1, const Vector4 &v2, const Vector4 &v3)
    {
        Init(v0, v1, v2, v3);
    }

    __forceinline explicit Matrix4(float f1,
        float f2,
        float f3,
        float f4,
        float f5,
        float f6,
        float f7,
        float f8,
        float f9,
        float f10,
        float f11,
        float f12,
        float f13,
        float f14,
        float f15,
        float f16)
    {
        Init(f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16);
    }

    __forceinline void Make_Identity()
    {
        Row[0].Set(1.0f, 0.0f, 0.0f, 0.0f);
        Row[1].Set(0.0f, 1.0f, 0.0f, 0.0f);
        Row[2].Set(0.0f, 0.0f, 1.0f, 0.0f);
        Row[3].Set(0.0f, 0.0f, 0.0f, 1.0f);
    }

    __forceinline void Init(const Matrix3D &m)
    {
        Row[0] = m[0];
        Row[1] = m[1];
        Row[2] = m[2];
        Row[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    __forceinline void Init(const Vector4 &v0, const Vector4 &v1, const Vector4 &v2, const Vector4 &v3)
    {
        Row[0] = v0;
        Row[1] = v1;
        Row[2] = v2;
        Row[3] = v3;
    }

    __forceinline void Init(float f1,
        float f2,
        float f3,
        float f4,
        float f5,
        float f6,
        float f7,
        float f8,
        float f9,
        float f10,
        float f11,
        float f12,
        float f13,
        float f14,
        float f15,
        float f16)
    {
        Row[0].Set(f1, f2, f3, f4);
        Row[1].Set(f5, f6, f7, f8);
        Row[2].Set(f9, f10, f11, f12);
        Row[3].Set(f13, f14, f15, f16);
    }

    __forceinline void Init_Ortho(float left, float right, float bottom, float top, float znear, float zfar)
    {
        captainslog_assert(znear >= 0.0f);
        captainslog_assert(zfar > znear);

        Make_Identity();
        Row[0][0] = 2.0f / (right - left);
        Row[0][3] = -(right + left) / (right - left);
        Row[1][1] = 2.0f / (top - bottom);
        Row[1][3] = -(top + bottom) / (top - bottom);
        Row[2][2] = -2.0f / (zfar - znear);
        Row[2][3] = -(zfar + znear) / (zfar - znear);
    }

    __forceinline void Init_Perspective(float hfov, float vfov, float znear, float zfar)
    {
        captainslog_assert(znear > 0.0f);
        captainslog_assert(zfar > znear);

        Make_Identity();
        Row[0][0] = static_cast<float>(1.0 / GameMath::Tan(hfov * 0.5));
        Row[1][1] = static_cast<float>(1.0 / GameMath::Tan(vfov * 0.5));
        Row[2][2] = -(zfar + znear) / (zfar - znear);
        Row[2][3] = static_cast<float>(-(2.0 * zfar * znear) / (zfar - znear));
        Row[3][2] = -1.0f;
        Row[3][3] = 0.0f;
    }

    __forceinline void Init_Perspective(float left, float right, float bottom, float top, float znear, float zfar)
    {
        captainslog_assert(znear > 0.0f);
        captainslog_assert(zfar > 0.0f);

        Make_Identity();
        Row[0][0] = static_cast<float>(2.0 * znear / (right - left));
        Row[0][2] = (right + left) / (right - left);
        Row[1][1] = static_cast<float>(2.0 * znear / (top - bottom));
        Row[1][2] = (top + bottom) / (top - bottom);
        Row[2][2] = -(zfar + znear) / (zfar - znear);
        Row[2][3] = static_cast<float>(-(2.0 * zfar * znear) / (zfar - znear));
        Row[3][2] = -1.0f;
        Row[3][3] = 0.0f;
    }

    __forceinline Vector4 &operator[](int i) { return Row[i]; }
    __forceinline const Vector4 &operator[](int i) const { return Row[i]; }

    __forceinline Matrix4 Transpose() const
    {
        return Matrix4(Vector4(Row[0][0], Row[1][0], Row[2][0], Row[3][0]),
            Vector4(Row[0][1], Row[1][1], Row[2][1], Row[3][1]),
            Vector4(Row[0][2], Row[1][2], Row[2][2], Row[3][2]),
            Vector4(Row[0][3], Row[1][3], Row[2][3], Row[3][3]));
    }

    __forceinline float Determinant() const
    {
        float det;
        det = (Row[0].X * Row[1].Y - Row[0].Y * Row[1].X) * (Row[2].Z * Row[3].W - Row[2].W * Row[3].Z);
        det -= (Row[0].X * Row[1].Z - Row[0].Z * Row[1].X) * (Row[2].Y * Row[3].W - Row[2].W * Row[3].Y);
        det += (Row[0].X * Row[1].W - Row[0].W * Row[1].X) * (Row[2].Y * Row[3].Z - Row[2].Z * Row[3].Y);
        det += (Row[0].Y * Row[1].Z - Row[0].Z * Row[1].Y) * (Row[2].X * Row[3].W - Row[2].W * Row[3].X);
        det -= (Row[0].Y * Row[1].W - Row[0].W * Row[1].Y) * (Row[2].X * Row[3].Z - Row[2].Z * Row[3].X);
        det += (Row[0].Z * Row[1].W - Row[0].W * Row[1].Z) * (Row[2].X * Row[3].Y - Row[2].Y * Row[3].X);

        return det;
    }

    __forceinline Matrix4 Inverse() const
    {
        float s = Determinant();

        if (s == 0.0f) {
            return Matrix4();
        }

        s = 1 / s;
        Matrix4 t;
        t.Row[0].X = s
            * (Row[1].Y * (Row[2].Z * Row[3].W - Row[2].W * Row[3].Z)
                + Row[1].Z * (Row[2].W * Row[3].Y - Row[2].Y * Row[3].W)
                + Row[1].W * (Row[2].Y * Row[3].Z - Row[2].Z * Row[3].Y));
        t.Row[0].Y = s
            * (Row[2].Y * (Row[0].Z * Row[3].W - Row[0].W * Row[3].Z)
                + Row[2].Z * (Row[0].W * Row[3].Y - Row[0].Y * Row[3].W)
                + Row[2].W * (Row[0].Y * Row[3].Z - Row[0].Z * Row[3].Y));
        t.Row[0].Z = s
            * (Row[3].Y * (Row[0].Z * Row[1].W - Row[0].W * Row[1].Z)
                + Row[3].Z * (Row[0].W * Row[1].Y - Row[0].Y * Row[1].W)
                + Row[3].W * (Row[0].Y * Row[1].Z - Row[0].Z * Row[1].Y));
        t.Row[0].W = s
            * (Row[0].Y * (Row[1].W * Row[2].Z - Row[1].Z * Row[2].W)
                + Row[0].Z * (Row[1].Y * Row[2].W - Row[1].W * Row[2].Y)
                + Row[0].W * (Row[1].Z * Row[2].Y - Row[1].Y * Row[2].Z));
        t.Row[1].X = s
            * (Row[1].Z * (Row[2].X * Row[3].W - Row[2].W * Row[3].X)
                + Row[1].W * (Row[2].Z * Row[3].X - Row[2].X * Row[3].Z)
                + Row[1].X * (Row[2].W * Row[3].Z - Row[2].Z * Row[3].W));
        t.Row[1].Y = s
            * (Row[2].Z * (Row[0].X * Row[3].W - Row[0].W * Row[3].X)
                + Row[2].W * (Row[0].Z * Row[3].X - Row[0].X * Row[3].Z)
                + Row[2].X * (Row[0].W * Row[3].Z - Row[0].Z * Row[3].W));
        t.Row[1].Z = s
            * (Row[3].Z * (Row[0].X * Row[1].W - Row[0].W * Row[1].X)
                + Row[3].W * (Row[0].Z * Row[1].X - Row[0].X * Row[1].Z)
                + Row[3].X * (Row[0].W * Row[1].Z - Row[0].Z * Row[1].W));
        t.Row[1].W = s
            * (Row[0].Z * (Row[1].W * Row[2].X - Row[1].X * Row[2].W)
                + Row[0].W * (Row[1].X * Row[2].Z - Row[1].Z * Row[2].X)
                + Row[0].X * (Row[1].Z * Row[2].W - Row[1].W * Row[2].Z));
        t.Row[2].X = s
            * (Row[1].W * (Row[2].X * Row[3].Y - Row[2].Y * Row[3].X)
                + Row[1].X * (Row[2].Y * Row[3].W - Row[2].W * Row[3].Y)
                + Row[1].Y * (Row[2].W * Row[3].X - Row[2].X * Row[3].W));
        t.Row[2].Y = s
            * (Row[2].W * (Row[0].X * Row[3].Y - Row[0].Y * Row[3].X)
                + Row[2].X * (Row[0].Y * Row[3].W - Row[0].W * Row[3].Y)
                + Row[2].Y * (Row[0].W * Row[3].X - Row[0].X * Row[3].W));
        t.Row[2].Z = s
            * (Row[3].W * (Row[0].X * Row[1].Y - Row[0].Y * Row[1].X)
                + Row[3].X * (Row[0].Y * Row[1].W - Row[0].W * Row[1].Y)
                + Row[3].Y * (Row[0].W * Row[1].X - Row[0].X * Row[1].W));
        t.Row[2].W = s
            * (Row[0].W * (Row[1].Y * Row[2].X - Row[1].X * Row[2].Y)
                + Row[0].X * (Row[1].W * Row[2].Y - Row[1].Y * Row[2].W)
                + Row[0].Y * (Row[1].X * Row[2].W - Row[1].W * Row[2].X));
        t.Row[3].X = s
            * (Row[1].X * (Row[2].Z * Row[3].Y - Row[2].Y * Row[3].Z)
                + Row[1].Y * (Row[2].X * Row[3].Z - Row[2].Z * Row[3].X)
                + Row[1].Z * (Row[2].Y * Row[3].X - Row[2].X * Row[3].Y));
        t.Row[3].Y = s
            * (Row[2].X * (Row[0].Z * Row[3].Y - Row[0].Y * Row[3].Z)
                + Row[2].Y * (Row[0].X * Row[3].Z - Row[0].Z * Row[3].X)
                + Row[2].Z * (Row[0].Y * Row[3].X - Row[0].X * Row[3].Y));
        t.Row[3].Z = s
            * (Row[3].X * (Row[0].Z * Row[1].Y - Row[0].Y * Row[1].Z)
                + Row[3].Y * (Row[0].X * Row[1].Z - Row[0].Z * Row[1].X)
                + Row[3].Z * (Row[0].Y * Row[1].X - Row[0].X * Row[1].Y));
        t.Row[3].W = s
            * (Row[0].X * (Row[1].Y * Row[2].Z - Row[1].Z * Row[2].Y)
                + Row[0].Y * (Row[1].Z * Row[2].X - Row[1].X * Row[2].Z)
                + Row[0].Z * (Row[1].X * Row[2].Y - Row[1].Y * Row[2].X));

        return t;
    }

    __forceinline Matrix4 &operator=(const Matrix4 &m)
    {
        Row[0] = m.Row[0];
        Row[1] = m.Row[1];
        Row[2] = m.Row[2];
        Row[3] = m.Row[3];

        return *this;
    }

    __forceinline Matrix4 &operator+=(const Matrix4 &m)
    {
        Row[0] += m.Row[0];
        Row[1] += m.Row[1];
        Row[2] += m.Row[2];
        Row[3] += m.Row[3];

        return *this;
    }

    __forceinline Matrix4 &operator-=(const Matrix4 &m)
    {
        Row[0] -= m.Row[0];
        Row[1] -= m.Row[1];
        Row[2] -= m.Row[2];
        Row[3] -= m.Row[3];

        return *this;
    }

    __forceinline Matrix4 &operator*=(float d)
    {
        Row[0] *= d;
        Row[1] *= d;
        Row[2] *= d;
        Row[3] *= d;

        return *this;
    }

    __forceinline Matrix4 &operator/=(float d)
    {
        float ood = d;
        Row[0] *= ood;
        Row[1] *= ood;
        Row[2] *= ood;
        Row[3] *= ood;

        return *this;
    }

    friend Matrix4 operator-(const Matrix4 &a);
    friend Matrix4 operator*(const Matrix4 &a, float d);
    friend Matrix4 operator*(float d, const Matrix4 &a);
    friend Matrix4 operator/(const Matrix4 &a, float d);
    friend Matrix4 operator+(const Matrix4 &a, const Matrix4 &b);
    friend Matrix4 Add(const Matrix4 &a);
    friend Matrix4 operator-(const Matrix4 &a, const Matrix4 &b);
    friend Matrix4 Subtract(const Matrix4 &a, const Matrix4 &b);
    friend Matrix4 operator*(const Matrix4 &a, const Matrix4 &b);
    friend Matrix4 Multiply(const Matrix4 &a, const Matrix4 &b);
    friend Matrix4 operator*(const Matrix4 &a, const Matrix3D &b);
    friend Matrix4 operator*(const Matrix3D &a, const Matrix4 &b);
    friend int operator==(const Matrix4 &a, const Matrix4 &b);
    friend int operator!=(const Matrix4 &a, const Matrix4 &b);
    friend void Swap(Matrix4 &a, Matrix4 &b);
    friend Vector4 operator*(const Matrix4 &a, const Vector4 &v);
    friend Vector4 operator*(const Matrix4 &a, const Vector3 &v);
    static void Multiply(const Matrix4 &A, const Matrix4 &B, Matrix4 *set_result);
    static void Multiply(const Matrix3D &A, const Matrix4 &B, Matrix4 *set_result);
    static void Multiply(const Matrix4 &A, const Matrix3D &B, Matrix4 *set_result);

    static __forceinline void Transform_Vector(const Matrix4 &A, const Vector3 &in, Vector3 *out)
    {
        Vector3 tmp;
        const Vector3 *v;

        if (out == &in) {
            tmp = in;
            v = &tmp;
        } else {
            v = &in;
        }

        out->X = (A[0][0] * v->X + A[0][1] * v->Y + A[0][2] * v->Z + A[0][3]);
        out->Y = (A[1][0] * v->X + A[1][1] * v->Y + A[1][2] * v->Z + A[1][3]);
        out->Z = (A[2][0] * v->X + A[2][1] * v->Y + A[2][2] * v->Z + A[2][3]);
    }

    static __forceinline void Transform_Vector(const Matrix4 &A, const Vector3 &in, Vector4 *out)
    {
        out->X = (A[0][0] * in.X + A[0][1] * in.Y + A[0][2] * in.Z + A[0][3]);
        out->Y = (A[1][0] * in.X + A[1][1] * in.Y + A[1][2] * in.Z + A[1][3]);
        out->Z = (A[2][0] * in.X + A[2][1] * in.Y + A[2][2] * in.Z + A[2][3]);
        out->W = 1.0f;
    }

    static __forceinline void Transform_Vector(const Matrix4 &A, const Vector4 &in, Vector4 *out)
    {
        Vector4 tmp;
        const Vector4 *v;

        if (out == &in) {
            tmp = in;
            v = &tmp;
        } else {
            v = &in;
        }

        out->X = (A[0][0] * v->X + A[0][1] * v->Y + A[0][2] * v->Z + A[0][3] * v->W);
        out->Y = (A[1][0] * v->X + A[1][1] * v->Y + A[1][2] * v->Z + A[1][3] * v->W);
        out->Z = (A[2][0] * v->X + A[2][1] * v->Y + A[2][2] * v->Z + A[2][3] * v->W);
        out->W = (A[3][0] * v->X + A[3][1] * v->Y + A[3][2] * v->Z + A[3][3] * v->W);
    }

    static const Matrix4 IDENTITY;

public:
    Vector4 Row[4];
};

__forceinline Matrix4 operator-(const Matrix4 &a)
{
    return Matrix4(-a.Row[0], -a.Row[1], -a.Row[2], -a.Row[3]);
}

__forceinline Matrix4 operator*(const Matrix4 &a, float d)
{
    return Matrix4(a.Row[0] * d, a.Row[1] * d, a.Row[2] * d, a.Row[3] * d);
}

__forceinline Matrix4 operator*(float d, const Matrix4 &a)
{
    return a * d;
}

__forceinline Matrix4 operator/(const Matrix4 &a, float d)
{
    float ood = 1.0f / d;
    return Matrix4(a.Row[0] * ood, a.Row[1] * ood, a.Row[2] * ood, a.Row[3] * ood);
}

__forceinline Matrix4 operator+(const Matrix4 &a, const Matrix4 &b)
{
    return Matrix4(a.Row[0] + b.Row[0], a.Row[1] + b.Row[1], a.Row[2] + b.Row[2], a.Row[3] + b.Row[3]);
}

__forceinline Matrix4 Add(const Matrix4 &a, const Matrix4 &b)
{
    return a + b;
}

__forceinline Matrix4 operator-(const Matrix4 &a, const Matrix4 &b)
{
    return Matrix4(a.Row[0] - b.Row[0], a.Row[1] - b.Row[1], a.Row[2] - b.Row[2], a.Row[3] - b.Row[3]);
}

__forceinline Matrix4 Subtract(const Matrix4 &a, const Matrix4 &b)
{
    return a - b;
}

__forceinline Matrix4 operator*(const Matrix4 &a, const Matrix4 &b)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j]

    return Matrix4(Vector4(ROWCOL(0, 0), ROWCOL(0, 1), ROWCOL(0, 2), ROWCOL(0, 3)),
        Vector4(ROWCOL(1, 0), ROWCOL(1, 1), ROWCOL(1, 2), ROWCOL(1, 3)),
        Vector4(ROWCOL(2, 0), ROWCOL(2, 1), ROWCOL(2, 2), ROWCOL(2, 3)),
        Vector4(ROWCOL(3, 0), ROWCOL(3, 1), ROWCOL(3, 2), ROWCOL(3, 3)));

#undef ROWCOL
}

__forceinline Matrix4 Multiply(const Matrix4 &a, const Matrix4 &b)
{
    return a * b;
}

__forceinline Matrix4 operator*(const Matrix4 &a, const Matrix3D &b)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]
#define ROWCOL_LAST(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3]
    return Matrix4(Vector4(ROWCOL(0, 0), ROWCOL(0, 1), ROWCOL(0, 2), ROWCOL_LAST(0, 3)),
        Vector4(ROWCOL(1, 0), ROWCOL(1, 1), ROWCOL(1, 2), ROWCOL_LAST(1, 3)),
        Vector4(ROWCOL(2, 0), ROWCOL(2, 1), ROWCOL(2, 2), ROWCOL_LAST(2, 3)),
        Vector4(ROWCOL(3, 0), ROWCOL(3, 1), ROWCOL(3, 2), ROWCOL_LAST(3, 3)));
#undef ROWCOL
#undef ROWCOL_LAST
}

__forceinline Matrix4 operator*(const Matrix3D &a, const Matrix4 &b)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j]
    return Matrix4(Vector4(ROWCOL(0, 0), ROWCOL(0, 1), ROWCOL(0, 2), ROWCOL(0, 3)),
        Vector4(ROWCOL(1, 0), ROWCOL(1, 1), ROWCOL(1, 2), ROWCOL(1, 3)),
        Vector4(ROWCOL(2, 0), ROWCOL(2, 1), ROWCOL(2, 2), ROWCOL(2, 3)),
        Vector4(b[3][0], b[3][1], b[3][2], b[3][3]));
#undef ROWCOL
}

__forceinline Vector4 operator*(const Matrix4 &a, const Vector3 &v)
{
    return Vector4(a[0][0] * v[0] + a[0][1] * v[1] + a[0][2] * v[2] + a[0][3] * 1.0f,
        a[1][0] * v[0] + a[1][1] * v[1] + a[1][2] * v[2] + a[1][3] * 1.0f,
        a[2][0] * v[0] + a[2][1] * v[1] + a[2][2] * v[2] + a[2][3] * 1.0f,
        a[3][0] * v[0] + a[3][1] * v[1] + a[3][2] * v[2] + a[3][3] * 1.0f);
}

__forceinline Vector4 operator*(const Matrix4 &a, const Vector4 &v)
{
    return Vector4(a[0][0] * v[0] + a[0][1] * v[1] + a[0][2] * v[2] + a[0][3] * v[3],
        a[1][0] * v[0] + a[1][1] * v[1] + a[1][2] * v[2] + a[1][3] * v[3],
        a[2][0] * v[0] + a[2][1] * v[1] + a[2][2] * v[2] + a[2][3] * v[3],
        a[3][0] * v[0] + a[3][1] * v[1] + a[3][2] * v[2] + a[3][3] * v[3]);
}

__forceinline int operator==(const Matrix4 &a, const Matrix4 &b)
{
    return ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]));
}

__forceinline int operator!=(const Matrix4 &a, const Matrix4 &b)
{
    return (!(a == b));
}

#ifdef BUILD_WITH_D3D8
struct _D3DMATRIX;

namespace Thyme
{
// When converting Matrix4 to D3DMATRIX or vice versa always use conversion function below.
// Reason being, D3DMATRIX is row major matrix, and Matrix4 is column major matrix.
// Thus copying from one to another will always require a transpose (or invert).

void To_D3DMATRIX(_D3DMATRIX &dxm, const Matrix4 &m);
[[nodiscard]] _D3DMATRIX To_D3DMATRIX(const Matrix4 &m);

void To_Matrix4(Matrix4 &m, const _D3DMATRIX &dxm);
[[nodiscard]] Matrix4 To_Matrix4(const _D3DMATRIX &dxm);

} // namespace Thyme
#endif
