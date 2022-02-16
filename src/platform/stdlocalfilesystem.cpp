/**
 * @file
 *
 * @author feliwir
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
#include "stdlocalfilesystem.h"
#include "standardfile.h"

#include <filesystem>
using fs = std::filesystem;

File *StdLocalFileSystem::Open_File(const char *filename, int mode)
{
    if (filename == nullptr || *filename == '\0') {
        return nullptr;
    }

    File *file;

    file = new Thyme::StandardFile;

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

bool StdLocalFileSystem::Does_File_Exist(const char *filename)
{
    return access(filename, 0) == 0;
}

void StdLocalFileSystem::Get_File_List_From_Dir(Utf8String const &subdir,
    Utf8String const &dirpath,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdirs)
{
    Utf8String search_path = dirpath;
    search_path += subdir;
    search_path += filter;
}

bool StdLocalFileSystem::Get_File_Info(Utf8String const &filename, FileInfo *info)
{
    std::error_code ec;
    auto file_size = fs::file_size(filename.Str(), ec);
    if (ec) {
        return false;
    }

    // Assumes 64bit stat.
    info->file_size_high = file_size >> 32;
    info->file_size_low = file_size & UINT32_MAX;

    auto write_time = fs::last_write_time(filename.Str(), ec);
    if (ec) {
        return false;
    }

    // TODO: fix this to be win compatible (time since 1601)
    auto time = write_time.time_since_epoch().count();
    info->write_time_high = time >> 32;
    info->write_time_low = time & UINT32_MAX;

    return true;
}

bool StdLocalFileSystem::Create_Directory(Utf8String dir_path)
{
    if (dir_path.Is_Empty() || dir_path.Get_Length() > PATH_MAX) {
        return false;
    }

    std::error_code ec;
    bool result = fs::create_directory(dir_path.Str(), ec);
    return result && !ec;
}
