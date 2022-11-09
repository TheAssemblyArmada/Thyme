/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for an archive file system implementations.
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
#include <map>
#include <set>

class File;
class ArchiveFile;
struct FileInfo;

class ArchivedDirectoryInfo
{
public:
    void Clear()
    {
        name.Clear();
        directories.clear();
        files.clear();
    }

    Utf8String name;
    std::map<Utf8String, ArchivedDirectoryInfo> directories;
    std::map<Utf8String, Utf8String> files; // Maps the filenames in the archive to the actual filename on disk
};

class ArchiveFileSystem : public SubsystemInterface
{
public:
    virtual ~ArchiveFileSystem();
    virtual ArchiveFile *Open_Archive_File(const char *filename) = 0;
    virtual void Close_Archive_File(const char *filename) = 0;
    virtual void Close_All_Archives() = 0;
    virtual File *Open_File(const char *filename, int mode);
    virtual void Close_All_Files() = 0;
    virtual bool Does_File_Exist(const char *filename) const;
    virtual bool Load_Big_Files_From_Directory(Utf8String dir, Utf8String filter, bool overwrite) = 0;
    virtual void Load_Into_Directory_Tree(ArchiveFile const *file, Utf8String const &dir, bool overwrite);

    bool Get_File_Info(Utf8String const &name, FileInfo *info) const;
    Utf8String Get_Archive_Filename_For_File(Utf8String const &filename) const;
    void Get_File_List_In_Directory(Utf8String const &subdir,
        Utf8String const &dirpath,
        Utf8String const &filter,
        std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
        bool search_subdirs) const;
    void Load_Mods();

protected:
    std::map<Utf8String, ArchiveFile *> m_archiveFiles;
    ArchivedDirectoryInfo m_archiveDirInfo;
};

#ifdef GAME_DLL
extern ArchiveFileSystem *&g_theArchiveFileSystem;
#else
extern ArchiveFileSystem *g_theArchiveFileSystem;
#endif