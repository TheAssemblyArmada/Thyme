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
#pragma once

#include "always.h"
#include "endiantype.h"
#include "wwfile.h"

// Pack all the structures to 1 byte alignment
#pragma pack(push, 1)
struct TGAHeader
{
    int8_t id_length;
    int8_t cmap_type;
    int8_t image_type;
    int16_t cmap_start;
    int16_t cmap_length;
    int8_t cmap_depth;
    int16_t x_offset;
    int16_t y_offset;
    int16_t width;
    int16_t height;
    int8_t pixel_depth;
    int8_t image_descriptor;
};

struct TGA2DateStamp
{
    int16_t month;
    int16_t day;
    int16_t year;
};

struct TGA2TimeStamp
{
    int16_t hour;
    int16_t minute;
    int16_t second;
};

struct TGA2SoftVer
{
    int16_t number;
    char letter;
};

struct TGA2Ratio
{
    int16_t numerator;
    int16_t denominator;
};

struct TGA2Extension
{
    int16_t ext_size;
    char auth_name[41];
    char auth_comment[324];
    TGA2DateStamp date;
    TGA2TimeStamp time;
    char job_name[41];
    TGA2TimeStamp job_time;
    char software_id[41];
    TGA2SoftVer software_vers;
    int32_t key_color;
    TGA2Ratio aspect;
    TGA2Ratio gamma;
    int32_t color_cor;
    int32_t post_stamp;
    int32_t scan_line;
    int8_t attributes;
};

struct TGA2Footer
{
    int32_t extension;
    int32_t developer;
    char signature[16];
    char dot_char;
    char null_char;
};
#pragma pack(pop)

enum TGAReturn
{
    TGA_RET_OK = 0,
    TGA_RET_UNABLE_TO_LOAD = -1,
    TGA_RET_NOT_TGA = -2,
    TGA_RET_UNABLE_TO_SAVE = -3,
    TGA_RET_FILE_NOT_FOUND = -4,
    TGA_RET_OUT_OF_MEMORY = -5,
    TGA_RET_UNSUPPORTED = -6,
};

enum TGAType
{
    TGA_TYPE_NONE = 0,
    TGA_TYPE_MAPPED = 1,
    TGA_TYPE_COLOR = 2,
    TGA_TYPE_GREY = 3,
    TGA_TYPE_RLE_MAPPED = 9,
    TGA_TYPE_RLE_COLOR = 10,
    TGA_TYPE_RLE_GREY = 11,
};

enum TGAFlags
{
    TGA_FLAG_IMAGE_ALLOC = 1,
    TGA_FLAG_PAL_ALLOC = 2,
    TGA_FLAG_COMPRESS = 4,
    TGA_FLAG_INVALID = 8,
};

class TargaImage
{
    ALLOW_HOOKING
public:
    enum
    {
        TARGA_READ = 0,
        TARGA_WRITE = 1,
        TARGA_READWRITE = 2,
    };

    TargaImage();
    ~TargaImage();

    int Open(const char *name, int mode);
    void Close();
    int Load(const char *name, int flags, bool invert_image);
    int Save(const char *name, int flags, bool add_extension);
    char *Set_Image(char *buffer);
    char *Get_Image() { return m_image; }
    char *Set_Palette(char *buffer);
    char *Get_Palette() { return m_palette; }
    bool Is_Compressed() { return m_header.image_type > 8; }
    TGA2Extension *Get_Extension() { return m_flags & 8 ? &m_extension : nullptr; }
    void Clear_File();
    bool Is_File_Open();
    void Toggle_Flip_Y() { m_header.image_descriptor ^= 0x20; }
    const TGAHeader &Get_Header() const { return m_header; }

