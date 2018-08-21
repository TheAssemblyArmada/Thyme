/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief W3D format info.
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

enum WW3DFormat : int32_t
{
    WW3D_FORMAT_UNKNOWN = 0x0,
    WW3D_FORMAT_R8G8B8 = 0x1,
    WW3D_FORMAT_A8R8G8B8 = 0x2,
    WW3D_FORMAT_X8R8G8B8 = 0x3,
    WW3D_FORMAT_R5G6B5 = 0x4,
    WW3D_FORMAT_X1R5G5B5 = 0x5,
    WW3D_FORMAT_A1R5G5B5 = 0x6,
    WW3D_FORMAT_A4R4G4B4 = 0x7,
    WW3D_FORMAT_R3G3B2 = 0x8,
    WW3D_FORMAT_A8 = 0x9,
    WW3D_FORMAT_A8R3G3B2 = 0xA,
    WW3D_FORMAT_X4R4G4B4 = 0xB,
    WW3D_FORMAT_A8P8 = 0xC,
    WW3D_FORMAT_P8 = 0xD,
    WW3D_FORMAT_L8 = 0xE,
    WW3D_FORMAT_A8L8 = 0xF,
    WW3D_FORMAT_A4L4 = 0x10,
    WW3D_FORMAT_U8V8 = 0x11,
    WW3D_FORMAT_L6V5U5 = 0x12,
    WW3D_FORMAT_X8L8V8U8 = 0x13,
    WW3D_FORMAT_DXT1 = 0x14,
    WW3D_FORMAT_DXT2 = 0x15,
    WW3D_FORMAT_DXT3 = 0x16,
    WW3D_FORMAT_DXT4 = 0x17,
    WW3D_FORMAT_DXT5 = 0x18,
    WW3D_FORMAT_COUNT = 0x19,
};

WW3DFormat D3DFormat_To_WW3DFormat(uint32_t format);
unsigned Get_Bytes_Per_Pixel(WW3DFormat format);

inline void Color_To_Format(uint8_t *dst, uint32_t color, WW3DFormat format)
{
    switch (format) {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:
            *reinterpret_cast<uint32_t *>(dst) = color;
            break;
        case WW3D_FORMAT_R8G8B8:
            *reinterpret_cast<uint32_t *>(dst) = color & 0xFFFFFF;
            break;
        case WW3D_FORMAT_A4R4G4B4:
            *reinterpret_cast<uint16_t *>(dst) =
                ((color & 0xF0) >> 4) | ((color & 0xF000) >> 8) | ((color & 0xF00000) >> 12) | ((color & 0xF0000000) >> 16);
            break;
        case WW3D_FORMAT_A1R5G5B5:
            *reinterpret_cast<uint16_t *>(dst) =
                ((color & 0xF8) >> 3) | ((color & 0xF800) >> 6) | ((color & 0xF8000000) >> 9) | ((color & 0x80000000) >> 16);
            break;
        case WW3D_FORMAT_R5G6B5:
            *reinterpret_cast<uint16_t *>(dst) =
                ((color & 0xF8) >> 3) | ((color & 0xF800) >> 6) | ((color & 0xF8000000) >> 9);
            break;
        case WW3D_FORMAT_L8:
            *dst = 4725 * (color & 0xFF) + 46885 * ((color & 0xFF00) >> 8) + 13926 * ((color & 0xFF0000) >> 16);
            break;
        case WW3D_FORMAT_A8:
            *dst = color;
            break;
        default:
            break;
    }
}

#ifndef THYME_STANDALONE
#include "hooker.h"

extern WW3DFormat *g_D3DFormatToWW3DFormatConversionArray;
#else
extern WW3DFormat g_D3DFormatToWW3DFormatConversionArray[62];
#endif
