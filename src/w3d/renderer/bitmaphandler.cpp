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
#include "bitmaphandler.h"
#include "colorspace.h"

namespace
{

unsigned Scale_Bits(unsigned src, unsigned src_bits, unsigned dest_bits)
{
    return ((src) & ((1 << (src_bits)) - 1)) * ((1 << (dest_bits)) - 1) / ((1 << (src_bits)) - 1);
}

} // namespace

/**
 * Read an ARGB formatted pixel value from a bitmap of any supported uncompressed format.
 */
void BitmapHandlerClass::Read_B8G8R8A8(uint8_t *dest_surface,
    uint8_t const *src_surface,
    WW3DFormat src_surface_format,
    uint8_t const *src_palette,
    unsigned src_palete_bpp)
{
    switch (src_surface_format) {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:
            *reinterpret_cast<uint32_t *>(dest_surface) = *reinterpret_cast<const uint32_t *>(src_surface);
            return;
        case WW3D_FORMAT_R8G8B8:
            dest_surface[0] = src_surface[0];
            dest_surface[1] = src_surface[1];
            dest_surface[2] = src_surface[2];
            dest_surface[3] = 0xFF;
            return;
        case WW3D_FORMAT_A4R4G4B4: {
            uint16_t sh2 = *reinterpret_cast<const uint16_t *>(src_surface);
            int a = Scale_Bits(sh2 >> 12, 4, 8);
            int r = Scale_Bits(sh2 >> 8, 4, 8);
            int g = Scale_Bits(sh2 >> 4, 4, 8);
            int b = Scale_Bits(sh2, 4, 8);
            *reinterpret_cast<uint32_t *>(dest_surface) = Make_Color(r, g, b, a);
            return;
        }

        case WW3D_FORMAT_R5G6B5: {
            uint16_t sh2 = *reinterpret_cast<const uint16_t *>(src_surface);
            int r = Scale_Bits(sh2 >> 11, 5, 8);
            int g = Scale_Bits(sh2 >> 5, 6, 8);
            int b = Scale_Bits(sh2, 5, 8);
            *reinterpret_cast<uint32_t *>(dest_surface) = Make_Color(r, g, b, 0xFF);
            return;
        }

        case WW3D_FORMAT_A1R5G5B5: {
            uint16_t sh2 = *reinterpret_cast<const uint16_t *>(src_surface);
            int a = Scale_Bits(sh2 >> 15, 1, 8);
            int r = Scale_Bits(sh2 >> 10, 5, 8);
            int g = Scale_Bits(sh2 >> 5, 5, 8);
            int b = Scale_Bits(sh2, 5, 8);
            *reinterpret_cast<uint32_t *>(dest_surface) = Make_Color(r, g, b, a);
            return;
        }

        case WW3D_FORMAT_R3G3B2: {
            uint8_t sh3 = *src_surface;
            int r = Scale_Bits(sh3 >> 5, 3, 8);
            int g = Scale_Bits(sh3 >> 2, 3, 8);
            int b = Scale_Bits(sh3, 2, 8);
            *reinterpret_cast<uint32_t *>(dest_surface) = Make_Color(r, g, b, 0xff);
            return;
        }

        case WW3D_FORMAT_L8: {
            int l = *src_surface;
            *reinterpret_cast<uint32_t *>(dest_surface) = Make_Color(l, l, l, 0xff);
            return;
        }

        case WW3D_FORMAT_A8: {
            *reinterpret_cast<uint32_t *>(dest_surface) = Make_Color(0, 0, 0, *src_surface);
            return;
        }

        case WW3D_FORMAT_P8: {
            if (src_palete_bpp == 3) {
                unsigned index = 3 * src_surface[0];
                dest_surface[0] = src_palette[index + 2];
                dest_surface[1] = src_palette[index + 1];
                dest_surface[2] = src_palette[index];
                dest_surface[3] = 0xFF;
            } else {
                if (src_palete_bpp <= 3 || src_palete_bpp != 4) {
                    // Bitmap_Assert()
                    return;
                } else {
                    unsigned index = 4 * src_surface[0];
                    dest_surface[0] = src_palette[index + 3];
                    dest_surface[1] = src_palette[index + 2];
                    dest_surface[2] = src_palette[index + 1];
                    dest_surface[3] = src_palette[index];
                }
            }
            return;
        }

        default:
            // Bitmap_Assert()
            return;
    }
}

