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
#include "w3dformat.h"
#include "rtsutils.h"

using rts::FourCC;

#ifndef THYME_STANDALONE
WW3DFormat *g_D3DFormatToWW3DFormatConversionArray = Make_Pointer<WW3DFormat>(0x00A5243C);
#else
WW3DFormat g_D3DFormatToWW3DFormatConversionArray[62];
#endif

WW3DFormat D3DFormat_To_WW3DFormat(uint32_t format)
{
    switch (format) {
        case FourCC<'D', 'X', 'T', '1'>::value:
            return WW3D_FORMAT_DXT1;
        case FourCC<'D', 'X', 'T', '2'>::value:
            return WW3D_FORMAT_DXT2;
        case FourCC<'D', 'X', 'T', '3'>::value:
            return WW3D_FORMAT_DXT3;
        case FourCC<'D', 'X', 'T', '4'>::value:
            return WW3D_FORMAT_DXT4;
        case FourCC<'D', 'X', 'T', '5'>::value:
            return WW3D_FORMAT_DXT5;
        default:
            break;
    }

    if (format < 63) {
        return g_D3DFormatToWW3DFormatConversionArray[format];
    }

    return WW3D_FORMAT_UNKNOWN;
}

unsigned Get_Bytes_Per_Pixel(WW3DFormat format)
{
    switch (format) {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8: // fallthrough
        case WW3D_FORMAT_X8L8V8U8: // fallthrough
            return 4;
        case WW3D_FORMAT_R8G8B8:
            return 3;
        case WW3D_FORMAT_R5G6B5:
        case WW3D_FORMAT_A1R5G5B5: // fallthrough
        case WW3D_FORMAT_A4R4G4B4: // fallthrough
        case WW3D_FORMAT_U8V8: // fallthrough
        case WW3D_FORMAT_L6V5U5: // fallthrough
            return 2;
        case WW3D_FORMAT_R3G3B2:
        case WW3D_FORMAT_A8: // fallthrough
        case WW3D_FORMAT_P8: // fallthrough
        case WW3D_FORMAT_L8: // fallthrough
            return 1;
        default:
            break;
    }

    return 0;
}
