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
#include "archivefile.h"
#include "file.h"
bool Search_String_Matches(Utf8String string, Utf8String search);

const ArchivedFileInfo *ArchiveFile::Get_Archived_File_Info(Utf8String const &filename) const
{
    Utf8String path = filename;
    Utf8String token;
    const DetailedArchivedDirectoryInfo *dirp = &m_archiveInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while (token.Find('.') == nullptr || path.Find('.') != nullptr) {
        auto it = dirp->directories.find(token);

        if (!(it != dirp->directories.end())) {
            return nullptr;
        }

        dirp = &it->second;
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    auto file_it = dirp->files.find(token);

    if (file_it != dirp->files.end()) {
        return &file_it->second;
    }

    return nullptr;
}

void ArchiveFile::Add_File(Utf8String const &filepath, ArchivedFileInfo const *info)
{
    Utf8String path = filepath;
    Utf8String token;
    DetailedArchivedDirectoryInfo *dirp = &m_archiveInfo;
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    while (token.Get_Length() > 0) {
        if (dirp->directories.find(token) == dirp->directories.end()) {
            dirp->directories[token].Clear();
            dirp->directories[token].name = token;
        }

        dirp = &dirp->directories[token];
        path.Next_Token(&token, "\\/");
    }

    dirp->files[info->file_name] = *info;
}

void ArchiveFile::Attach_File(File *file)
{
    if (m_attachedFile != nullptr) {
        m_attachedFile->Close();
        m_attachedFile = nullptr;
    }

    m_attachedFile = file;
}

void ArchiveFile::Get_File_List_In_Directory(Utf8String const &subdir,
    Utf8String const &dirpath,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdir) const
{
    Utf8String path = dirpath;
    path.To_Lower();
    Utf8String token;
    DetailedArchivedDirectoryInfo const *dirp = &m_archiveInfo;
    path.Next_Token(&token, "\\/");

    while (token.Get_Length() > 0) {
        auto it = dirp->directories.find(token);

        if (!(it != dirp->directories.end())) {
            return;
        }

        dirp = &it->second;
        path.Next_Token(&token, "\\/");
    }

    Get_File_List_In_Directory(dirp, dirpath, filter, filelist, search_subdir);
}

void ArchiveFile::Get_File_List_In_Directory(DetailedArchivedDirectoryInfo const *dir_info,
    Utf8String const &dirpath,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdir) const
{
    // Add the files from any subdirectories, recursive call.
    for (auto it = dir_info->directories.begin(); it != dir_info->directories.end(); ++it) {
        const DetailedArchivedDirectoryInfo *info = &it->second;
        Utf8String path = dirpath;

        if (path.Get_Length() > 0 && !path.Ends_With("\\") && !path.Ends_With("/")) {
            path.Concat("/");
        }

        path += info->name;
        Get_File_List_In_Directory(info, path, filter, filelist, search_subdir);
    }

    // Add all the files that match the search pattern.
    for (auto it = dir_info->files.begin(); it != dir_info->files.end(); ++it) {
        if (Search_String_Matches(it->second.file_name, filter)) {
            Utf8String path = dirpath;

            if (!path.Is_Empty() && !path.Ends_With("\\") && !path.Ends_With("/")) {
                path.Concat("/");
            }

            path += it->second.file_name;

            filelist.insert(path);
        }
    }
}

// Helper funtion to check if a string matches the search string.
bool Search_String_Matches(Utf8String string, Utf8String search)
{
    // Trivial case if first string is empty.
    if (string.Get_Length() == 0) {
        if (search.Get_Length() == 0) {
            return true;
        }

        return false;
    }

    // If there is no seach string, there cannot be a match.
    if (search.Get_Length() == 0) {
        return false;
    }

    const char *cstring = string.Str();
    const char *csearch = search.Str();

    // ? is wildcard for a single character.
    // * is wildcard for a run of characters.
    while (true) {
        if (*cstring != *csearch && *csearch != '?' && *csearch != '*') {
            return false;
        }

        // Move on to the next character to check
        if (*cstring == *csearch || *csearch == '?') {
            ++cstring;
            ++csearch;
            // Move on to the rest of the search string and
            // recursively check to see if it matches.
        } else if (*csearch == '*') {
            ++csearch;

            if (*csearch == '\0') {
                return true;
            }

            while (*cstring != '\0') {
                if (Search_String_Matches(cstring, csearch)) {
                    return true;
                }

                ++cstring;
            }
        }

        if (*cstring == '\0') {
            break;
        }

        if (*csearch == '\0') {
            return false;
        }
    }

    if (*csearch == '\0') {
        return true;
    } else {
        return false;
    }
}
