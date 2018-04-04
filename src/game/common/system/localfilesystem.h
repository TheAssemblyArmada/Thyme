/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Interface for local file system implementations.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "rtsutils.h"
#include "subsysteminterface.h"
#include <set>

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

struct FileInfo;
class File;

class LocalFileSystem : public SubsystemInterface
{
public:
    virtual ~LocalFileSystem() {}

    virtual File *Open_File(const char *filename, int mode) = 0;
    virtual bool Does_File_Exist(const char *filename) = 0;
    virtual void Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter,
        std::set<AsciiString, rts::less_than_nocase<AsciiString>> &filelist, bool search_subdirs) = 0;
    virtual bool Get_File_Info(AsciiString const &filename, FileInfo *info) = 0;
    virtual bool Create_Directory(AsciiString) = 0;
};

#ifndef THYME_STANDALONE
extern LocalFileSystem *&g_theLocalFileSystem;
#else
extern LocalFileSystem *g_theLocalFileSystem;
#endif