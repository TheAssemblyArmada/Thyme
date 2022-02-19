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
#pragma once

#include "always.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "vector3.h"

class Matrix3
{
public:
    __forceinline Matrix3() {}

    __forceinline Matrix3(const Matrix3 &m)
    {
        Row[0] = m.Row[0];
        Row[1] = m.Row[1];
        Row[2] = m.Row[2];
    }

    __forceinline explicit Matrix3(bool identity)
    {
        if (identity) {
            Row[0].Set(1.0f, 0.0f, 0.0f);
            Row[1].Set(0.0f, 1.0f, 0.0f);
            Row[2].Set(0.0f, 0.0f, 1.0f);
        }
    }

    __forceinline explicit Matrix3(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2)
    {
        Row[0] = v0;
        Row[1] = v1;
        Row[2] = v2;
    }

    explicit Matrix3(const Matrix3D &m);
    explicit Matrix3(const Matrix4 &m);

    __forceinline explicit Matrix3(
        float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
    {
        Row[0].Set(m11, m12, m13);
        Row[1].Set(m21, m22, m23);
        Row[2].Set(m31, m32, m33);
    }

    __forceinline explicit Matrix3(const Vector3 &axis, float angle) { Set(axis, angle); }

    __forceinline explicit Matrix3(const Vector3 &axis, float s_angle, float c_angle) { Set(axis, s_angle, c_angle); }

    __forceinline explicit Matrix3(const Quaternion &q) { Set(q); }

    void Set(const Matrix3D &m);
    void Set(const Matrix4 &m);

    __forceinline void Set(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2)
    {
        Row[0] = v0;
        Row[1] = v1;
        Row[2] = v2;
    }

    __forceinline void Set(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
    {
        Row[0].Set(m11, m12, m13);
        Row[1].Set(m21, m22, m23);
        Row[2].Set(m31, m32, m33);
    }

    __forceinline void Set(const Vector3 &axis, float angle) { Set(axis, GameMath::Sin(angle), GameMath::Cos(angle)); }

    __forceinline void Set(const Vector3 &axis, float s, float c)
    {
        captainslog_assert(GameMath::Fabs(axis.Length2() - 1.0f) < 0.001f);

        Row[0].Set((float)(axis[0] * axis[0] + c * (1.0f - axis[0] * axis[0])),
            (float)(axis[0] * axis[1] * (1.0f - c) - axis[2] * s),
            (float)(axis[2] * axis[0] * (1.0f - c) + axis[1] * s));

        Row[1].Set((float)(axis[0] * axis[1] * (1.0f - c) + axis[2] * s),
            (float)(axis[1] * axis[1] + c * (1.0f - axis[1] * axis[1])),
            (float)(axis[1] * axis[2] * (1.0f - c) - axis[0] * s));

        Row[2].Set((float)(axis[2] * axis[0] * (1.0f - c) - axis[1] * s),
            (float)(axis[1] * axis[2] * (1.0f - c) + axis[0] * s),
            (float)(axis[2] * axis[2] + c * (1 - axis[2] * axis[2])));
    }

    void Set(const Quaternion &q);
    __forceinline Vector3 &operator[](int i) { return Row[i]; }
    __forceinline const Vector3 &operator[](int i) const { return Row[i]; }

    __forceinline Matrix3 Transpose() const
    {
        return Matrix3(Vector3(Row[0][0], Row[1][0], Row[2][0]),
            Vector3(Row[0][1], Row[1][1], Row[2][1]),
            Vector3(Row[0][2], Row[1][2], Row[2][2]));
    }

    __forceinline float Determinant() const
    {
        return Row[0][0] * (Row[1][1] * Row[2][2] - Row[1][2] * Row[2][1])
            - Row[0][1] * (Row[1][0] * Row[2][2] - Row[1][2] * Row[2][0])
            - Row[0][2] * (Row[1][0] * Row[2][1] - Row[1][1] * Row[2][0]);
    }

    __forceinline Matrix3 &operator=(const Matrix3 &m)
    {
        Row[0] = m.Row[0];
        Row[1] = m.Row[1];
        Row[2] = m.Row[2];

        return *this;
    }

    Matrix3 &operator=(const Matrix3D &m);
    Matrix3 &operator=(const Matrix4 &m);

    __forceinline Matrix3 &operator+=(const Matrix3 &m)
    {
        Row[0] += m.Row[0];
        Row[1] += m.Row[1];
        Row[2] += m.Row[2];

        return *this;
    }

    __forceinline Matrix3 &operator-=(const Matrix3 &m)
    {
        Row[0] -= m.Row[0];
        Row[1] -= m.Row[1];
        Row[2] -= m.Row[2];

        return *this;
    }

    __forceinline Matrix3 &operator*=(float d)
    {
        Row[0] *= d;
        Row[1] *= d;
        Row[2] *= d;

        return *this;
    }

    __forceinline Matrix3 &operator/=(float d)
    {
        Row[0] /= d;
        Row[1] /= d;
        Row[2] /= d;

        return *this;
    }

    __forceinline void Make_Identity()
    {
        Row[0].Set(1.0f, 0.0f, 0.0f);
        Row[1].Set(0.0f, 1.0f, 0.0f);
        Row[2].Set(0.0f, 0.0f, 1.0f);
    }

    __forceinline void Rotate_X(float theta) { Rotate_X(GameMath::Sin(theta), GameMath::Cos(theta)); }

    __forceinline void Rotate_X(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][1];
        tmp2 = Row[0][2];
        Row[0][1] = (float)(c * tmp1 + s * tmp2);
        Row[0][2] = (float)(-s * tmp1 + c * tmp2);

        tmp1 = Row[1][1];
        tmp2 = Row[1][2];
        Row[1][1] = (float)(c * tmp1 + s * tmp2);
        Row[1][2] = (float)(-s * tmp1 + c * tmp2);

        tmp1 = Row[2][1];
        tmp2 = Row[2][2];
        Row[2][1] = (float)(c * tmp1 + s * tmp2);
        Row[2][2] = (float)(-s * tmp1 + c * tmp2);
    }

    __forceinline void Rotate_Y(float theta) { Rotate_Y(GameMath::Sin(theta), GameMath::Cos(theta)); }

    __forceinline void Rotate_Y(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[0][2];
        Row[0][0] = (float)(c * tmp1 - s * tmp2);
        Row[0][2] = (float)(s * tmp1 + c * tmp2);

        tmp1 = Row[1][0];
        tmp2 = Row[1][2];
        Row[1][0] = (float)(c * tmp1 - s * tmp2);
        Row[1][2] = (float)(s * tmp1 + c * tmp2);

        tmp1 = Row[2][0];
        tmp2 = Row[2][2];
        Row[2][0] = (float)(c * tmp1 - s * tmp2);
        Row[2][2] = (float)(s * tmp1 + c * tmp2);
    }

    __forceinline void Rotate_Z(float theta) { Rotate_Z(GameMath::Sin(theta), GameMath::Cos(theta)); }

    __forceinline void Rotate_Z(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[0][1];
        Row[0][0] = (float)(c * tmp1 + s * tmp2);
        Row[0][1] = (float)(-s * tmp1 + c * tmp2);

        tmp1 = Row[1][0];
        tmp2 = Row[1][1];
        Row[1][0] = (float)(c * tmp1 + s * tmp2);
        Row[1][1] = (float)(-s * tmp1 + c * tmp2);

        tmp1 = Row[2][0];
        tmp2 = Row[2][1];
        Row[2][0] = (float)(c * tmp1 + s * tmp2);
        Row[2][1] = (float)(-s * tmp1 + c * tmp2);
    }

    __forceinline float Get_X_Rotation() const
    {
        Vector3 v = (*this) * Vector3(0.0f, 1.0f, 0.0f);
        return GameMath::Atan2(v[2], v[1]);
    }

    __forceinline float Get_Y_Rotation() const
    {
        Vector3 v = (*this) * Vector3(0.0f, 0.0f, 1.0f);
        return GameMath::Atan2(v[0], v[2]);
    }

    __forceinline float Get_Z_Rotation() const
    {
        Vector3 v = (*this) * Vector3(1.0f, 0.0f, 0.0f);
        return GameMath::Atan2(v[1], v[0]);
    }

    __forceinline Vector3 Get_X_Vector() const { return Vector3(Row[0][0], Row[1][0], Row[2][0]); }

    __forceinline Vector3 Get_Y_Vector() const { return Vector3(Row[0][1], Row[1][1], Row[2][1]); }

    __forceinline Vector3 Get_Z_Vector() const { return Vector3(Row[0][2], Row[1][2], Row[2][2]); }

    __forceinline void Get_X_Vector(Vector3 *set_x) const { set_x->Set(Row[0][0], Row[1][0], Row[2][0]); }

    __forceinline void Get_Y_Vector(Vector3 *set_y) const { set_y->Set(Row[0][1], Row[1][1], Row[2][1]); }

    __forceinline void Get_Z_Vector(Vector3 *set_z) const { set_z->Set(Row[0][2], Row[1][2], Row[2][2]); }

    friend Matrix3 operator-(const Matrix3 &a);
    friend Matrix3 operator*(const Matrix3 &a, float d);
    friend Matrix3 operator*(float d, const Matrix3 &a);
    friend Matrix3 operator/(const Matrix3 &a, float d);
    friend Matrix3 operator+(const Matrix3 &a, const Matrix3 &b);
    friend Matrix3 operator-(const Matrix3 &a, const Matrix3 &b);
    friend Matrix3 operator*(const Matrix3 &a, const Matrix3 &b);
    friend Matrix3 operator*(const Matrix3D &a, const Matrix3 &b);
    friend Matrix3 operator*(const Matrix3 &a, const Matrix3D &b);
    friend int operator==(const Matrix3 &a, const Matrix3 &b);
    friend int operator!=(const Matrix3 &a, const Matrix3 &b);
    friend void Swap(Matrix3 &a, Matrix3 &b);
    friend Vector3 operator*(const Matrix3 &a, const Vector3 &v);

    __forceinline static void Add(const Matrix3 &a, const Matrix3 &b, Matrix3 *c)
    {
        captainslog_assert(c != nullptr);
        Vector3::Add(a.Row[0], b.Row[0], &(c->Row[0]));
        Vector3::Add(a.Row[1], b.Row[1], &(c->Row[1]));
        Vector3::Add(a.Row[2], b.Row[2], &(c->Row[2]));
    }

    __forceinline static void Subtract(const Matrix3 &a, const Matrix3 &b, Matrix3 *c)
    {
        captainslog_assert(c != nullptr);
        Vector3::Subtract(a.Row[0], b.Row[0], &(c->Row[0]));
        Vector3::Subtract(a.Row[1], b.Row[1], &(c->Row[1]));
        Vector3::Subtract(a.Row[2], b.Row[2], &(c->Row[2]));
    }

    static void Multiply(const Matrix3 &a, const Matrix3 &b, Matrix3 *res);
    static void Multiply(const Matrix3D &a, const Matrix3 &b, Matrix3 *res);
    static void Multiply(const Matrix3 &a, const Matrix3D &b, Matrix3 *res);

    int Is_Orthogonal() const;
    void Re_Orthogonalize();

    __forceinline static void Rotate_Vector(const Matrix3 &A, const Vector3 &in, Vector3 *out)
    {
        Vector3 tmp;
        const Vector3 *v;

        if (out == &in) {
            tmp = in;
            v = &tmp;
        } else {
            v = &in;
        }

        out->X = (A[0][0] * v->X + A[0][1] * v->Y + A[0][2] * v->Z);
        out->Y = (A[1][0] * v->X + A[1][1] * v->Y + A[1][2] * v->Z);
        out->Z = (A[2][0] * v->X + A[2][1] * v->Y + A[2][2] * v->Z);
    }

    __forceinline static void Transpose_Rotate_Vector(const Matrix3 &A, const Vector3 &in, Vector3 *out)
    {
        Vector3 tmp;
        const Vector3 *v;

        if (out == &in) {
            tmp = in;
            v = &tmp;
        } else {
            v = &in;
        }

        out->X = (A[0][0] * v->X + A[1][0] * v->Y + A[2][0] * v->Z);
        out->Y = (A[0][1] * v->X + A[1][1] * v->Y + A[2][1] * v->Z);
        out->Z = (A[0][2] * v->X + A[1][2] * v->Y + A[2][2] * v->Z);
    }

    __forceinline void Rotate_AABox_Extent(const Vector3 &extent, Vector3 *new_extent)
    {
        for (int i = 0; i < 3; i++) {
            (*new_extent)[i] = 0.0f;

            for (int j = 0; j < 3; j++) {
                (*new_extent)[i] += GameMath::Fabs(Row[i][j] * extent[j]);
            }
        }
    }

    __forceinline Matrix3 Inverse() const
    {
        Matrix3 a(*this); // As a evolves from original mat into identity
        Matrix3 b(true); // b evolves from identity into inverse(a)
        int i, j, i1;

        // Loop over cols of a from left to right, eliminating above and below diagonal
        for (j = 0; j < 3; j++) {
            // Find largest pivot in column j among rows j..3
            i1 = j;

            for (i = j + 1; i < 3; i++) {
                if (GameMath::Fabs(a[i][j]) > GameMath::Fabs(a[i1][j])) {
                    i1 = i;
                }
            }

            // Swap rows i1 and j in a and b to put pivot on diagonal
            Swap(a.Row[i1], a.Row[j]);
            Swap(b.Row[i1], b.Row[j]);

            // Scale row j to have a unit diagonal
            if (a[j][j] == 0.) {
                // Matrix3::inverse: singular matrix; can't invert
            }

            b.Row[j] /= a.Row[j][j];
            a.Row[j] /= a.Row[j][j];

            // Eliminate off-diagonal elems in col j of a, doing identical ops to b
            for (i = 0; i < 3; i++) {
                if (i != j) {
                    b.Row[i] -= a[i][j] * b.Row[j];
                    a.Row[i] -= a[i][j] * a.Row[j];
                }
            }
        }

        return b;
    }

    static const Matrix3 Identity;

protected:
    Vector3 Row[3];
};

__forceinline Matrix3 operator-(const Matrix3 &a)
{
    return Matrix3(-a.Row[0], -a.Row[1], -a.Row[2]);
}

__forceinline Matrix3 operator*(const Matrix3 &a, float d)
{
    return Matrix3(a.Row[0] * d, a.Row[1] * d, a.Row[2] * d);
}

__forceinline Matrix3 operator*(float d, const Matrix3 &a)
{
    return a * d;
}

__forceinline Matrix3 operator/(const Matrix3 &a, float d)
{
    float ood = 1.0f / d;
    return Matrix3(a.Row[0] * ood, a.Row[1] * ood, a.Row[2] * ood);
}

__forceinline Matrix3 operator+(const Matrix3 &a, const Matrix3 &b)
{
    return Matrix3(a.Row[0] + b.Row[0], a.Row[1] + b.Row[1], a.Row[2] + b.Row[2]);
}

__forceinline Matrix3 operator-(const Matrix3 &a, const Matrix3 &b)
{
    return Matrix3(a.Row[0] - b.Row[0], a.Row[1] - b.Row[1], a.Row[2] - b.Row[2]);
}

__forceinline Matrix3 operator*(const Matrix3 &a, const Matrix3 &b)
{
#define ROWCOL(i, j) a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]
    return Matrix3(Vector3(ROWCOL(0, 0), ROWCOL(0, 1), ROWCOL(0, 2)),
        Vector3(ROWCOL(1, 0), ROWCOL(1, 1), ROWCOL(1, 2)),
        Vector3(ROWCOL(2, 0), ROWCOL(2, 1), ROWCOL(2, 2)));
#undef ROWCOL
}

__forceinline Vector3 operator*(const Matrix3 &a, const Vector3 &v)
{
    return Vector3(a[0][0] * v[0] + a[0][1] * v[1] + a[0][2] * v[2],
        a[1][0] * v[0] + a[1][1] * v[1] + a[1][2] * v[2],
        a[2][0] * v[0] + a[2][1] * v[1] + a[2][2] * v[2]);
}

__forceinline int operator==(const Matrix3 &a, const Matrix3 &b)
{
    return ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]));
}

