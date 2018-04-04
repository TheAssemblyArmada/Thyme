/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Interface for an archive file system implementations.
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

#include "always.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include <map>
#include <set>

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

class File;
class ArchiveFile;
struct FileInfo;

struct ArchivedDirectoryInfo
{
    AsciiString name;
    std::map<AsciiString, ArchivedDirectoryInfo> directories;
    std::map<AsciiString, AsciiString> files; // Maps the filenames in the archive to the actual filename on disk
};

class ArchiveFileSystem : public SubsystemInterface
{
public:
    ArchiveFileSystem(){};
    virtual ~ArchiveFileSystem(){};
    virtual ArchiveFile *Open_Archive_File(const char *filename) = 0;
    virtual void Close_Archive_File(const char *filename) = 0;
    virtual void Close_All_Archives() = 0;
    virtual File *Open_File(const char *filename, int mode);
    virtual void Close_All_Files() = 0;
    virtual bool Does_File_Exist(const char *filename);
    virtual void Load_Archives_From_Dir(AsciiString dir, AsciiString filter, bool read_subdirs) = 0;
    virtual void Load_Into_Dir_Tree(ArchiveFile const *file, AsciiString const &dir, bool unk);

    bool Get_File_Info(AsciiString const &name, FileInfo *info);
    AsciiString Get_Archive_Filename_For_File(AsciiString const &filename);
    void Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter,
        std::set<AsciiString, rts::less_than_nocase<AsciiString>> &filelist, bool search_subdirs);
    void Load_Mods();

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif
protected:
    std::map<AsciiString, ArchiveFile *> m_archiveFiles;
    ArchivedDirectoryInfo m_archiveDirInfo;
};

#ifndef THYME_STANDALONE
inline void ArchiveFileSystem::Hook_Me()
{
    Hook_Method(0x0048F410, &Get_File_List_From_Dir);
    Hook_Method(0x0048F250, &Get_Archive_Filename_For_File);
    Hook_Method(0x0048F160, &Get_File_Info);
}

extern ArchiveFileSystem *&g_theArchiveFileSystem;
#else
extern ArchiveFileSystem *g_theArchiveFileSystem;
#endif