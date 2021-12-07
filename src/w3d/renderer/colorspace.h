/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Convert between RGB and HSV color spaces.
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
#include "vector3.h"
#include "vector4.h"

void RGB_To_HSV(Vector3 &hsv, const Vector3 &rgb);
void HSV_To_RGB(Vector3 &rgb, const Vector3 &hsv);

void RGBA_To_HSV(Vector3 &hsv, const Vector4 &rgba);
void HSV_To_RGBA(Vector4 &rgba, const Vector3 &hsv);

void Adjust_RGBA(Vector4 &rgba, const Vector3 &hsv_adj);

/**
 * @brief Utility function to pack separate bytes into an RGBA pixel.
 */
inline constexpr uint32_t Make_Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return b | (g << 8) | (r << 16) | (a << 24);
}

inline void Color_To_RGBA(Vector4 &rgba, uint32_t color)
{
    rgba.X = ((color & 0xFF0000) >> 16) / 255.0f;
    rgba.Y = ((color & 0xFF00) >> 8) / 255.0f;
    rgba.Z = (color & 0xFF) / 255.0f;
    rgba.W = ((color & 0xFF000000) >> 24) / 255.0f;
}

inline void Color_To_RGB(Vector3 &rgb, uint32_t color)
{
    rgb.X = ((color & 0xFF0000) >> 16) / 255.0f;
    rgb.Y = ((color & 0xFF00) >> 8) / 255.0f;
    rgb.Z = (color & 0xFF) / 255.0f;
}

inline void RGBA_To_Color(uint32_t &color, const Vector4 &rgba)
{
    color = Make_Color(rgba.X * 255.0f, rgba.Y * 255.0f, rgba.Z * 255.0f, rgba.W * 255.0f);
}

inline void Bytes_To_RGBA(Vector4 &rgba, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    rgba.X = r / 255.0f;
    rgba.Y = g / 255.0f;
    rgba.Z = b / 255.0f;
    rgba.W = a / 255.0f;
}

inline void RGBA_To_Bytes(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a, const Vector4 &rgba)
{
    r = rgba.X * 255.0f;
    g = rgba.Y * 255.0f;
    b = rgba.Z * 255.0f;
    a = rgba.W * 255.0f;
}

inline void Recolor(uint32_t &color, const Vector3 &adj)
{
    Vector4 rgba;
    Color_To_RGBA(rgba, color);
    Adjust_RGBA(rgba, adj);
    RGBA_To_Color(color, rgba);
}
