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
#include "ddsfile.h"
#include "colorspace.h"
#include "ffactory.h"
#include "gamedebug.h"
#include "minmax.h"
#include "rtsutils.h"
#include "stringex.h"

using rts::FourCC;

/**
 * 0x00879BF0
 */
DDSFileClass::DDSFileClass(const char *filename, unsigned reduction_factor) :
    m_width(0),
    m_height(0),
    m_depth(0),
    m_maxWidth(0),
    m_maxHeight(0),
    m_maxDepth(0),
    m_mipLevels(0),
    m_time(0),
    m_reductionFactor(reduction_factor),
    m_DDSMemory(nullptr),
    m_format(WW3D_FORMAT_UNKNOWN),
    m_textureType(TEXTURE),
    m_levelSizes(nullptr),
    m_levelOffsets(nullptr),
    m_totalSizeMaybe(0)
{
    // Copy the name across and replace the extension with dds.
    strlcpy(m_name, filename, sizeof(m_name));
    size_t name_len = strlen(m_name);
    m_name[name_len - 3] = 'd';
    m_name[name_len - 2] = 'd';
    m_name[name_len - 1] = 's';

    auto_file_ptr fp(g_theFileFactory, m_name);

    if (!fp->Is_Available() || !fp->Open(FM_READ)) {
        return;
    }

    m_time = fp->Get_Date_Time();
    uint32_t header_fourcc;

    if (fp->Read(&header_fourcc, sizeof(header_fourcc)) != sizeof(header_fourcc)) {
        return;
    }

    // The original doesn't actually check this, but might as well log if we have oddly formatted .dds files.
    if (header_fourcc != FourCC<'d', 'd', 's', ' '>::value) {
        DEBUG_LOG("DDS file '%s' does not have the correct FourCC value in the first 4 bytes.", m_name);
    }

    int header_size = fp->Read(&m_fileHeader, sizeof(m_fileHeader));

    // Check that we read the header correctly, if not, return?
    if (header_size != sizeof(m_fileHeader) || header_size != m_fileHeader.dwSize) {
        DEBUG_LOG("File load failed, read %d of %d bytes and header size reports %d.",
            header_size,
            sizeof(m_fileHeader),
            m_fileHeader.dwSize);

        return;
    }

    // Convert the texture format to our own internal format type enum.
    m_format = D3DFormat_To_WW3DFormat(m_fileHeader.ddspf.dwFourCC);
    m_mipLevels = Max(m_fileHeader.dwMipMapCount, 1u);

    if (m_mipLevels <= m_reductionFactor) {
        m_mipLevels = 1;
        --m_reductionFactor;
    } else {
        m_mipLevels -= m_reductionFactor;
    }

    if (m_mipLevels <= 2) {
        m_mipLevels = 1;
    } else {
        m_mipLevels -= 2;
    }

    if (m_fileHeader.dwCaps2 & DDSCAPS2_CUBEMAP) {
        m_textureType = CUBEMAP;
    } else if (m_fileHeader.dwCaps2 & DDSCAPS2_VOLUME) {
        m_textureType = VOLUME;
    }

    m_maxWidth = m_fileHeader.dwWidth;
    m_maxHeight = m_fileHeader.dwHeight;
    m_maxDepth = m_fileHeader.dwDepth;
    m_width = m_maxWidth >> m_reductionFactor;
    m_height = m_maxHeight >> m_reductionFactor;
    m_depth = m_maxDepth;

    unsigned surface_size = Calculate_S3TC_Surface_Size(m_maxWidth, m_maxHeight, m_format);
    unsigned divisor = 4;
    m_levelSizes = new unsigned[m_mipLevels];
    m_levelOffsets = new unsigned[m_mipLevels];
    unsigned data_size = 0;

    if (m_textureType == VOLUME) {
        surface_size *= m_depth;
        divisor = 8;
    }

    for (unsigned i = 0; i < m_reductionFactor; ++i) {
        if (surface_size > 16) {
            surface_size /= divisor;
        }
    }

    for (unsigned i = 0; i < m_mipLevels; ++i) {
        m_levelSizes[i] = surface_size;
        m_levelOffsets[i] = data_size;
        data_size += surface_size;

        if (surface_size > 16) {
            surface_size /= divisor;
        }
    }

    if (m_textureType == CUBEMAP) {
        for (unsigned i = 0; i < m_mipLevels; ++i) {
            m_totalSizeMaybe += m_levelSizes[i];
        }

        if (m_mipLevels > 2) {
            m_totalSizeMaybe += 16;
        }
    }

    fp->Close();
}

