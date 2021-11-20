/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief 3D Vector class.
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

class Vector3
{
public:
    __forceinline Vector3() {}

    __forceinline Vector3(const Vector3 &v)
    {
        X = v.X;
        Y = v.Y;
        Z = v.Z;
    }

    __forceinline Vector3(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    bool Is_Valid() const;

    __forceinline Vector3(const float vector[3])
    {
        X = vector[0];
        Y = vector[1];
        Z = vector[2];
    }

    __forceinline Vector3 &operator=(const Vector3 &v)
    {
        X = v.X;
        Y = v.Y;
        Z = v.Z;

        return *this;
    }

    __forceinline void Set(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    __forceinline void Set(const Vector3 &that)
    {
        X = that.X;
        Y = that.Y;
        Z = that.Z;
    }

    __forceinline float &operator[](int i) { return (&X)[i]; }

    __forceinline const float &operator[](int i) const { return (&X)[i]; }

    __forceinline void Normalize()
    {
        float len2 = Length2();

        if (len2 != 0.0f) {
            float oolen = GameMath::Inv_Sqrt(Length2());
            X *= oolen;
            Y *= oolen;
            Z *= oolen;
        }
    }

    __forceinline float Length() const { return GameMath::Sqrt(Length2()); }

    __forceinline float Length2() const { return X * X + Y * Y + Z * Z; }

    __forceinline void Scale(const Vector3 &scale)
    {
        X *= scale.X;
        Y *= scale.Y;
        Z *= scale.Z;
    }

    __forceinline void Rotate_X(float angle) { Rotate_X(GameMath::Sin(angle), GameMath::Cos(angle)); }

    __forceinline void Rotate_X(float s_angle, float c_angle)
    {
        float tmp_y = Y;
        float tmp_z = Z;
        Y = c_angle * tmp_y - s_angle * tmp_z;
        Z = s_angle * tmp_y + c_angle * tmp_z;
    }

    __forceinline void Rotate_Y(float angle) { Rotate_Y(GameMath::Sin(angle), GameMath::Cos(angle)); }

    __forceinline void Rotate_Y(float s_angle, float c_angle)
    {
        float tmp_x = X;
        float tmp_z = Z;
        X = c_angle * tmp_x + s_angle * tmp_z;
        Z = -s_angle * tmp_x + c_angle * tmp_z;
    }

    __forceinline void Rotate_Z(float angle) { Rotate_Z(GameMath::Sin(angle), GameMath::Cos(angle)); }

    __forceinline void Rotate_Z(float s_angle, float c_angle)
    {
        float tmp_x = X;
        float tmp_y = Y;
        X = c_angle * tmp_x - s_angle * tmp_y;
        Y = s_angle * tmp_x + c_angle * tmp_y;
    }

    __forceinline Vector3 operator-() const { return Vector3(-X, -Y, -Z); }

    __forceinline Vector3 operator+() const { return *this; }

    __forceinline Vector3 &operator+=(const Vector3 &v)
    {
        X += v.X;
        Y += v.Y;
        Z += v.Z;

        return *this;
    }

    __forceinline Vector3 &operator-=(const Vector3 &v)
    {
        X -= v.X;
        Y -= v.Y;
        Z -= v.Z;

        return *this;
    }

    __forceinline Vector3 &operator*=(float k)
    {
        X = X * k;
        Y = Y * k;
        Z = Z * k;

        return *this;
    }

    __forceinline Vector3 &operator/=(float k)
    {
        float ook = 1.0f / k;
        X = X * ook;
        Y = Y * ook;
        Z = Z * ook;

        return *this;
    }

    friend Vector3 operator*(const Vector3 &a, float k);
    friend Vector3 operator*(float k, const Vector3 &a);
    friend Vector3 operator/(const Vector3 &a, float k);
    friend Vector3 operator+(const Vector3 &a, const Vector3 &b);
    friend Vector3 operator-(const Vector3 &a, const Vector3 &b);
    friend bool operator==(const Vector3 &a, const Vector3 &b);
    friend bool operator!=(const Vector3 &a, const Vector3 &b);
    friend float operator*(const Vector3 &a, const Vector3 &b);

    __forceinline static float Dot_Product(const Vector3 &a, const Vector3 &b) { return a * b; }

    __forceinline static Vector3 Cross_Product(const Vector3 &a, const Vector3 &b)
    {
        return Vector3((a.Y * b.Z - a.Z * b.Y), (a.Z * b.X - a.X * b.Z), (a.X * b.Y - a.Y * b.X));
    }

    __forceinline static void Cross_Product(const Vector3 &a, const Vector3 &b, Vector3 *set_result)
    {
        captainslog_assert(set_result != &a);
        set_result->X = (a.Y * b.Z - a.Z * b.Y);
        set_result->Y = (a.Z * b.X - a.X * b.Z);
        set_result->Z = (a.X * b.Y - a.Y * b.X);
    }

    __forceinline static void Normalized_Cross_Product(const Vector3 &a, const Vector3 &b, Vector3 *set_result)
    {
        set_result->X = (a.Y * b.Z - a.Z * b.Y);
        set_result->Y = (a.Z * b.X - a.X * b.Z);
        set_result->Z = (a.X * b.Y - a.Y * b.X);
        set_result->Normalize();
    }

    __forceinline static float Cross_Product_X(const Vector3 &a, const Vector3 &b) { return a.Y * b.Z - a.Z * b.Y; }

    __forceinline static float Cross_Product_Y(const Vector3 &a, const Vector3 &b) { return a.Z * b.X - a.X * b.Z; }

    __forceinline static float Cross_Product_Z(const Vector3 &a, const Vector3 &b) { return a.X * b.Y - a.Y * b.X; }

    __forceinline static void Add(const Vector3 &a, const Vector3 &b, Vector3 *set_result)
    {
        captainslog_assert(set_result != NULL);
        set_result->X = a.X + b.X;
        set_result->Y = a.Y + b.Y;
        set_result->Z = a.Z + b.Z;
    }

    __forceinline static void Subtract(const Vector3 &a, const Vector3 &b, Vector3 *set_result)
    {
        captainslog_assert(set_result != NULL);
        set_result->X = a.X - b.X;
        set_result->Y = a.Y - b.Y;
        set_result->Z = a.Z - b.Z;
    }

    __forceinline static float Find_X_At_Y(float y, const Vector3 &p1, const Vector3 &p2)
    {
        return p1.X + ((y - p1.Y) * ((p2.X - p1.X) / (p2.Y - p1.Y)));
    }

    __forceinline static float Find_X_At_Z(float z, const Vector3 &p1, const Vector3 &p2)
    {
        return p1.X + ((z - p1.Z) * ((p2.X - p1.X) / (p2.Z - p1.Z)));
    }

    __forceinline static float Find_Y_At_X(float x, const Vector3 &p1, const Vector3 &p2)
    {
        return p1.Y + ((x - p1.X) * ((p2.Y - p1.Y) / (p2.X - p1.X)));
    }

    __forceinline static float Find_Y_At_Z(float z, const Vector3 &p1, const Vector3 &p2)
    {
        return p1.Y + ((z - p1.Z) * ((p2.Y - p1.Y) / (p2.Z - p1.Z)));
    }

    __forceinline static float Find_Z_At_X(float x, const Vector3 &p1, const Vector3 &p2)
    {
        return p1.Z + ((x - p1.X) * ((p2.Z - p1.Z) / (p2.X - p1.X)));
    }

    __forceinline static float Find_Z_At_Y(float y, const Vector3 &p1, const Vector3 &p2)
    {
        return p1.Z + ((y - p1.Y) * ((p2.Z - p1.Z) / (p2.Y - p1.Y)));
    }

    __forceinline void Update_Min(const Vector3 &a)
    {
        if (a.X < X) {
            X = a.X;
        }

        if (a.Y < Y) {
            Y = a.Y;
        }

        if (a.Z < Z) {
            Z = a.Z;
        }
    }

    __forceinline void Update_Max(const Vector3 &a)
    {
        if (a.X > X) {
            X = a.X;
        }

        if (a.Y > Y) {
            Y = a.Y;
        }

        if (a.Z > Z) {
            Z = a.Z;
        }
    }

    __forceinline void Cap_Absolute_To(const Vector3 &a)
    {
        if (X > 0) {
            if (a.X < X) {
                X = a.X;
            }
        } else {
            if (-a.X > X) {
                X = -a.X;
            }
        }

        if (Y > 0) {
            if (a.Y < Y) {
                Y = a.Y;
            }
        } else {
            if (-a.Y > Y) {
                Y = -a.Y;
            }
        }

        if (Z > 0) {
            if (a.Z < Z) {
                Z = a.Z;
            }
        } else {
            if (-a.Z > Z) {
                Z = -a.Z;
            }
        }
    }

    __forceinline static float Distance(const Vector3 &p1, const Vector3 &p2)
    {
        Vector3 temp;
        temp = p1 - p2;

        return temp.Length();
    }

    __forceinline static float Quick_Distance(const Vector3 &p1, const Vector3 &p2)
    {
        Vector3 temp;
        temp = p1 - p2;

        return temp.Quick_Length();
    }

    __forceinline static void Lerp(const Vector3 &a, const Vector3 &b, float alpha, Vector3 *set_result)
    {
        captainslog_assert(set_result != NULL);
        set_result->X = (a.X + (b.X - a.X) * alpha);
        set_result->Y = (a.Y + (b.Y - a.Y) * alpha);
        set_result->Z = (a.Z + (b.Z - a.Z) * alpha);
    }

    uint32_t Convert_To_ABGR() const;
    uint32_t Convert_To_ARGB() const;

    __forceinline float Quick_Length() const
    {
        float max = GameMath::Fabs(X);
        float mid = GameMath::Fabs(Y);
        float min = GameMath::Fabs(Z);
        float tmp;

        if (max < mid) {
            tmp = max;
            max = mid;
            mid = tmp;
        }

        if (max < min) {
            tmp = max;
            max = min;
            min = tmp;
        }

        if (mid < min) {
            tmp = mid;
            mid = min;
            min = mid;
        }

        return max + (11.0f / 32.0f) * mid + (1.0f / 4.0f) * min;
    }

public:
    float X;
    float Y;
    float Z;
};

__forceinline Vector3 operator*(const Vector3 &a, float k)
{
    return Vector3((a.X * k), (a.Y * k), (a.Z * k));
}

__forceinline Vector3 operator*(float k, const Vector3 &a)
{
    return Vector3((a.X * k), (a.Y * k), (a.Z * k));
}

__forceinline Vector3 operator/(const Vector3 &a, float k)
{
    float ook = 1.0f / k;
    return Vector3((a.X * ook), (a.Y * ook), (a.Z * ook));
}

__forceinline Vector3 operator+(const Vector3 &a, const Vector3 &b)
{
    return Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
}

__forceinline Vector3 operator-(const Vector3 &a, const Vector3 &b)
{
    return Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
}

__forceinline float operator*(const Vector3 &a, const Vector3 &b)
{
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
}

__forceinline bool operator==(const Vector3 &a, const Vector3 &b)
{
    return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z);
}

__forceinline bool operator!=(const Vector3 &a, const Vector3 &b)
{
    return (a.X != b.X) || (a.Y != b.Y) || (a.Z != b.Z);
}

__forceinline bool Equal_Within_Epsilon(const Vector3 &a, const Vector3 &b, float epsilon)
{
    return (GameMath::Fabs(a.X - b.X) < epsilon) && (GameMath::Fabs(a.Y - b.Y) < epsilon)
        && (GameMath::Fabs(a.Z - b.Z) < epsilon);
}

__forceinline Vector3 Normalize(const Vector3 &vec)
{
    float len2 = vec.Length2();

    if (len2 != 0.0f) {
        float oolen = GameMath::Inv_Sqrt(len2);
        return vec * oolen;
    }

    return vec;
}

__forceinline void Swap(Vector3 &a, Vector3 &b)
{
    Vector3 tmp(a);
    a = b;
    b = tmp;
}

__forceinline Vector3 Lerp(const Vector3 &a, const Vector3 &b, float alpha)
{
    return Vector3((a.X + (b.X - a.X) * alpha), (a.Y + (b.Y - a.Y) * alpha), (a.Z + (b.Z - a.Z) * alpha));
}

__forceinline void Lerp(const Vector3 &a, const Vector3 &b, float alpha, Vector3 *set_result)
{
    captainslog_assert(set_result != NULL);
    set_result->X = (a.X + (b.X - a.X) * alpha);
    set_result->Y = (a.Y + (b.Y - a.Y) * alpha);
    set_result->Z = (a.Z + (b.Z - a.Z) * alpha);
}

__forceinline bool Vector3::Is_Valid() const
{
    return GameMath::Is_Valid_Float(X) && GameMath::Is_Valid_Float(Y) && GameMath::Is_Valid_Float(Z);
}

__forceinline uint32_t Vector3::Convert_To_ABGR() const
{
    return (unsigned(255) << 24) | (unsigned(Z * 255.0f) << 16) | (unsigned(Y * 255.0f) << 8) | (unsigned(X * 255.0f));
}

__forceinline uint32_t Vector3::Convert_To_ARGB() const
{
    return (unsigned(255) << 24) | (unsigned(X * 255.0f) << 16) | (unsigned(Y * 255.0f) << 8) | (unsigned(Z * 255.0f));
}
