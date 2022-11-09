/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Filesystem abstraction merging local and archive file handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "filesystem.h"
#include "archivefilesystem.h"
#include "localfilesystem.h"
#include "namekeygenerator.h"

#ifndef GAME_DLL
FileSystem *g_theFileSystem = nullptr;
#endif

void FileSystem::Init()
{
    g_theLocalFileSystem->Init();
    g_theArchiveFileSystem->Init();
}

void FileSystem::Reset()
{
    g_theLocalFileSystem->Reset();
    g_theArchiveFileSystem->Reset();
}

void FileSystem::Update()
{
    g_theLocalFileSystem->Update();
    g_theArchiveFileSystem->Update();
}

File *FileSystem::Open_File(const char *filename, int mode)
{
    File *file = nullptr;

    if (g_theLocalFileSystem != nullptr) {
        file = g_theLocalFileSystem->Open_File(filename, mode);
    }

    if (file == nullptr && g_theArchiveFileSystem != nullptr) {
        file = g_theArchiveFileSystem->Open_File(filename, 0);
    }

    return file;
}

bool FileSystem::Does_File_Exist(const char *filename) const
{
    NameKeyType name_id = g_theNameKeyGenerator->Name_To_Lower_Case_Key(filename);
    auto it = m_availableFiles.find(name_id);

    if (it != m_availableFiles.end()) {
        return it->second;
    }

    if (g_theLocalFileSystem->Does_File_Exist(filename) || g_theArchiveFileSystem->Does_File_Exist(filename)) {
        m_availableFiles[name_id] = true;
        return true;
    } else {
        m_availableFiles[name_id] = false;
        return false;
    }
}

void FileSystem::Get_File_List_In_Directory(Utf8String const &dir,
    Utf8String const &filter,
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> &filelist,
    bool search_subdirs) const
{
    g_theLocalFileSystem->Get_File_List_In_Directory("", dir, filter, filelist, search_subdirs);
    g_theArchiveFileSystem->Get_File_List_In_Directory("", dir, filter, filelist, search_subdirs);
}

bool FileSystem::Create_Directory(Utf8String name)
{
    if (g_theLocalFileSystem != nullptr) {
        return g_theLocalFileSystem->Create_Directory(name);
    } else {
        return false;
    }
}

bool FileSystem::Are_Music_Files_On_CD()
{
    // This doesn't need implementing as its never called (it was originally called by IsFirstCDPresent,
    // CheckForCDAtGameStart and checkCDCallback but our implementation patches those so its never called anymore)
    return true;
}

void FileSystem::Load_Music_Files_From_CD()
{
    // This doesn't need implementing as its never called (it was originally called by AudioManager::Init but ours doesn't
    // call it anymore)
}

void FileSystem::Unload_Music_Files_From_CD()
{
    // Because Is_Music_Playing_From_CD will always return false, this is a no-op and does not need implementing
}

bool FileSystem::Get_File_Info(const Utf8String &filename, FileInfo *info) const
{
    if (!info) {
        return false;
    }

    memset(info, 0, sizeof(FileInfo));

    return g_theLocalFileSystem->Get_File_Info(filename, info) || g_theArchiveFileSystem->Get_File_Info(filename, info) != 0;
}
