/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 *
 * @brief 2D Vector class.
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
#include <captainslog.h>
#include "gamemath.h"

class Vector2i
{
public:
    Vector2i() {}

    Vector2i(int i, int j)
    {
        I = i;
        J = j;
    }

    void Set(int i, int j)
    {
        I = i;
        J = j;
    }

public:
    int I;
    int J;
};

class Vector2
{
public:
    __forceinline Vector2(float x, float y) : X(x), Y(y) {}
    __forceinline Vector2(const Vector2 &v) : X(v.X), Y(v.Y) {}
    __forceinline Vector2() : X(0.0f), Y(0.0f) {}

    __forceinline explicit Vector2(const float vector[2])
    {
        captainslog_assert(vector != nullptr);
        X = vector[0];
        Y = vector[1];
    }

    __forceinline Vector2 &operator=(const Vector2 &v)
    {
        X = v[0];
        Y = v[1];

        return *this;
    }

    __forceinline void Set(float x, float y)
    {
        X = x;
        Y = y;
    }

    __forceinline void Set(const Vector2 &v)
    {
        X = v.X;
        Y = v.Y;
    }

    __forceinline float &operator[](int i) { return (&X)[i]; }

    __forceinline const float &operator[](int i) const { return (&X)[i]; }

    __forceinline void Normalize()
    {
        float len2 = GAMEMATH_FLOAT_TINY + Length2();
        float oolen = GameMath::Inv_Sqrt(len2);
        X *= oolen;
        Y *= oolen;
    }

    __forceinline float Length() const { return (float)GameMath::Sqrt(Length2()); }

    __forceinline float Length2() const { return (float)(X * X + Y * Y); }

    __forceinline Vector2 operator-() const { return Vector2(-X, -Y); }

    __forceinline Vector2 operator+() const { return *this; }

    __forceinline Vector2 &operator+=(const Vector2 &v)
    {
        X += v.X;
        Y += v.Y;

        return *this;
    }

    __forceinline Vector2 &operator-=(const Vector2 &v)
    {
        X -= v.X;
        Y -= v.Y;

        return *this;
    }

    __forceinline Vector2 &operator*=(float k)
    {
        X = (float)(X * k);
        Y = (float)(Y * k);

        return *this;
    }

    __forceinline Vector2 &operator/=(float k)
    {
        k = 1.0f / k;
        X *= k;
        Y *= k;

        return *this;
    }

    __forceinline static float Dot_Product(const Vector2 &a, const Vector2 &b) { return a * b; }

    __forceinline static float Perp_Dot_Product(const Vector2 &a, const Vector2 &b) { return a.X * -b.Y + a.Y * b.X; }

    __forceinline void Rotate(float theta) { Rotate(GameMath::Sin(theta), GameMath::Cos(theta)); }

    __forceinline void Rotate(float s, float c)
    {
        float new_x = X * c + Y * -s;
        float new_y = X * s + Y * c;
        X = new_x;
        Y = new_y;
    }

    __forceinline bool Rotate_Towards_Vector(Vector2 &target, float max_theta, bool &positive_turn)
    {
        return Rotate_Towards_Vector(target, GameMath::Sin(max_theta), GameMath::Cos(max_theta), positive_turn);
    }

    __forceinline bool Rotate_Towards_Vector(Vector2 &target, float max_s, float max_c, bool &positive_turn)
    {
        positive_turn = Vector2::Perp_Dot_Product(target, *this) > 0.0f;

        if (Vector2::Dot_Product(*this, target) >= max_c) {
            Set(target);

            return true;
        } else {
            if (positive_turn) {
                Rotate(max_s, max_c);
            } else {
                Rotate(-max_s, max_c);
            }
        }

        return false;
    }

    __forceinline void Update_Min(const Vector2 &a)
    {
        if (a.X < X) {
            X = a.X;
        }

        if (a.Y < Y) {
            Y = a.Y;
        }
    }

