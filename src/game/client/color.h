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
    int Get_As_Int() const
    {
        return ((unsigned int)(int)(green * 255.0f) << 8) | ((unsigned int)(int)(red * 255.0f) << 16) | (int)(blue * 255.0f);
    }
    void Set_From_Int(int color)
    {
        red = (float)((color & 0xFF0000) >> 16) / 255.0f;
        green = (float)((color & 0xFF00) >> 8) / 255.0f;
        blue = (float)(unsigned char)color / 255.0f;
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

void Get_Color_Components(int color, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha);
void Get_Color_Components_Real(int color, float *red, float *green, float *blue, float *alpha);
int Darken_Color(int color, int percent);
