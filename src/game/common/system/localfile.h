////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: LOCALFILE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Cross platform class for file IO.
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

#ifndef _LOCALFILE_H_
#define _LOCALFILE_H_

#include "always.h"
#include "file.h"

class RAMFile;

#define TotalOpen (Make_Global<int>(0x00A3E31C))

class LocalFile : public File
{
    public:
        enum
        {
            INVALID_HANDLE = -1
        };

        LocalFile();
        virtual ~LocalFile();

        virtual MemoryPool *Get_Object_Pool() = 0;

        virtual bool Open(char const *filename, int mode);
        virtual void Close();
        virtual int Read(void *dst, int bytes);
        virtual int Write(void *src, int bytes);
        virtual int Seek(int offset, File::SeekMode mode);
        virtual void Next_Line(char *dst, int bytes);
        virtual bool Scan_Int(int &integer);
        virtual bool Scan_Real(float &real);
        virtual bool Scan_String(AsciiString &string);

        virtual void *Read_All_And_Close();
        virtual RAMFile *Convert_To_RAM();

    protected:
        int FileHandle;
};

#endif // _LOCALFILE_H_