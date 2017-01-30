////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: FILE.H
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _FILE_H_
#define _FILE_H_

#include "asciistring.h"
#include "gamememory.h"

struct FileInfo
{
    int FileSizeHigh;
    int FileSizeLow;
    int WriteTimeHigh;
    int WriteTimeLow;
};

class File : public MemoryPoolObject
{
    public:
        enum SeekMode {
            START   = 0,
            CURRENT = 1,
            END     = 2,
        };

        enum FileMode
        {
            READ = 0x01,
            WRITE = 0x02,
            APPEND = 0x04,
            CREATE = 0x08,
            TRUNCATE = 0x10,
            TEXT = 0x20,
            BINARY = 0x40,
            STREAMING = 0x100,
        };

        File();
        virtual ~File();

        virtual bool Open(char const *filename, int mode);
        virtual void Close();
        virtual int Read(void *dst, int bytes) = 0;
        virtual int Write(void *src, int bytes) = 0;
        virtual int Seek(int offset, File::SeekMode mode) = 0;
        virtual void Next_Line(char *dst, int bytes) = 0;
        virtual bool Scan_Int(int &integer) = 0;
        virtual bool Scan_Real(float &real) = 0;
        virtual bool Scan_String(AsciiString &string) = 0;
        virtual bool Print(char const *format, ...);
        virtual int Size();
        virtual int Position();

        AsciiString &Get_File_Name() { return FileName; }
        int Get_File_Mode() { return OpenMode; }
        void Set_Del_On_Close(bool del) { DeleteOnClose = del;  }

    protected:
        AsciiString FileName;
        int OpenMode;
        bool Access;
        bool DeleteOnClose;
};

#endif // _FILE_H_