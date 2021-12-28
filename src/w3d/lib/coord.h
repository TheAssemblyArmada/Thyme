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
    Coord2D() : x(0.0f), y(0.0f) {}
    Coord2D(float x_val, float y_val) : x(x_val), y(y_val) {}
    float Length() { return float(Sqrt(float(float(x * x) + float(y * y)))); }

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

    Coord2D &operator/=(const Coord2D &rhs)
    {
        x /= rhs.x;
        y /= rhs.y;

        return *this;
    }

    Coord2D &operator*=(const Coord2D &rhs)
    {
        x *= rhs.x;
        y *= rhs.y;

        return *this;
    }

    friend Coord2D operator+(Coord2D lhs, const Coord2D &rhs)
    {
        lhs += rhs;

        return lhs;
    }

    friend Coord2D operator-(Coord2D lhs, const Coord2D &rhs)
    {
        lhs -= rhs;

        return lhs;
    }

    friend Coord2D operator/(Coord2D lhs, const Coord2D &rhs)
    {
        lhs /= rhs;

        return lhs;
    }

    friend Coord2D operator*(Coord2D lhs, const Coord2D &rhs)
    {
        lhs *= rhs;

        return lhs;
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

public:
    float x;
    float y;
};

class Coord3D
{
public:
    Coord3D() : x(0.0f), y(0.0f), z(0.0f) {}
    Coord3D(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val) {}
    float Length() { return float(Sqrt(float(float(float(x * x) + float(y * y)) + float(z * z)))); }
    static void Cross_Product(const Coord3D *a, const Coord3D *b, Coord3D *set_result)
    {
        set_result->x = a->y * b->z - a->z * b->y;
        set_result->y = a->z * b->x - a->x * b->z;
        set_result->z = a->x * b->y - a->y * b->x;
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

    Coord3D &operator/=(const Coord3D &rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;

        return *this;
    }

    Coord3D &operator*=(const Coord3D &rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;

        return *this;
    }

    friend Coord3D operator+(Coord3D lhs, const Coord3D &rhs)
    {
        lhs += rhs;

        return lhs;
    }

    friend Coord3D operator-(Coord3D lhs, const Coord3D &rhs)
    {
        lhs -= rhs;

        return lhs;
    }

    friend Coord3D operator/(Coord3D lhs, const Coord3D &rhs)
    {
        lhs /= rhs;

        return lhs;
    }

    friend Coord3D operator*(Coord3D lhs, const Coord3D &rhs)
    {
        lhs *= rhs;

        return lhs;
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

    bool operator==(const Coord3D &that) { return x == that.x && y == that.y && z == that.z; }

public:
    float x;
    float y;
    float z;
};

class ICoord2D
{
public:
    int32_t x;
    int32_t y;
};

class ICoord3D
{
public:
    int32_t x;
    int32_t y;
    int32_t z;
};

class Region2D
{
public:
    Coord2D lo;
    Coord2D hi;
};

class IRegion2D
{
public:
    ICoord2D lo;
    ICoord2D hi;
};

class Region3D
{
public:
    Coord3D lo;
    Coord3D hi;
};

class IRegion3D
{
public:
    ICoord3D lo;
    ICoord3D hi;
};