/**
 * Write an ARGB formatted pixel value to a bitmap of any supported uncompressed format.
 */
void BitmapHandlerClass::Write_B8G8R8A8(uint8_t *dest_surface, WW3DFormat dest_surface_format, uint8_t const *src_surface)
{
    switch (dest_surface_format) {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:
            *reinterpret_cast<uint32_t *>(dest_surface) = *reinterpret_cast<const uint32_t *>(src_surface);
            return;
        case WW3D_FORMAT_R8G8B8:
            dest_surface[0] = src_surface[0];
            dest_surface[1] = src_surface[1];
            dest_surface[2] = src_surface[2];
            return;
        case WW3D_FORMAT_A4R4G4B4:
            *reinterpret_cast<uint16_t *>(dest_surface) = ((src_surface[0]) >> 4) | (uint8_t)(src_surface[1] & 0xF0)
                | ((src_surface[2] & 0xF0) << 4) | ((src_surface[3] & 0xF0) << 8);
            return;
        case WW3D_FORMAT_A1R5G5B5:
            *reinterpret_cast<uint16_t *>(dest_surface) = ((src_surface[0] & 0xF8) >> 3) | ((src_surface[1] & 0xF8) << 2)
                | ((src_surface[2] & 0xF8) << 7) | (src_surface[3] != 0 ? 0x8000 : 0);
            return;
        case WW3D_FORMAT_R5G6B5:
            *reinterpret_cast<uint16_t *>(dest_surface) =
                ((src_surface[0]) >> 3) | ((src_surface[1] & 0xFC) << 3) | ((src_surface[2] & 0xF8) << 8);
            return;
        case WW3D_FORMAT_R3G3B2:
            *reinterpret_cast<uint16_t *>(dest_surface) =
                ((uint8_t)(*src_surface & 0xC0) >> 6) | ((uint8_t)(src_surface[1] & 0xE0) >> 3) | src_surface[2] & 0xE0;
            return;
        case WW3D_FORMAT_L8:
            *dest_surface = (46885 * src_surface[1] + 4725 * src_surface[0] + 13926 * (uint32_t)src_surface[2]) >> 16;
            return;
        case WW3D_FORMAT_A8:
            *dest_surface = *src_surface;
            return;
        default:
            // Bitmap_Assert()
            return;
    }
}

/**
 * Create a mipmap from an ARGB surface.
 */
void BitmapHandlerClass::Create_Mipmap_B8G8R8A8(uint8_t *dest_surface,
    unsigned dest_surface_pitch,
    uint8_t *src_surface,
    unsigned src_surface_pitch,
    unsigned width,
    unsigned height)
{
    int pitch = src_surface_pitch >> 2;

    for (unsigned i = 0; i < height; i += 2) {
        uint32_t *dest = reinterpret_cast<uint32_t *>(dest_surface);
        dest_surface += dest_surface_pitch;
        const uint32_t *src = reinterpret_cast<const uint32_t *>(src_surface);
        src_surface += src_surface_pitch;

        for (unsigned j = 0; j < width; j += 2) {
            *dest++ = ((src[pitch + 1] & 0xFCFCFCFC) >> 2) + ((src[pitch] & 0xFCFCFCFC) >> 2) + ((src[1] & 0xFCFCFCFC) >> 2)
                + ((src[0] & 0xFCFCFCFC) >> 2);
            src += 2;
        }
    }
}

/**
 * Copies an image and generates a mipmap at the same time.
 *
 * 0x008202B0
 */
