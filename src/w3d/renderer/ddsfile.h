/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling DDS textures.
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
#include "w3dformat.h"
#include "w3dtypes.h"
#include <new>

enum DDSPixelFormatFlags
{
    DDPF_ALPHAPIXELS = 0x1,
    DDPF_ALPHA = 0x2,
    DDPF_FOURCC = 0x4,
    DDPF_RGB = 0x40,
    DDPF_YUV = 0x200,
    DDPF_LUMINANCE = 0x20000,
};

enum DDSHeaderFlags
{
    DDSD_CAPS = 0x1,
    DDSD_HEIGHT = 0x2,
    DDSD_WIDTH = 0x4,
    DDSD_PITCH = 0x8,
    DDSD_PIXELFORMAT = 0x1000,
    DDSD_MIPMAPCOUNT = 0x20000,
    DDSD_LINEARSIZE = 0x80000,
    DDSD_DEPTH = 0x800000,
};

enum DDSHeaderCaps
{
    DDSCAPS_COMPLEX = 0x8,
    DDSCAPS_TEXTURE = 0x1000,
    DDSCAPS_MIPMAP = 0x400000,
};

enum DDSHeaderCaps2
{
    DDSCAPS2_CUBEMAP = 0x200,
    DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
    DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
    DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
    DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
    DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
    DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
    DDSCAPS2_VOLUME = 0x200000,
};

struct DDSPixelFormat
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
};

struct DDSHeader
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDSPixelFormat ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

class DDSFileClass
{
    ALLOW_HOOKING
    enum
    {
        TEXTURE,
        CUBEMAP,
        VOLUME,
    };

public:
    DDSFileClass(const char *filename, unsigned reduction_factor);
    ~DDSFileClass();

    unsigned Get_Width(unsigned level);
    unsigned Get_Height(unsigned level);
    unsigned Get_Depth(unsigned level);
    unsigned Get_Mip_Level_Count() { return m_mipLevels; }
    uint8_t *Get_Memory_Pointer(unsigned level) { return &m_DDSMemory[m_levelOffsets[level]]; }
    unsigned Get_Level_Size(unsigned level) { return m_levelSizes[level]; }
    WW3DFormat Get_Format() { return m_format; }
    time_t Get_Time() const { return m_time; }
    bool Have_Level_Sizes() const { return m_levelSizes != nullptr; }
    void Copy_Level_To_Surface(unsigned level,
        WW3DFormat dst_format,
        unsigned dst_width,
        unsigned dst_height,
        uint8_t *dst_surface,
        unsigned dst_pitch,
        const Vector3 &color_shift);
    void Copy_Level_To_Surface(unsigned level, w3dsurface_t d3d_surface, const Vector3 &color_shift);
    void Copy_CubeMap_Level_To_Surface(unsigned face,
        unsigned level,
        WW3DFormat dst_format,
        unsigned dst_width,
        unsigned dst_height,
        uint8_t *dst_surface,
        unsigned dst_pitch,
        const Vector3 &color_shift);
    bool Load();

#ifdef GAME_DLL
    DDSFileClass *Hook_Ctor(const char *filename, unsigned reduction_factor)
    {
        return new (this) DDSFileClass(filename, reduction_factor);
    }
#endif

private:
    bool Get_4x4_Block(uint8_t *dst_ptr,
        unsigned dst_pitch,
        WW3DFormat dst_format,
        unsigned level,
        unsigned src_x,
        unsigned src_y,
        const Vector3 &color_shift);

    static unsigned Calculate_S3TC_Surface_Size(unsigned width, unsigned height, WW3DFormat format);
    static uint32_t Decode_Packed_565(uint8_t *packed);
    static uint32_t Decode_Line_Code(uint8_t *packed);
    static uint32_t Merge_Color(uint32_t color_a, uint32_t color_b, uint32_t s1);

private:
    unsigned m_width;
    unsigned m_height;
    unsigned m_depth;
    unsigned m_maxWidth;
    unsigned m_maxHeight;
    unsigned m_maxDepth;
    unsigned m_mipLevels;
    time_t m_time;
    unsigned m_reductionFactor;
    uint8_t *m_DDSMemory;
    WW3DFormat m_format;
    unsigned m_textureType;
    unsigned *m_levelSizes;
    unsigned *m_levelOffsets;
    unsigned m_totalSizeMaybe;
    DDSHeader m_fileHeader;
    char m_name[256];
};

inline uint32_t DDSFileClass::Decode_Packed_565(uint8_t *packed)
{
    uint16_t value = uint16_t(packed[0]) | (uint16_t(packed[1]) << 8);
    return ((value & 0x1F) | ((value & 0x7E0) | ((value & 0xF800) << 3) << 2)) << 3;
}

inline uint32_t DDSFileClass::Merge_Color(uint32_t color_a, uint32_t color_b, uint32_t color_b_amount)
{
    uint32_t color_a_amount = 0xFF - color_b_amount;
    const uint32_t G_MASK = 0x0000FF00;
    const uint32_t R_B_MASK = 0x00FF00FF;

    return (((color_a_amount * (color_a & G_MASK)) + (color_b_amount * (color_b & G_MASK))) >> 8) & G_MASK
        | (((color_a_amount * (color_a & R_B_MASK)) + (color_b_amount * (color_b & R_B_MASK))) >> 8) & R_B_MASK;
}

inline uint32_t DDSFileClass::Decode_Line_Code(uint8_t *packed)
{
    return uint32_t(packed[0]) | (uint32_t(packed[1]) << 8) | (uint32_t(packed[2]) << 16) | (uint32_t(packed[3]) << 24);
}
