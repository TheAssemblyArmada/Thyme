/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Color
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "color.h"
#include "colorspace.h"

void Get_Color_Components(int color, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha)
{
    *alpha = ((color & 0xFF000000) >> 24);
    *red = ((color & 0xFF0000) >> 16);
    *green = ((color & 0xFF00) >> 8);
    *blue = (unsigned char)color;
}

void Get_Color_Components_Real(int color, float *red, float *green, float *blue, float *alpha)
{
    *alpha = (float)((color & 0xFF000000) >> 24) / 255.0f;
    *red = (float)((color & 0xFF0000) >> 16) / 255.0f;
    *green = (float)((color & 0xFF00) >> 8) / 255.0f;
    *blue = (float)(unsigned char)color / 255.0f;
}

int Darken_Color(int color, int percent)
{
    if (percent >= 90 || percent <= 0) {
        return color;
    }

    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    Get_Color_Components(color, &red, &green, &blue, &alpha);
    return Make_Color(red - (percent * red / 100), green - (percent * green / 100), blue - (percent * blue / 100), alpha);
}
