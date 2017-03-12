////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: FILESYSTEM.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Filesystem abstraction merging local and archive file
//                 handling.
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

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "subsysteminterface.h"
#include "file.h"
#include "rtsutils.h"
#include <set>
#include <map>

#define TheFileSystem (Make_Global<FileSystem*>(0x00A2B670))

class FileSystem : public SubsystemInterface
{
    public:
        FileSystem();
        virtual ~FileSystem();

        // SubsystemInterface implementations
        virtual void Init();
        virtual void Reset();
        virtual void Update();

        // Filesystem
        File *Open(char const *filename, int mode);
        bool Does_File_Exist(char const *filename);
        void Get_File_List_From_Dir(AsciiString const &dir, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool a5);

    private:
        std::map<unsigned int, bool> AvailableFiles;
};

#endif // _FILESYSTEM_H_
