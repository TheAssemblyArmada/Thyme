/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Class for handling TARGA images.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "targa.h"
#include "ffactory.h"
#include <captainslog.h>
#include <cstring>
#include <malloc.h>

#ifdef GAME_DLL
#include "hookcrt.h"
#endif

using std::memcpy;
using std::memset;

TargaImage::TargaImage() : m_TGAFile(nullptr), m_access(0), m_flags(0), m_image(nullptr), m_palette(nullptr)
{
    memset(&m_header, 0, sizeof(m_header));
    memset(&m_extension, 0, sizeof(m_extension));
}

TargaImage::~TargaImage()
{
    Close();

    if (m_palette != nullptr && (m_flags & TGA_FLAG_PAL_ALLOC)) {
        free(m_palette);
        m_palette = nullptr;
    }

    if (m_image != nullptr && (m_flags & TGA_FLAG_IMAGE_ALLOC)) {
        free(m_image);
        m_image = nullptr;
    }
}

int TargaImage::Open(const char *name, int mode)
{
    if (Is_File_Open() && m_access == mode) {
        return 0;
    }

    Close();
    m_flags &= ~TGA_FLAG_INVALID;
    m_access = mode;

    TGA2Footer footer;

    switch (mode) {
        case TARGA_READ:
            if (!File_Open_Read(name)) {
                Close();

                return TGA_RET_UNABLE_TO_LOAD;
            }

            // If we can't read enough data for a footer, not a TGA file.
            if (File_Seek(-((int)sizeof(footer)), FileSeekType::FS_SEEK_END) == -1
                || File_Read(&footer, sizeof(footer)) != sizeof(footer)) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            // If we don't have the correct footer info, not a TGA file.
            if (strncasecmp(footer.signature, "TRUEVISION-XFILE", sizeof(footer.signature)) == 0 && footer.extension != 0) {
                m_flags |= TGA_FLAG_INVALID;
            }

            footer.extension = le32toh(footer.extension);

            // If we can't seek to the extension offset or we can't read enough data,
            // not a TGA file.
            if ((m_flags & TGA_FLAG_INVALID) != 0
                && (File_Seek(footer.extension, FileSeekType::FS_SEEK_START) != -1
                    || File_Read(&m_extension, sizeof(m_extension) != sizeof(m_extension)))) {
                Close();

                return TGA_RET_NOT_TGA;
            } else if (File_Seek(0, FileSeekType::FS_SEEK_START) == -1) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            // Convert endian if needed.
            Extension_To_Host(m_extension);

            // If file is shorter than header then its not a TGA.
            if (File_Read(&m_header, sizeof(m_header)) != sizeof(m_header)) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            // Convert endian if needed.
            Header_To_Host(m_header);

            // If file seek past ID fails, then not a TGA.
            if (m_header.id_length != 0 && File_Seek(m_header.id_length, FileSeekType::FS_SEEK_CURRENT) == -1) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            break;
        case TARGA_WRITE:
            if (!File_Open_Write(name)) {
                Close();

                return TGA_RET_UNABLE_TO_LOAD;
            }

            break;
        case TARGA_READWRITE:
            if (!File_Open_ReadWrite(name)) {
                Close();

                return TGA_RET_UNABLE_TO_LOAD;
            }

            // If file is shorter than header then its not a TGA.
            if (File_Read(&m_header, sizeof(m_header)) != sizeof(m_header)) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            Header_To_Host(m_header);

            // If file seek past ID fails, then not a TGA.
            if (m_header.id_length != 0 && File_Seek(m_header.id_length, FileSeekType::FS_SEEK_CURRENT) == -1) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            break;
        default:
            break;
    }

    return TGA_RET_OK;
}

void TargaImage::Close()
{
    if (m_TGAFile != nullptr) {
        m_TGAFile->Close();
        g_theFileFactory->Return_File(m_TGAFile);
        Clear_File();
    }
}

