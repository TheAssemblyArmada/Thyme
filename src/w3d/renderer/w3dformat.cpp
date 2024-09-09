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
#include "w3dformat.h"
#include "dx8wrapper.h"
#include "rtsutilsw3d.h"
#include "targa.h"
#include "w3d.h"
#include "wwstring.h"
#include <captainslog.h>
#include <cstring>

#ifdef BUILD_WITH_D3D8
#include <d3d8.h>
#endif

#ifdef GAME_DLL
#include "hooker.h"
#endif

using rts::FourCC;
using std::memcpy;
using std::memset;

#ifndef GAME_DLL
WW3DFormat g_D3DFormatToWW3DFormatConversionArray[63];
WW3DZFormat g_D3DFormatToWW3DZFormatConversionArray[80];
#endif

void Init_D3D_To_WW3_Conversion()
{
#ifdef BUILD_WITH_D3D8
    memset(g_D3DFormatToWW3DFormatConversionArray, 0, sizeof(g_D3DFormatToWW3DFormatConversionArray));
    memset(g_D3DFormatToWW3DZFormatConversionArray, 0, sizeof(g_D3DFormatToWW3DZFormatConversionArray));
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_UNKNOWN] = WW3D_FORMAT_X1R5G5B5;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_R8G8B8] = WW3D_FORMAT_R8G8B8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A8R8G8B8] = WW3D_FORMAT_A8R8G8B8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_X8R8G8B8] = WW3D_FORMAT_X8R8G8B8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_R5G6B5] = WW3D_FORMAT_R5G6B5;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_X1R5G5B5] = WW3D_FORMAT_X1R5G5B5;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A1R5G5B5] = WW3D_FORMAT_A1R5G5B5;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A4R4G4B4] = WW3D_FORMAT_A4R4G4B4;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_R3G3B2] = WW3D_FORMAT_R3G3B2;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A8] = WW3D_FORMAT_A8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A8R3G3B2] = WW3D_FORMAT_A8R3G3B2;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_X4R4G4B4] = WW3D_FORMAT_X4R4G4B4;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A8P8] = WW3D_FORMAT_A8P8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_P8] = WW3D_FORMAT_P8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_L8] = WW3D_FORMAT_L8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A8L8] = WW3D_FORMAT_A8L8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_A4L4] = WW3D_FORMAT_A4L4;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_V8U8] = WW3D_FORMAT_U8V8;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_L6V5U5] = WW3D_FORMAT_L6V5U5;
    g_D3DFormatToWW3DFormatConversionArray[D3DFMT_X8L8V8U8] = WW3D_FORMAT_X8L8V8U8;
    g_D3DFormatToWW3DZFormatConversionArray[D3DFMT_D16_LOCKABLE] = WW3DZ_FORMAT_D16_LOCKABLE;
    g_D3DFormatToWW3DZFormatConversionArray[D3DFMT_D32] = WW3DZ_FORMAT_D32;
    g_D3DFormatToWW3DZFormatConversionArray[D3DFMT_D15S1] = WW3DZ_FORMAT_D15S1;
    g_D3DFormatToWW3DZFormatConversionArray[D3DFMT_D24S8] = WW3DZ_FORMAT_D24S8;
    g_D3DFormatToWW3DZFormatConversionArray[D3DFMT_D24X8] = WW3DZ_FORMAT_D24X8;
    g_D3DFormatToWW3DZFormatConversionArray[D3DFMT_D24X4S4] = WW3DZ_FORMAT_D24X4S4;
#endif
}

uint32_t WW3DFormat_To_D3DFormat(WW3DFormat format)
{
    static uint32_t _conversion_array[WW3D_FORMAT_COUNT] = { 0,
        20,
        21,
        22,
        23,
        24,
        25,
        26,
        27,
        28,
        29,
        30,
        40,
        41,
        50,
        51,
        52,
        60,
        61,
        62,
        FourCC<'D', 'X', 'T', '1'>::value,
        FourCC<'D', 'X', 'T', '2'>::value,
        FourCC<'D', 'X', 'T', '3'>::value,
        FourCC<'D', 'X', 'T', '4'>::value,
        FourCC<'D', 'X', 'T', '5'>::value };

    if (format < WW3D_FORMAT_COUNT) {
        return _conversion_array[format];
    }

    return 0;
}

