/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
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

class StringClass;
class TargaImage;

enum WW3DFormat : int32_t
{
    WW3D_FORMAT_UNKNOWN,
    WW3D_FORMAT_R8G8B8,
    WW3D_FORMAT_A8R8G8B8,
    WW3D_FORMAT_X8R8G8B8,
    WW3D_FORMAT_R5G6B5,
    WW3D_FORMAT_X1R5G5B5,
    WW3D_FORMAT_A1R5G5B5,
    WW3D_FORMAT_A4R4G4B4,
    WW3D_FORMAT_R3G3B2,
    WW3D_FORMAT_A8,
    WW3D_FORMAT_A8R3G3B2,
    WW3D_FORMAT_X4R4G4B4,
    WW3D_FORMAT_A8P8,
    WW3D_FORMAT_P8,
    WW3D_FORMAT_L8,
    WW3D_FORMAT_A8L8,
    WW3D_FORMAT_A4L4,
    WW3D_FORMAT_U8V8,
    WW3D_FORMAT_L6V5U5,
    WW3D_FORMAT_X8L8V8U8,
    WW3D_FORMAT_DXT1,
    WW3D_FORMAT_DXT2,
    WW3D_FORMAT_DXT3,
    WW3D_FORMAT_DXT4,
    WW3D_FORMAT_DXT5,
    WW3D_FORMAT_COUNT,
};

DEFINE_ENUMERATION_OPERATORS(WW3DFormat);

enum WW3DZFormat : int32_t
{
    WW3DZ_FORMAT_UNKNOWN,
    WW3DZ_FORMAT_D16_LOCKABLE,
    WW3DZ_FORMAT_D32,
    WW3DZ_FORMAT_D15S1,
    WW3DZ_FORMAT_D24S8,
    WW3DZ_FORMAT_D16,
    WW3DZ_FORMAT_D24X8,
    WW3DZ_FORMAT_D24X4S4,
    WW3DZ_FORMAT_COUNT,
};

DEFINE_ENUMERATION_OPERATORS(WW3DZFormat);

void Init_D3D_To_WW3_Conversion();

uint32_t WW3DFormat_To_D3DFormat(WW3DFormat format);
WW3DFormat D3DFormat_To_WW3DFormat(uint32_t format);
void Get_WW3D_Format_Name(WW3DFormat format, StringClass &name);

uint32_t WW3DZFormat_To_D3DFormat(WW3DZFormat format);
WW3DZFormat D3DFormat_To_WW3DZFormat(uint32_t format);
void Get_WW3DZ_Format_Name(WW3DZFormat format, StringClass &name);

unsigned Get_Bytes_Per_Pixel(WW3DFormat format);
WW3DFormat Get_Valid_Texture_Format(WW3DFormat format, bool allow_compression);

void Get_WW3D_Format(WW3DFormat &format, unsigned &bpp, const TargaImage &tga);
void Get_WW3D_Format(WW3DFormat &dest_format, WW3DFormat &src_format, unsigned &bpp, const TargaImage &tga);

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

/**
 * Determine if a surface format supports alpha.
 */
inline bool Has_Alpha(WW3DFormat format)
{
    switch (format) {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_A8:
        case WW3D_FORMAT_A8R3G3B2:
        case WW3D_FORMAT_A8P8:
        case WW3D_FORMAT_A8L8:
        case WW3D_FORMAT_A4R4G4B4:
        case WW3D_FORMAT_A4L4:
        case WW3D_FORMAT_DXT3:
        case WW3D_FORMAT_DXT4:
        case WW3D_FORMAT_DXT5:
        case WW3D_FORMAT_A1R5G5B5:
        case WW3D_FORMAT_DXT2:
            return true;
        default:
            break;
    }

    return false;
}

/**
 * Determine the number of bits used for the alpha mask.
 */
inline uint8_t Alpha_Bits(WW3DFormat format)
{
    switch (format) {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_A8:
        case WW3D_FORMAT_A8R3G3B2:
        case WW3D_FORMAT_A8P8:
        case WW3D_FORMAT_A8L8:
            return 8;
        case WW3D_FORMAT_A4R4G4B4:
        case WW3D_FORMAT_A4L4:
        case WW3D_FORMAT_DXT3:
        case WW3D_FORMAT_DXT4:
        case WW3D_FORMAT_DXT5:
            return 4;
        case WW3D_FORMAT_A1R5G5B5:
        case WW3D_FORMAT_DXT2:
            return 1;
        default:
            break;
    }

    return 0;
}

/**
 * Determine if the format is a compressed format.
 */
inline bool Is_Compressed(WW3DFormat format)
{
    switch (format) {
        // All cases fall through.
        case WW3D_FORMAT_DXT1:
        case WW3D_FORMAT_DXT2:
        case WW3D_FORMAT_DXT3:
        case WW3D_FORMAT_DXT4:
        case WW3D_FORMAT_DXT5:
            return true;
        default:
            break;
    }

    return false;
}

#ifdef GAME_DLL
#include "hooker.h"
extern ARRAY_DEC(WW3DFormat, g_D3DFormatToWW3DFormatConversionArray, 63);
extern ARRAY_DEC(WW3DZFormat, g_D3DFormatToWW3DZFormatConversionArray, 80);
#else
extern WW3DFormat g_D3DFormatToWW3DFormatConversionArray[63];
extern WW3DZFormat g_D3DFormatToWW3DZFormatConversionArray[80];
#endif
