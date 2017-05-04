////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32LOCALFILE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Implementation of LocalFile file io interface. Despite the
//                 class name, the API used is Posix even on windows.
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
#pragma once

#ifndef WIN32LOCALFILE_H
#define WIN32LOCALFILE_H

#include "localfile.h"

class Win32LocalFile : public LocalFile
{
    IMPLEMENT_POOL(Win32LocalFile);
    friend class Win32LocalFileSystem;

public:
    enum
    {
        INVALID_HANDLE = -1
    };

private:
    // Only the factory class, Win32LocalFileSystem can create file instances.
    Win32LocalFile();

public:
    virtual ~Win32LocalFile();

    virtual bool Open(char const *filename, int mode);
    virtual int Read(void *dst, int bytes);
    virtual int Write(void const *src, int bytes);
    virtual int Seek(int offset, File::SeekMode mode);
    virtual void Next_Line(char *dst, int bytes);
    virtual bool Scan_Int(int &integer);
    virtual bool Scan_Real(float &real);
    virtual bool Scan_String(AsciiString &string);

private:
    int FileHandle;
};

#endif // _WIN32LOCALFILE_H
