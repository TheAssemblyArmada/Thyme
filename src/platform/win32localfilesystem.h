////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32LOCALFILESYSTEM.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Implements the LocalFileSystem interface.
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

#ifndef _WIN32LOCALFILESYSTEM_H_
#define _WIN32LOCALFILESYSTEM_H_

#include "localfilesystem.h"
#include "win32localfile.h"

class Win32LocalFileSystem : public LocalFileSystem
{
    public:
        Win32LocalFileSystem() {}
        virtual ~Win32LocalFileSystem() {}

        // Subsystem interface implementations.
        virtual void Init() {}
        virtual void Reset() {}
        virtual void Update() {}

        // LocalFileSystem interface implementations.
        virtual File *Open_File(char const *filename, int mode);
        virtual bool Does_File_Exist(char const *filename);
        virtual void Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdirs);
        virtual bool Get_File_Info(AsciiString const &filename, FileInfo *info);
        virtual bool Create_Directory(AsciiString);
    private:
        
};
#endif // _WIN32LOCALFILESYSTEM_H_
