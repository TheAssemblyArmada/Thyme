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
#include "colorspace.h"
#include "gamemath.h"
#include <algorithm>

/**
 * @brief Converts the RGB values to HSV, the alpha value is discarded.
 */
void RGBA_To_HSV(Vector3 &hsv, const Vector4 &rgba)
{
    Vector3 rgb;
    rgb.X = rgba.X;
    rgb.Y = rgba.Y;
    rgb.Z = rgba.Z;
    RGB_To_HSV(hsv, rgb);
}

/**
 * @brief Converts the HSV values to RGB, the alpha value is not altered.
 */
void HSV_To_RGBA(Vector4 &rgba, const Vector3 &hsv)
{
    Vector3 rgb;
    HSV_To_RGB(rgb, hsv);
    rgba.X = rgb.X;
    rgba.Y = rgb.Y;
    rgba.Z = rgb.Z;
}

/**
 * @brief Converts the RGB values to HSV.
 */
void RGB_To_HSV(Vector3 &hsv, const Vector3 &rgb)
{
    float min = std::min(rgb.X, std::min(rgb.Y, rgb.Z));
    float max = std::max(rgb.X, std::max(rgb.Y, rgb.Z));
    float delta = max - min;
    hsv.Z = max; // V

    if (max != 0.0f) {
        hsv.Y = delta / max; // S
    } else {
        hsv.Y = 0.0f;
        hsv.X = -1.0f;
        return;
    }

    // H
    if (rgb.X == max) {
        hsv.X = (rgb.Y - rgb.Z) / delta;
    } else if (rgb.Y == max) {
        hsv.X = 2.0f + (rgb.Z - rgb.X) / delta;
    } else {
        hsv.X = 4.0f + (rgb.X - rgb.Y) / delta;
    }

    hsv.X *= 60.0f;

    if (hsv.X < 0.0f) {
        hsv.X += 360.0f;
    }
}

/**
 * @brief Converts the HSV values to RGB.
 */
void HSV_To_RGB(Vector3 &rgb, const Vector3 &hsv)
{
    if (hsv.Y == 0.0f) {
        // Grey scale
        rgb.X = rgb.Y = rgb.Z = hsv.Z;

        return;
    }

    float sector = hsv.X / 60.0f;
    int i = GameMath::Floor(sector);
    float f = sector - i;
    float p = hsv.Z * (1.0f - hsv.Y);
    float q = hsv.Z * (1.0f - (hsv.Y * f));
    float t = hsv.Z * (1.0f - (hsv.Y * (1.0f - f)));

    switch (i) {
        case 0:
            rgb.X = hsv.Z;
            rgb.Y = t;
            rgb.Z = p;
            break;
        case 1:
            rgb.X = q;
            rgb.Y = hsv.Z;
            rgb.Z = p;
            break;
        case 2:
            rgb.X = p;
            rgb.Y = hsv.Z;
            rgb.Z = t;
            break;
        case 3:
            rgb.X = p;
            rgb.Y = q;
            rgb.Z = hsv.Z;
            break;
        case 4:
            rgb.X = t;
            rgb.Y = p;
            rgb.Z = hsv.Z;
            break;
        case 5: // Fallthrough
        default:
            rgb.X = hsv.Z;
            rgb.Y = p;
            rgb.Z = q;
            break;
    }
}

/**
 * @brief Applies a HSV color shift to an RGBA value, alpha is untouched.
 */
void Adjust_RGBA(Vector4 &rgba, const Vector3 &hsv_adj)
{
    Vector3 hsv;
    RGBA_To_HSV(hsv, rgba);

    if (hsv.X > 0.0f) {
        hsv += hsv_adj;
    } else {
        hsv.Z += hsv_adj.Z;
    }

    if (hsv.X < 0.0f) {
        hsv.X += 360.f;
    }

    if (hsv.X > 360.0f) {
        hsv.X -= 360.0f;
    }

    hsv.Y = std::clamp(hsv.Y, 0.0f, 1.0f);
    hsv.Z = std::clamp(hsv.Z, 0.0f, 1.0f);
    HSV_To_RGBA(rgba, hsv);
}
