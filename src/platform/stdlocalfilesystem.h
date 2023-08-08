/**
 * @file
 *
 * @author feliwir
 *
 * @brief Implements the LocalFileSystem interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "localfilesystem.h"
#include "win32localfile.h"
#include <captainslog.h>

namespace Thyme
{
class StdLocalFileSystem : public LocalFileSystem
{
public:
    StdLocalFileSystem() { captainslog_trace("Creating StdLocalFileSystem."); }
    ~StdLocalFileSystem() override {}

    // Subsystem interface implementations.
    void Init() override {}
    void Reset() override {}
    void Update() override {}

    // LocalFileSystem interface implementations.
    File *Open_File(const char *filename, int mode) override;
    bool Does_File_Exist(const char *filename) const override;
    void Get_File_List_In_Directory(Utf8String const &subdir,
        Utf8String const &dirpath,
        Utf8String const &filter,
        std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
        bool search_subdirs) const override;
    bool Get_File_Info(Utf8String const &filename, FileInfo *info) const override;
    bool Create_Directory(Utf8String dir_path) override;
};
} // namespace Thyme
