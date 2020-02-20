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
    __forceinline Vector4(const Vector4 &v) { X = v.X; Y = v.Y; Z = v.Z; W = v.W; }
    __forceinline Vector4(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }
    __forceinline explicit Vector4(const float v[4]) { captainslog_assert(v != nullptr); X = v[0]; Y = v[1]; Z = v[2]; W = v[3]; }

    // conversion constructors
    //__forceinline explicit Vector4(const Vector2& v) { X = v.X; Y = v.Y; Z = 0.0f; W = 0.0f; }
    //__forceinline explicit Vector4(const Vector3& v) { X = v.X; Y = v.Y; Z = v.Z; W = 0.0f; }

    // assignment operators
    __forceinline Vector4 &operator=(const Vector4 &v) { X = v.X; Y = v.Y; Z = v.Z; W = v.W; return *this; }
    __forceinline void Set(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }

    // array access operators
    __forceinline float &operator[](int i) { return (&X)[i]; }
    __forceinline const float &operator[](int i) const { return (&X)[i]; }

    __forceinline void Normalize(void)
    {
        float len2 = GAMEMATH_FLOAT_TINY + Length2();
        float oolen = GameMath::Inv_Sqrt(len2);
        X *= oolen;
        Y *= oolen;
        Z *= oolen;
        W *= oolen;
    }

    __forceinline float Length(void) const
    {
        return GameMath::Sqrt(Length2());
    }

    __forceinline float Length2(void) const
    {
        return X * X + Y * Y + Z * Z + W * W;
    }

    // unary operators
    __forceinline Vector4 operator-()const { return(Vector4(-X, -Y, -Z, -W)); }
    __forceinline Vector4 operator+()const { return *this; }

    __forceinline Vector4 &operator+=(const Vector4 &v) { X += v.X; Y += v.Y; Z += v.Z; W += v.W; return *this; }
    __forceinline Vector4 &operator-=(const Vector4 &v) { X -= v.X; Y -= v.Y; Z -= v.Z; W -= v.W; return *this; }
    __forceinline Vector4 &operator*=(float f) { X *= f; Y *= f; Z *= f; W *= f; return *this; }
    __forceinline Vector4 &operator/=(float f) { f = 1.0f / f; X /= f; Y /= f; Z /= f; W /= f;	return *this; }

    // vector addition/subtraction
    friend Vector4 operator+(const Vector4 &a, const Vector4 &b);
    friend Vector4 operator-(const Vector4 &a, const Vector4 &b);

    // scalar multiplication/division
    friend Vector4 operator*(const Vector4 &a, float b);
    friend Vector4 operator*(float a, const Vector4 &b);
    friend Vector4 operator/(const Vector4 &a, float b);

    // equality operators
    friend bool operator== (const Vector4 &a, const Vector4 &b);
    friend bool operator!= (const Vector4 &a, const Vector4 &b);

    friend float operator * (const Vector4 &a, const Vector4 &b);
    __forceinline static float Dot_Product(const Vector4 &a, const Vector4 &b)
    {
        return a * b;
    }

    __forceinline static Vector4 Lerp(const Vector4 &a, const Vector4 &b, float alpha)
    {
        return Vector4((a.X + (b.X - a.X) * alpha), (a.Y + (b.Y - a.Y) * alpha), (a.Z + (b.Z - a.Z) * alpha), (a.W + (b.W - a.W) * alpha));
    }

    __forceinline static void Lerp(const Vector4 &a, const Vector4 &b, float alpha, Vector4 * set_result)
    {
        set_result->X = (a.X + (b.X - a.X) * alpha);
        set_result->Y = (a.Y + (b.Y - a.Y) * alpha);
        set_result->Z = (a.Z + (b.Z - a.Z) * alpha);
        set_result->W = (a.W + (b.W - a.W) * alpha);
    }

public:
    float X;
    float Y;
    float Z;
    float W;
};

__forceinline Vector4 operator+(const Vector4 &a, const Vector4 &b)
{
    return Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
}

__forceinline Vector4 operator-(const Vector4 &a, const Vector4 &b)
{
    return Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
}

__forceinline Vector4 operator*(const Vector4 &a, float b)
{
    return Vector4(a.X * b, a.Y * b, a.Z * b, a.W * b);
}

__forceinline Vector4 operator*(float a, const Vector4 &b)
{
    return b * a;
}

__forceinline Vector4 operator/(const Vector4 &a, float k)
{
    return Vector4(a.X * 1.0f / k, a.Y * 1.0f / k, a.Z * 1.0f / k, a.W * 1.0f / k);
}

__forceinline bool operator==(const Vector4 &a, const Vector4 &b)
{
    return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z) && (a.W == b.W);
}

__forceinline bool operator!=(const Vector4 &a, const Vector4 &b)
{
    return (a.X != b.X) || (a.Y != b.Y) || (a.Z != b.Z) || (a.W != b.W);
}

__forceinline float operator*(const Vector4 &a, const Vector4 &b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

__forceinline Vector4 Normalize(const Vector4 &vec)
{
    float len2 = GAMEMATH_FLOAT_TINY + vec.Length2();
    return vec * GameMath::Inv_Sqrt(len2);
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
        (a.X + (b.X - a.X) * alpha),
        (a.Y + (b.Y - a.Y) * alpha),
        (a.Z + (b.Z - a.Z) * alpha),
        (a.W + (b.W - a.W) * alpha)
    );
}