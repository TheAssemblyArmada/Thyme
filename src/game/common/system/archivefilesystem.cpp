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
#include "archivefilesystem.h"
#include "archivefile.h"
#include "globaldata.h"

#ifndef THYME_STANDALONE
ArchiveFileSystem *&g_theArchiveFileSystem = Make_Global<ArchiveFileSystem *>(0x00A2BA00);
#else
ArchiveFileSystem *g_theArchiveFileSystem = nullptr;
#endif

File *ArchiveFileSystem::Open_File(const char *filename, int mode)
{
    AsciiString archive = Get_Archive_Filename_For_File(filename);

    if (archive.Is_Empty()) {
        return nullptr;
    }

    ArchiveFile *file = m_archiveFiles[archive];

    DEBUG_ASSERT_PRINT(file != nullptr, "Did not find matching archive file.\n");

    return file->Open_File(filename, mode);
}

bool ArchiveFileSystem::Does_File_Exist(const char *filename)
{
    AsciiString path = filename;
    AsciiString token;
    ArchivedDirectoryInfo *dirp = &m_archiveDirInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while (strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr) {
        // If we ran out of directories, we don't have the directory that has the file.
        if (dirp->directories.find(token) == dirp->directories.end()) {
            return false;
        }

        dirp = &dirp->directories[token];
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
void ArchiveFileSystem::Load_Into_Dir_Tree(ArchiveFile const *file, AsciiString const &archive_path, bool overwrite)
{
    std::set<AsciiString, rts::less_than_nocase<AsciiString>> file_list;

    // Retrieve a list of files in the archive
    file->Get_File_List_From_Dir("", "", "*", file_list, true);

    for (auto it = file_list.begin(); it != file_list.end(); ++it) {
        AsciiString path = *it;
        AsciiString token;
        // AsciiString fullname;
        ArchivedDirectoryInfo *dirp = &m_archiveDirInfo;

        // Lower case for matching.
        path.To_Lower();

        // Consider existence of '.' to indicate file as all should have .ext format
        // checks the remaining path does not contain one to catch directories in path
        // that do.
        while (
            path.Next_Token(&token, "\\/") && (strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr)) {
            // If we can't find the directory in our map, make it.
            if (dirp->directories.find(token) == dirp->directories.end()) {
                dirp->directories[token].name = token;
            }

            dirp = &dirp->directories[token];
            // fullname += token;
            // fullname += "/";
        }

        if (dirp->files.find(token) == dirp->files.end() || overwrite) {
            dirp->files[token] = archive_path;
        }
    }
}

bool ArchiveFileSystem::Get_File_Info(AsciiString const &name, FileInfo *info)
{
    if (info == nullptr || name.Is_Empty()) {
        return false;
    }

    // Find the archive that corresponds to this file name.
    AsciiString archive = Get_Archive_Filename_For_File(name);

    // Find the archive file pointer for the archive name we retrieved.
    if (m_archiveFiles.find(archive) == m_archiveFiles.end()) {
        return false;
    }

    return m_archiveFiles[archive]->Get_File_Info(name, info);
}

// Returns the filname of the archive file containing the passed in file name.
AsciiString ArchiveFileSystem::Get_Archive_Filename_For_File(AsciiString const &filename)
{
    AsciiString path = filename;
    AsciiString token;
    ArchivedDirectoryInfo *dirp = &m_archiveDirInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while (strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr) {
        if (dirp->directories.find(token) == dirp->directories.end()) {
            return AsciiString();
        }

        dirp = &dirp->directories[token];
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    if (dirp->files.find(token) == dirp->files.end()) {
        return AsciiString();
    }

    return dirp->files[token];
}

// Populates a std::set of file paths based on the passed in filter and path to examine.
void ArchiveFileSystem::Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath,
    AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString>> &filelist, bool search_subdirs)
{
    // Get files from all archive files.
    for (auto it = m_archiveFiles.begin(); it != m_archiveFiles.end(); ++it) {
        it->second->Get_File_List_From_Dir(subdir, dirpath, filter, filelist, search_subdirs);
    }
}

// Load mods based on two path options set in the global data fields m_userModFile and
// m_userModDirectory. These options are parsed from the -mod command line option.
// See GlobalData.h
void ArchiveFileSystem::Load_Mods()
{
    if (!g_theWriteableGlobalData->m_userModFile.Is_Empty()) {
        DEBUG_LOG("Loading mod file '%s'.\n", g_theWriteableGlobalData->m_userModFile.Str());
        ArchiveFile *file = Open_Archive_File(g_theWriteableGlobalData->m_userModFile.Str());

        if (file != nullptr) {
            Load_Into_Dir_Tree(file, g_theWriteableGlobalData->m_userModFile, true);
            m_archiveFiles[g_theWriteableGlobalData->m_userModFile] = file;
        }
    }

    if (!g_theWriteableGlobalData->m_userModDirectory.Is_Empty()) {
        DEBUG_LOG("Loading mod files from '%s'.\n", g_theWriteableGlobalData->m_userModDirectory.Str());
        Load_Archives_From_Dir(g_theWriteableGlobalData->m_userModDirectory, "*.big", true);
    }
}
