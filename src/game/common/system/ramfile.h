////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: RAMFILE.H
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
#pragma once

#include "file.h"

class RAMFile : public File
{
    IMPLEMENT_POOL(RAMFile);

    public:
        RAMFile();
        virtual ~RAMFile();

        virtual bool Open(const char *filename, int mode);
        virtual void Close();
        virtual int Read(void *dst, int bytes);
        virtual int Write(void const *src, int bytes);
        virtual int Seek(int offset, File::SeekMode mode);
        virtual void Next_Line(char *dst, int bytes);
        virtual bool Scan_Int(int &integer);
        virtual bool Scan_Real(float &real);
        virtual bool Scan_String(AsciiString &string);

        virtual void *Read_All_And_Close();
        virtual RAMFile *Convert_To_RAM() { return this; }
        virtual bool Open(File *file);
        virtual bool Open_From_Archive(File *file, AsciiString const &name, int pos, int size);
        virtual bool Copy_To_File(File *file);

    protected:
        char *Data;
        int Pos;
        int Size;
};