int TargaImage::Load(const char *name, int flags, bool invert_image)
{
    if (Open(name, TARGA_READ) != TGA_RET_OK) {
        return TGA_RET_UNABLE_TO_LOAD;
    }

    if ((flags & TGA_FLAG_PAL_ALLOC) != 0 && m_header.cmap_type == 1) {
        if (m_palette != nullptr && (m_flags & TGA_FLAG_PAL_ALLOC) == 0) {
            free(m_palette);
            m_palette = nullptr;
            m_flags &= ~TGA_FLAG_PAL_ALLOC;
        }

        if (m_palette == nullptr && (m_flags & TGA_FLAG_PAL_ALLOC) == 0) {
            if ((m_header.cmap_length * (m_header.cmap_depth / 8)) > 0) {
                m_palette = (char *)malloc(m_header.cmap_length * (m_header.cmap_depth / 8));
            }

            if (m_palette == nullptr) {
                Close();

                return TGA_RET_OUT_OF_MEMORY;
            }

            m_flags |= TGA_FLAG_PAL_ALLOC;
        }
    }

    if ((flags & TGA_FLAG_IMAGE_ALLOC) != 0) {
        if (m_image != nullptr && (m_flags & TGA_FLAG_IMAGE_ALLOC) == 0) {
            free(m_image);
            m_image = nullptr;
            m_flags &= ~TGA_FLAG_IMAGE_ALLOC;
        }

        if (m_image == nullptr && (m_flags & TGA_FLAG_IMAGE_ALLOC) == 0) {
            if ((m_header.width * m_header.height * ((m_header.pixel_depth + 7) / 8)) > 0) {
                m_image = (char *)malloc(m_header.width * m_header.height * ((m_header.pixel_depth + 7) / 8));
            }

            if (m_image == nullptr) {
                Close();

                return TGA_RET_OUT_OF_MEMORY;
            }

            m_flags |= TGA_FLAG_IMAGE_ALLOC;
        }
    }

    int ret = Load(name, m_palette, m_image, invert_image);
    Close();

    return ret;
}

int TargaImage::Load(const char *name, char *palette, char *image, bool invert_image)
{
    if (Open(name, TARGA_READ) != TGA_RET_OK) {
        return TGA_RET_UNABLE_TO_LOAD;
    }

    if (m_header.cmap_type == 1) {
        int pixel_bytes = m_header.cmap_depth / 8;

        if (palette != nullptr && m_header.cmap_length > 0) {
            int read_bytes = File_Read(&palette[pixel_bytes * m_header.cmap_start], pixel_bytes * m_header.cmap_length);

            if (read_bytes != pixel_bytes * m_header.cmap_length) {
                Close();

                return TGA_RET_NOT_TGA;
            }
        } else if (File_Seek(pixel_bytes * m_header.cmap_length, FileSeekType::FS_SEEK_CURRENT) == -1) {
            Close();

            return TGA_RET_NOT_TGA;
        }
    }

    if (image != nullptr) {
        int total_bytes = m_header.width * m_header.height * ((m_header.pixel_depth + 7) / 8);

        switch (m_header.image_type) {
            case TGA_TYPE_MAPPED:
            case TGA_TYPE_COLOR:
            case TGA_TYPE_GREY:
                if (File_Read(image, total_bytes) != total_bytes) {
                    Close();

                    return TGA_RET_NOT_TGA;
                }
                break;
            case TGA_TYPE_RLE_MAPPED:
            case TGA_TYPE_RLE_COLOR: {
                int ret = Decode_Image();

                if (ret != TGA_RET_OK) {
                    return ret;
                }
            } break;
            default:
                Close();
                return TGA_RET_UNSUPPORTED;
        }

        if (invert_image) {
            Invert_Image();
        }

        if ((m_header.image_descriptor & 0x10) != 0) {
            X_Flip();
            m_header.image_descriptor &= ~0x10;
        }

        if ((m_header.image_descriptor & 0x20) != 0) {
            Y_Flip();
            m_header.image_descriptor &= ~0x20;
        }
    }
    Close();

    return TGA_RET_OK;
}

