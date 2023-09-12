/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Object containing thumbnail information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "thumbnail.h"
#include "bitmaphandler.h"
#include "ddsfile.h"
#include "ffactory.h"
#include "targa.h"
#include "thumbnailmanager.h"
#include "vector3.h"

ThumbnailClass::ThumbnailClass(ThumbnailManagerClass *manager,
    const char *texture,
    uint8_t *bits,
    unsigned width,
    unsigned height,
    unsigned maxwidth,
    unsigned maxheight,
    unsigned miplevels,
    WW3DFormat format,
    bool isallocated,
    unsigned time) :
    m_filename(texture),
    m_bitmap(bits),
    m_width(width),
    m_height(height),
    m_maxWidth(maxwidth),
    m_maxHeight(maxheight),
    m_mipLevels(miplevels),
    m_format(format),
    m_time(time),
    m_isAllocated(isallocated),
    m_manager(manager)
{
    manager->Insert_To_Hash(this);
}

/**
 * 0x0086A040
 */
ThumbnailClass::ThumbnailClass(ThumbnailManagerClass *manager, const StringClass &texture) :
    m_filename(texture),
    m_bitmap(nullptr),
    m_width(0),
    m_height(0),
    m_maxWidth(0),
    m_maxHeight(0),
    m_mipLevels(0),
    m_format(WW3D_FORMAT_UNKNOWN),
    m_time(0),
    m_isAllocated(false),
    m_manager(manager)
{
    DDSFileClass dds(texture, 3);
    unsigned mips = dds.Get_Mip_Level_Count();

    // Try loading a dds version of a texture first, then fall back to looking for targa.
    if (mips != 0 && dds.Load()) {
        m_time = dds.Get_Time();
        int len = m_filename.Get_Length();
        m_filename[len - 3] = 'd';
        m_filename[len - 2] = 'd';
        m_filename[len - 1] = 's';

        unsigned levels = 0;
        while (levels < mips - 1 && (dds.Get_Width(levels) > 32 || dds.Get_Height(levels) > 32)) {
            ++levels;
        }

        m_maxWidth = dds.Get_Width(0);
        m_maxHeight = dds.Get_Height(0);
        m_format = dds.Get_Format();
        m_mipLevels = dds.Get_Mip_Level_Count();
        m_width = dds.Get_Width(levels);
        m_height = dds.Get_Height(levels);
        m_bitmap = new uint8_t[2 * m_height * m_width];
        m_isAllocated = true;

        dds.Copy_Level_To_Surface(
            0, WW3D_FORMAT_A4R4G4B4, m_width, m_height, m_bitmap, 2 * m_width, Vector3(0.0f, 0.0f, 0.0f));
    } else {
        TargaImage targa;

        if (TargaImage::Error_Handler(targa.Open(texture, 0), texture) != TGA_RET_OK) {
            return;
        }

        targa.Toggle_Flip_Y();
        unsigned src_bpp;
        WW3DFormat dest_format;
        Get_WW3D_Format(dest_format, src_bpp, targa);

        if (dest_format) {
            m_maxWidth = targa.Get_Header().width;
            m_maxHeight = targa.Get_Header().height;
            m_format = dest_format;
            m_width = (uint16_t)targa.Get_Header().width / 8;
            m_height = (uint16_t)targa.Get_Header().height / 8;
            m_mipLevels = 1;

            int i;
            int j;
            for (i = 1, j = 1; i < m_maxWidth && j < m_maxHeight; i *= 2, j *= 2) {
                m_mipLevels++;
            }

            for (; m_height > 0x20 && m_width > 0x20; m_height >>= 2, m_width >>= 2) {
            }

            int height;
            for (height = 1; height < m_height; height *= 2) {
            }

            int width;
            for (width = 1; width < m_width; width *= 2) {
            }

            m_height = height;
            m_width = width;
            char palette[1024];
            targa.Set_Palette(palette);

            if (TargaImage::Error_Handler(targa.Load(texture.Peek_Buffer(), 1, 0), texture) != TGA_RET_OK) {
                return;
            }

            {
                auto_file_ptr ptr(g_theFileFactory, texture.Peek_Buffer());
                ptr->Open(1);
                m_time = ptr->Get_Date_Time();
                ptr->Close();
            }

            int len = m_filename.Get_Length();
            m_filename[len - 3] = 't';
            m_filename[len - 2] = 'g';
            m_filename[len - 1] = 'a';
            m_bitmap = new uint8_t[2 * m_width * m_height];
            m_isAllocated = true;

            BitmapHandlerClass::Copy_Image(m_bitmap,
                m_width,
                m_height,
                2 * m_width,
                WW3D_FORMAT_A4R4G4B4,
                reinterpret_cast<uint8_t *>(targa.Get_Image()),
                targa.Get_Header().width,
                targa.Get_Header().height,
                src_bpp * targa.Get_Header().width,
                dest_format,
                reinterpret_cast<uint8_t *>(targa.Get_Palette()),
                static_cast<uint8_t>(targa.Get_Header().cmap_depth) / 8,
                false,
                Vector3(0.0f, 0.0f, 0.0f));
        }
    }

    m_manager->Insert_To_Hash(this);
}

ThumbnailClass::~ThumbnailClass()
{
    if (m_isAllocated) {
        delete[] m_bitmap;
    }

    m_manager->Remove_From_Hash(this);
}
