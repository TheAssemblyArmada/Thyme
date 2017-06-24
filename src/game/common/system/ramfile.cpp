////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: RAMFILE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: RAM file IO.
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
#include "ramfile.h"
#include "filesystem.h"
#include "minmax.h"

RAMFile::RAMFile() :
    Data(nullptr),
    Pos(0),
    Size(0)
{

}

RAMFile::~RAMFile()
{
    if ( Data != nullptr ) {
        delete[] Data;
    }
}

bool RAMFile::Open(const char *filename, int mode)
{
    File *basefile = g_theFileSystem->Open(filename, mode);

    if ( basefile == nullptr ) {
        return false;
    }

    bool retval = Open(basefile);
    basefile->Close();

    return retval;
}

void RAMFile::Close()
{
    if ( Data != nullptr ) {
        delete[] Data;
        Data = nullptr;
    }

    File::Close();
}

int RAMFile::Read(void *dst, int bytes)
{
    if ( dst == nullptr ) {
        return -1;
    }

    // Clip the amount to read to be within the data remaining.
    bytes = MIN(bytes, Size - Pos);

    if ( bytes > 0 ) {
        memcpy(dst, Data + Pos, bytes);
    }

    Pos += bytes;

    return bytes;
}

int RAMFile::Write(void const *src, int bytes)
{
    // No writing to RAM files.
    return - 1;
}

int RAMFile::Seek(int offset, File::SeekMode mode)
{
    switch ( mode ) {
        case START:
            Pos = offset;
            break;

        case CURRENT:
            Pos += offset;
            break;

        case END:
            Pos = offset + Size;
            break;

        default:
            return -1;
    }

    // Don't seek to outside the file.
    Pos = Clamp(Pos, 0, Size); // MAX(0, MIN(Pos, Size));

    return Pos;
}

void RAMFile::Next_Line(char *dst, int bytes)
{
    int i = 0;

    // Copy data until newline.
    for ( ; Pos < Size; ++Pos ) {
        char tmp = Data[Pos];

        if ( tmp == '\n' ) {
            break;
        }

        if ( dst != nullptr && i < bytes - 1 ) {
            dst[i++] = tmp;
        }
    }

    // If we broke on newline, increment past it.
    if ( Pos < Size ) {
        // Copy newline to destination if not full?
        if ( dst != nullptr &&  i < bytes ) {
            dst[i++] = Data[Pos];
        }

        ++Pos;
    }

    // If we have a data pointer to copy to, make sure its null terminiated
    if ( dst != nullptr ) {
        if ( i >= bytes ) {
            dst[bytes] = '\0';
        } else {
            dst[i] = '\0';
        }
    }

    // Make sure our position is still within data.
    Pos = MIN(Pos, Size);
}

bool RAMFile::Scan_Int(int &integer)
{
    char tmp;
    AsciiString number;

    integer = 0;

    if ( Pos >= Size ) {
        Pos = Size;

        return false;
    }

    // Find first digit or '-' symbol.
    do {
        tmp = Data[Pos];

        if ( isdigit(tmp) || tmp == '-' ) {
            break;
        }

    } while ( ++Pos < Size );

    if ( Pos >= Size ) {
        Pos = Size;

        return false;
    }

    for ( tmp = Data[Pos]; Pos < Size && isdigit(tmp); tmp = Data[++Pos] ) {
        number.Concat(tmp);
    }

    integer = atoi(number.Str());

    return true;
}

bool RAMFile::Scan_Real(float &real)
{
    char tmp;
    AsciiString number;

    real = 0.0f;

    if ( Pos >= Size ) {
        Pos = Size;

        return false;
    }

    // Find first digit, '-' or '.' symbol.
    do {
        tmp = Data[Pos];

        if ( isdigit(tmp) || tmp == '-' || tmp == '.') {
            break;
        }

    } while ( ++Pos < Size );

    // Check we are still in bounds
    if ( Pos >= Size ) {
        Pos = Size;

        return false;
    }

    bool have_point = false;

    for ( tmp = Data[Pos]; Pos < Size && (isdigit(tmp) || ( tmp == '.' && !have_point)); tmp = Data[++Pos] ) {
        number.Concat(tmp);

        if ( tmp == '.' ) {
            have_point = true;
        }
    }

    real = atof(number.Str());

    return true;
}

bool RAMFile::Scan_String(AsciiString &string)
{
    string.Clear();

    // Find first none space.
    for ( ; Pos < Size; ++Pos ) {
        if ( !isspace(Data[Pos]) ) {
            break;
        }
    }

    // Check we are still in bounds
    if ( Pos >= Size ) {
        Pos = Size;

        return false;
    }

    // Read into AsciiString
    for ( ; Pos < Size; ++Pos ) {
        if ( isspace(Data[Pos]) ) {
            break;
        }

        string.Concat(Data[Pos]);
    }

    return true;
}

void *RAMFile::Read_All_And_Close()
{
    char *data;

    if ( Data != nullptr ) {
        data = Data;
        Data = nullptr;
        Close();
    } else {
        // Valid pointer return seems expected.
        data = new char[1];
    }

    return data;
}

bool RAMFile::Open(File *file)
{
    if ( file == nullptr ) {
        return false;
    }

    if ( File::Open(file->Get_File_Name().Str(), file->Get_File_Mode()) ) {
        Size = file->Size();
        Data = new char[Size];

        if ( Data != nullptr ) {
            // Read the entire file into our buffer.
            Size = file->Read(Data, Size);
            
            // If we didn't read any data into our buffer, abort.
            if ( Size >= 0 ) {
                Pos = 0;

                return true;
            } else {
                delete[] Data;
                Data = nullptr;
            }
        }
    }

    return false;
}

bool RAMFile::Open_From_Archive(File *file, AsciiString const &name, int pos, int size)
{
    if ( file == nullptr ) {
        return false;
    }

    if ( File::Open(name.Str(), READ | BINARY) ) {
        if ( Data != nullptr ) {
            delete[] Data;
            Data = nullptr;
        }

        Size = size;
        Data = new char[size];

        if ( file->Seek(pos, START) == pos ) {
            if ( file->Read(Data, Size) == size ) {
                FileName = file->Get_File_Name();

                return true;
            }
        }
    }

    return false;
}

bool RAMFile::Copy_To_File(File *file)
{
    return file != nullptr && file->Write(Data, Size) == Size;
}