int TargaImage::Save(const char *name, int flags, bool add_extension)
{
    if (Open(name, TARGA_WRITE) != TGA_RET_OK) {
        return TGA_RET_UNABLE_TO_LOAD;
    }

    m_header.id_length = 0;

    // Flag can force compression even if its not currently set.
    if ((flags & TGA_FLAG_COMPRESS) != 0) {
        switch (m_header.image_type) {
            case TGA_TYPE_MAPPED:
            case TGA_TYPE_COLOR:
            case TGA_TYPE_GREY:
                m_header.image_type += 8; // Convert to RLE type then fall through
            case TGA_TYPE_RLE_MAPPED:
            case TGA_TYPE_RLE_COLOR:
            case TGA_TYPE_RLE_GREY:
                break;
            default:
                flags &= ~TGA_FLAG_COMPRESS;
                break;
        }
    }

    if (File_Write(&m_header, sizeof(m_header)) != sizeof(m_header)) {
        Close();

        return TGA_RET_UNABLE_TO_SAVE;
    }

    // Write palette if we have one.
    if ((flags & TGA_FLAG_PAL_ALLOC) != 0) {
        if (m_palette != nullptr && m_header.cmap_length > 0) {
            int pal_depth = m_header.cmap_depth / 8;
            int pal_size = pal_depth * m_header.cmap_length;

            if (File_Write(&m_palette[pal_depth * m_header.cmap_start], pal_size) != pal_size) {
                Close();

                return TGA_RET_UNABLE_TO_SAVE;
            }
        }
    }

    // Write the image, flipping if color and flipping back afterward, compressing if required
    if ((flags & TGA_FLAG_IMAGE_ALLOC) != 0 && m_image != nullptr) {
        bool inverted = false;

        if (m_header.image_type == TGA_TYPE_COLOR || m_header.image_type == TGA_TYPE_RLE_COLOR) {
            Invert_Image();
            inverted = true;
        }

        if ((flags & TGA_FLAG_COMPRESS) != 0) {
            Encode_Image();
        } else {
            int img_size = m_header.width * m_header.height * ((m_header.pixel_depth + 7) / 8);

            if (File_Write(m_image, img_size) != img_size) {
                Close();

                return TGA_RET_UNABLE_TO_SAVE;
            }
        }

        if (inverted) {
            Invert_Image();
        }
    }

    TGA2Footer footer;

    // Write the extension if required, if not just write the footer.
    if (add_extension) {
        m_extension.ext_size = sizeof(m_extension);
        strlcpy_tpl(m_extension.software_id, "Thyme Game Engine");
        m_extension.software_vers.number = 1;
        m_extension.software_vers.letter = '\0';
        footer.extension = File_Seek(0, FileSeekType::FS_SEEK_CURRENT);

        if (footer.extension == -1 || File_Write(&m_extension, sizeof(m_extension)) != sizeof(m_extension)) {
            Close();

            return TGA_RET_UNABLE_TO_SAVE;
        }
    } else {
        footer.extension = 0;
    }

    strlcpy_tpl(footer.signature, "TRUEVISION-XFILE");
    footer.dot_char = '.';
    footer.null_char = '\0';

    if (File_Write(&footer, sizeof(footer)) != sizeof(footer)) {
        Close();

        return TGA_RET_UNABLE_TO_SAVE;
    }

    Close();

    return TGA_RET_OK;
}

void TargaImage::X_Flip()
{
    int pixel_size = (m_header.pixel_depth + 7) / 8;

    // Must have at least one row of pixels.
    if (m_header.height <= 0) {
        captainslog_dbgassert(false, "Image has no pixels.");
        return;
    }

    // Iterate over the image and move bytes from start of a row to the
    // end and vice versa, keeping the order of bytes within the pixel.
    for (int y = 0; y < m_header.height; ++y) {
        char *startp = &m_image[y * m_header.width * pixel_size];
        char *endp = &startp[pixel_size * (m_header.width - 1)];

        for (int x = 0; x < (m_header.width / 2); ++x) {
            if (pixel_size > 0) {
                char *modp = endp;
                ptrdiff_t diff = startp - endp;

                for (int i = pixel_size; i > 0; --i) {
                    char tmp = modp[diff];
                    modp[diff] = modp[0];
                    modp[0] = tmp;
                    ++modp;
                }
            }

            startp += pixel_size;
            endp -= pixel_size;
        }
    }
}

void TargaImage::Y_Flip()
{
    int pixel_size = (m_header.pixel_depth + 7) / 8;

    for (int y = 0; y < (m_header.height / 2); ++y) {
        // Set pointers to the start and end rows.
        char *startp = &m_image[m_header.width * pixel_size * y];
        char *endp = &m_image[m_header.width * pixel_size * (m_header.height - 1)] - (m_header.width * pixel_size * y);

        // Iterate over the rows, swapping bytes between them.
        for (int x = 0; x < (pixel_size * m_header.width); ++x) {
            char tmp = startp[x];
            startp[x] = endp[x];
            endp[x] = tmp;
        }
    }
}