__forceinline int operator!=(const Matrix3 &a, const Matrix3 &b)
{
    return (!(a == b));
}

__forceinline Matrix3 Create_X_Rotation_Matrix3(float s, float c)
{
    Matrix3 mat;
    mat[0][0] = 1.0f;
    mat[0][1] = 0.0f;
    mat[0][2] = 0.0f;
    mat[1][0] = 0.0f;
    mat[1][1] = c;
    mat[1][2] = -s;
    mat[2][0] = 0.0f;
    mat[2][1] = s;
    mat[2][2] = c;

    return mat;
}

__forceinline Matrix3 Create_X_Rotation_Matrix3(float rad)
{
    return Create_X_Rotation_Matrix3(GameMath::Sin(rad), GameMath::Cos(rad));
}

__forceinline Matrix3 Create_Y_Rotation_Matrix3(float s, float c)
{
    Matrix3 mat;
    mat[0][0] = c;
    mat[0][1] = 0.0f;
    mat[0][2] = s;
    mat[1][0] = 0.0f;
    mat[1][1] = 1.0f;
    mat[1][2] = 0.0f;
    mat[2][0] = -s;
    mat[2][1] = 0.0f;
    mat[2][2] = c;

    return mat;
}

__forceinline Matrix3 Create_Y_Rotation_Matrix3(float rad)
{
    return Create_Y_Rotation_Matrix3(GameMath::Sin(rad), GameMath::Cos(rad));
}

__forceinline Matrix3 Create_Z_Rotation_Matrix3(float s, float c)
{
    Matrix3 mat;
    mat[0][0] = c;
    mat[0][1] = -s;
    mat[0][2] = 0.0f;
    mat[1][0] = s;
    mat[1][1] = c;
    mat[1][2] = 0.0f;
    mat[2][0] = 0.0f;
    mat[2][1] = 0.0f;
    mat[2][2] = 1.0f;

    return mat;
}

__forceinline Matrix3 Create_Z_Rotation_Matrix3(float rad)
{
    return Create_Z_Rotation_Matrix3(GameMath::Sin(rad), GameMath::Cos(rad));
}