/**
 * 0x00879EF0
 */
DDSFileClass::~DDSFileClass()
{
    delete[] m_DDSMemory;
    delete[] m_levelSizes;
    delete[] m_levelOffsets;
}

/**
 * @brief Get width of the requested level.
 *
 * 0x00879F20
 */
unsigned DDSFileClass::Get_Width(unsigned level)
{
    return Max(m_width >> level, 4u);
}

/**
 * @brief Get width of the requested level.
 *
 * 0x00879F40
 */
unsigned DDSFileClass::Get_Height(unsigned level)
{
    return Max(m_height >> level, 4u);
}

/**
 * @brief Get width of the requested level.
 *
 * 0x00879F60
 */
unsigned DDSFileClass::Get_Depth(unsigned level)
{
    return Max(m_depth >> level, 4u);
}

/**
 * @brief Copy requested level to provided surface, converting format if needed.
 *
 * 0x0087A170
 */
void DDSFileClass::Copy_Level_To_Surface(unsigned level, WW3DFormat dst_format, unsigned dst_width, unsigned dst_height,
    uint8_t *dst_surface, unsigned dst_pitch, const Vector3 &color_shift)
{
    if (m_DDSMemory == nullptr) {
        return;
    }

    // uint8_t *level_ptr = Get_Memory_Pointer(level);
    bool adjust_color = false;

    if (color_shift.X != 0.0f && color_shift.Y != 0.0f && color_shift.Z != 0.0f) {
        adjust_color = true;
    }

    // Source and Dest match, great stuff, just copy the data.
    if (dst_format == m_format && dst_width == Get_Width(level) && dst_height == Get_Height(level) && !adjust_color) {
        // TODO unpacks color component, adjusts it and then repacks it into destination buffer.
        if (adjust_color) {
            DEBUG_ASSERT_PRINT(false, "Tried to color adjust DXT1 or DXT5.\n");
        } else {
            memcpy(dst_surface, Get_Memory_Pointer(level), Get_Level_Size(level));
        }

        return;
    }

    // If formats aren't the same, but the height and width are, decode the data. Only works for DXT1 and DXT5 source data.
    if (dst_width == Get_Width(level) && dst_height == Get_Height(level)) {
        if (m_format != WW3D_FORMAT_DXT1 || dst_format != WW3D_FORMAT_DXT2) {
            unsigned dst_bpp = Get_Bytes_Per_Pixel(dst_format);

            for (unsigned j = 0; j < dst_height; j += 4) {
                uint8_t *putp = dst_surface;

                for (unsigned i = 0; i < dst_width; i += 4) {
                    Get_4x4_Block(putp, dst_pitch, dst_format, level, i, j, color_shift);
                    putp += 4 * dst_bpp;
                }

                dst_surface += 4 * dst_pitch;
            }
        } else if (adjust_color) {
            DEBUG_ASSERT_PRINT(false, "Tried to color adjust DXT1 to DXT2.\n");
        } else {
            // Calc block count, rounding up to nearest.
            unsigned bc_y = (dst_height + 3) / 4;
            unsigned bc_x = (dst_width + 3) / 4;
            const uint32_t *getp = reinterpret_cast<uint32_t *>(Get_Memory_Pointer(level));
            uint32_t *putp = reinterpret_cast<uint32_t *>(dst_surface);

            for (unsigned j = 0; j < bc_y; ++j) {
                for (unsigned i = 0; i < bc_x; ++i) {
                    *putp++ = 0xFFFFFFFF;
                    *putp++ = 0xFFFFFFFF;
                    *putp++ = *getp++;
                    *putp++ = *getp++;
                }
            }
        }
    }
}

/**
 * @brief Copy requested level to provided surface, converting format if needed.
 *
 * 0x0087A0E0
 */