void BitmapHandlerClass::Copy_Image_Generate_Mipmap(unsigned width,
    unsigned height,
    uint8_t *dest_surface,
    unsigned dest_pitch,
    WW3DFormat dest_format,
    uint8_t *src_surface,
    unsigned src_pitch,
    WW3DFormat src_format,
    uint8_t *mip_surface,
    unsigned mip_pitch,
    const Vector3 &adjust)
{
    bool recolor = adjust.X != 0.0f || adjust.Y != 0.0f || adjust.Z != 0.0f;

    if (src_format != dest_format || dest_format != WW3D_FORMAT_A8R8G8B8) {
        int bpp1 = Get_Bytes_Per_Pixel(src_format);
        int bpp2 = Get_Bytes_Per_Pixel(dest_format);

        for (unsigned i = 0; i < (height >> 1); ++i) {
            uint8_t *dest = &dest_surface[2 * dest_pitch * i];
            uint8_t *src = &src_surface[2 * src_pitch * i];
            uint8_t *mip = &mip_surface[mip_pitch * i];

            for (unsigned b = 0; b < (width >> 1); b++) {
                uint8_t tmp1[4];
                uint8_t tmp2[4];
                uint8_t tmp3[4];
                uint8_t tmp4[4];
                uint8_t tmp5[4];
                Read_B8G8R8A8(tmp1, src, src_format, 0, 0);
                Write_B8G8R8A8(dest, dest_format, tmp1);
                Read_B8G8R8A8(tmp2, &src[bpp1], src_format, 0, 0);
                Write_B8G8R8A8(&dest[bpp2], dest_format, tmp2);
                Read_B8G8R8A8(tmp3, &src[src_pitch], src_format, 0, 0);
                Write_B8G8R8A8(&dest[dest_pitch], dest_format, tmp3);
                Read_B8G8R8A8(tmp4, &src[bpp1] + src_pitch, src_format, 0, 0);
                Write_B8G8R8A8(&dest[bpp2] + dest_pitch, dest_format, tmp4);
                *reinterpret_cast<uint32_t *>(tmp5) = ((*reinterpret_cast<uint32_t *>(tmp4) & 0xFCFCFCFCu) >> 2)
                    + ((*reinterpret_cast<uint32_t *>(tmp3) & 0xFCFCFCFCu) >> 2)
                    + ((*reinterpret_cast<uint32_t *>(tmp2) & 0xFCFCFCFCu) >> 2)
                    + ((*reinterpret_cast<uint32_t *>(tmp1) & 0xFCFCFCFCu) >> 2);

                if (recolor) {
                    Recolor(*reinterpret_cast<uint32_t *>(tmp5), adjust);
                }

                Write_B8G8R8A8(mip, dest_format, tmp5);
                mip += bpp2;
                dest += 2 * bpp2;
                src += 2 * bpp1;
            }
        }
    } else {
        int pitch1 = dest_pitch >> 2;
        int pitch2 = src_pitch >> 2;
        int pitch3 = mip_pitch >> 2;

        for (unsigned j = 0; j < (height >> 1); j++) {
            uint32_t *dest2 = reinterpret_cast<uint32_t *>(&dest_surface[8 * pitch1 * j]);
            uint32_t *src2 = reinterpret_cast<uint32_t *>(&src_surface[8 * pitch2 * j]);
            uint32_t *mip2 = reinterpret_cast<uint32_t *>(&mip_surface[4 * pitch3 * j]);

            if (recolor) {
                for (unsigned k = 0; k < (width >> 1); k++) {
                    dest2[pitch1] = src2[pitch2];
                    dest2[0] = src2[0];
                    dest2[pitch1 + 1] = src2[pitch2 + 1];
                    dest2[1] = src2[1];
                    *mip2 = ((src2[pitch2 + 1] & 0xFCFCFCFCu) >> 2) + ((src2[pitch2] & 0xFCFCFCFCu) >> 2)
                        + ((src2[1] & 0xFCFCFCFCu) >> 2) + ((src2[0] & 0xFCFCFCFCu) >> 2);
                    Recolor(*reinterpret_cast<uint32_t *>(mip2), adjust);
                    src2 += 2;
                    dest2 += 2;
                    ++mip2;
                }
            } else {
                for (unsigned k = 0; k < (width >> 1); k++) {
                    dest2[pitch1] = src2[pitch2];
                    dest2[0] = src2[0];
                    dest2[pitch1 + 1] = src2[pitch2 + 1];
                    dest2[1] = src2[1];
                    *mip2 = ((src2[pitch2 + 1] & 0xFCFCFCFCu) >> 2) + ((src2[pitch2] & 0xFCFCFCFCu) >> 2)
                        + ((src2[1] & 0xFCFCFCFCu) >> 2) + ((src2[0] & 0xFCFCFCFCu) >> 2);
                    src2 += 2;
                    dest2 += 2;
                }
            }
        }
    }
}

