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
#pragma once

#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

class Matrix3;
class Matrix4;
class Quaternion;
class PlaneClass;

class Matrix3D
{
public:
    __forceinline Matrix3D() {}

    __forceinline explicit Matrix3D(float m11,
        float m12,
        float m13,
        float m14,
        float m21,
        float m22,
        float m23,
        float m24,
        float m31,
        float m32,
        float m33,
        float m34)
    {
        Row[0].Set(m11, m12, m13, m14);
        Row[1].Set(m21, m22, m23, m24);
        Row[2].Set(m31, m32, m33, m34);
    }

    __forceinline explicit Matrix3D(bool init)
    {
        if (init)
            Make_Identity();
    }

    __forceinline explicit Matrix3D(float m[12])
    {
        Row[0].Set(m[0], m[1], m[2], m[3]);
        Row[1].Set(m[4], m[5], m[6], m[7]);
        Row[2].Set(m[8], m[9], m[10], m[11]);
    }

    __forceinline explicit Matrix3D(const Vector3 &x, const Vector3 &y, const Vector3 &z, const Vector3 &pos)
    {
        Set(x, y, z, pos);
    }

    __forceinline explicit Matrix3D(const Vector3 &axis, float angle) { Set(axis, angle); }

    __forceinline explicit Matrix3D(const Vector3 &axis, float sine, float cosine) { Set(axis, sine, cosine); }

    __forceinline explicit Matrix3D(const Matrix3 &rot, const Vector3 &pos) { Set(rot, pos); }

    __forceinline explicit Matrix3D(const Quaternion &rot, const Vector3 &pos) { Set(rot, pos); }

    __forceinline explicit Matrix3D(const Vector3 &position) { Set(position); }

    __forceinline Matrix3D(const Matrix3D &m)
    {
        Row[0] = m.Row[0];
        Row[1] = m.Row[1];
        Row[2] = m.Row[2];
    }

    __forceinline Vector4 &operator[](int i) { return Row[i]; }
    __forceinline const Vector4 &operator[](int i) const { return Row[i]; }

    __forceinline Matrix3D &operator=(const Matrix3D &m)
    {
        Row[0] = m.Row[0];
        Row[1] = m.Row[1];
        Row[2] = m.Row[2];

        return *this;
    }

    void Get_Orthogonal_Inverse(Matrix3D &set_inverse) const;
    float Get_X_Rotation() const;
    float Get_Y_Rotation() const;
    float Get_Z_Rotation() const;
    void Look_At(const Vector3 &p, const Vector3 &t, float roll);
    void Obj_Look_At(const Vector3 &p, const Vector3 &t, float roll);
    Vector3 Rotate_Vector(const Vector3 &vect) const;
    Vector3 Inverse_Rotate_Vector(const Vector3 &vect) const;

    __forceinline void Set(float m[12])
    {
        Row[0].Set(m[0], m[1], m[2], m[3]);
        Row[1].Set(m[4], m[5], m[6], m[7]);
        Row[2].Set(m[8], m[9], m[10], m[11]);
    }

    __forceinline void Set(float m11,
        float m12,
        float m13,
        float m14,
        float m21,
        float m22,
        float m23,
        float m24,
        float m31,
        float m32,
        float m33,
        float m34)
    {
        Row[0].Set(m11, m12, m13, m14);
        Row[1].Set(m21, m22, m23, m24);
        Row[2].Set(m31, m32, m33, m34);
    }

    __forceinline void Set(const Vector3 &x, const Vector3 &y, const Vector3 &z, const Vector3 &pos)
    {
        Row[0].Set(x[0], y[0], z[0], pos[0]);
        Row[1].Set(x[1], y[1], z[1], pos[1]);
        Row[2].Set(x[2], y[2], z[2], pos[2]);
    }

    __forceinline void Set(const Vector3 &axis, float angle)
    {
        float c = GameMath::Cos(angle);
        float s = GameMath::Sin(angle);
        Set(axis, s, c);
    }

