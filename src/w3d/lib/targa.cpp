////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: TARGA.CPP
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
#include "targa.h"
#include "ffactory.h"
#include "hookcrt.h"
#include "gamedebug.h"

TargaImage::TargaImage() :
    TGAFile(nullptr),
    Access(0),
    Flags(0),
    Image(nullptr),
    Palette(nullptr)
{
    memset(&Header, 0, sizeof(Header));
    memset(&Extension, 0, sizeof(Extension));
}

TargaImage::~TargaImage()
{
    Close();

    if ( Palette != nullptr && Flags & 2 ) {
        crt_free(Palette);
        Palette = nullptr;
    }

    if ( Image != nullptr && Flags & 1 ) {
        crt_free(Image);
        Image = nullptr;
    }
}

int TargaImage::Open(char const *name, int mode)
{
    if ( Is_File_Open() && Access == mode ) {
        return 0;
    }

    Close();
    Flags &= ~8;
    Access = mode;

    TGA2Footer footer;

    switch ( mode ) {
        case TARGA_READ:
            if ( !File_Open_Read(name) ) {
                Close();

                return TGA_RET_UNABLE_TO_LOAD;
            }

            // If we can't read enough data for a footer, not a TGA file.
            if ( File_Seek(-((int)sizeof(footer)), FileSeekType::FS_SEEK_END) == -1
                || File_Read(&footer, sizeof(footer)) != sizeof(footer)
            ) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            // If we don't have the correct footer info, not a TGA file.
            if ( strncasecmp(footer.signature, "TRUEVISION-XFILE", sizeof(footer.signature))
                && footer.extension != 0
            ) {
                Flags |= 8;
            }

            footer.extension = le32toh(footer.extension);

            // If we can't seek to the extension offset or we can't read enough data,
            // not a TGA file.
            if ( (Flags & 8) != 0
                && (File_Seek(footer.extension, FileSeekType::FS_SEEK_START) != -1
                || File_Read(&Extension, sizeof(Extension) != sizeof(Extension)))
            ) {
                Close();

                return TGA_RET_NOT_TGA;
            } else if ( File_Seek(0, FileSeekType::FS_SEEK_START) == -1 ) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            // Convert endian if needed.
            Extension_To_Host(Extension);

            // If file is shorter than header then its not a TGA.
            if ( File_Read(&Header, sizeof(Header)) != sizeof(Header) ) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            // Convert endian if needed.
            Header_To_Host(Header);

            // If file seek past ID fails, then not a TGA.
            if ( Header.IDLength != 0 && File_Seek(Header.IDLength, FileSeekType::FS_SEEK_CURRENT) == -1 ) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            break;
        case TARGA_WRITE:
            if ( !File_Open_Write(name) ) {
                Close();

                return TGA_RET_UNABLE_TO_LOAD;
            }

            break;
        case TARGA_READWRITE:
            if ( !File_Open_ReadWrite(name) ) {
                Close();

                return TGA_RET_UNABLE_TO_LOAD;
            }

            // If file is shorter than header then its not a TGA.
            if ( File_Read(&Header, sizeof(Header)) != sizeof(Header) ) {
                Close();

                return TGA_RET_NOT_TGA;
            }

            Header_To_Host(Header);

            // If file seek past ID fails, then not a TGA.
            if ( Header.IDLength != 0 && File_Seek(Header.IDLength, FileSeekType::FS_SEEK_CURRENT) == -1 ) {
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
    if ( TGAFile != nullptr ) {
        TGAFile->Close();
        TheFileFactory->Return_File(TGAFile);
        Clear_File();
    }
}

int TargaImage::Load(char const *name, int flags, bool invert_image)
{
    if ( Open(name, TARGA_READ) != TGA_RET_OK ) {
        return TGA_RET_UNABLE_TO_LOAD;
    }

    if ( (flags & TGA_FLAG_PAL_ALLOC) != 0 && Header.ColorMapType == 1 ) {
        if ( Palette != nullptr && (Flags & TGA_FLAG_PAL_ALLOC) == 0 ) {
            crt_free(Palette);
            Palette = nullptr;
            Flags &= ~TGA_FLAG_PAL_ALLOC;
        }

        if ( Palette == nullptr && (Flags & TGA_FLAG_PAL_ALLOC) == 0 ) {
            if ( (Header.CMapLength * (Header.CMapDepth / 8)) > 0 ) {
                Palette = (char *)crt_malloc(Header.CMapLength * (Header.CMapDepth / 8));
            }

            if ( Palette == nullptr ) {
                Close();

                return TGA_RET_OUT_OF_MEMORY;
            }

            Flags |= 2;
        }
    }

    if ( (flags & TGA_FLAG_IMAGE_ALLOC) != 0 ) {
        if ( Image != nullptr && (Flags & TGA_FLAG_IMAGE_ALLOC) == 0 ) {
            crt_free(Image);
            Image = nullptr;
            Flags &= ~TGA_FLAG_IMAGE_ALLOC;
        }

        if ( Image == nullptr && (Flags & TGA_FLAG_IMAGE_ALLOC) == 0 ) {
            if ( (Header.Width * Header.Height * ((Header.PixelDepth + 7) / 8)) > 0 ) {
                Image = (char*)crt_malloc(Header.Width * Header.Height * ((Header.PixelDepth + 7) / 8));
            }

            if ( Image == nullptr ) {
                Close();

                return TGA_RET_OUT_OF_MEMORY;
            }

            Flags |= 1;
        }
    }

    int ret = Load(name, Palette, Image, invert_image);
    Close();

    return ret;
}

int TargaImage::Load(char const *name, char *palette, char *image, bool invert_image)
{
    if ( Open(name, TARGA_READ) != TGA_RET_OK ) {
        return TGA_RET_UNABLE_TO_LOAD;
    }

    if ( Header.ColorMapType == 1 ) {
        int pixel_bytes = Header.CMapDepth / 8;

        if ( palette != nullptr && Header.CMapLength > 0 ) {
            int read_bytes = File_Read(
                &palette[pixel_bytes * Header.CMapStart],
                pixel_bytes * Header.CMapLength
            );

            if ( read_bytes != pixel_bytes * Header.CMapLength ) {
                Close();

                return TGA_RET_NOT_TGA;
            }
        } else if ( File_Seek(pixel_bytes * Header.CMapLength, FileSeekType::FS_SEEK_CURRENT) == -1 ) {
            Close();

            return TGA_RET_NOT_TGA;
        }
    }

    if ( image != nullptr ) {
        int total_bytes = Header.Width * Header.Height * ((Header.PixelDepth + 7) / 8);

        switch ( Header.ImageType ) {
            case TGA_TYPE_MAPPED:
            case TGA_TYPE_COLOR:
            case TGA_TYPE_GREY:
                if ( File_Read(image, total_bytes) != total_bytes ) {
                    Close();

                    return TGA_RET_NOT_TGA;
                }
                break;
            case TGA_TYPE_RLE_MAPPED:
            case TGA_TYPE_RLE_COLOR:
            {
                int ret = Decode_Image();

                if ( ret != TGA_RET_OK ) {
                    return ret;
                }
            }
                break;
            default:
                Close();
                return TGA_RET_UNSUPPORTED;
        }

        if ( invert_image ) {
            Invert_Image();
        }

        if ( (Header.ImageDescriptor & 0x10) != 0 ) {
            DEBUG_LOG("Flipping X.\n");
            X_Flip();
            Header.ImageDescriptor &= ~0x10;
        }

        if ( (Header.ImageDescriptor & 0x20) != 0 ) {
            DEBUG_LOG("Flipping Y.\n");
            Y_Flip();
            Header.ImageDescriptor &= ~0x20;
        }
    }
    Close();

    return TGA_RET_OK;
}

int TargaImage::Save(char const *name, int flags, bool add_extension)
{
    if ( Open(name, TARGA_WRITE) != TGA_RET_OK ) {
        return TGA_RET_UNABLE_TO_LOAD;
    }

    Header.IDLength = 0;

    // Flag can force compression even if its not currently set.
    if ( (flags & TGA_FLAG_COMPRESS) != 0 ) {
        switch ( Header.ImageType ) {
            case TGA_TYPE_MAPPED:
            case TGA_TYPE_COLOR:
            case TGA_TYPE_GREY:
                Header.ImageType += 8;  // Convert to RLE type then fall through
            case TGA_TYPE_RLE_MAPPED:
            case TGA_TYPE_RLE_COLOR:
            case TGA_TYPE_RLE_GREY:
                break;
            default:
                flags &= ~TGA_FLAG_COMPRESS;
                break;
        }
    }

    if ( File_Write(&Header, sizeof(Header)) != sizeof(Header) ) {
        Close();

        return TGA_RET_UNABLE_TO_SAVE;
    }

    // Write palette if we have one.
    if ( (flags & TGA_FLAG_PAL_ALLOC) != 0 ) {
        if ( Palette != nullptr && Header.CMapLength > 0 ) {
            int pal_depth = Header.CMapDepth / 8;
            int pal_size = pal_depth * Header.CMapLength;

            if ( File_Write(&Palette[pal_depth * Header.CMapStart], pal_size) != pal_size ) {
                Close();

                return TGA_RET_UNABLE_TO_SAVE;
            }
        }
    }

    // Write the image, flipping if color and flipping back afterward, compressing if required
    if ( (flags & TGA_FLAG_IMAGE_ALLOC) != 0  && Image != nullptr ) {
        bool inverted = false;
        
        if ( Header.ImageType == TGA_TYPE_COLOR || Header.ImageType == TGA_TYPE_RLE_COLOR ) {
            Invert_Image();
            inverted = true;
        }

        if ( (flags & TGA_FLAG_COMPRESS) != 0 ) {
            Encode_Image();
        } else {
            int img_size = Header.Width * Header.Height * ((Header.PixelDepth + 7) / 8);

            if ( File_Write(Image, img_size) != img_size ) {
                Close();

                return TGA_RET_UNABLE_TO_SAVE;
            }
        }

        if ( inverted ) {
            Invert_Image();
        }
    }

    TGA2Footer footer;

    // Write the extension if required, if not just write the footer.
    if ( add_extension ) {
        Extension.ExtSize = sizeof(Extension);
        strncpy(Extension.SoftID, "Thyme Game Engine", sizeof(Extension.SoftID));
        Extension.SoftVer.Number = 1;
        Extension.SoftVer.Letter = '\0';
        footer.extension = File_Seek(0, FileSeekType::FS_SEEK_CURRENT);

        if ( footer.extension == -1 || File_Write(&Extension, sizeof(Extension)) != sizeof(Extension)) {
            Close();

            return TGA_RET_UNABLE_TO_SAVE;
        }
    } else {
        footer.extension = 0;
    }

    strncpy(footer.signature, "TRUEVISION-XFILE", sizeof(footer.signature));
    footer.dot_char = '.';
    footer.null_char = '\0';

    if ( File_Write(&footer, sizeof(footer)) != sizeof(footer) ) {
        Close();

        return TGA_RET_UNABLE_TO_SAVE;
    }

    Close();

    return TGA_RET_OK;
}

void TargaImage::X_Flip()
{
    int pixel_size = (Header.PixelDepth + 7) / 8;

    // Must have at least one row of pixels.
    if ( Header.Height <= 0 ) {
        return;
    }

    // Iterate over the image and move bytes from start of a row to the
    // end and vice versa, keeping the order of bytes within the pixel.
    for ( int y = 0; y < Header.Height; ++y ) {
        char *startp = &Image[y * Header.Width * pixel_size];
        char *endp = &startp[pixel_size * (Header.Width - 1)];

        for ( int x = 0; x < (Header.Width / 2); ++x ) {
            if ( pixel_size > 0 ) {
                char *modp = endp;
                ptrdiff_t diff = startp - endp;

                for ( int i = pixel_size; i > 0; --i ) {
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
    int pixel_size = (Header.PixelDepth + 7) / 8;

    // Height must be a multiple of 2 to be flippable.
    if ( (Header.Height % 2) != 0 ) {
        return;
    }

    for ( int y = 0; y < (Header.Height / 2); ++y ) {
        // Set pointers to the start and end rows.
        char *startp = &Image[Header.Width * pixel_size * y];
        char *endp = &Image[Header.Width * pixel_size * (Header.Height - 1)] - (Header.Width * pixel_size * y);

        // Iterate over the rows, swapping bytes between them.
        for ( int x = 0; x < (pixel_size * Header.Width); ++x ) {
            char tmp = startp[x];
            startp[x] = endp[x];
            endp[x] = tmp;
        }
    }
}

char *TargaImage::Set_Image(char *buffer)
{
    if ( Image != nullptr && (Flags & TGA_FLAG_IMAGE_ALLOC) == 0 ) {
        crt_free(Image);
        Image = nullptr;
        Flags &= ~TGA_FLAG_IMAGE_ALLOC;
    }

    char *old_image = Image;
    Image = buffer;

    return old_image;
}

char *TargaImage::Set_Palette(char *buffer)
{
    if ( Palette != nullptr && (Flags & TGA_FLAG_PAL_ALLOC) == 0 ) {
        crt_free(Palette);
        Palette = nullptr;
        Flags &= ~TGA_FLAG_PAL_ALLOC;
    }

    char *old_pal = Palette;
    Palette = buffer;

    return old_pal; return nullptr;
}

int TargaImage::Decode_Image()
{
    DEBUG_LOG("Decoding RLE encoded image data.\n");
    int pixel_bytes = (Header.PixelDepth + 7) / 8;
    int pixel_count = Header.Width * Header.Height;
    char *putp = Image;

    while ( pixel_count ) {
        uint8_t count;

        if ( File_Read(&count, sizeof(count)) != sizeof(count) ) {
            return TGA_RET_NOT_TGA;
        }

        if ( (count & 0x80) != 0 ) {
            count = (count & 0x7F) + 1;
            char *getp = putp;

            if ( File_Read(getp, pixel_bytes) != sizeof(pixel_bytes) ) {
                return TGA_RET_NOT_TGA;
            }

            putp += pixel_bytes;

            for ( int i = 1; i < count; ++i ) {
                memcpy(putp, getp, pixel_bytes);
                putp += pixel_bytes;
            }
        } else {
            int read_size = pixel_bytes * count;

            if ( File_Read(putp, read_size) != read_size ) {
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
    return 0;
}

void TargaImage::Invert_Image()
{
    int pixel_bytes = (Header.PixelDepth + 7) / 8;
    int pixel_count = Header.Width * Header.Height;

    if ( pixel_bytes <= 2 ) {
        return;
    }

    int half_pixel = pixel_bytes / 2;
    char *curr_pixel = Image;

    while ( pixel_count-- ) {
        for ( int i = 0; i < half_pixel; ++i ) {
            char tmp = curr_pixel[i];
            curr_pixel[i] = curr_pixel[pixel_bytes - 1 - i];
            curr_pixel[pixel_bytes - 1 - i] = tmp;
        }

        curr_pixel += pixel_bytes;
    }
}

void TargaImage::Clear_File()
{
    TGAFile = nullptr;
}

bool TargaImage::Is_File_Open()
{
    return TGAFile != nullptr;
}

bool TargaImage::File_Open_Read(char const *name)
{
    TGAFile = TheFileFactory->Get_File(name);

    if ( TGAFile != nullptr && TGAFile->Is_Available() ) {
        return TGAFile->Open(FM_READ);
    }

    return false;
}

bool TargaImage::File_Open_Write(char const *name)
{
    TGAFile = TheWritingFileFactory->Get_File(name);

    if ( TGAFile != nullptr && TGAFile->Is_Available() ) {
        return TGAFile->Open(FM_WRITE);
    }

    return false;
}

bool TargaImage::File_Open_ReadWrite(char const *name)
{
    TGAFile = TheWritingFileFactory->Get_File(name);

    if ( TGAFile != nullptr && TGAFile->Is_Available() ) {
        return TGAFile->Open(FM_READ_WRITE);
    }

    return false;
}

int TargaImage::Targa_Error_Handler(int load_err, char const *filename)
{
    switch ( load_err ) {
        case TGA_RET_OK:
            return load_err;

        case TGA_RET_UNABLE_TO_LOAD:
            DEBUG_LOG("Targa: Failed to open file \"%s\"\n", filename);
            return load_err;

        case TGA_RET_NOT_TGA:
            DEBUG_LOG("Targa: Failed to read file \"%s\"\n", filename);
            return load_err;

        case TGA_RET_UNSUPPORTED:
            DEBUG_LOG("Targa: File \"%s\" is an unsupported Targa type\n", filename);
            return load_err;

        case TGA_RET_OUT_OF_MEMORY:
            DEBUG_LOG("Targa: Failed to allocate memory for file \"%s\"\n", filename);
            return load_err;

        default:
            DEBUG_LOG("Targa: Unknown error when loading file '%s'\n", filename);
            return load_err;
    }
}