void DDSFileClass::Copy_Level_To_Surface(unsigned level, w3dsurface_t d3d_surface, const Vector3 &color_shift)
{
#ifdef D3D8_BUILD
    D3DSURFACE_DESC desc;
    D3DLOCKED_RECT rect;

    if (d3d_surface->GetDesc(&desc) != 0) {
        DEBUG_LOG("Error getting D3D surface description in DDSFileClass.\n");
    }

    if (d3d_surface->LockRect(&rect, nullptr, 0) != 0) {
        DEBUG_LOG("Error locking D3D surface in DDSFileClass.\n");
    }

    Copy_Level_To_Surface(level,
        D3DFormat_To_WW3DFormat(desc.Format),
        desc.Width,
        desc.Height,
        static_cast<uint8_t *>(rect.pBits),
        rect.Pitch,
        color_shift);

    if (d3d_surface->UnlockRect() != 0) {
        DEBUG_LOG("Error unlocking D3D surface in DDSFileClass.\n");
    }
#endif
}

/**
 * @brief Copy requested level and cube facing to provided surface, converting format if needed.
 *
 * 0x0087AEE0
 */
void DDSFileClass::Copy_CubeMap_Level_To_Surface(unsigned face, unsigned level, WW3DFormat dst_format, unsigned dst_width,
    unsigned dst_height, uint8_t *dst_surface, unsigned dst_pitch, const Vector3 &color_shift)
{
    if (m_DDSMemory == nullptr) {
        return;
    }

    uint8_t *level_ptr = Get_Memory_Pointer(level) + face * m_totalSizeMaybe;
    bool adjust_color = false;

    if (color_shift.X != 0.0f && color_shift.Y != 0.0f && color_shift.Z != 0.0f) {
        adjust_color = true;
    }

    // Source and Dest match, great stuff, just copy the data.
    if (dst_format == m_format && dst_width == Get_Width(level) && dst_height == Get_Height(level) && !adjust_color) {
        // TODO unpacks color component, adjusts it and then repacks it into destination buffer.
        if (adjust_color) {
            DEBUG_ASSERT_PRINT(false, "Tried to color adjust CubeMap DXT1 or DXT5.\n");
        } else {
            memcpy(dst_surface, level_ptr, Get_Level_Size(level));
        }

        return;
    }

    // If formats aren't the same, but the height and width are, decode the data. Only works for DXT1 and DXT5 source data.
    if (dst_width == Get_Width(level) && dst_height == Get_Height(level)) {
        if (m_format != WW3D_FORMAT_DXT1 || dst_format != WW3D_FORMAT_DXT2) {
            unsigned dst_bpp = Get_Bytes_Per_Pixel(dst_format);

            for (unsigned j = 0; j < dst_height; j += 4) {
                uint8_t *putp = dst_surface;

                for (unsigned i = 0; i < dst_width; i += 4) {
                    Get_4x4_Block(putp, dst_pitch, dst_format, level, i, j, color_shift);
                    putp += 4 * dst_bpp;
                }

                dst_surface += 4 * dst_pitch;
            }
        } else { // TODO Copy DXT1 to DXT2?
            DEBUG_ASSERT_PRINT(false, "Tried to copy CubeMap DXT1 to DXT2.\n");
        }
    }
}

/**
 * @brief Decode a 4x4 block in either DXT1 or DXT5 S3TC codec.
 *
 * 0x0087BBF0
 */