/**
 * Copies an image, converting format as required.
 *
 * 0x0087E7A0
 */
void BitmapHandlerClass::Copy_Image(uint8_t *dest_surface,
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
    const Vector3 &adjust)
{
    if (dest_surface_format == WW3D_FORMAT_U8V8 || dest_surface_format == WW3D_FORMAT_L6V5U5
        || dest_surface_format == WW3D_FORMAT_X8L8V8U8) {
        int bytes_per_pixel = Get_Bytes_Per_Pixel(src_surface_format);

        for (unsigned y = 0; y < dest_surface_height; ++y) {
            uint8_t *dest_pixel = &dest_surface[dest_surface_pitch * y];
            uint8_t *src_pixel = &src_surface[src_surface_pitch * y];
            uint8_t *src_up_pixel = src_pixel - src_surface_pitch;
            uint8_t *src_down_pixel = src_pixel + src_surface_pitch;

            if (y == src_surface_height - 1) {
                src_down_pixel = &src_surface[src_surface_pitch * y];
            } else if (y == 0) {
                src_up_pixel = &src_surface[src_surface_pitch * y];
            }

            for (unsigned x = 0; x < dest_surface_width; ++x) {
                uint8_t *src_left_pixel = src_pixel - bytes_per_pixel;
                uint8_t *src_right_pixel = src_pixel + bytes_per_pixel;

                if (x == src_surface_width - 1) {
                    src_right_pixel = src_pixel;
                } else if (x == 0) {
                    src_left_pixel = src_pixel;
                }

                uint8_t src_pixel_bgra[4];
                uint8_t src_pixel_left_bgra[4];
                uint8_t src_pixel_right_bgra[4];
                uint8_t src_pixel_up_bgra[4];
                uint8_t src_pixel_down_bgra[4];

                uint8_t src_pixel_l;
                uint8_t src_pixel_left_l;
                uint8_t src_pixel_right_l;
                uint8_t src_pixel_up_l;
                uint8_t src_pixel_down_l;

                Read_B8G8R8A8(src_pixel_bgra, src_pixel, src_surface_format, 0, 0);
                Read_B8G8R8A8(src_pixel_right_bgra, &src_pixel[bytes_per_pixel], src_surface_format, 0, 0);
                Read_B8G8R8A8(src_pixel_left_bgra, src_left_pixel, src_surface_format, 0, 0);
                Read_B8G8R8A8(src_pixel_up_bgra, src_up_pixel, src_surface_format, 0, 0);
                Read_B8G8R8A8(src_pixel_down_bgra, src_down_pixel, src_surface_format, 0, 0);

                Write_B8G8R8A8(&src_pixel_l, WW3D_FORMAT_L8, src_pixel_bgra);
                Write_B8G8R8A8(&src_pixel_right_l, WW3D_FORMAT_L8, src_pixel_right_bgra);
                Write_B8G8R8A8(&src_pixel_left_l, WW3D_FORMAT_L8, src_pixel_left_bgra);
                Write_B8G8R8A8(&src_pixel_up_l, WW3D_FORMAT_L8, src_pixel_up_bgra);
                Write_B8G8R8A8(&src_pixel_down_l, WW3D_FORMAT_L8, src_pixel_down_bgra);

                int dest_pixel_u = src_pixel_left_l - src_pixel_right_l;
                int dest_pixel_v = src_pixel_down_l - src_pixel_up_l;

                uint16_t dst_pixel_l = 63;
                if (src_pixel_l <= 1) {
                    dst_pixel_l = 127;
                }

                switch (dest_surface_format) {
                    case WW3D_FORMAT_U8V8:
                        dest_pixel[0] = (uint8_t)dest_pixel_u;
                        dest_pixel[1] = (uint8_t)dest_pixel_v;
                        dest_pixel += 2;

                        break;

                    case WW3D_FORMAT_X8L8V8U8:
                        dest_pixel[0] = (uint8_t)dest_pixel_u;
                        dest_pixel[1] = (uint8_t)dest_pixel_v;
                        dest_pixel[2] = (uint8_t)dst_pixel_l;
                        dest_pixel[3] = 0;
                        dest_pixel += 4;

                        break;

                    case WW3D_FORMAT_L6V5U5:
                        *reinterpret_cast<uint16_t *>(dest_pixel) =
                            (dest_pixel_u >> 3) & 0x1F | 32 * ((dest_pixel_v >> 3) & 0x1F) | (dst_pixel_l >> 2 << 10);
                        dest_pixel += 2;

                        break;
                }

                src_pixel += bytes_per_pixel;
                src_up_pixel += bytes_per_pixel;
                src_down_pixel += bytes_per_pixel;
            }
        }
    } else {
        bool recolor = adjust.X != 0.0f || adjust.Y != 0.0f || adjust.Z != 0.0f;

        if (src_surface_format != dest_surface_format
            || (src_surface_format != WW3D_FORMAT_A8R8G8B8 && src_surface_format != WW3D_FORMAT_X8R8G8B8)) {
            int bpp1 = Get_Bytes_Per_Pixel(dest_surface_format);
            int bpp2 = Get_Bytes_Per_Pixel(src_surface_format);

            if (dest_surface_width != src_surface_width || dest_surface_height != src_surface_height) {
                for (unsigned i = 0; i < dest_surface_height; i++) {
                    uint8_t *dst1 = &dest_surface[i * dest_surface_pitch];

                    if (recolor) {
                        for (unsigned j = 0; j < dest_surface_width; dst1 += bpp1) {
                            Copy_Pixel(dst1,
                                dest_surface_format,
                                &src_surface[src_surface_pitch * (i * src_surface_height / dest_surface_height)]
                                    + bpp2 * (j++ * src_surface_width / dest_surface_width),
                                src_surface_format,
                                src_palette,
                                src_palette_bpp,
                                adjust);
                        }
                    } else {
                        for (unsigned j = 0; j < dest_surface_width; dst1 += bpp1) {
                            Copy_Pixel(dst1,
                                dest_surface_format,
                                &src_surface[src_surface_pitch * (i * src_surface_height / dest_surface_height)]
                                    + bpp2 * (j++ * src_surface_width / dest_surface_width),
                                src_surface_format,
                                src_palette,
                                src_palette_bpp);
                        }
                    }
                }
            } else {
                if (generate_mip_level) {
                    if (dest_surface_width == 1) {
                        uint8_t pixel[4];
                        Read_B8G8R8A8(pixel, src_surface, src_surface_format, src_palette, src_palette_bpp);

                        if (recolor) {
                            Recolor(*reinterpret_cast<uint32_t *>(pixel), adjust);
                        }

                        Write_B8G8R8A8(dest_surface, dest_surface_format, pixel);
                    } else {
                        for (unsigned i = 0; i < (dest_surface_height >> 1); i++) {
                            uint8_t *dest = &dest_surface[2 * dest_surface_pitch * i];
                            uint8_t *src = &src_surface[2 * src_surface_pitch * i];
                            uint8_t *src2 = &src_surface[src_surface_pitch * i];

                            for (unsigned j = 0; j < (dest_surface_width >> 1); ++j) {
                                uint8_t pixel_1[4];
                                uint8_t pixel_2[4];
                                uint8_t pixel_3[4];
                                uint8_t pixel_4[4];
                                Read_B8G8R8A8(pixel_1, src, src_surface_format, src_palette, src_palette_bpp);
                                Read_B8G8R8A8(pixel_2, &src[bpp2], src_surface_format, src_palette, src_palette_bpp);
                                Read_B8G8R8A8(
                                    pixel_3, &src[src_surface_pitch], src_surface_format, src_palette, src_palette_bpp);
                                Read_B8G8R8A8(pixel_4,
                                    &src[bpp2] + src_surface_pitch,
                                    src_surface_format,
                                    src_palette,
                                    src_palette_bpp);

                                if (recolor) {
                                    Recolor(*reinterpret_cast<uint32_t *>(pixel_1), adjust);
                                    Recolor(*reinterpret_cast<uint32_t *>(pixel_2), adjust);
                                    Recolor(*reinterpret_cast<uint32_t *>(pixel_3), adjust);
                                    Recolor(*reinterpret_cast<uint32_t *>(pixel_4), adjust);
                                }

                                Write_B8G8R8A8(dest, dest_surface_format, pixel_1);
                                Write_B8G8R8A8(&dest[bpp1], dest_surface_format, pixel_2);
                                Write_B8G8R8A8(&dest[dest_surface_pitch], dest_surface_format, pixel_3);
                                Write_B8G8R8A8(&dest[bpp1] + dest_surface_pitch, dest_surface_format, pixel_4);
                                /*uint8_t i6[4];
                                *(unsigned *)i6 = ((*(unsigned *)pixel_4 & 0xFCFCFCFCu) >> 2)
                                    + ((*(unsigned *)pixel_3 & 0xFCFCFCFCu) >> 2) + ((*(unsigned *)pixel_2 & 0xFCFCFCFCu) >>
                                2)
                                    + ((*(unsigned *)pixel_1 & 0xFCFCFCFCu) >> 2);
                                Write_B8G8R8A8(src2, src_surface_format, i6);*/
                                Write_B8G8R8A8(src2, src_surface_format, pixel_1);
                                dest += 2 * bpp1;
                                src += 2 * bpp2;
                                src2 += bpp2;
                            }
                        }
                    }
                } else {
                    for (unsigned i = 0; i < dest_surface_height; ++i) {
                        uint8_t *dst = &dest_surface[i * dest_surface_pitch];
                        uint8_t *src3 = &src_surface[i * src_surface_pitch];

                        if (recolor) {
                            for (unsigned j = 0; j < dest_surface_width; ++j) {
                                Copy_Pixel(dst,
                                    dest_surface_format,
                                    src3,
                                    src_surface_format,
                                    src_palette,
                                    src_palette_bpp,
                                    adjust);
                                dst += bpp1;
                                src3 += bpp2;
                            }
                        } else {
                            for (unsigned j = 0; j < dest_surface_width; ++j) {
                                Copy_Pixel(dst, dest_surface_format, src3, src_surface_format, src_palette, src_palette_bpp);
                                dst += bpp1;
                                src3 += bpp2;
                            }
                        }
                    }
                }
            }
        } else {
            int dest_quarter_pitch = dest_surface_pitch >> 2;
            int src_quarter_pitch = src_surface_pitch >> 2;

            if (dest_surface_width != src_surface_width || dest_surface_height != src_surface_height) {
                for (unsigned i = 0; i < dest_surface_height; i++) {
                    uint32_t *dst1 = reinterpret_cast<uint32_t *>(&dest_surface[i * dest_surface_pitch]);

                    for (unsigned j = 0; j < dest_surface_width; dst1 += 4) {
                        uint32_t *src = reinterpret_cast<uint32_t *>(
                            &src_surface[src_surface_pitch * (i * src_surface_height / dest_surface_height)]
                            + 4 * (j++ * src_surface_width / dest_surface_width));

                        uint32_t tmp = *src;

                        if (recolor) {
                            Recolor(tmp, adjust);
                        }

                        *dst1 = tmp;
                    }
                }
            } else {
                if (generate_mip_level) {
                    if (dest_surface_width == 1) {
                        uint32_t tmp = *reinterpret_cast<const uint32_t *>(src_surface);

                        if (recolor) {
                            Recolor(tmp, adjust);
                        }

                        *reinterpret_cast<uint32_t *>(dest_surface) = tmp;
                    } else {
                        for (unsigned i = 0; i < (dest_surface_height >> 1); ++i) {
                            uint32_t *dst = reinterpret_cast<uint32_t *>(&dest_surface[8 * dest_quarter_pitch * i]);
                            uint32_t *src = reinterpret_cast<uint32_t *>(&src_surface[8 * src_quarter_pitch * i]);
                            uint32_t *mip = reinterpret_cast<uint32_t *>(&src_surface[4 * src_quarter_pitch * i]);

                            for (unsigned j = 0; j < (dest_surface_width >> 1); ++j) {
                                dst[dest_quarter_pitch] = src[src_quarter_pitch];
                                dst[0] = src[0];
                                dst[dest_quarter_pitch + 1] = src[src_quarter_pitch + 1];
                                dst[1] = src[1];

                                if (recolor) {
                                    Recolor(dst[dest_quarter_pitch], adjust);
                                    Recolor(dst[0], adjust);
                                    Recolor(dst[dest_quarter_pitch + 1], adjust);
                                    Recolor(dst[1], adjust);
                                }

                                mip[j] = ((src[src_quarter_pitch + 1] & 0xFCFCFCFCu) >> 2)
                                    + ((src[src_quarter_pitch] & 0xFCFCFCFC) >> 2) + ((src[1] & 0xFCFCFCFCu) >> 2)
                                    + ((src[0] & 0xFCFCFCFCu) >> 2);
                                src += 2;
                                dst += 2;
                            }
                        }
                    }
                } else {
                    for (unsigned i = 0; i < dest_surface_height; i++) {
                        uint32_t *dst = reinterpret_cast<uint32_t *>(&dest_surface[4 * i * dest_quarter_pitch]);
                        uint32_t *src = reinterpret_cast<uint32_t *>(&src_surface[4 * i * src_quarter_pitch]);

                        for (unsigned j = 0; j < dest_surface_width; j++) {
                            uint32_t tmp = src[j];

                            if (recolor) {
                                Recolor(tmp, adjust);
                            }

                            dst[j] = tmp;
                        }
                    }
                }
            }
        }
    }
}