WW3DFormat D3DFormat_To_WW3DFormat(uint32_t format)
{
    char buff[sizeof(format) + 1];

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

    // Log if the engine tries to load a DDS format we don't currently handle.
    memcpy(buff, &format, sizeof(format));
    buff[sizeof(format)] = '\0';
    captainslog_error("TODO Unhandled format FourCC '%s', implementations welcome.", buff);

    return WW3D_FORMAT_UNKNOWN;
}

void Get_WW3D_Format_Name(WW3DFormat format, StringClass &name)
{
    switch (format) {
        case WW3D_FORMAT_R8G8B8:
            name = "R8G8B8";
            break;
        case WW3D_FORMAT_A8R8G8B8:
            name = "A8R8G8B8";
            break;
        case WW3D_FORMAT_X8R8G8B8:
            name = "X8R8G8B8";
            break;
        case WW3D_FORMAT_R5G6B5:
            name = "R5G6B5";
            break;
        case WW3D_FORMAT_X1R5G5B5:
            name = "X1R5G5B5";
            break;
        case WW3D_FORMAT_A1R5G5B5:
            name = "A1R5G5B5";
            break;
        case WW3D_FORMAT_A4R4G4B4:
            name = "A4R4G4B4";
            break;
        case WW3D_FORMAT_R3G3B2:
            name = "R3G3B2";
            break;
        case WW3D_FORMAT_A8:
            name = "A8";
            break;
        case WW3D_FORMAT_A8R3G3B2:
            name = "A8R3G3B2";
            break;
        case WW3D_FORMAT_X4R4G4B4:
            name = "X4R4G4B4";
            break;
        case WW3D_FORMAT_A8P8:
            name = "A8P8";
            break;
        case WW3D_FORMAT_P8:
            name = "P8";
            break;
        case WW3D_FORMAT_L8:
            name = "L8";
            break;
        case WW3D_FORMAT_A8L8:
            name = "A8L8";
            break;
        case WW3D_FORMAT_A4L4:
            name = "A4L4";
            break;
        case WW3D_FORMAT_U8V8:
            name = "U8V8";
            break;
        case WW3D_FORMAT_L6V5U5:
            name = "L6V5U5";
            break;
        case WW3D_FORMAT_X8L8V8U8:
            name = "X8L8V8U8";
            break;
        case WW3D_FORMAT_DXT1:
            name = "DXT1";
            break;
        case WW3D_FORMAT_DXT2:
            name = "DXT2";
            break;
        case WW3D_FORMAT_DXT3:
            name = "DXT3";
            break;
        case WW3D_FORMAT_DXT4:
            name = "DXT4";
            break;
        case WW3D_FORMAT_DXT5:
            name = "DXT5";
            break;
        default:
            name = "Unknown";
            break;
    };
}

uint32_t WW3DZFormat_To_D3DFormat(WW3DZFormat format)
{
    uint32_t _conversion_array[WW3DZ_FORMAT_COUNT] = { 0, 70, 71, 73, 75, 80, 77, 79 };

    if (format < WW3DZ_FORMAT_COUNT) {
        return _conversion_array[format];
    }

    return 0;
}

WW3DZFormat D3DFormat_To_WW3DZFormat(uint32_t format)
{
    return WW3DZFormat();
}

