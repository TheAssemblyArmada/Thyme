/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for local file system implementations.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include <set>

struct FileInfo;
class File;

class LocalFileSystem : public SubsystemInterface
{
public:
    virtual ~LocalFileSystem() {}

    virtual File *Open_File(const char *filename, int mode) = 0;
    virtual bool Does_File_Exist(const char *filename) const = 0;
    virtual void Get_File_List_In_Directory(Utf8String const &subdir,
        Utf8String const &dirpath,
        Utf8String const &filter,
        std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
        bool search_subdirs) const = 0;
    virtual bool Get_File_Info(Utf8String const &filename, FileInfo *info) const = 0;
    virtual bool Create_Directory(Utf8String) = 0;
};

#ifdef GAME_DLL
extern LocalFileSystem *&g_theLocalFileSystem;
#else
extern LocalFileSystem *g_theLocalFileSystem;
#endif
