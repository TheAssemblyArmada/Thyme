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
#include "minmax.h"

INITIALISE_POOL(RAMFile);

RAMFile::RAMFile() :
    Data(nullptr),
    Pos(0),
    Size(0)
{

}

RAMFile::~RAMFile()
{

}

bool RAMFile::Open(char const *filename, int mode)
{
    return false; //TODO
}

void RAMFile::Close()
{
    if ( Data != nullptr ) {
        delete[] Data;
    }

    File::Close();
}

int RAMFile::Read(void *dst, int bytes)
{
    if ( dst == nullptr ) {
        return -1;
    }

    // Clip the amount to read to be within the data remaining.
    int data_read = MIN(bytes, Size - Pos);

    if ( data_read > 0 ) {
        memcpy(dst, Data + Pos, data_read);
    }

    Pos += data_read;

    return data_read;
}

int RAMFile::Write(void *src, int bytes)
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

    // Don't seek to ourside the file.
    Pos = MAX(0, MIN(Pos, Size));

    return Pos;
}

void RAMFile::Next_Line(char *dst, int bytes)
{

}

bool RAMFile::Scan_Int(int &integer)
{

}

bool RAMFile::Scan_Real(float &real)
{

}

bool RAMFile::Scan_String(AsciiString &string)
{

}

void *RAMFile::Read_All_And_Close()
{

}

bool RAMFile::Open(File *file)
{
    if ( file == nullptr ) {
        return false;
    }

    if ( File::Open(file->Get_File_Name().Str(), file->Get_File_Mode()) ) {
        Size = file->Size();
        Data = new uint8_t[Size];

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
        Data = new uint8_t[size];

        if ( file->Seek(pos, START) == pos ) {
            if ( file->Read(Data, Size) == size ) {
                FileName = file->Get_File_Name();

                return true;
            }
        }
    }

    return false;
}