    __forceinline void Set(const Vector3 &axis, float s, float c)
    {
        captainslog_assert(GameMath::Fabs(axis.Length2() - 1.0f) < 0.001f);
        Row[0].Set((axis[0] * axis[0] + c * (1.0f - axis[0] * axis[0])),
            (axis[0] * axis[1] * (1.0f - c) - axis[2] * s),
            (axis[2] * axis[0] * (1.0f - c) + axis[1] * s),
            0.0f);

        Row[1].Set((axis[0] * axis[1] * (1.0f - c) + axis[2] * s),
            (axis[1] * axis[1] + c * (1.0f - axis[1] * axis[1])),
            (axis[1] * axis[2] * (1.0f - c) - axis[0] * s),
            0.0f);

        Row[2].Set((axis[2] * axis[0] * (1.0f - c) - axis[1] * s),
            (axis[1] * axis[2] * (1.0f - c) + axis[0] * s),
            (axis[2] * axis[2] + c * (1 - axis[2] * axis[2])),
            0.0f);
    }

    __forceinline void Set(const Vector3 &position)
    {
        Row[0].Set(1.0f, 0.0f, 0.0f, position[0]);
        Row[1].Set(0.0f, 1.0f, 0.0f, position[1]);
        Row[2].Set(0.0f, 0.0f, 1.0f, position[2]);
    }

    void Set(const Matrix3 &rotation, const Vector3 &position);
    void Set(const Quaternion &rotation, const Vector3 &position);
    void Set_Rotation(const Quaternion &q);
    __forceinline Vector3 Get_Translation() const { return Vector3(Row[0][3], Row[1][3], Row[2][3]); }
    __forceinline void Get_Translation(Vector3 *set) const
    {
        set->X = Row[0][3];
        set->Y = Row[1][3];
        set->Z = Row[2][3];
    }
    __forceinline void Set_Translation(const Vector3 &t)
    {
        Row[0][3] = t[0];
        Row[1][3] = t[1];
        Row[2][3] = t[2];
    }
    void Set_Rotation(const Matrix3 &m);
    __forceinline float Get_X_Translation() const { return Row[0][3]; };
    __forceinline float Get_Y_Translation() const { return Row[1][3]; };
    __forceinline float Get_Z_Translation() const { return Row[2][3]; };
    __forceinline void Set_X_Translation(float x) { Row[0][3] = x; };
    __forceinline void Set_Y_Translation(float y) { Row[1][3] = y; };
    __forceinline void Set_Z_Translation(float z) { Row[2][3] = z; };
    __forceinline void Adjust_Translation(const Vector3 &t)
    {
        Row[0][3] += t[0];
        Row[1][3] += t[1];
        Row[2][3] += t[2];
    };
    __forceinline void Adjust_X_Translation(float x) { Row[0][3] += x; };
    __forceinline void Adjust_Y_Translation(float y) { Row[1][3] += y; };
    __forceinline void Adjust_Z_Translation(float z) { Row[2][3] += z; };

    __forceinline void Make_Identity()
    {
        Row[0].Set(1.0f, 0.0f, 0.0f, 0.0f);
        Row[1].Set(0.0f, 1.0f, 0.0f, 0.0f);
        Row[2].Set(0.0f, 0.0f, 1.0f, 0.0f);
    }

    __forceinline void Translate(float x, float y, float z)
    {
        Row[0][3] += (Row[0][0] * x + Row[0][1] * y + Row[0][2] * z);
        Row[1][3] += (Row[1][0] * x + Row[1][1] * y + Row[1][2] * z);
        Row[2][3] += (Row[2][0] * x + Row[2][1] * y + Row[2][2] * z);
    }

