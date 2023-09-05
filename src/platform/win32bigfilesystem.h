/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implements the ArchiveFileSystem interface on the BIG file format.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "archivefilesystem.h"

class Win32BIGFileSystem : public ArchiveFileSystem
{
    enum
    {
        BIG_PATH_MAX = 260,
    };

public:
    ~Win32BIGFileSystem() override {}

    // SubsystemInterface implementations
    void Init() override;
    void Reset() override {}
    void Update() override {}

    // ArchiveFileSystem implementations
    ArchiveFile *Open_Archive_File(const char *filename) override;
    void Close_Archive_File(const char *filename) override;
    void Close_All_Archives() override {}
    void Close_All_Files() override {}
    bool Load_Big_Files_From_Directory(Utf8String dir, Utf8String filter, bool overwrite) override;
};
