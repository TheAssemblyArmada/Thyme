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
#pragma once

#ifndef WIN32LOCALFILESYSTEM_H
#define WIN32LOCALFILESYSTEM_H

#include "localfilesystem.h"
#include "win32localfile.h"

class Win32LocalFileSystem : public LocalFileSystem
{
    public:
        Win32LocalFileSystem() { DEBUG_LOG("Creating Win32LocalFileSystem.\n"); }
        virtual ~Win32LocalFileSystem() {}

        // Subsystem interface implementations.
        virtual void Init() {}
        virtual void Reset() {}
        virtual void Update() {}

        // LocalFileSystem interface implementations.
        virtual File *Open_File(const char *filename, int mode);
        virtual bool Does_File_Exist(const char *filename);
        virtual void Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdirs);
        virtual bool Get_File_Info(AsciiString const &filename, FileInfo *info);
        virtual bool Create_Directory(AsciiString);
    private:
        
};
#endif // _WIN32LOCALFILESYSTEM_H