    __forceinline void Translate(const Vector3 &t)
    {
        Row[0][3] += Row[0][0] * t[0] + Row[0][1] * t[1] + Row[0][2] * t[2];
        Row[1][3] += Row[1][0] * t[0] + Row[1][1] * t[1] + Row[1][2] * t[2];
        Row[2][3] += Row[2][0] * t[0] + Row[2][1] * t[1] + Row[2][2] * t[2];
    }

    __forceinline void Translate_X(float x)
    {
        Row[0][3] += (Row[0][0] * x);
        Row[1][3] += (Row[1][0] * x);
        Row[2][3] += (Row[2][0] * x);
    }

    __forceinline void Translate_Y(float y)
    {
        Row[0][3] += (Row[0][1] * y);
        Row[1][3] += (Row[1][1] * y);
        Row[2][3] += (Row[2][1] * y);
    }

    __forceinline void Translate_Z(float z)
    {
        Row[0][3] += (Row[0][2] * z);
        Row[1][3] += (Row[1][2] * z);
        Row[2][3] += (Row[2][2] * z);
    }

    __forceinline void Rotate_X(float theta)
    {
        float tmp1, tmp2;
        float s, c;
        s = GameMath::Sin(theta);
        c = GameMath::Cos(theta);
        tmp1 = Row[0][1];
        tmp2 = Row[0][2];
        Row[0][1] = (c * tmp1 + s * tmp2);
        Row[0][2] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[1][1];
        tmp2 = Row[1][2];
        Row[1][1] = (c * tmp1 + s * tmp2);
        Row[1][2] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[2][1];
        tmp2 = Row[2][2];
        Row[2][1] = (c * tmp1 + s * tmp2);
        Row[2][2] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void Rotate_Y(float theta)
    {
        float tmp1, tmp2;
        float s, c;
        s = GameMath::Sin(theta);
        c = GameMath::Cos(theta);
        tmp1 = Row[0][0];
        tmp2 = Row[0][2];
        Row[0][0] = (c * tmp1 - s * tmp2);
        Row[0][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][0];
        tmp2 = Row[1][2];
        Row[1][0] = (c * tmp1 - s * tmp2);
        Row[1][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[2][0];
        tmp2 = Row[2][2];
        Row[2][0] = (c * tmp1 - s * tmp2);
        Row[2][2] = (s * tmp1 + c * tmp2);
    }

    __forceinline void Rotate_Z(float theta)
    {
        float tmp1, tmp2;
        float c, s;
        c = GameMath::Cos(theta);
        s = GameMath::Sin(theta);
        tmp1 = Row[0][0];
        tmp2 = Row[0][1];
        Row[0][0] = (c * tmp1 + s * tmp2);
        Row[0][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[1][0];
        tmp2 = Row[1][1];
        Row[1][0] = (c * tmp1 + s * tmp2);
        Row[1][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[2][0];
        tmp2 = Row[2][1];
        Row[2][0] = (c * tmp1 + s * tmp2);
        Row[2][1] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void Rotate_X(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][1];
        tmp2 = Row[0][2];
        Row[0][1] = (c * tmp1 + s * tmp2);
        Row[0][2] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[1][1];
        tmp2 = Row[1][2];
        Row[1][1] = (c * tmp1 + s * tmp2);
        Row[1][2] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[2][1];
        tmp2 = Row[2][2];
        Row[2][1] = (c * tmp1 + s * tmp2);
        Row[2][2] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void Rotate_Y(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[0][2];
        Row[0][0] = (c * tmp1 - s * tmp2);
        Row[0][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][0];
        tmp2 = Row[1][2];
        Row[1][0] = (c * tmp1 - s * tmp2);
        Row[1][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[2][0];
        tmp2 = Row[2][2];
        Row[2][0] = (c * tmp1 - s * tmp2);
        Row[2][2] = (s * tmp1 + c * tmp2);
    }

    __forceinline void Rotate_Z(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[0][1];
        Row[0][0] = (c * tmp1 + s * tmp2);
        Row[0][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[1][0];
        tmp2 = Row[1][1];
        Row[1][0] = (c * tmp1 + s * tmp2);
        Row[1][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[2][0];
        tmp2 = Row[2][1];
        Row[2][0] = (c * tmp1 + s * tmp2);
        Row[2][1] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void Scale(float scale)
    {
        Row[0][0] *= scale;
        Row[1][0] *= scale;
        Row[2][0] *= scale;
        Row[0][1] *= scale;
        Row[1][1] *= scale;
        Row[2][1] *= scale;
        Row[0][2] *= scale;
        Row[1][2] *= scale;
        Row[2][2] *= scale;
    }

    __forceinline void Scale(float x, float y, float z)
    {
        Row[0][0] *= x;
        Row[1][0] *= x;
        Row[2][0] *= x;
        Row[0][1] *= y;
        Row[1][1] *= y;
        Row[2][1] *= y;
        Row[0][2] *= z;
        Row[1][2] *= z;
        Row[2][2] *= z;
    }

    __forceinline void Scale(Vector3 &scale) { Scale(scale.X, scale.Y, scale.Z); }

    __forceinline void Pre_Rotate_X(float theta)
    {
        float tmp1, tmp2;
        float c, s;
        c = GameMath::Cos(theta);
        s = GameMath::Sin(theta);
        tmp1 = Row[1][0];
        tmp2 = Row[2][0];
        Row[1][0] = (c * tmp1 - s * tmp2);
        Row[2][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][1];
        tmp2 = Row[2][1];
        Row[1][1] = (c * tmp1 - s * tmp2);
        Row[2][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][2];
        tmp2 = Row[2][2];
        Row[1][2] = (c * tmp1 - s * tmp2);
        Row[2][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][3];
        tmp2 = Row[2][3];
        Row[1][3] = (c * tmp1 - s * tmp2);
        Row[2][3] = (s * tmp1 + c * tmp2);
    }

    __forceinline void Pre_Rotate_Y(float theta)
    {
        float tmp1, tmp2;
        float c, s;
        c = GameMath::Cos(theta);
        s = GameMath::Sin(theta);
        tmp1 = Row[0][0];
        tmp2 = Row[2][0];
        Row[0][0] = (c * tmp1 + s * tmp2);
        Row[2][0] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[2][1];
        Row[0][1] = (c * tmp1 + s * tmp2);
        Row[2][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[2][2];
        Row[0][2] = (c * tmp1 + s * tmp2);
        Row[2][2] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][3];
        tmp2 = Row[2][3];
        Row[0][3] = (c * tmp1 + s * tmp2);
        Row[2][3] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void Pre_Rotate_Z(float theta)
    {
        float tmp1, tmp2;
        float c, s;
        c = GameMath::Cos(theta);
        s = GameMath::Sin(theta);
        tmp1 = Row[0][0];
        tmp2 = Row[1][0];
        Row[0][0] = (c * tmp1 - s * tmp2);
        Row[1][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[1][1];
        Row[0][1] = (c * tmp1 - s * tmp2);
        Row[1][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[1][2];
        Row[0][2] = (c * tmp1 - s * tmp2);
        Row[1][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][3];
        tmp2 = Row[1][3];
        Row[0][3] = (c * tmp1 - s * tmp2);
        Row[1][3] = (s * tmp1 + c * tmp2);
    }

    __forceinline void Pre_Rotate_X(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[1][0];
        tmp2 = Row[2][0];
        Row[1][0] = (c * tmp1 - s * tmp2);
        Row[2][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][1];
        tmp2 = Row[2][1];
        Row[1][1] = (c * tmp1 - s * tmp2);
        Row[2][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][2];
        tmp2 = Row[2][2];
        Row[1][2] = (c * tmp1 - s * tmp2);
        Row[2][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][3];
        tmp2 = Row[2][3];
        Row[1][3] = (c * tmp1 - s * tmp2);
        Row[2][3] = (s * tmp1 + c * tmp2);
    }

    __forceinline void Pre_Rotate_Y(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[2][0];
        Row[0][0] = (c * tmp1 + s * tmp2);
        Row[2][0] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[2][1];
        Row[0][1] = (c * tmp1 + s * tmp2);
        Row[2][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[2][2];
        Row[0][2] = (c * tmp1 + s * tmp2);
        Row[2][2] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][3];
        tmp2 = Row[2][3];
        Row[0][3] = (c * tmp1 + s * tmp2);
        Row[2][3] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void Pre_Rotate_Z(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[1][0];
        Row[0][0] = (c * tmp1 - s * tmp2);
        Row[1][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[1][1];
        Row[0][1] = (c * tmp1 - s * tmp2);
        Row[1][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[1][2];
        Row[0][2] = (c * tmp1 - s * tmp2);
        Row[1][2] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][3];
        tmp2 = Row[1][3];
        Row[0][3] = (c * tmp1 - s * tmp2);
        Row[1][3] = (s * tmp1 + c * tmp2);
    }

    __forceinline void In_Place_Pre_Rotate_X(float theta)
    {
        float tmp1, tmp2;
        float c, s;
        c = GameMath::Cos(theta);
        s = GameMath::Sin(theta);
        tmp1 = Row[1][0];
        tmp2 = Row[2][0];
        Row[1][0] = (c * tmp1 - s * tmp2);
        Row[2][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][1];
        tmp2 = Row[2][1];
        Row[1][1] = (c * tmp1 - s * tmp2);
        Row[2][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][2];
        tmp2 = Row[2][2];
        Row[1][2] = (c * tmp1 - s * tmp2);
        Row[2][2] = (s * tmp1 + c * tmp2);
    }

    __forceinline void In_Place_Pre_Rotate_Y(float theta)
    {
        float tmp1, tmp2;
        float c, s;
        c = GameMath::Cos(theta);
        s = GameMath::Sin(theta);
        tmp1 = Row[0][0];
        tmp2 = Row[2][0];
        Row[0][0] = (c * tmp1 + s * tmp2);
        Row[2][0] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[2][1];
        Row[0][1] = (c * tmp1 + s * tmp2);
        Row[2][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[2][2];
        Row[0][2] = (c * tmp1 + s * tmp2);
        Row[2][2] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void In_Place_Pre_Rotate_Z(float theta)
    {
        float tmp1, tmp2;
        float c, s;
        c = GameMath::Cos(theta);
        s = GameMath::Sin(theta);
        tmp1 = Row[0][0];
        tmp2 = Row[1][0];
        Row[0][0] = (c * tmp1 - s * tmp2);
        Row[1][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[1][1];
        Row[0][1] = (c * tmp1 - s * tmp2);
        Row[1][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[1][2];
        Row[0][2] = (c * tmp1 - s * tmp2);
        Row[1][2] = (s * tmp1 + c * tmp2);
    }

    __forceinline void In_Place_Pre_Rotate_X(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[1][0];
        tmp2 = Row[2][0];
        Row[1][0] = (c * tmp1 - s * tmp2);
        Row[2][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][1];
        tmp2 = Row[2][1];
        Row[1][1] = (c * tmp1 - s * tmp2);
        Row[2][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[1][2];
        tmp2 = Row[2][2];
        Row[1][2] = (c * tmp1 - s * tmp2);
        Row[2][2] = (s * tmp1 + c * tmp2);
    }

    __forceinline void In_Place_Pre_Rotate_Y(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[2][0];
        Row[0][0] = (c * tmp1 + s * tmp2);
        Row[2][0] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[2][1];
        Row[0][1] = (c * tmp1 + s * tmp2);
        Row[2][1] = (-s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[2][2];
        Row[0][2] = (c * tmp1 + s * tmp2);
        Row[2][2] = (-s * tmp1 + c * tmp2);
    }

    __forceinline void In_Place_Pre_Rotate_Z(float s, float c)
    {
        float tmp1, tmp2;
        tmp1 = Row[0][0];
        tmp2 = Row[1][0];
        Row[0][0] = (c * tmp1 - s * tmp2);
        Row[1][0] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][1];
        tmp2 = Row[1][1];
        Row[0][1] = (c * tmp1 - s * tmp2);
        Row[1][1] = (s * tmp1 + c * tmp2);

        tmp1 = Row[0][2];
        tmp2 = Row[1][2];
        Row[0][2] = (c * tmp1 - s * tmp2);
        Row[1][2] = (s * tmp1 + c * tmp2);
    }

    int Is_Orthogonal() const;

    void Post_Mul(const Matrix3D &that)
    {
        float f1 = that.Row[1].Y * Row[0].Y + Row[0].Z * that.Row[2].Y + that.Row[0].Y * Row[0].X;
        float f2 = that.Row[1].Z * Row[0].Y + that.Row[2].Z * Row[0].Z + that.Row[0].Z * Row[0].X;
        float f3 = that.Row[0].W * Row[0].X + Row[0].Z * that.Row[2].W + that.Row[1].W * Row[0].Y;
        float f4 = that.Row[1].X * Row[0].Y + that.Row[0].X * Row[0].X;
        float f5 = that.Row[2].X * Row[0].Z;

        Row[0].Z = f2;
        Row[0].X = f4 + f5;
        Row[0].Y = f1;
        Row[0].W = f3 + Row[0].W;

        float f6 = Row[1].Y * that.Row[1].Y + Row[1].Z * that.Row[2].Y + that.Row[0].Y * Row[1].X;
        float f7 = Row[1].Z * that.Row[2].Z + Row[1].Y * that.Row[1].Z + that.Row[0].Z * Row[1].X;
        float f8 = Row[1].Z * that.Row[2].W + Row[1].Y * that.Row[1].W + Row[1].X * that.Row[0].W;

        Row[1].X = Row[1].Z * that.Row[2].X + Row[1].Y * that.Row[1].X + Row[1].X * that.Row[0].X;
        Row[1].Y = f6;
        Row[1].Z = f7;
        Row[1].W = f8 + Row[1].W;

        float f9 = Row[2].Y * that.Row[1].Y + Row[2].Z * that.Row[2].Y + Row[2].X * that.Row[0].Y;
        float f10 = Row[2].Z * that.Row[2].Z + Row[2].Y * that.Row[1].Z + that.Row[0].Z * Row[2].X;
        float f11 = Row[2].Z * that.Row[2].W + Row[2].Y * that.Row[1].W + that.Row[0].W * Row[2].X;
        float f12 = Row[2].Z * that.Row[2].X + Row[2].Y * that.Row[1].X;
        float f13 = Row[2].X * that.Row[0].X;

        Row[2].Z = f10;
        Row[2].X = f12 + f13;
        Row[2].Y = f9;
        Row[2].W = f11 + Row[2].W;
    }

    void Mul(const Matrix3D &A, const Matrix3D &B)
    {
        float f1 = B.Row[0].X;
        float f2 = B.Row[1].X;
        float f3 = B.Row[2].X;

        Row[0].X = f1 * A.Row[0].X + f3 * A.Row[0].Z + f2 * A.Row[0].Y;
        Row[1].X = f3 * A.Row[1].Z + f2 * A.Row[1].Y + f1 * A.Row[1].X;
        Row[2].X = f3 * A.Row[2].Z + f2 * A.Row[2].Y + f1 * A.Row[2].X;

        float f4 = B.Row[0].Y;
        float f5 = B.Row[1].Y;
        float f6 = B.Row[2].Y;

        Row[0].Y = f4 * A.Row[0].X + f6 * A.Row[0].Z + f5 * A.Row[0].Y;
        Row[1].Y = f6 * A.Row[1].Z + f5 * A.Row[1].Y + f4 * A.Row[1].X;
        Row[2].Y = f6 * A.Row[2].Z + f5 * A.Row[2].Y + f4 * A.Row[2].X;

        float f7 = B.Row[0].Z;
        float f8 = B.Row[1].Z;
        float f9 = B.Row[2].Z;

        Row[0].Z = f7 * A.Row[0].X + f9 * A.Row[0].Z + f8 * A.Row[0].Y;
        Row[1].Z = f9 * A.Row[1].Z + f8 * A.Row[1].Y + f7 * A.Row[1].X;
        Row[2].Z = f9 * A.Row[2].Z + f8 * A.Row[2].Y + f7 * A.Row[2].X;

        float f10 = B.Row[0].W;
        float f11 = B.Row[1].W;
        float f12 = B.Row[2].W;

        Row[0].W = f10 * A.Row[0].X + f12 * A.Row[0].Z + f11 * A.Row[0].Y + A.Row[0].W;
        Row[1].W = f12 * A.Row[1].Z + f11 * A.Row[1].Y + f10 * A.Row[1].X + A.Row[1].W;
        Row[2].W = f12 * A.Row[2].Z + f11 * A.Row[2].Y + f10 * A.Row[2].X + A.Row[2].W;
    }

    void Mul_Vector3(Vector3 &v) const { Mul_Vector3(v, v); }

    void Mul_Vector3(const Vector3 &in, Vector3 &out) const
    {
        out.Set(Row[0].X * in.X + Row[0].Y * in.Y + Row[0].Z * in.Z + Row[0].W,
            Row[1].X * in.X + Row[1].Y * in.Y + Row[1].Z * in.Z + Row[1].W,
            Row[2].X * in.X + Row[2].Y * in.Y + Row[2].Z * in.Z + Row[2].W);
    }

    static __forceinline void Transform_Vector(const Matrix3D &A, const Vector3 &in, Vector3 *out)
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

    __forceinline Vector3 Get_X_Vector() const { return Vector3(Row[0][0], Row[1][0], Row[2][0]); }
    __forceinline Vector3 Get_Y_Vector() const { return Vector3(Row[0][1], Row[1][1], Row[2][1]); }
    __forceinline Vector3 Get_Z_Vector() const { return Vector3(Row[0][2], Row[1][2], Row[2][2]); }
    __forceinline void Get_X_Vector(Vector3 *set_x) const { set_x->Set(Row[0][0], Row[1][0], Row[2][0]); }
    __forceinline void Get_Y_Vector(Vector3 *set_y) const { set_y->Set(Row[0][1], Row[1][1], Row[2][1]); }
    __forceinline void Get_Z_Vector(Vector3 *set_z) const { set_z->Set(Row[0][2], Row[1][2], Row[2][2]); }
    void Get_Inverse(Matrix3D &set_inverse) const;
    void Copy_3x3_Matrix(float matrix[3][3]);
    void Transform_Min_Max_AABox(const Vector3 &min, const Vector3 &max, Vector3 *set_min, Vector3 *set_max) const;
    void Transform_Center_Extent_AABox(
        const Vector3 &center, const Vector3 &extent, Vector3 *set_center, Vector3 *set_extent) const;
    void Mul(Matrix3D const &m) { Mul(m, *this); }
    static void Multiply(const Matrix3D &A, const Matrix3D &B, Matrix3D *set_result);

    static __forceinline void Rotate_Vector(const Matrix3D &A, const Vector3 &in, Vector3 *out)
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

    static __forceinline void Inverse_Transform_Vector(const Matrix3D &A, const Vector3 &in, Vector3 *out)
    {
        Vector3 tmp;
        const Vector3 *v;

        if (out == &in) {
            tmp = in;
            v = &tmp;
        } else {
            v = &in;
        }

        Vector3 diff(v->X - A[0][3], v->Y - A[1][3], v->Z - A[2][3]);
        Inverse_Rotate_Vector(A, diff, out);
    }

    static __forceinline void Inverse_Rotate_Vector(const Matrix3D &A, const Vector3 &in, Vector3 *out)
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

    void Build_Transform_Matrix(Vector3 &p, Vector3 &dir);

    static bool Solve_Linear_System(Matrix3D &system);
    void Re_Orthogonalize();

    static const Matrix3D Identity;

protected:
    Vector4 Row[3];
    friend Vector3 operator*(const Matrix3D &A, const Vector3 &a);
};

__forceinline Matrix3D operator*(const Matrix3D &A, const Matrix3D &B)
{
    Matrix3D C;
    float tmp1, tmp2, tmp3;
    tmp1 = B[0][0];
    tmp2 = B[1][0];
    tmp3 = B[2][0];
    C[0][0] = (A[0][0] * tmp1 + A[0][1] * tmp2 + A[0][2] * tmp3);
    C[1][0] = (A[1][0] * tmp1 + A[1][1] * tmp2 + A[1][2] * tmp3);
    C[2][0] = (A[2][0] * tmp1 + A[2][1] * tmp2 + A[2][2] * tmp3);
    tmp1 = B[0][1];
    tmp2 = B[1][1];
    tmp3 = B[2][1];
    C[0][1] = (A[0][0] * tmp1 + A[0][1] * tmp2 + A[0][2] * tmp3);
    C[1][1] = (A[1][0] * tmp1 + A[1][1] * tmp2 + A[1][2] * tmp3);
    C[2][1] = (A[2][0] * tmp1 + A[2][1] * tmp2 + A[2][2] * tmp3);
    tmp1 = B[0][2];
    tmp2 = B[1][2];
    tmp3 = B[2][2];
    C[0][2] = (A[0][0] * tmp1 + A[0][1] * tmp2 + A[0][2] * tmp3);
    C[1][2] = (A[1][0] * tmp1 + A[1][1] * tmp2 + A[1][2] * tmp3);
    C[2][2] = (A[2][0] * tmp1 + A[2][1] * tmp2 + A[2][2] * tmp3);
    tmp1 = B[0][3];
    tmp2 = B[1][3];
    tmp3 = B[2][3];
    C[0][3] = (A[0][0] * tmp1 + A[0][1] * tmp2 + A[0][2] * tmp3 + A[0][3]);
    C[1][3] = (A[1][0] * tmp1 + A[1][1] * tmp2 + A[1][2] * tmp3 + A[1][3]);
    C[2][3] = (A[2][0] * tmp1 + A[2][1] * tmp2 + A[2][2] * tmp3 + A[2][3]);
    return C;
}

__forceinline Vector3 operator*(const Matrix3D &A, const Vector3 &a)
{
    return Vector3((A.Row[0].X * a.X + A.Row[0].Y * a.Y + A.Row[0].Z * a.Z + A.Row[0].W),
        (A.Row[1].X * a.X + A.Row[1].Y * a.Y + A.Row[1].Z * a.Z + A.Row[1].W),
        (A.Row[2].X * a.X + A.Row[2].Y * a.Y + A.Row[2].Z * a.Z + A.Row[2].W));
}

__forceinline bool operator==(const Matrix3D &A, const Matrix3D &B)
{
    return (A[0].X == B[0].X && A[0].Y == B[0].Y && A[0].Z == B[0].Z && A[0].W == B[0].W && A[1].X == B[1].X
        && A[1].Y == B[1].Y && A[1].Z == B[1].Z && A[1].W == B[1].W && A[2].X == B[2].X && A[2].Y == B[2].Y
        && A[2].Z == B[2].Z && A[2].W == B[2].W);
}

__forceinline bool operator!=(const Matrix3D &A, const Matrix3D &B)
{
    return !(A == B);
}
