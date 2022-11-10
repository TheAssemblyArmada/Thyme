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
#include "archivefilesystem.h"
#include "archivefile.h"
#include "globaldata.h"
#include <captainslog.h>

#ifndef GAME_DLL
ArchiveFileSystem *g_theArchiveFileSystem = nullptr;
#endif

ArchiveFileSystem::~ArchiveFileSystem()
{
    for (auto it = m_archiveFiles.begin(); it != m_archiveFiles.end(); ++it) {
        delete it->second;
    }
}

File *ArchiveFileSystem::Open_File(const char *filename, int mode)
{
    Utf8String archive = Get_Archive_Filename_For_File(filename);

    if (archive.Get_Length() == 0) {
        return nullptr;
    }

    ArchiveFile *file = m_archiveFiles[archive];
    captainslog_dbgassert(file != nullptr, "Did not find matching archive file.");
    return file->Open_File(filename, mode);
}

bool ArchiveFileSystem::Does_File_Exist(const char *filename) const
{
    Utf8String path = filename;
    Utf8String token;
    const ArchivedDirectoryInfo *dirp = &m_archiveDirInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while (token.Find('.') == nullptr || path.Find('.') != nullptr) {
        // If we ran out of directories, we don't have the directory that has the file.
        auto it = dirp->directories.find(token);

        if (it == dirp->directories.end()) {
            return false;
        }

        dirp = &it->second;
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    if (dirp->files.find(token) == dirp->files.end()) {
        return false;
    }

    return true;
}

// Loads an archive file into the virtual directory tree. The over write option allows it to use this archive to
// replace the backing for a file name if it already has an entry in the tree.
void ArchiveFileSystem::Load_Into_Directory_Tree(ArchiveFile const *file, Utf8String const &archive_path, bool overwrite)
{
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> file_list;

    // Retrieve a list of files in the archive
    file->Get_File_List_In_Directory("", "", "*", file_list, true);

    for (auto it = file_list.begin(); it != file_list.end(); ++it) {
        Utf8String path = *it;
        Utf8String token;
        ArchivedDirectoryInfo *dirp = &m_archiveDirInfo;

        // Lower case for matching.
        path.To_Lower();

        // Consider existence of '.' to indicate file as all should have .ext format
        // checks the remaining path does not contain one to catch directories in path
        // that do.
        while (path.Next_Token(&token, "\\/") && (token.Find('.') == nullptr || path.Find('.') != nullptr)) {
            // If we can't find the directory in our map, make it.
            if (dirp->directories.find(token) == dirp->directories.end()) {
                dirp->directories[token].Clear();
                dirp->directories[token].name = token;
            }

            dirp = &dirp->directories[token];
        }

        if (dirp->files.find(token) == dirp->files.end() || overwrite) {
            dirp->files[token] = archive_path;
        }
    }
}

bool ArchiveFileSystem::Get_File_Info(Utf8String const &name, FileInfo *info) const
{
    if (info == nullptr) {
        return false;
    }

    if (name.Get_Length() <= 0) {
        return false;
    }

    // Find the archive that corresponds to this file name.
    Utf8String archive = Get_Archive_Filename_For_File(name);

    // Find the archive file pointer for the archive name we retrieved.
    auto it = m_archiveFiles.find(archive);

    if (it != m_archiveFiles.end()) {
        return it->second->Get_File_Info(name, info);
    }

    return false;
}

// Returns the filname of the archive file containing the passed in file name.
Utf8String ArchiveFileSystem::Get_Archive_Filename_For_File(Utf8String const &filename) const
{
    Utf8String path = filename;
    Utf8String token;
    const ArchivedDirectoryInfo *dirp = &m_archiveDirInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while (token.Find('.') == nullptr || path.Find('.') != nullptr) {
        auto it = dirp->directories.find(token);

        if (it == dirp->directories.end()) {
            return Utf8String();
        }

        dirp = &it->second;
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    auto it = dirp->files.find(token);

    if (it != dirp->files.end()) {
        return it->second;
    }

    return Utf8String();
}

// Populates a std::set of file paths based on the passed in filter and path to examine.
void ArchiveFileSystem::Get_File_List_In_Directory(Utf8String const &subdir,
    Utf8String const &dirpath,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdirs) const
{
    // Get files from all archive files.
    for (auto it = m_archiveFiles.begin(); it != m_archiveFiles.end(); ++it) {
        it->second->Get_File_List_In_Directory(subdir, dirpath, filter, filelist, search_subdirs);
    }
}

// Load mods based on two path options set in the global data fields m_userModFile and
// m_userModDirectory. These options are parsed from the -mod command line option.
// See GlobalData.h
void ArchiveFileSystem::Load_Mods()
{
    if (g_theWriteableGlobalData->m_userModFile.Is_Not_Empty()) {
        ArchiveFile *file = Open_Archive_File(g_theWriteableGlobalData->m_userModFile.Str());

        if (file != nullptr) {
            captainslog_debug("ArchiveFileSystem::Load_Mods - loading %s into the directory tree.",
                g_theWriteableGlobalData->m_userModFile.Str());
            Load_Into_Directory_Tree(file, g_theWriteableGlobalData->m_userModFile, true);
            m_archiveFiles[g_theWriteableGlobalData->m_userModFile] = file;
            captainslog_debug("ArchiveFileSystem::Load_Mods - %s inserted into the archive file map.",
                g_theWriteableGlobalData->m_userModFile.Str());
        }
    }

    if (g_theWriteableGlobalData->m_userModDirectory.Is_Not_Empty()) {
        if (!Load_Big_Files_From_Directory(g_theWriteableGlobalData->m_userModDirectory, "*.big", true)) {
            captainslog_debug(
                "Load_Big_Files_From_Directory(%s) returned FALSE!", g_theWriteableGlobalData->m_userModDirectory.Str());
        }
    }
}