bool DDSFileClass::Get_4x4_Block(uint8_t *dst_ptr, unsigned dst_pitch, WW3DFormat dst_format, unsigned level, unsigned src_x,
    unsigned src_y, const Vector3 &color_shift)
{
    unsigned dst_bpp = Get_Bytes_Per_Pixel(dst_format);
    bool adjust_color = false;
    bool has_alpha = false;

    if (color_shift.X != 0.0f && color_shift.Y != 0.0f && color_shift.Z != 0.0f) {
        adjust_color = true;
    }

    // Gen and ZH only handle DXT1 and DXT5
    switch (m_format) {
        case WW3D_FORMAT_DXT1: {
            int offset = (src_x / 4) + (src_y / 4) * (Get_Width(level) / 4);
            unsigned dst_pixel = 0;
            uint8_t *block_mem = &Get_Memory_Pointer(level)[8 * offset];
            uint32_t color_a = Decode_Packed_565(block_mem);
            uint32_t color_b = Decode_Packed_565(block_mem + 2);
            // uint32_t code = Decode_Line_Code(block_mem + 4);

            if (color_a > color_b) {
                if (adjust_color) {
                    Vector4 rgba;
                    Color_To_RGBA(rgba, color_a);
                    Adjust_RGBA(rgba, color_shift);
                    RGBA_To_Color(color_a, rgba);
                    Color_To_RGBA(rgba, color_b);
                    Adjust_RGBA(rgba, color_shift);
                    RGBA_To_Color(color_b, rgba);
                }

                for (int j = 0; j < 4; ++j) {
                    uint8_t *putp = dst_ptr;
                    dst_ptr += dst_pitch;

                    for (int i = 0; i < 4; ++i) {
                        uint32_t final_color = 0;

                        switch (block_mem[j + 4] & 3) {
                            case 0:
                                final_color = color_a;
                                break;
                            case 1:
                                final_color = color_b;
                                break;
                            case 2:
                                final_color = ((85 * (color_b & 0xFF00) + 170 * (color_a & 0xFF00)) >> 8) & 0xFF00
                                    | ((85 * (color_b & 0xFF00FF) + 170 * (color_a & 0xFF00FF)) >> 8) & 0xFF00FF;
                                break;
                            case 3:
                                final_color = ((85 * (color_a & 0xFF00) + 170 * (color_b & 0xFF00)) >> 8) & 0xFF00
                                    | ((85 * (color_a & 0xFF00FF) + 170 * (color_b & 0xFF00FF)) >> 8) & 0xFF00FF;
                                break;
                            default:
                                break;
                        }

                        final_color |= 0xFF000000;
                        Color_To_Format(putp, final_color, dst_format);
                        putp += dst_bpp;
                    }
                }
            } else {
                if (adjust_color) {
                    Vector4 rgba;
                    Color_To_RGBA(rgba, color_a);
                    Adjust_RGBA(rgba, color_shift);
                    RGBA_To_Color(color_a, rgba);
                    Color_To_RGBA(rgba, color_b);
                    Adjust_RGBA(rgba, color_shift);
                    RGBA_To_Color(color_b, rgba);
                }

                for (int j = 0; j < 4; ++j) {
                    uint8_t *putp = dst_ptr;
                    dst_ptr += dst_pitch;

                    for (int i = 0; i < 4; ++i) {
                        uint32_t final_color = 0;

                        switch (block_mem[j + 4] & 3) {
                            case 0:
                                final_color = color_a;
                                break;
                            case 1:
                                final_color = color_b;
                                break;
                            case 2:
                                final_color =
                                    ((127 * (color_a & 0xFF00) + ((uint16_t)(color_b & 0xFF00) << 7)) >> 8) & 0xFF00
                                    | ((((color_b & 0xFF00FF) << 7) + 0x7F * (color_a & 0xFF00FF)) >> 8) & 0xFF00FF;
                                break;
                            case 3:
                                final_color = 0;
                                break;
                            default:
                                break;
                        }

                        final_color |= 0xFF000000;
                        Color_To_Format(putp, final_color, dst_format);
                        putp += dst_bpp;
                    }
                }

                has_alpha = true;
            }
        }
            return has_alpha;
        case WW3D_FORMAT_DXT5: {
            int offset = (src_x / 4) + (src_y / 4) * (Get_Width(level) / 4);
            unsigned dst_pixel = 0;
            uint8_t *block_mem = &Get_Memory_Pointer(level)[16 * offset];

            unsigned alpha0 = *(block_mem);
            unsigned alpha1 = *(block_mem + 1);
            unsigned alphas[8];

            alphas[0] = alpha0;
            alphas[1] = alpha1;

            if (alpha0 > alpha1) {
                alphas[2] = (alpha1 + 6 * alpha0 + 3) / 7;
                alphas[3] = (5 * alpha0 + 2 * alpha1 + 3) / 7;
                alphas[4] = (3 * alpha1 + 3 + 4 * alpha0) / 7;
                alphas[5] = (3 * alpha0 + 3 + 4 * alpha1) / 7;
                alphas[6] = (5 * alpha1 + 2 * alpha0 + 3) / 7;
                alphas[7] = (alpha0 + 6 * alpha1 + 3) / 7;
            } else {
                alphas[2] = (alpha1 + 4 * alpha0 + 2) / 5;
                alphas[6] = 0;
                alphas[7] = 255;
                alphas[3] = (3 * alpha0 + 2 * alpha1 + 2) / 5;
                alphas[4] = (3 * alpha1 + 2 * alpha0 + 2) / 5;
                alphas[5] = (alpha0 + 4 * alpha1 + 2) / 5;
            }

            uint32_t color_a = Decode_Packed_565(block_mem + 8);
            uint32_t color_b = Decode_Packed_565(block_mem + 10);
            unsigned alpha_indices[16];

            for (int i = 0; i < 2; ++i) {
                // grab 3 bytes
                int value = 0;

                for (int j = 0; j < 3; ++j) {
                    int byte = block_mem[2 + j];
                    value |= (byte << 8 * j);
                }

                // unpack 8 3-bit values from it
                for (int j = 0; j < 8; ++j) {
                    unsigned index = (value >> 3 * j) & 0x7;
                    alpha_indices[i * 8 + j] = index;
                }
            }

            for (int j = 0; j < 4; j++) {
                uint8_t *putp = dst_ptr;
                dst_ptr += dst_pitch;

                for (int i = 0; i < 4; i++) {
                    unsigned alpha = alphas[alpha_indices[j * 4 + i]];
                    has_alpha = alpha < 255;
                    uint32_t final_color = 0;

                    switch (((block_mem[j + 12]) >> (2 * i)) & 3) {
                        case 0:
                            final_color = color_a | (alpha << 24);
                            break;
                        case 1:
                            final_color = color_b | (alpha << 24);
                            break;
                        case 2:
                            final_color = ((85 * (color_b & 0xFF00) + 170 * (color_a & 0xFF00)) >> 8) & 0xFF00
                                | ((85 * (color_b & 0xFF00FF) + 170 * (color_a & 0xFF00FF)) >> 8) & 0xFF00FF;
                            break;
                        case 3:
                            final_color = ((85 * (color_a & 0xFF00) + 170 * (color_b & 0xFF00)) >> 8) & 0xFF00
                                | ((85 * (color_a & 0xFF00FF) + 170 * (color_b & 0xFF00FF)) >> 8) & 0xFF00FF;
                            break;
                    }

                    final_color |= alpha;
                    Color_To_Format(putp, final_color, dst_format);
                    putp += dst_bpp;
                }
            }

            return has_alpha;
        } break;
        default:
            break;
    }

    return false;
}

