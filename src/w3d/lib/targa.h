////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: TARGA.H
//
//        Author:: CCHyper
//
//  Contributors:: OmniBlade
//
//   Description:: Class for handling TARGA images.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _TARGA_H_
#define _TARGA_H_

#include "always.h"
#include "endiantype.h"
#include "fileclass.h"
#include "hooker.h"

//http://www.programminghomeworkhelp.com/c-programming/

// Pack all the structures to 1 byte alignment
#pragma pack(push, 1)
struct TGAHeader
{
    int8_t IDLength;
    int8_t ColorMapType;
    int8_t ImageType;
    int16_t CMapStart;
    int16_t CMapLength;
    int8_t CMapDepth;
    int16_t XOffset;
    int16_t YOffset;
    int16_t Width;
    int16_t Height;
    int8_t PixelDepth;
    int8_t ImageDescriptor;
};

struct TGAHandle
{
    int32_t fh;
    int32_t mode;
    TGAHeader *header;
};

struct TGA2DateStamp
{
    int16_t Month;
    int16_t Day;
    int16_t Year;
};

struct TGA2TimeStamp
{
    int16_t Hour;
    int16_t Minute;
    int16_t Second;
};

struct TGA2SoftVer
{
    int16_t Number;
    char Letter;
};

struct TGA2Ratio
{
    // Numerator
    int16_t Numer;

    // Denominator
    int16_t Denom;
};

struct TGA2Extension
{
    int16_t ExtSize;
    char AuthName[41];
    char AuthComment[324];
    TGA2DateStamp Date;
    TGA2TimeStamp Time;
    char JobName[41];
    TGA2TimeStamp JobTime;
    char SoftID[41];
    TGA2SoftVer SoftVer;
    int32_t KeyColor;
    TGA2Ratio Aspect;
    TGA2Ratio Gamma;
    int32_t ColorCor;
    int32_t PostStamp;
    int32_t ScanLine;
    int8_t Attributes;
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
};

class TargaImage
{
public:
    enum
    {
        TARGA_READ = 0,
        TARGA_WRITE = 1,
        TARGA_READWRITE = 2,
    };

    TargaImage();
    ~TargaImage();
 
    int Open(char const *name, int mode);
    void Close();
    int Load(char const *name, int flags, bool invert_image);
    int Load(char const *name, char* palette, char* image, bool invert_image);
    int Save(char const *name, int flags, bool add_extension);
    void X_Flip();
    void Y_Flip();
    char *Set_Image(char *buffer);
    char *Get_Image() { return Image; }
    char *Set_Palette(char *buffer);
    char *Get_Palette() { return Palette; }
    bool Is_Compressed() { return Header.ImageType > 8; }
    TGA2Extension *Get_Extension() { return Flags & 8 ? &Extension : nullptr; }
    int Decode_Image();
    int Encode_Image();
    void Invert_Image();
    void Clear_File();
    bool Is_File_Open();

    static void Header_To_Host(TGAHeader &header);
    static void Header_To_File(TGAHeader &header);
    static void Extension_To_Host(TGA2Extension &header);
    static void Extension_To_File(TGA2Extension &header);
    static int Targa_Error_Handler(int load_err, const char *filename);

    static void Hook_Me();
private:
    bool File_Open_Read(char const *name);
    bool File_Open_Write(char const *name);
    bool File_Open_ReadWrite(char const *name);
    int File_Seek(int pos, int dir) { return TGAFile->Seek(pos, dir); }
    int File_Read(void* buffer, int size) { return TGAFile->Read(buffer, size); }
    int File_Write(void* buffer, int size) { return TGAFile->Write(buffer, size); }

    TGAHeader Header;
    FileClass *TGAFile;
    int Access;
    int Flags;
    char *Image;
    char *Palette;
    TGA2Extension Extension;
};

inline void TargaImage::Hook_Me()
{
    Hook_Method(Make_Method_Ptr<int, TargaImage, char const*, int, bool>(0x008A43F0), &Load);
}

inline void TargaImage::Header_To_Host(TGAHeader &header)
{
    header.CMapStart = le16toh(header.CMapStart);
    header.CMapLength = le16toh(header.CMapLength);
    header.XOffset = le16toh(header.XOffset);
    header.YOffset = le16toh(header.YOffset);
    header.Width = le16toh(header.Width);
    header.Height = le16toh(header.Height);
}

inline void TargaImage::Header_To_File(TGAHeader &header)
{
    header.CMapStart = htole16(header.CMapStart);
    header.CMapLength = htole16(header.CMapLength);
    header.XOffset = htole16(header.XOffset);
    header.YOffset = htole16(header.YOffset);
    header.Width = htole16(header.Width);
    header.Height = htole16(header.Height);
}

inline void TargaImage::Extension_To_Host(TGA2Extension &extension)
{
    extension.ExtSize = le16toh(extension.ExtSize);
    extension.Date.Day = le16toh(extension.Date.Day);
    extension.Date.Month = le16toh(extension.Date.Month);
    extension.Date.Year = le16toh(extension.Date.Year);
    extension.Time.Hour = le16toh(extension.Time.Hour);
    extension.Time.Minute = le16toh(extension.Time.Minute);
    extension.Time.Second = le16toh(extension.Time.Second);
    extension.JobTime.Hour = le16toh(extension.JobTime.Hour);
    extension.JobTime.Minute = le16toh(extension.JobTime.Minute);
    extension.JobTime.Second = le16toh(extension.JobTime.Second);
    extension.SoftVer.Number = le16toh(extension.SoftVer.Number);
    extension.KeyColor = le32toh(extension.KeyColor);
    extension.Aspect.Numer = le16toh(extension.Aspect.Numer);
    extension.Aspect.Denom = le16toh(extension.Aspect.Denom);
    extension.Gamma.Numer = le16toh(extension.Gamma.Numer);
    extension.Gamma.Denom = le16toh(extension.Gamma.Denom);
    extension.ColorCor = le32toh(extension.ColorCor);
    extension.PostStamp = le32toh(extension.PostStamp);
    extension.ScanLine = le32toh(extension.ScanLine);
}

inline void TargaImage::Extension_To_File(TGA2Extension &extension)
{
    extension.ExtSize = htole16(extension.ExtSize);
    extension.Date.Day = htole16(extension.Date.Day);
    extension.Date.Month = htole16(extension.Date.Month);
    extension.Date.Year = htole16(extension.Date.Year);
    extension.Time.Hour = htole16(extension.Time.Hour);
    extension.Time.Minute = htole16(extension.Time.Minute);
    extension.Time.Second = htole16(extension.Time.Second);
    extension.JobTime.Hour = htole16(extension.JobTime.Hour);
    extension.JobTime.Minute = htole16(extension.JobTime.Minute);
    extension.JobTime.Second = htole16(extension.JobTime.Second);
    extension.SoftVer.Number = htole16(extension.SoftVer.Number);
    extension.KeyColor = htole32(extension.KeyColor);
    extension.Aspect.Numer = htole16(extension.Aspect.Numer);
    extension.Aspect.Denom = htole16(extension.Aspect.Denom);
    extension.Gamma.Numer = htole16(extension.Gamma.Numer);
    extension.Gamma.Denom = htole16(extension.Gamma.Denom);
    extension.ColorCor = htole32(extension.ColorCor);
    extension.PostStamp = htole32(extension.PostStamp);
    extension.ScanLine = htole32(extension.ScanLine);
}

#endif // _TARGA_H_
