/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Filesystem abstraction merging local and archive file handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "filesystem.h"
#include "archivefilesystem.h"
#include "localfilesystem.h"
#include "namekeygenerator.h"

#ifndef THYME_STANDALONE
FileSystem *&g_theFileSystem = Make_Global<FileSystem *>(0x00A2B670);
#else
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

File *FileSystem::Open(const char *filename, int mode)
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

bool FileSystem::Does_File_Exist(const char *filename)
{
    NameKeyType name_id = g_theNameKeyGenerator->Name_To_Lower_Case_Key(filename);

    auto it = m_availableFiles.find(name_id);

    if (it != m_availableFiles.end()) {
        return it->second;
    }

    if (g_theLocalFileSystem->Does_File_Exist(filename)) {
        m_availableFiles[name_id] = true;

        return true;
    }

    if (g_theArchiveFileSystem->Does_File_Exist(filename)) {
        m_availableFiles[name_id] = true;

        return true;
    }

    m_availableFiles[name_id] = false;

    return false;
}

void FileSystem::Get_File_List_From_Dir(AsciiString const &dir, AsciiString const &filter,
    std::set<AsciiString, rts::less_than_nocase<AsciiString>> &filelist, bool search_subdirs)
{
    g_theLocalFileSystem->Get_File_List_From_Dir("", dir, filter, filelist, search_subdirs);
    g_theArchiveFileSystem->Get_File_List_From_Dir("", dir, filter, filelist, search_subdirs);
}

bool FileSystem::Create_Dir(AsciiString name)
{
    if (g_theLocalFileSystem == nullptr) {
        return false;
    }

    return g_theLocalFileSystem->Create_Directory(name);
}

bool FileSystem::Are_Music_Files_On_CD()
{
    // TODO do we want to implement correct CD handling given most new copies will be digital download?
    // Mac version just returns true here.
    return true;
}

bool FileSystem::Load_Music_Files_From_CD()
{
    // TODO needs CDManager
    return false;
}

void FileSystem::Unload_Music_Files_From_CD()
{
    // TODO Needs audio interface.
}