/**
 * @brief Load image data into memory.
 *
 * 0x00879F80
 */
bool DDSFileClass::Load()
{
    if (m_DDSMemory != nullptr || m_levelSizes == nullptr || m_levelOffsets == nullptr) {
        return false;
    }

    auto_file_ptr fp(g_theFileFactory, m_name);

    if (!fp->Is_Available()) {
        DEBUG_LOG("DDSFile '%s' is not available.\n", m_name);
        return false;
    }

    fp->Open(FM_READ);

    // Get size of the data, accounting for header.
    unsigned data = fp->Size() - m_fileHeader.dwSize - sizeof(uint32_t);

    if (data == 0) {
        DEBUG_LOG("DDSFile '%s' appears to contain no data, size is %d.\n", m_name, fp->Size());
        return false;
    }

    unsigned surface_size = Calculate_S3TC_Surface_Size(m_fileHeader.dwWidth, m_fileHeader.dwHeight, m_format);
    unsigned offset = 0;

    // Calculate offset for the mip map level we want.
    for (unsigned i = m_reductionFactor; i > 0; --i) {
        offset += surface_size;
        data -= surface_size;

        if (surface_size > 16) {
            surface_size /= 4;
        }
    }

    // Seek to start of required data, accounting for header.
    fp->Seek(offset + m_fileHeader.dwSize + sizeof(uint32_t), FS_SEEK_CURRENT);

    if (data > 0 && data <= INT32_MAX) {
        m_DDSMemory = new uint8_t[data];
        fp->Read(m_DDSMemory, data);
    } else {
        DEBUG_LOG("Data size out of range at %d for '%s' at reduction factor %u.\n", data, m_name, m_reductionFactor);
    }

    fp->Close();

    return true;
}

/**
 * @brief Get size of S3TC compressed surface given its dimensions and format.
 */
unsigned DDSFileClass::Calculate_S3TC_Surface_Size(unsigned width, unsigned height, WW3DFormat format)
{
    unsigned ret = (width / 4) * (height / 4);

    if (format == WW3D_FORMAT_DXT1) {
        ret *= 8;
    } else if (format > WW3D_FORMAT_DXT1 && format <= WW3D_FORMAT_DXT5) {
        ret *= 16;
    }

    return ret;
}