char *TargaImage::Set_Image(char *buffer)
{
    if (m_image != nullptr && (m_flags & TGA_FLAG_IMAGE_ALLOC) == 0) {
        free(m_image);
        m_image = nullptr;
        m_flags &= ~TGA_FLAG_IMAGE_ALLOC;
    }

    char *old_image = m_image;
    m_image = buffer;

    return old_image;
}

char *TargaImage::Set_Palette(char *buffer)
{
    if (m_palette != nullptr && (m_flags & TGA_FLAG_PAL_ALLOC) == 0) {
        free(m_palette);
        m_palette = nullptr;
        m_flags &= ~TGA_FLAG_PAL_ALLOC;
    }

    char *old_pal = m_palette;
    m_palette = buffer;

    return old_pal;
}

int TargaImage::Decode_Image()
{
    int pixel_bytes = (m_header.pixel_depth + 7) / 8;
    int pixel_count = m_header.width * m_header.height;
    char *putp = m_image;

    while (pixel_count) {
        uint8_t count;

        if (File_Read(&count, sizeof(count)) != sizeof(count)) {
            return TGA_RET_NOT_TGA;
        }

        if ((count & 0x80) != 0) {
            count = (count & 0x7F) + 1;
            char *getp = putp;

            if (File_Read(getp, pixel_bytes) != pixel_bytes) {
                return TGA_RET_NOT_TGA;
            }

            putp += pixel_bytes;

            for (int i = 1; i < count; ++i) {
                memcpy(putp, getp, pixel_bytes);
                putp += pixel_bytes;
            }
        } else {
            ++count;
            int read_size = pixel_bytes * count;

            if (File_Read(putp, read_size) != read_size) {
                return TGA_RET_NOT_TGA;
            }

            putp += read_size;
        }

        pixel_count -= count;
    }

    return TGA_RET_OK;
}

int TargaImage::Encode_Image()
{
    // TODO
    return 0;
}

void TargaImage::Invert_Image()
{
    int pixel_bytes = (m_header.pixel_depth + 7) / 8;
    int pixel_count = m_header.width * m_header.height;

    if (pixel_bytes <= 2) {
        return;
    }

    int half_pixel = pixel_bytes / 2;
    char *curr_pixel = m_image;

    while (pixel_count--) {
        for (int i = 0; i < half_pixel; ++i) {
            char tmp = curr_pixel[i];
            curr_pixel[i] = curr_pixel[pixel_bytes - 1 - i];
            curr_pixel[pixel_bytes - 1 - i] = tmp;
        }

        curr_pixel += pixel_bytes;
    }
}

void TargaImage::Clear_File()
{
    m_TGAFile = nullptr;
}

bool TargaImage::Is_File_Open()
{
    return m_TGAFile != nullptr;
}

bool TargaImage::File_Open_Read(const char *name)
{
    m_TGAFile = g_theFileFactory->Get_File(name);

    if (m_TGAFile != nullptr && m_TGAFile->Is_Available()) {
        return m_TGAFile->Open(FM_READ);
    }

    return false;
}

bool TargaImage::File_Open_Write(const char *name)
{
    m_TGAFile = g_theWritingFileFactory->Get_File(name);

    if (m_TGAFile != nullptr && m_TGAFile->Is_Available()) {
        return m_TGAFile->Open(FM_WRITE);
    }

    return false;
}

bool TargaImage::File_Open_ReadWrite(const char *name)
{
    m_TGAFile = g_theWritingFileFactory->Get_File(name);

    if (m_TGAFile != nullptr && m_TGAFile->Is_Available()) {
        return m_TGAFile->Open(FM_READ_WRITE);
    }

    return false;
}

int TargaImage::Error_Handler(int load_err, const char *filename)
{
    switch (load_err) {
        case TGA_RET_OK:
            return load_err;

        case TGA_RET_UNABLE_TO_LOAD:
            captainslog_error("Targa: Failed to open file \"%s\"", filename);
            return load_err;

        case TGA_RET_NOT_TGA:
            captainslog_error("Targa: Failed to read file \"%s\"", filename);
            return load_err;

        case TGA_RET_UNSUPPORTED:
            captainslog_error("Targa: File \"%s\" is an unsupported Targa type", filename);
            return load_err;

        case TGA_RET_OUT_OF_MEMORY:
            captainslog_error("Targa: Failed to allocate memory for file \"%s\"", filename);
            return load_err;

        default:
            captainslog_error("Targa: Unknown error when loading file '%s'", filename);
            return load_err;
    }
}
