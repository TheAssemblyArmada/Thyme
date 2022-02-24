/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implements the LocalFileSystem interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "win32localfilesystem.h"
#include "standardfile.h"
#include "win32localfile.h"

// Headers needed for posix open, close, read... etc.
#ifdef PLATFORM_WINDOWS
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

File *Win32LocalFileSystem::Open_File(const char *filename, int mode)
{
    if (filename == nullptr || *filename == '\0') {
        return nullptr;
    }

    File *file;

    if (mode & File::BUFFERED) {
        file = new Thyme::StandardFile;
    } else {
        file = new Win32LocalFile;
    }

    // If we need to write a file, ensure the needed directory exists.
    if ((mode & File::WRITE) != 0) {
        captainslog_trace("Preparing file '%s' for write access.", filename);
        Utf8String name = filename;
        Utf8String token;
        Utf8String path;

        name.Next_Token(&token, "\\/");
        path = token;

        // Iterate over the path and create them as needed for entire path.
        while (strchr(token.Str(), '.') == nullptr || strchr(name.Str(), '.') != nullptr) {
            Create_Directory(path);
            path.Concat('/');
            name.Next_Token(&token, "\\/");
            path.Concat(token.Str());
        }
    }

    // Try and open the file, if not, delete instance and return null.
    if (file->Open(filename, mode)) {
        file->Set_Del_On_Close(true);
    } else {
        file->Delete_Instance();
        file = nullptr;
    }

    return file;
}

bool Win32LocalFileSystem::Does_File_Exist(const char *filename)
{
    return access(filename, 0) == 0;
}

#ifndef PLATFORM_WINDOWS
void Posix_List_Dir(const char *name, std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist, bool search_subdirs)
{
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir(name);
    if (dp != nullptr) {
        while ((entry = readdir(dp)) != nullptr) {
            char path[1024];
            if (search_subdirs && entry->d_type == DT_DIR) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
                Posix_List_Dir(path, filelist, search_subdirs);
            } else if (entry->d_type == DT_REG) {
                snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
                filelist.insert(path);
            }
        }
    }

    closedir(dp);
}
#endif

void Win32LocalFileSystem::Get_File_List_From_Dir(Utf8String const &subdir,
    Utf8String const &dirpath,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdirs)
{
    Utf8String search_path = dirpath;
    search_path += subdir;

#ifdef PLATFORM_WINDOWS
    search_path += filter;
    WIN32_FIND_DATAW data;
    HANDLE hndl = FindFirstFileW(UTF8To16(search_path.Windows_Path()), &data);

    if (hndl != INVALID_HANDLE_VALUE) {
        // Loop over all files in the directory, ignoring other directories
        do {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                Utf8String name = static_cast<const char *>(UTF16To8(data.cFileName)); // Force conversion for MinGW.

                if (name != "." && name != "..") {
                    Utf8String filepath = dirpath;
                    filepath += subdir;
                    filepath += name;
                    // filelist.insert(filepath.Posix_Path()); // TODO 0x006C8400 relies on '\' as path sep.
                    filelist.insert(filepath);
                }
            }
        } while (FindNextFileW(hndl, &data) != FALSE);

        // #BUGFIX Moved into scope to avoid close call on invalid handle.
        FindClose(hndl);
    }

    // Recurse into subdirectories if required.
    if (search_subdirs) {
        Utf8String sub_path = dirpath;
        sub_path += subdir;
        sub_path += "*.";

        hndl = FindFirstFileW(UTF8To16(sub_path.Windows_Path()), &data);

        if (hndl != INVALID_HANDLE_VALUE) {
            // Loop over all files in the directory finding only directories.
            do {
                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    Utf8String name = static_cast<const char *>(UTF16To8(data.cFileName)); // Force conversion for MinGW.

                    if (name != "." && name != "..") {
                        Utf8String filepath;
                        filepath += subdir;
                        filepath += name;
                        filepath += '\\';

                        Get_File_List_From_Dir(filepath, dirpath, filter, filelist, search_subdirs);
                    }
                }
            } while (FindNextFileW(hndl, &data) != FALSE);

            // #BUGFIX Moved into scope to avoid close call on invalid handle.
            FindClose(hndl);
        }
    }
#else
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir(search_path);
    if (dp != nullptr) {
        while ((entry = readdir(dp)) != nullptr) {
            if (search_subdirs && entry->d_type == DT_DIR) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                Utf8String subdir = search_path;
                subdir += '/';
                Get_File_List_From_Dir(subdir, entry->d_name, filter, filelist, search_subdirs);
            } else if (entry->d_type == DT_REG) {
                // TODO: apply filter:
                Utf8String filepath = search_path;
                filepath += '/';
                filepath += entry->d_name;
                filelist.insert(filepath);
            }
        }
    }

    closedir(dp);
#endif
}

bool Win32LocalFileSystem::Get_File_Info(Utf8String const &filename, FileInfo *info)
{
    // TODO Make this cross platform.
#ifdef PLATFORM_WINDOWS
    WIN32_FIND_DATAW data;
    HANDLE hndl = FindFirstFileW(UTF8To16(filename.Windows_Path()), &data);

    if (hndl == INVALID_HANDLE_VALUE) {
        return false;
    }

    info->write_time_high = data.ftLastWriteTime.dwHighDateTime;
    info->write_time_low = data.ftLastWriteTime.dwLowDateTime;
    info->file_size_high = data.nFileSizeHigh;
    info->file_size_low = data.nFileSizeLow;

    FindClose(hndl);

    return true;
#else
    struct stat data;
    int rc = stat(filename, &data);

    if (rc != 0) {
        return false;
    }

    // Assumes 64bit stat.
    info->file_size_high = data.st_size >> 32;
    info->file_size_low = data.st_size & UINT32_MAX;

    int64_t time = data.st_mtime * 10000000ll;
    info->write_time_high = time >> 32;
    info->write_time_low = time & UINT32_MAX;

    return true;
#endif
}

bool Win32LocalFileSystem::Create_Directory(Utf8String dir_path)
{
    if (dir_path.Is_Empty() || dir_path.Get_Length() > PATH_MAX) {
        return false;
    }

    // So much for mkdir being more cross platform
#ifdef PLATFORM_WINDOWS
    return CreateDirectoryW(UTF8To16(dir_path), nullptr) != 0;
#else
    return mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#endif
}
