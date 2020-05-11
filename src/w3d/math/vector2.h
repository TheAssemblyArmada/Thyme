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
#include "gamemath.h"
#include <captainslog.h>

__forceinline float Quick_Distance(float x1, float y1, float x2, float y2);
class Vector2
{
public:
    __forceinline Vector2(float x, float y)
    {
        X = x;
        Y = y;
    }
    __forceinline Vector2(const Vector2 &v)
    {
        X = v.X;
        Y = v.Y;
    }
    __forceinline Vector2() {}

    __forceinline Vector2(const float vector[2])
    {
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
        float len2 = Length2();

        if (len2 != 0.0f) {
            float oolen = GameMath::Inv_Sqrt(len2);
            X *= oolen;
            Y *= oolen;
        }
    }

    __forceinline float Length() const { return (float)GameMath::Sqrt(Length2()); }

    __forceinline float Length2() const { return X * X + Y * Y; }

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
        bool return_value = false;
        positive_turn = Perp_Dot_Product(target, *this) > 0.0f;

        if (Vector2::Dot_Product(*this, target) >= max_c) {
            Set(target);

            return_value = true;
        } else {
            if (positive_turn) {
                Rotate(max_s, max_c);
            } else {
                Rotate(-max_s, max_c);
            }
        }

        return return_value;
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

    __forceinline static float Distance(const Vector2 &p1, const Vector2 &p2)
    {
        Vector2 temp;
        temp = p1 - p2;

        return temp.Length();
    }

    __forceinline static void Lerp(const Vector2 &a, const Vector2 &b, float t, Vector2 *set_result)
    {
        captainslog_assert(set_result != nullptr);
        set_result->X = (a.X + (b.X - a.X) * t);
        set_result->Y = (a.Y + (b.Y - a.Y) * t);
    }

    __forceinline bool Is_Valid() const { return GameMath::Is_Valid_Float(X) && GameMath::Is_Valid_Float(Y); }
    static __forceinline float Quick_Distance(const Vector2 &a, const Vector2 &b)
    {
        return ::Quick_Distance(a.X, a.Y, b.X, b.Y);
    }
    friend Vector2 operator*(const Vector2 &a, float k);
    friend Vector2 operator*(float k, const Vector2 &a);
    friend Vector2 operator/(const Vector2 &a, float k);
    friend Vector2 operator+(const Vector2 &a, const Vector2 &b);
    friend Vector2 operator-(const Vector2 &a, const Vector2 &b);
    friend float operator*(const Vector2 &a, const Vector2 &b);
    friend bool operator==(const Vector2 &a, const Vector2 &b);
    friend bool operator!=(const Vector2 &a, const Vector2 &b);
    friend bool Equal_Within_Epsilon(const Vector2 &a, const Vector2 &b, float epsilon);

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
    float a0k(a[0] * k);
    float a1k(a[1] * k);
    return Vector2(a0k, a1k);
}

__forceinline Vector2 operator*(float k, const Vector2 &a)
{
    float a0k(a[0] * k);
    float a1k(a[1] * k);
    return Vector2(a0k, a1k);
}

__forceinline Vector2 operator/(const Vector2 &a, float k)
{
    float ook = 1.0f / k;
    float a0ook(a[0] * ook);
    float a1ook(a[1] * ook);
    return Vector2(a0ook, a1ook);
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
    bool a0b0(a[0] == b[0]);
    bool a1b1(a[1] == b[1]);
    return a0b0 & a1b1;
}

__forceinline bool operator!=(const Vector2 &a, const Vector2 &b)
{
    bool a0b0(a[0] != b[0]);
    bool a1b1(a[1] != b[1]);
    return a0b0 | a1b1;
}

__forceinline Vector2 Normalize(const Vector2 &vec)
{
    float len2 = vec.Length2();

    if (len2 != 0.0f) {
        float oolen = GameMath::Inv_Sqrt(len2);
        return vec / oolen;
    }

    return Vector2(0.0f, 0.0f);
}

__forceinline void Swap(Vector2 &a, Vector2 &b)
{
    Vector2 tmp(a);
    a = b;
    b = tmp;
}

__forceinline float Quick_Distance(float x1, float y1, float x2, float y2)
{
    float x_diff = x1 - x2;
    float y_diff = y1 - y2;
    GameMath::Fabs(x_diff);
    GameMath::Fabs(y_diff);

    if (x_diff > y_diff) {
        return (y_diff / 2) + x_diff;
    } else {
        return (x_diff / 2) + y_diff;
    }
}

__forceinline float Distance(float x1, float y1, float x2, float y2)
{
    float x_diff = x1 - x2;
    float y_diff = y1 - y2;

    return GameMath::Sqrt((x_diff * x_diff) + (y_diff * y_diff));
}

__forceinline bool Equal_Within_Epsilon(const Vector2 &a, const Vector2 &b, float epsilon)
{
    return (GameMath::Fabs(a.X - b.X) < epsilon) && (GameMath::Fabs(a.Y - b.Y) < epsilon);
}