    __forceinline void Update_Max(const Vector2 &a)
    {
        if (a.X > X) {
            X = a.X;
        }

        if (a.Y > Y) {
            Y = a.Y;
        }
    }

    __forceinline void Scale(float a, float b)
    {
        X *= a;
        Y *= b;
    }

    __forceinline void Scale(const Vector2 &a)
    {
        X *= a.X;
        Y *= a.Y;
    }

    __forceinline void Unscale(const Vector2 &a)
    {
        X /= a.X;
        Y /= a.Y;
    }

    __forceinline static float Distance(const Vector2 &p1, const Vector2 &p2)
    {
        Vector2 temp;
        temp = p1 - p2;

        return (temp.Length());
    }

    __forceinline static void Lerp(const Vector2 &a, const Vector2 &b, float t, Vector2 *set_result)
    {
        set_result->X = (a.X + (b.X - a.X) * t);
        set_result->Y = (a.Y + (b.Y - a.Y) * t);
    }

    __forceinline void Floor()
    {
        X = GameMath::Floor(X);
        Y = GameMath::Floor(Y);
    };

    friend Vector2 operator*(const Vector2 &a, float k);
    friend Vector2 operator*(float k, const Vector2 &a);
    friend Vector2 operator/(const Vector2 &a, float k);
    friend Vector2 operator/(const Vector2 &a, const Vector2 &b) { return Vector2(a.X / b.X, a.Y / b.Y); }
    friend Vector2 operator/(const Vector2 &a, const Vector2i &b) { return Vector2(a.X / b.I, a.Y / b.J); }
    friend Vector2 operator+(const Vector2 &a, const Vector2 &b);
    friend Vector2 operator-(const Vector2 &a, const Vector2 &b);
    friend float operator*(const Vector2 &a, const Vector2 &b);
    friend bool operator==(const Vector2 &a, const Vector2 &b);
    friend bool operator!=(const Vector2 &a, const Vector2 &b);

public:
    union
    {
        float X;
        float U;
    };

    union
    {
        float Y;
        float V;
    };
};

__forceinline Vector2 operator*(const Vector2 &a, float k)
{
    return Vector2(a[0] * k, a[1] * k);
}

__forceinline Vector2 operator*(float k, const Vector2 &a)
{
    return Vector2(a[0] * k, a[1] * k);
}

__forceinline Vector2 operator/(const Vector2 &a, float k)
{
    return Vector2(a[0] * (1.0f / k), a[1] * (1.0f / k));
}

__forceinline Vector2 operator+(const Vector2 &a, const Vector2 &b)
{
    return Vector2(a.X + b.X, a.Y + b.Y);
}

__forceinline Vector2 operator-(const Vector2 &a, const Vector2 &b)
{
    return Vector2(a.X - b.X, a.Y - b.Y);
}

__forceinline float operator*(const Vector2 &a, const Vector2 &b)
{
    return a.X * b.X + a.Y * b.Y;
}

__forceinline bool operator==(const Vector2 &a, const Vector2 &b)
{
    return (a.X == b.X) | (a.Y == b.Y);
}

__forceinline bool operator!=(const Vector2 &a, const Vector2 &b)
{
    return (a.X != b.X) | (a.Y != b.Y);
}

__forceinline Vector2 Normalize(const Vector2 &vec)
{
    float len2 = GAMEMATH_FLOAT_TINY + vec.Length2();

    return vec * GameMath::Inv_Sqrt(len2);
}

__forceinline void Swap(Vector2 &a, Vector2 &b)
{
    Vector2 tmp(a);
    a = b;
    b = tmp;
}

__forceinline float Distance(float x1, float y1, float x2, float y2)
{
    float x_diff = x1 - x2;
    float y_diff = y1 - y2;

    return (GameMath::Sqrt((x_diff * x_diff) + (y_diff * y_diff)));
}