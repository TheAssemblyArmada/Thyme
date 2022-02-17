/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief 4D Vector class.
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
#include "gamemath.h"
#include <captainslog.h>

class Vector4
{
public:
    // default constructors
    __forceinline Vector4() {}
    __forceinline Vector4(const Vector4 &v)
    {
        X = v.X;
        Y = v.Y;
        Z = v.Z;
        W = v.W;
    }
    __forceinline Vector4(float x, float y, float z, float w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }
    __forceinline Vector4(const float vector[4])
    {
        X = vector[0];
        Y = vector[1];
        Z = vector[2];
        W = vector[3];
    }

    // conversion constructors
    //__forceinline explicit Vector4(const Vector2& v) { X = v.X; Y = v.Y; Z = 0.0f; W = 0.0f; }
    //__forceinline explicit Vector4(const Vector3& v) { X = v.X; Y = v.Y; Z = v.Z; W = 0.0f; }

    // assignment operators
    __forceinline Vector4 &operator=(const Vector4 &v)
    {
        X = v.X;
        Y = v.Y;
        Z = v.Z;
        W = v.W;
        return *this;
    }
    __forceinline void Set(float x, float y, float z, float w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    // array access operators
    __forceinline float &operator[](int i) { return (&X)[i]; }
    __forceinline const float &operator[](int i) const { return (&X)[i]; }

    __forceinline void Normalize()
    {
        float len2 = Length2();

        if (len2 != 0.0f) {
            float oolen = GameMath::Inv_Sqrt(len2);
            X *= oolen;
            Y *= oolen;
            Z *= oolen;
            W *= oolen;
        }
    }

    __forceinline float Length() const { return GameMath::Sqrt(Length2()); }

    __forceinline float Length2() const { return X * X + Y * Y + Z * Z + W * W; }

    // unary operators
    __forceinline Vector4 operator-() const { return Vector4(-X, -Y, -Z, -W); }
    __forceinline Vector4 operator+() const { return *this; }

    __forceinline Vector4 &operator+=(const Vector4 &v)
    {
        X += v.X;
        Y += v.Y;
        Z += v.Z;
        W += v.W;
        return *this;
    }
    __forceinline Vector4 &operator-=(const Vector4 &v)
    {
        X -= v.X;
        Y -= v.Y;
        Z -= v.Z;
        W -= v.W;
        return *this;
    }
    __forceinline Vector4 &operator*=(float k)
    {
        X = X * k;
        Y = Y * k;
        Z = Z * k;
        W = W * k;
        return *this;
    }
    __forceinline Vector4 &operator/=(float k)
    {
        k = 1.0f / k;
        X = X * k;
        Y = Y * k;
        Z = Z * k;
        W = W * k;
        return *this;
    }

    // vector addition/subtraction
    friend Vector4 operator+(const Vector4 &a, const Vector4 &b);
    friend Vector4 operator-(const Vector4 &a, const Vector4 &b);

    // scalar multiplication/division
    friend Vector4 operator*(const Vector4 &a, float b);
    friend Vector4 operator*(float a, const Vector4 &b);
    friend Vector4 operator/(const Vector4 &a, float b);

    // equality operators
    friend bool operator==(const Vector4 &a, const Vector4 &b);
    friend bool operator!=(const Vector4 &a, const Vector4 &b);

    friend float operator*(const Vector4 &a, const Vector4 &b);
    __forceinline static float Dot_Product(const Vector4 &a, const Vector4 &b) { return a * b; }

    __forceinline static Vector4 Lerp(const Vector4 &a, const Vector4 &b, float alpha)
    {
        return Vector4((a.X + (b.X - a.X) * alpha),
            (a.Y + (b.Y - a.Y) * alpha),
            (a.Z + (b.Z - a.Z) * alpha),
            (a.W + (b.W - a.W) * alpha));
    }

    __forceinline static void Lerp(const Vector4 &a, const Vector4 &b, float alpha, Vector4 *set_result)
    {
        set_result->X = (a.X + (b.X - a.X) * alpha);
        set_result->Y = (a.Y + (b.Y - a.Y) * alpha);
        set_result->Z = (a.Z + (b.Z - a.Z) * alpha);
        set_result->W = (a.W + (b.W - a.W) * alpha);
    }

    __forceinline bool Is_Valid() const
    {
        return GameMath::Is_Valid_Float(X) && GameMath::Is_Valid_Float(Y) && GameMath::Is_Valid_Float(Z)
            && GameMath::Is_Valid_Float(W);
    }

public:
    float X;
    float Y;
    float Z;
    float W;
};

__forceinline Vector4 operator+(const Vector4 &a, const Vector4 &b)
{
    return Vector4(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

__forceinline Vector4 operator-(const Vector4 &a, const Vector4 &b)
{
    return Vector4(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);
}

__forceinline Vector4 operator*(const Vector4 &a, float k)
{
    return Vector4((a.X * k), (a.Y * k), (a.Z * k), (a.W * k));
}

__forceinline Vector4 operator*(float k, const Vector4 &a)
{
    return a * k;
}

__forceinline Vector4 operator/(const Vector4 &a, float k)
{
    float ook = 1.0f / k;
    return Vector4((a[0] * ook), (a[1] * ook), (a[2] * ook), (a[3] * ook));
}

__forceinline bool operator==(const Vector4 &a, const Vector4 &b)
{
    return (a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]);
}

__forceinline bool operator!=(const Vector4 &a, const Vector4 &b)
{
    return (a[0] != b[0]) || (a[1] != b[1]) || (a[2] != b[2]) || (a[3] != b[3]);
}

__forceinline float operator*(const Vector4 &a, const Vector4 &b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

__forceinline Vector4 Normalize(const Vector4 &vec)
{
    float len2 = vec.Length2();

    if (len2 != 0.0f) {
        float oolen = GameMath::Inv_Sqrt(len2);
        return vec * oolen;
    }

    return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

__forceinline void Swap(Vector4 &a, Vector4 &b)
{
    Vector4 tmp(a);
    a = b;
    b = tmp;
}

__forceinline Vector4 Lerp(const Vector4 &a, const Vector4 &b, float alpha)
{
    return Vector4(
        (a.X + (b.X - a.X) * alpha), (a.Y + (b.Y - a.Y) * alpha), (a.Z + (b.Z - a.Z) * alpha), (a.W + (b.W - a.W) * alpha));
}

class Vector3;

namespace Thyme
{
// Casting from Vector4 pointer to Vector3 pointer is considered safe.

inline const Vector3 *To_Vector3_Ptr(const Vector4 *vector)
{
    return reinterpret_cast<const Vector3 *>(vector);
}

inline Vector3 *To_Vector3_Ptr(Vector4 *vector)
{
    return reinterpret_cast<Vector3 *>(vector);
}
} // namespace Thyme
