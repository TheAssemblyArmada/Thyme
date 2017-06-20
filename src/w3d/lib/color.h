////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: COLOR.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Color objects.
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
#pragma once

#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

// Pack individual bytes into a dword
inline uint32_t Make_Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return b | (g << 8) | (r << 16) | (a << 24);
}

class RGBColor
{
public:
    float red;
    float green;
    float blue;
};

class RGBAColorReal
{
public:
    float red;
    float green;
    float blue;
    float alpha;
};

class RGBAColorInt
{
public:
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t alpha;
};

#endif // _COLOR_H
