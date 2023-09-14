/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Quaternion class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "vector3.h"

class Vector2;
class Vector3;
class Matrix3;
class Matrix3D;
class Matrix4;

// Struct used for cached linear interpolation.
struct SlerpInfoStruct
{
    float sin_theta;
    float theta;
    bool flip;
    bool linear;
};

class Quaternion
{
public:
    Quaternion() {}
    __forceinline explicit Quaternion(bool init)
    {
        if (init) {
            X = 0.0f;
            Y = 0.0f;
            Z = 0.0f;
            W = 1.0f;
        }
    }
    __forceinline explicit Quaternion(float a, float b, float c, float d)
    {
        X = a;
        Y = b;
        Z = c;
        W = d;
    }
    __forceinline explicit Quaternion(const Vector3 &axis, float angle);

    __forceinline Quaternion &operator=(const Quaternion &source);

    void Normalize();

    float &operator[](int i) { return (&X)[i]; }
    const float &operator[](int i) const { return (&X)[i]; }

    void Scale(float s)
    {
        X = (float)(s * X);
        Y = (float)(s * Y);
        Z = (float)(s * Z);
        W = (float)(s * W);
    }

    bool Is_Valid() const;
    __forceinline void Make_Identity() { Set(); }
    __forceinline void Set(float a = 0.0f, float b = 0.0f, float c = 0.0f, float d = 1.0f)
    {
        X = a;
        Y = b;
        Z = c;
        W = d;
    }

    Vector3 Rotate_Vector(const Vector3 &v) const;
    void Rotate_Vector(const Vector3 &v, Vector3 *set_result) const;
    __forceinline Quaternion operator-() const { return (Quaternion(-X, -Y, -Z, -W)); }
    __forceinline Quaternion operator+() const { return *this; }
    Quaternion &Make_Closest(const Quaternion &qto);
    __forceinline float Length2() const { return (X * X + Y * Y + Z * Z + W * W); }
    __forceinline float Length() const { return GameMath::Sqrt(Length2()); }
    void Rotate_X(float theta);
    void Rotate_Y(float theta);
    void Rotate_Z(float theta);
    void Randomize();

public:
    float X;
    float Y;
    float Z;
    float W;
};

__forceinline Quaternion operator*(const Quaternion &a, const Quaternion &b)
{
    return Quaternion(a.W * b.X + b.W * a.X + (a.Y * b.Z - b.Y * a.Z),
        a.W * b.Y + b.W * a.Y - (a.X * b.Z - b.X * a.Z),
        a.W * b.Z + b.W * a.Z + (a.X * b.Y - b.X * a.Y),
        a.W * b.W - (a.X * b.X + a.Y * b.Y + a.Z * b.Z));
}

__forceinline bool Quaternion::Is_Valid() const
{
    return (GameMath::Is_Valid_Float(X) && GameMath::Is_Valid_Float(Y) && GameMath::Is_Valid_Float(Z)
        && GameMath::Is_Valid_Float(W));
}

__forceinline Vector3 Quaternion::Rotate_Vector(const Vector3 &v) const
{
    float x = W * v.X + (Y * v.Z - v.Y * Z);
    float y = W * v.Y - (X * v.Z - v.X * Z);
    float z = W * v.Z + (X * v.Y - v.X * Y);
    float w = -(X * v.X + Y * v.Y + Z * v.Z);

    return Vector3(w * (-X) + W * x + (y * (-Z) - (-Y) * z),
        w * (-Y) + W * y - (x * (-Z) - (-X) * z),
        w * (-Z) + W * z + (x * (-Y) - (-X) * y));
}

__forceinline void Quaternion::Rotate_Vector(const Vector3 &v, Vector3 *result) const
{
    captainslog_assert(result != nullptr);

    float x = W * v.X + (Y * v.Z - v.Y * Z);
    float y = W * v.Y - (X * v.Z - v.X * Z);
    float z = W * v.Z + (X * v.Y - v.X * Y);
    float w = -(X * v.X + Y * v.Y + Z * v.Z);

    result->X = w * (-X) + W * x + (y * (-Z) - (-Y) * z);
    result->Y = w * (-Y) + W * y - (x * (-Z) - (-X) * z);
    result->Z = w * (-Z) + W * z + (x * (-Y) - (-X) * y);
}

__forceinline Quaternion &Quaternion::operator=(const Quaternion &source)
{
    X = source[0];
    Y = source[1];
    Z = source[2];
    W = source[3];

    return *this;
}

__forceinline Quaternion Inverse(const Quaternion &a)
{
    return Quaternion(-a[0], -a[1], -a[2], a[3]);
}

__forceinline Quaternion operator/(const Quaternion &a, const Quaternion &b)
{
    return a * Inverse(b);
}

__forceinline Quaternion operator*(float scl, const Quaternion &a)
{
    return Quaternion(scl * a[0], scl * a[1], scl * a[2], scl * a[3]);
}

__forceinline Quaternion operator*(const Quaternion &a, float scl)
{
    return scl * a;
}

__forceinline Quaternion Conjugate(const Quaternion &a)
{
    return Quaternion(-a[0], -a[1], -a[2], a[3]);
}

__forceinline Quaternion operator+(const Quaternion &a, const Quaternion &b)
{
    return Quaternion(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

__forceinline Quaternion operator-(const Quaternion &a, const Quaternion &b)
{
    return Quaternion(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);
}

__forceinline Quaternion Normalize(const Quaternion &a)
{
    float mag = a.Length();
    if (0.0f == mag) {
        return a;
    } else {
        float oomag = 1.0f / mag;
        return Quaternion(a[0] * oomag, a[1] * oomag, a[2] * oomag, a[3] * oomag);
    }
}

__forceinline bool Equal_Within_Epsilon(const Quaternion &a, const Quaternion &b, float epsilon)
{
    return ((GameMath::Fabs(a.X - b.X) < epsilon) && (GameMath::Fabs(a.Y - b.Y) < epsilon)
        && (GameMath::Fabs(a.Z - b.Z) < epsilon) && (GameMath::Fabs(a.W - b.W) < epsilon));
}

Quaternion Axis_To_Quat(const Vector3 &a, float angle);
Quaternion Trackball(float x0, float y0, float x1, float y1, float sphsize);
void Slerp(Quaternion &result, const Quaternion &a, const Quaternion &b, float t);
void Fast_Slerp(Quaternion &result, const Quaternion &a, const Quaternion &b, float t);
Quaternion Build_Quaternion(const Matrix3 &matrix);
Quaternion Build_Quaternion(const Matrix3D &matrix);
Quaternion Build_Quaternion(const Matrix4 &matrix);
Matrix3 Build_Matrix3(const Quaternion &quat);
Matrix3D Build_Matrix3D(const Quaternion &quat);
Matrix4 Build_Matrix4(const Quaternion &quat);
void Slerp_Setup(const Quaternion &p, const Quaternion &q, SlerpInfoStruct *slerpinfo);
void Cached_Slerp(const Quaternion &p, const Quaternion &q, float alpha, SlerpInfoStruct *slerpinfo, Quaternion *set_q);
Quaternion Cached_Slerp(const Quaternion &p, const Quaternion &q, float alpha, SlerpInfoStruct *slerpinfo);
