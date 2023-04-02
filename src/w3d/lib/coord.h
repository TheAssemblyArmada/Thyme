/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Coordinate objects.
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

using GameMath::Sqrt;

class Coord2D
{
public:
    float Length() const { return Sqrt(x * x + y * y); }

    void Normalize()
    {
        float len = Length();

        if (len != 0.0f) {
            x /= len;
            y /= len;
        }
    }

    float To_Angle() const
    {
        float len = Length();

        if (len == 0.0f) {
            return 0.0f;
        }

        float f = x / len;

        if (f >= -1.0f) {
            if (f > 1.0f)
                f = 1.0f;
        } else {
            f = -1.0f;
        }

        if (y >= 0.0f) {
            return GameMath::Acos(f);
        } else {
            return -GameMath::Acos(f);
        }
    }

    Coord2D &operator+=(const Coord2D &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Coord2D &operator-=(const Coord2D &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;

        return *this;
    }

    Coord2D &operator/=(const float &rhs)
    {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    Coord2D &operator*=(const float &rhs)
    {
        x *= rhs;
        y *= rhs;

        return *this;
    }

    bool operator==(const Coord2D &that) const { return x == that.x && y == that.y; }
    bool operator!=(const Coord2D &that) const { return x != that.x && y != that.y; }

    float x;
    float y;
};

inline Coord2D operator*(const Coord2D &a, float k)
{
    Coord2D ret = a;
    ret *= k;
    return ret;
}

inline Coord2D operator*(float k, const Coord2D &a)
{
    Coord2D ret = a;
    ret *= k;
    return ret;
}

inline Coord2D operator/(const Coord2D &a, float k)
{
    Coord2D ret = a;
    ret /= k;
    return ret;
}

inline Coord2D operator+(const Coord2D &a, const Coord2D &b)
{
    Coord2D ret = a;
    ret += b;
    return ret;
}

inline Coord2D operator-(const Coord2D &a, const Coord2D &b)
{
    Coord2D ret = a;
    ret -= b;
    return ret;
}

class Coord3D
{
public:
    float Length() const { return Sqrt(x * x + y * y + z * z); }
    float Length2() const { return x * x + y * y + z * z; }

    static void Cross_Product(const Coord3D *a, const Coord3D *b, Coord3D *set_result)
    {
        set_result->x = a->y * b->z - a->z * b->y;
        set_result->y = a->z * b->x - a->x * b->z;
        set_result->z = a->x * b->y - a->y * b->x;
    }

    void Set(float set_x, float set_y, float set_z)
    {
        x = set_x;
        y = set_y;
        z = set_z;
    }

    void Set(const Coord3D *c)
    {
        x = c->x;
        y = c->y;
        z = c->z;
    }

    void Add(const Coord3D *c)
    {
        x += c->x;
        y += c->y;
        z += c->z;
    }

    void Sub(const Coord3D *c)
    {
        x -= c->x;
        y -= c->y;
        z -= c->z;
    }

    void Scale(float scale)
    {
        x *= scale;
        y *= scale;
        z *= scale;
    }

    void Zero()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    void Normalize()
    {
        float len = Length();

        if (len != 0.0f) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    Coord3D &operator+=(const Coord3D &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;

        return *this;
    }

    Coord3D &operator-=(const Coord3D &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;

        return *this;
    }

    Coord3D &operator/=(const float &rhs)
    {
        x /= rhs;
        y /= rhs;
        z /= rhs;

        return *this;
    }

    Coord3D &operator*=(const float &rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;

        return *this;
    }

    bool operator==(const Coord3D &that) const { return x == that.x && y == that.y && z == that.z; }
    bool operator!=(const Coord3D &that) const { return x != that.x && y != that.y && z != that.z; }

    float x;
    float y;
    float z;
};

inline Coord3D operator*(const Coord3D &a, float k)
{
    Coord3D ret = a;
    ret *= k;
    return ret;
}

inline Coord3D operator*(float k, const Coord3D &a)
{
    Coord3D ret = a;
    ret *= k;
    return ret;
}

inline Coord3D operator/(const Coord3D &a, float k)
{
    Coord3D ret = a;
    ret /= k;
    return ret;
}

inline Coord3D operator+(const Coord3D &a, const Coord3D &b)
{
    Coord3D ret = a;
    ret += b;
    return ret;
}

inline Coord3D operator-(const Coord3D &a, const Coord3D &b)
{
    Coord3D ret = a;
    ret -= b;
    return ret;
}

class ICoord2D
{
public:
    // ICoord2D::Length may exist, haven't found an instance yet.

    int32_t x;
    int32_t y;
};

class ICoord3D
{
public:
    void Zero()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    // ICoord3D::Length may exist, haven't found an instance yet.

    int32_t x;
    int32_t y;
    int32_t z;
};

class Region2D
{
public:
    float Height() const { return hi.y - lo.y; }
    float Width() const { return hi.x - lo.x; }

    Coord2D lo;
    Coord2D hi;
};

class IRegion2D
{
public:
    int Height() const { return hi.y - lo.y; }
    int Width() const { return hi.x - lo.x; }

    ICoord2D lo;
    ICoord2D hi;
};

class Region3D
{
public:
    void Zero()
    {
        lo.Zero();
        hi.Zero();
    }

    float Height() const { return hi.y - lo.y; }
    float Width() const { return hi.x - lo.x; }
    float Depth() const { return hi.z - lo.z; }
    bool Is_In_Region_No_Z(const Coord3D *c) const { return lo.x < c->x && c->x < hi.x && lo.y < c->y && c->y < hi.y; }

    bool Is_In_Region_With_Z(const Coord3D *c) const
    {
        return lo.x < c->x && c->x < hi.x && lo.y < c->y && c->y < hi.y && lo.z < c->z && c->z < hi.z;
    }

    Coord3D lo;
    Coord3D hi;
};

class IRegion3D
{
public:
    ICoord3D lo;
    ICoord3D hi;
};
