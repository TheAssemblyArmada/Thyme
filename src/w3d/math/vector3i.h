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

class Vector3i
{
public:
    Vector3i() {}

    Vector3i(int i, int j, int k)
    {
        I = i;
        J = j;
        K = k;
    }

    bool operator==(const Vector3i &v) const { return I == v.I && J == v.J && K == v.K; }
    bool operator!=(const Vector3i &v) const { return !(I == v.I && J == v.J && K == v.K); }
    const int &operator[](int n) const { return ((int *)this)[n]; }
    int &operator[](int n) { return ((int *)this)[n]; }

    int32_t I;
    int32_t J;
    int32_t K;
};

class Vector3i16
{
public:
    Vector3i16() {}

    Vector3i16(unsigned short i, unsigned short j, unsigned short k)
    {
        I = i;
        J = j;
        K = k;
    }

    bool operator==(const Vector3i16 &v) const { return I == v.I && J == v.J && K == v.K; }
    bool operator!=(const Vector3i16 &v) const { return !(I == v.I && J == v.J && K == v.K); }
    const unsigned short &operator[](int n) const { return ((unsigned short *)this)[n]; }
    unsigned short &operator[](int n) { return ((unsigned short *)this)[n]; }

    uint16_t I;
    uint16_t J;
    uint16_t K;
};
