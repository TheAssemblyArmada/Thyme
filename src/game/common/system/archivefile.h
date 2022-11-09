/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for archive file handling.
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
#include "asciistring.h"
#include "file.h"
#include "rtsutils.h"
#include <map>
#include <set>

struct FileInfo;
class File;

class ArchivedFileInfo
{
public:
    ArchivedFileInfo() { Clear(); }
    void Clear()
    {
        file_name.Clear();
        archive_name.Clear();
        position = 0;
        size = 0;
    }

    Utf8String file_name;
    Utf8String archive_name;
    int position;
    int size;
};

class DetailedArchivedDirectoryInfo
{
public:
    void Clear()
    {
        name.Clear();
        directories.clear();
        files.clear();
    }

    Utf8String name;
    std::map<Utf8String, DetailedArchivedDirectoryInfo> directories;
    std::map<Utf8String, ArchivedFileInfo> files;
};

class ArchiveFile
{
public:
    ArchiveFile() : m_attachedFile(nullptr) { m_archiveInfo.Clear(); }

    virtual ~ArchiveFile()
    {
        if (m_attachedFile != nullptr) {
            m_attachedFile->Close();
            m_attachedFile = nullptr;
        }
    }

    virtual bool Get_File_Info(Utf8String const &name, FileInfo *info) const = 0;
    virtual File *Open_File(const char *filename, int mode) = 0;
    virtual void Close_All_Files() = 0;
    virtual Utf8String Get_Name() = 0;
    virtual Utf8String Get_Path() = 0;
    virtual void Set_Search_Priority(int priority) = 0;
    virtual void Close() = 0;

    const ArchivedFileInfo *Get_Archived_File_Info(Utf8String const &filename) const;
    void Add_File(Utf8String const &filename, ArchivedFileInfo const *info);
    void Attach_File(File *file);
    void Get_File_List_In_Directory(Utf8String const &subdir,
        Utf8String const &dirpath,
        Utf8String const &filter,
        std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
        bool search_subdir) const;

protected:
    void Get_File_List_In_Directory(DetailedArchivedDirectoryInfo const *dir_info,
        Utf8String const &dirpath,
        Utf8String const &filter,
        std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
        bool search_subdir) const;

    File *m_attachedFile;
    DetailedArchivedDirectoryInfo m_archiveInfo;
};