/**
 * Copies a pixel between two surfaces.
 */
void BitmapHandlerClass::Copy_Pixel(uint8_t *dest_surface,
    WW3DFormat dest_surface_format,
    const uint8_t *src_surface,
    WW3DFormat src_surface_format,
    const uint8_t *src_palette,
    unsigned src_palette_bpp)
{
    // Perform optimised handling when formats are the same.
    if (dest_surface_format == src_surface_format) {
        switch (dest_surface_format) {
            case WW3D_FORMAT_A8R8G8B8:
            case WW3D_FORMAT_X8R8G8B8:
                *reinterpret_cast<uint32_t *>(dest_surface) = *reinterpret_cast<const uint32_t *>(src_surface);
                break;
            case WW3D_FORMAT_R8G8B8:
                *dest_surface = *src_surface;
                dest_surface[1] = src_surface[1];
                dest_surface[2] = src_surface[2];
                break;
            case WW3D_FORMAT_A4R4G4B4: {
                uint16_t pixel = *reinterpret_cast<const uint16_t *>(src_surface);
                *reinterpret_cast<uint16_t *>(dest_surface) =
                    (pixel >> 12) | ((pixel & 0xF00) >> 4) | ((pixel & 0xF0) << 4) | (pixel << 12);
                break;
            }

            case WW3D_FORMAT_A1R5G5B5: {
                uint16_t pixel = *reinterpret_cast<const uint16_t *>(src_surface);
                *reinterpret_cast<uint16_t *>(dest_surface) =
                    (pixel >> 15) | (((pixel & 0x7C00) >> 9) | 2 * (pixel & 0x3E0) | (pixel << 11));
                break;
            }

            case WW3D_FORMAT_R5G6B5: {
                uint16_t pixel = *reinterpret_cast<const uint16_t *>(src_surface);
                *reinterpret_cast<uint16_t *>(dest_surface) = (pixel >> 11) | ((pixel & 0x1F) << 11) | pixel & 0x7E0;
                break;
            }

            case WW3D_FORMAT_R3G3B2:
            case WW3D_FORMAT_A8:
            case WW3D_FORMAT_L8:
                *dest_surface = *src_surface;
                break;
            default:
                // Bitmap_Assert()
                break;
        }
    } else { // If not, unpack to ARGB and repack to destination format.
        uint8_t pixel[4];
        Read_B8G8R8A8(pixel, src_surface, src_surface_format, src_palette, src_palette_bpp);
        Write_B8G8R8A8(dest_surface, dest_surface_format, pixel);
    }
}

/**
 * Copies a pixel between two surfaces, applying a color adjustment.
 */
void BitmapHandlerClass::Copy_Pixel(uint8_t *dest_surface,
    WW3DFormat dest_surface_format,
    const uint8_t *src_surface,
    WW3DFormat src_surface_format,
    const uint8_t *src_palette,
    unsigned src_palette_bpp,
    const Vector3 &adjust)
{
    // Cannot optimise when we have an adjustment to make.
    uint8_t pixel[4];
    Read_B8G8R8A8(pixel, src_surface, src_surface_format, src_palette, src_palette_bpp);
    Recolor(*reinterpret_cast<uint32_t *>(pixel), adjust);
    Write_B8G8R8A8(dest_surface, dest_surface_format, pixel);
}
