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
#pragma once

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "subsysteminterface.h"
#include "file.h"
#include "rtsutils.h"
#include <set>
#include <map>

#define g_theFileSystem (Make_Global<FileSystem*>(0x00A2B670))

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

    static bool Create_Dir(AsciiString name);
    static bool Are_Music_Files_On_CD();
    static bool Load_Music_Files_From_CD();
    static void Unload_Music_Files_From_CD();

    static void Hook_Me();
private:
    std::map<unsigned int, bool> m_availableFiles;
};

inline void FileSystem::Hook_Me()
{
    Hook_Function((Make_Function_Ptr<bool>(0x004469C0)), Are_Music_Files_On_CD);
    Hook_Method(Make_Method_Ptr<void, FileSystem, AsciiString const&, AsciiString const&, std::set<AsciiString, rts::less_than_nocase<AsciiString>>&, bool>(0x00446770), &Get_File_List_From_Dir);
}

#endif // _FILESYSTEM_H
