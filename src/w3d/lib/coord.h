////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: COORD.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Coordinate objects.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _COORD_H_
#define _COORD_H_

#include <cstdint>
#include <math.h>

class Coord2D
{
    public:
        float x;
        float y;

        float Length() { return (float)sqrtf(x * x + y * y); }
};

class Coord3D
{
    public:
        float x;
        float y;
        float z;

        float Length() { return (float)sqrtf(x * x + y * y + z * z); }
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

#endif // _COORD_H_
