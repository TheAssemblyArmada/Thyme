/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Objects for containing color values.
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

class RGBColor
{
public:
    float red;
    float green;
    float blue;
    int Get_As_Int()
    {
        return ((unsigned int)(int)(green * 255.0f) << 8) | ((unsigned int)(int)(red * 255.0f) << 16) | (int)(blue * 255.0f);
    }
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

struct RGBColorKeyframe
{
    RGBColor color;
    uint32_t frame;
};