    static int Error_Handler(int load_err, const char *filename);

private:
    int Load(const char *name, char *palette, char *image, bool invert_image);
    int Decode_Image();
    int Encode_Image();
    void Invert_Image();
    void X_Flip();
    void Y_Flip();
    bool File_Open_Read(const char *name);
    bool File_Open_Write(const char *name);
    bool File_Open_ReadWrite(const char *name);
    int File_Seek(int pos, int dir) { return m_TGAFile->Seek(pos, dir); }
    int File_Read(void *buffer, int size) { return m_TGAFile->Read(buffer, size); }
    int File_Write(void *buffer, int size) { return m_TGAFile->Write(buffer, size); }

    static void Header_To_Host(TGAHeader &header);
    static void Header_To_File(TGAHeader &header);
    static void Extension_To_Host(TGA2Extension &header);
    static void Extension_To_File(TGA2Extension &header);

    TGAHeader m_header;
    FileClass *m_TGAFile;
    int m_access;
    int m_flags;
    char *m_image;
    char *m_palette;
    TGA2Extension m_extension;
};

inline void TargaImage::Header_To_Host(TGAHeader &header)
{
    header.cmap_start = le16toh(header.cmap_start);
    header.cmap_length = le16toh(header.cmap_length);
    header.x_offset = le16toh(header.x_offset);
    header.y_offset = le16toh(header.y_offset);
    header.width = le16toh(header.width);
    header.height = le16toh(header.height);
}

inline void TargaImage::Header_To_File(TGAHeader &header)
{
    header.cmap_start = htole16(header.cmap_start);
    header.cmap_length = htole16(header.cmap_length);
    header.x_offset = htole16(header.x_offset);
    header.y_offset = htole16(header.y_offset);
    header.width = htole16(header.width);
    header.height = htole16(header.height);
}

inline void TargaImage::Extension_To_Host(TGA2Extension &extension)
{
    extension.ext_size = le16toh(extension.ext_size);
    extension.date.day = le16toh(extension.date.day);
    extension.date.month = le16toh(extension.date.month);
    extension.date.year = le16toh(extension.date.year);
    extension.time.hour = le16toh(extension.time.hour);
    extension.time.minute = le16toh(extension.time.minute);
    extension.time.second = le16toh(extension.time.second);
    extension.job_time.hour = le16toh(extension.job_time.hour);
    extension.job_time.minute = le16toh(extension.job_time.minute);
    extension.job_time.second = le16toh(extension.job_time.second);
    extension.software_vers.number = le16toh(extension.software_vers.number);
    extension.key_color = le32toh(extension.key_color);
    extension.aspect.numerator = le16toh(extension.aspect.numerator);
    extension.aspect.denominator = le16toh(extension.aspect.denominator);
    extension.gamma.numerator = le16toh(extension.gamma.numerator);
    extension.gamma.denominator = le16toh(extension.gamma.denominator);
    extension.color_cor = le32toh(extension.color_cor);
    extension.post_stamp = le32toh(extension.post_stamp);
    extension.scan_line = le32toh(extension.scan_line);
}

inline void TargaImage::Extension_To_File(TGA2Extension &extension)
{
    extension.ext_size = htole16(extension.ext_size);
    extension.date.day = htole16(extension.date.day);
    extension.date.month = htole16(extension.date.month);
    extension.date.year = htole16(extension.date.year);
    extension.time.hour = htole16(extension.time.hour);
    extension.time.minute = htole16(extension.time.minute);
    extension.time.second = htole16(extension.time.second);
    extension.job_time.hour = htole16(extension.job_time.hour);
    extension.job_time.minute = htole16(extension.job_time.minute);
    extension.job_time.second = htole16(extension.job_time.second);
    extension.software_vers.number = htole16(extension.software_vers.number);
    extension.key_color = htole32(extension.key_color);
    extension.aspect.numerator = htole16(extension.aspect.numerator);
    extension.aspect.denominator = htole16(extension.aspect.denominator);
    extension.gamma.numerator = htole16(extension.gamma.numerator);
    extension.gamma.denominator = htole16(extension.gamma.denominator);
    extension.color_cor = htole32(extension.color_cor);
    extension.post_stamp = htole32(extension.post_stamp);
    extension.scan_line = htole32(extension.scan_line);
}