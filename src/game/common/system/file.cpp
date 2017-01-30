////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: FILE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Base class for file IO.
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
#include "file.h"

File::File() :
    FileName("<no file>"),
    OpenMode(0),
    Access(false),
    DeleteOnClose(false)
{

}

File::~File()
{
    File::Close();
}

bool File::Open(char const *filename, int mode)
{
    if ( Access ) {
        return false;
    }

    FileName = filename;

    if ( (mode & 0x102) == 0x102 ) {
        return false;
    }

    if ( (mode & 0x60) == 0x60 ) {
        return false;
    }

    if ( (mode & 3) == 0 ) {
        OpenMode = mode | 1;
    }

    if ( (mode & 5) == 0 ) {
        OpenMode |= 0x10;
    }

    if ( (mode & 0x60) == 0 ) {
        OpenMode |= 0x40;
    }

    Access = true;

    return true;
}

void File::Close()
{
    if ( !Access ) {
        return;
    }

    FileName = "<no file>";

    Access = false;

    if ( DeleteOnClose ) {
        Delete_Instance(this);
    }
}

bool File::Print(char const *format, ...)
{
    va_list va;
    char buffer[10240];

    va_start(va, format);

    if ( OpenMode & 0x20 ) {
        // Format our message to be written out
        int length = vsnprintf(buffer, 10240, format, va);

        // Only write if we didn't truncate due to buffer overrun.
        if ( length < 10240 ) {
            return Write(buffer, length) == length;
        }
    }

    return false;
}

int File::Size()
{
    int current = Seek(0, CURRENT);
    int end = Seek(0, END);
    Seek(current, START);

    return end < 0 ? 0 : end;
}

int File::Position()
{
    return Seek(0, CURRENT);
}
