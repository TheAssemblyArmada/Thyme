////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: LOCALFILESYSTEM.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Interface for local file system implementations.
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

#ifndef _LOCALFILESYSTEM_H_
#define _LOCALFILESYSTEM_H_

#include "subsysteminterface.h"
#include "rtsutils.h"
#include <set>

class LocalFile;
struct FileInfo;

class LocalFileSystem : public SubsystemInterface
{
    public:
        virtual ~LocalFileSystem() {}

        virtual LocalFile *Open_File(char const *filename, int mode) = 0;
        virtual bool Does_File_Exist(char const *filename) = 0;
        virtual void Get_File_List_From_Dir(AsciiString &a1, AsciiString &a2, AsciiString &a3, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool a5) = 0;
        virtual bool Get_File_Info(AsciiString const &filename, FileInfo *info) = 0;
        virtual bool Create_Directory(AsciiString) = 0;
};

#endif
