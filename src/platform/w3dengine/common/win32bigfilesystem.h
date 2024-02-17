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

#include "always.h"
#include "archivefilesystem.h"

class Win32BIGFileSystem : public ArchiveFileSystem
{
    enum
    {
        BIG_PATH_MAX = 260,
    };

public:
    virtual ~Win32BIGFileSystem() {}

    // SubsystemInterface implementations
    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override {}

    // ArchiveFileSystem implementations
    virtual ArchiveFile *Open_Archive_File(const char *filename) override;
    virtual void Close_Archive_File(const char *filename) override;
    virtual void Close_All_Archives() override {}
    virtual void Close_All_Files() override {}
    virtual bool Load_Big_Files_From_Directory(Utf8String dir, Utf8String filter, bool overwrite) override;
};
