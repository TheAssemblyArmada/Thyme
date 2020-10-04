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

ArchivedFileInfo *ArchiveFile::Get_Archived_File_Info(Utf8String const &filename)
{
    Utf8String path = filename;
    Utf8String token;
    DetailedArchiveDirectoryInfo *dirp = &m_archiveInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while (strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr) {
        if (dirp->directories.find(token) == dirp->directories.end()) {
            return nullptr;
        }

        dirp = &dirp->directories[token];
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    auto file_it = dirp->files.find(token);

    if (file_it == dirp->files.end()) {
        return nullptr;
    }

    return &file_it->second;
}

void ArchiveFile::Add_File(Utf8String const &filepath, ArchivedFileInfo const *info)
{
    // DEBUG_LOG("Adding '%s' to interal archive for '%s'.\n", filepath.Str(), info->archive_name.Str());
    Utf8String path = filepath;
    Utf8String token;
    DetailedArchiveDirectoryInfo *dirp = &m_archiveInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();

    for (path.Next_Token(&token, "\\/"); !token.Is_Empty(); path.Next_Token(&token, "\\/")) {
        // If an element of the path doesn't have a directory node, add it.
        // DEBUG_LOG("Searching for path element '%s'.\n", token.Str());
        if (dirp->directories.find(token) == dirp->directories.end()) {
            // DEBUG_LOG("Adding path element '%s'.\n", token.Str());
            dirp->directories[token].name = token;
        }

        dirp = &dirp->directories[token];
    }

    dirp->files[info->file_name] = *info;
}

void ArchiveFile::Attach_File(File *file)
{
    if (m_backingFile != nullptr) {
        m_backingFile->Close();
        m_backingFile = nullptr;
    }

    m_backingFile = file;
}

void ArchiveFile::Get_File_List_From_Dir(Utf8String const &subdir,
    Utf8String const &dirpath,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdir) const
{
    Utf8String path = dirpath;
    Utf8String token;
    DetailedArchiveDirectoryInfo const *dirp = &m_archiveInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();

    // Go to the last InfoNode in the path to extract file contents from.
    for (path.Next_Token(&token, "\\/"); token.Is_Not_Empty(); path.Next_Token(&token, "\\/")) {
        // If an element of the path doesn't have a node for our next directory, return.
        if (dirp->directories.find(token) == dirp->directories.end()) {
            return;
        }

        dirp = &dirp->directories[token];
    }

    Get_File_List_From_Dir(dirp, dirpath, filter, filelist, search_subdir);
}

void ArchiveFile::Get_File_List_From_Dir(DetailedArchiveDirectoryInfo const *dir_info,
    Utf8String const &dirpath,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdir) const
{
    // Add the files from any subdirectories, recursive call.
    for (auto it = dir_info->directories.begin(); it != dir_info->directories.end(); ++it) {
        Utf8String path = dirpath;

        if (!path.Is_Empty() && !path.Ends_With("\\") && !path.Ends_With("/")) {
            path += "/";
        }

        path += it->second.name;
        Get_File_List_From_Dir(&(it->second), path, filter, filelist, search_subdir);
    }

    // Add all the files that match the search pattern.
    for (auto it = dir_info->files.begin(); it != dir_info->files.end(); ++it) {
        if (Search_String_Matches(it->second.file_name, filter)) {
            Utf8String path = dirpath;

            if (!path.Is_Empty() && !path.Ends_With("\\") && !path.Ends_With("/")) {
                path += "/";
            }

            path += it->second.file_name;
            filelist.insert(path);
        }
    }
}

// Helper funtion to check if a string matches the search string.
bool ArchiveFile::Search_String_Matches(Utf8String string, Utf8String search)
{
    // Trivial case if first string is empty.
    if (string.Is_Empty()) {
        return search.Is_Empty();
    }

    // If there is no seach string, there cannot be a match.
    if (search.Is_Empty()) {
        return false;
    }

    const char *cstring = string.Peek();
    const char *csearch = search.Peek();

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

    return *csearch == '\0';
}
