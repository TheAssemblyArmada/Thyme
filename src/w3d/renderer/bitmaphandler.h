/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Static class for handling raw bitmap data.
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
#include "w3dformat.h"

class Vector3;

class BitmapHandlerClass
{
public:
    static void Create_Mipmap_B8G8R8A8(uint8_t *dest_surface,
        unsigned dest_surface_pitch,
        uint8_t *src_surface,
        unsigned src_surface_pitch,
        unsigned width,
        unsigned height);
    static void Copy_Image_Generate_Mipmap(unsigned width,
        unsigned height,
        uint8_t *dest_surface,
        unsigned dest_pitch,
        WW3DFormat dest_format,
        uint8_t *src_surface,
        unsigned src_pitch,
        WW3DFormat src_format,
        uint8_t *mip_surface,
        unsigned mip_pitch,
        const Vector3 &adjust);
    static void Copy_Image(uint8_t *dest_surface,
        unsigned dest_surface_width,
        unsigned dest_surface_height,
        unsigned dest_surface_pitch,
        WW3DFormat dest_surface_format,
        uint8_t *src_surface,
        unsigned src_surface_width,
        unsigned src_surface_height,
        unsigned src_surface_pitch,
        WW3DFormat src_surface_format,
        uint8_t *src_palette,
        unsigned src_palette_bpp,
        bool generate_mip_level,
        const Vector3 &adjust);

private:
    static void Read_B8G8R8A8(uint8_t *dest_surface,
        uint8_t const *src_surface,
        WW3DFormat src_surface_format,
        uint8_t const *src_palette,
        unsigned src_palete_bpp);
    static void Write_B8G8R8A8(uint8_t *dest_surface, WW3DFormat dest_surface_format, uint8_t const *src_surface);
    static void Copy_Pixel(uint8_t *dest_surface,
        WW3DFormat dest_surface_format,
        const uint8_t *src_surface,
        WW3DFormat src_surface_format,
        const uint8_t *src_palette,
        unsigned src_palette_bpp);
    static void Copy_Pixel(uint8_t *dest_surface,
        WW3DFormat dest_surface_format,
        const uint8_t *src_surface,
        WW3DFormat src_surface_format,
        const uint8_t *src_palette,
        unsigned src_palette_bpp,
        const Vector3 &adjust);
};
