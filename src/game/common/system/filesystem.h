/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Filesystem abstraction merging local and archive file handling.
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
#include "file.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include <map>
#include <set>

class FileSystem : public SubsystemInterface
{
public:
    virtual ~FileSystem() {}

    // SubsystemInterface implementations
    virtual void Init();
    virtual void Reset();
    virtual void Update();

    // Filesystem
    File *Open_File(const char *filename, int mode);
    bool Does_File_Exist(const char *filename) const;
    void Get_File_List_In_Directory(Utf8String const &dir,
        Utf8String const &filter,
        std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
        bool a5) const;
    bool Get_File_Info(const Utf8String &filename, FileInfo *info) const;

    bool Create_Directory(Utf8String name);
    bool Are_Music_Files_On_CD();
    void Load_Music_Files_From_CD();
    void Unload_Music_Files_From_CD();

private:
    mutable std::map<unsigned int, bool> m_availableFiles;
};

#ifdef GAME_DLL
extern FileSystem *&g_theFileSystem;
#else
extern FileSystem *g_theFileSystem;
#endif