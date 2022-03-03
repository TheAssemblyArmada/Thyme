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

class Vector2i
{
public:
    __forceinline Vector2i();
    __forceinline Vector2i(int i, int j);
    __forceinline void Set(int i, int j);
    __forceinline void Swap(Vector2i &other);
    __forceinline bool operator==(const Vector2i &v) const;
    __forceinline bool operator!=(const Vector2i &v) const;
    __forceinline const int &operator[](int n) const;
    __forceinline int &operator[](int n);

    union
    {
        struct
        {
            int I;
            int J;
        };
        struct
        {
            int Data[2];
        };
    };
};

__forceinline Vector2i::Vector2i() {}

__forceinline Vector2i::Vector2i(int i, int j)
{
    I = i;
    J = j;
}

__forceinline bool Vector2i::operator==(const Vector2i &v) const
{
    return (I == v.I && J == v.J);
}

__forceinline bool Vector2i::operator!=(const Vector2i &v) const
{
    return !(I == v.I && J == v.J);
}

__forceinline const int &Vector2i::operator[](int n) const
{
    return Data[n];
}

__forceinline int &Vector2i::operator[](int n)
{
    return Data[n];
}

__forceinline void Vector2i::Set(int i, int j)
{
    I = i;
    J = j;
}

__forceinline void Vector2i::Swap(Vector2i &other)
{
    I ^= other.I;
    other.I ^= I;
    I ^= other.I;
    J ^= other.J;
    other.J ^= J;
    J ^= other.J;
}
