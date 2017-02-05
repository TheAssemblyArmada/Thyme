////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32LOCALFILESYSTEM.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Implements the LocalFileSystem interface.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "win32localfilesystem.h"
#include "win32localfile.h"

// Headers needed for posix open, close, read... etc.
#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

File *Win32LocalFileSystem::Open_File(char const *filename, int mode)
{
    DEBUG_LOG("Attempting to open local file %s.\n", filename);

    if ( strlen(filename) <= 0 ) {
        return nullptr;
    }

    Win32LocalFile *file = new Win32LocalFile;

    // If we need to write a file, ensure the needed directory exists.
    if ( (mode & File::WRITE) != 0 ) {
        DEBUG_LOG("Preparing file for write access.\n");
        AsciiString name = filename;
        AsciiString token;
        AsciiString path;

        name.Next_Token(&token, "\\/");
        path = token;

        // Iterate over the path and create them as needed for entire path.
        while ( strchr(token.Str(), '.') == nullptr || strchr(name.Str(), '.') != nullptr ) {
            Create_Directory(path);
            path.Concat('/');
            name.Next_Token(&token, "\\/");
            path.Concat(token.Str());
        }
    }

    DEBUG_LOG("Win32LocalFile instance allocated, opening %s.\n", filename);
    // Try and open the file, if not, delete instance and return null.
    if ( file->Open(filename, mode) ) {
        file->Set_Del_On_Close(true);
    } else {
        Delete_Instance(file);
        file = nullptr;
    }

    return file;
}

bool Win32LocalFileSystem::Does_File_Exist(char const *filename)
{
    DEBUG_LOG("Checking File %s Exists\n", filename);
    return access(filename, 0) == 0;
}

void Win32LocalFileSystem::Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdirs)
{
    DEBUG_LOG("Getting file list for %s%s.\n", dirpath.Str(), filter.Str());

    Call_Method<void, Win32LocalFileSystem, AsciiString const &, AsciiString const &, AsciiString const &, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &, bool>(0x00777800, this, subdir, dirpath, filter, filelist, search_subdirs);
#if 0
    AsciiString search_path = dirpath;
    search_path += subdir;
    search_path += filter;

#ifdef PLATFORM_WINDOWS
    WIN32_FIND_DATA data;
    HANDLE hndl = FindFirstFileA(search_path.Str(), &data);

    if ( hndl != INVALID_HANDLE_VALUE ) {
        // Loop over all files in the directory, ignoring other directories
        do {
            if ( (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0
                && strcmp(data.cFileName, ".") != 0
                && strcmp(data.cFileName, "..") != 0
            ) {
                AsciiString filepath = dirpath;
                filepath += subdir;
                filepath += data.cFileName;
                filelist.insert(filepath);
            }
        } while ( FindNextFileA(hndl, &data) != 0 );
    }

    FindClose(hndl);

    // Recurse into subdirectories if required.
    if ( search_subdirs ) {
        AsciiString sub_path = dirpath;
        sub_path += subdir;
        sub_path += filter;

        hndl = FindFirstFileA(sub_path.Str(), &data);

        if ( hndl != INVALID_HANDLE_VALUE ) {
            // Loop over all files in the directory, ignoring other directories
            do {
                if ( (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                    && strcmp(data.cFileName, ".") != 0
                    && strcmp(data.cFileName, "..") != 0
                ) {
                    AsciiString filepath = dirpath;
                    filepath += subdir;
                    filepath += data.cFileName;
                    Get_File_List_From_Dir(
                        filepath,
                        dirpath,
                        filter,
                        filelist,
                        search_subdirs
                    );
                }
            } while ( FindNextFileA(hndl, &data) != 0 );
        }

        FindClose(hndl);
    }
#else
    //TODO Some combo of dirent and fnmatch to get same functionality for posix?
#endif
#endif
}

bool Win32LocalFileSystem::Get_File_Info(AsciiString const &filename, FileInfo *info)
{
    DEBUG_LOG("Getting info for %s.\n", filename.Str());
    //TODO Make this cross platform.
#ifdef PLATFORM_WINDOWS
    WIN32_FIND_DATA data;
    HANDLE hndl = FindFirstFileA(filename.Str(), &data);

    if ( hndl == INVALID_HANDLE_VALUE ) {
        return false;
    }

    info->WriteTimeHigh = data.ftLastWriteTime.dwHighDateTime;
    info->WriteTimeLow = data.ftLastWriteTime.dwLowDateTime;
    info->FileSizeHigh = data.nFileSizeHigh;
    info->FileSizeLow = data.nFileSizeLow;

    FindClose(hndl);

    return true;
#else
    return false;
#endif
}

bool Win32LocalFileSystem::Create_Directory(AsciiString dir_path)
{
    DEBUG_LOG("Creating directory %s.\n", dir_path.Str());

    if ( dir_path.Is_Empty() || dir_path.Get_Length() > PATH_MAX ) {
        return false;
    }

    // Original uses CreateDirectoryA here, mkdir more cross platform.
    return mkdir(dir_path.Peek()) == 0;
}