void Get_WW3DZ_Format_Name(WW3DZFormat format, StringClass &name)
{
    switch (format) {
        case WW3DZ_FORMAT_D16_LOCKABLE:
            name = "D16Lockable";
            break;
        case WW3DZ_FORMAT_D32:
            name = "D32";
            break;
        case WW3DZ_FORMAT_D15S1:
            name = "D15S1";
            break;
        case WW3DZ_FORMAT_D24S8:
            name = "D24S8";
            break;
        case WW3DZ_FORMAT_D16:
            name = "D16";
            break;
        case WW3DZ_FORMAT_D24X8:
            name = "D24X8";
            break;
        case WW3DZ_FORMAT_D24X4S4:
            name = "D24X4S4";
            break;
        default:
            name = "Unknown";
            break;
    };
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

WW3DFormat Get_Valid_Texture_Format(WW3DFormat format, bool allow_compression)
{
    WW3DFormat check_format = format;

    if (!DX8Wrapper::Get_Current_Caps()->Supports_DXTC() || !allow_compression) {
        if (format == WW3D_FORMAT_DXT1) {
            check_format = WW3D_FORMAT_X8R8G8B8;
        }

        if (format > WW3D_FORMAT_DXT1 && format <= WW3D_FORMAT_DXT5) {
            check_format = WW3D_FORMAT_A8R8G8B8;
        }

        if (check_format == WW3D_FORMAT_R8G8B8) {
            check_format = WW3D_FORMAT_X8R8G8B8;
        }
    } else {
        if (format != WW3D_FORMAT_DXT1) {
            if (format > WW3D_FORMAT_DXT1 && format <= WW3D_FORMAT_DXT5
                && !DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(format)) {
                check_format = WW3D_FORMAT_A8R8G8B8;
            } else if (check_format == WW3D_FORMAT_R8G8B8) {
                check_format = WW3D_FORMAT_X8R8G8B8;
            }
        } else if (!DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_DXT1)
            && DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(WW3D_FORMAT_DXT2)) {
            check_format = WW3D_FORMAT_DXT2;
        }
    }

    int width;
    int height;
    int bit_depth;
    bool windowed;

    W3D::Get_Device_Resolution(width, height, bit_depth, windowed);

    if (W3D::Get_Texture_Bit_Depth() == 16) {
        bit_depth = 16;
    } else if (bit_depth <= 16) {
        switch (check_format) {
            case WW3D_FORMAT_R8G8B8:
                return WW3D_FORMAT_R5G6B5;
            case WW3D_FORMAT_A8R8G8B8:
                return WW3D_FORMAT_A4R4G4B4;
            case WW3D_FORMAT_X8R8G8B8:
                return WW3D_FORMAT_R5G6B5;
        }
    }

    if (!DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(check_format)) {
        check_format = WW3D_FORMAT_A8R8G8B8;
        if (!DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(check_format)) {
            check_format = WW3D_FORMAT_A4R4G4B4;
            if (!DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(check_format)) {
                check_format = WW3D_FORMAT_X8R8G8B8;
                if (!DX8Wrapper::Get_Current_Caps()->Supports_Texture_Format(check_format)) {
                    check_format = WW3D_FORMAT_R5G6B5;
                } else {
                    captainslog_dbgassert(false, "Get_Valid_Texture_Format - No valid texture format found");
                }
            }
        }
    }

    return check_format;
}

/**
 * Calculates the format based and bytes per pixel based on the Targa header info.
 *
 * 0x008202B0
 */
void Get_WW3D_Format(WW3DFormat &format, unsigned &bpp, const TargaImage &tga)
{
    switch (tga.Get_Header().pixel_depth) {
        case 8:
            bpp = 1;

            if (tga.Get_Header().cmap_type == 1) {
                format = WW3D_FORMAT_P8;
            } else {
                format = tga.Get_Header().image_type != TGA_TYPE_GREY ? WW3D_FORMAT_A8 : WW3D_FORMAT_L8;
            }

            break;
        case 16:
            bpp = 2;
            format = WW3D_FORMAT_A1R5G5B5;
            break;
        case 24:
            bpp = 3;
            format = WW3D_FORMAT_R8G8B8;
            break;
        case 32:
            bpp = 4;
            format = WW3D_FORMAT_A8R8G8B8;
            break;
        default:
            break;
    }
}

void Get_WW3D_Format(WW3DFormat &dest_format, WW3DFormat &src_format, unsigned &bpp, const TargaImage &tga)
{
    Get_WW3D_Format(src_format, bpp, tga);
    dest_format = src_format;

    if (dest_format == WW3D_FORMAT_P8 || dest_format == WW3D_FORMAT_L8) {
        dest_format = WW3D_FORMAT_X8R8G8B8;
    }

    dest_format = Get_Valid_Texture_Format(dest_format, false);
}
