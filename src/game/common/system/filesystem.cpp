////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: FILESYSTEM.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Filesystem abstraction merging local and archive file
//                 handling.
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
#include "filesystem.h"
#include "archivefilesystem.h"
#include "localfilesystem.h"
#include "namekeygenerator.h"

FileSystem::FileSystem() :
    m_availableFiles()
{

}

FileSystem::~FileSystem()
{

}

void FileSystem::Init()
{
    TheLocalFileSystem->Init();
    TheArchiveFileSystem->Init();
}

void FileSystem::Reset()
{
    TheLocalFileSystem->Reset();
    TheArchiveFileSystem->Reset();
}

void FileSystem::Update()
{
    TheLocalFileSystem->Update();
    TheArchiveFileSystem->Update();
}

File *FileSystem::Open(char const *filename, int mode)
{
    File *file = nullptr;

    if ( TheLocalFileSystem != nullptr ) {
        file = TheLocalFileSystem->Open_File(filename, mode);
    }

    if ( file == nullptr && TheArchiveFileSystem != nullptr ) {
        file = TheArchiveFileSystem->Open_File(filename, 0);
    }

    return file;
}

bool FileSystem::Does_File_Exist(char const *filename)
{
    NameKeyType name_id = TheNameKeyGenerator->Name_To_Lower_Case_Key(filename);

    auto it = m_availableFiles.find(name_id);

    if ( it != m_availableFiles.end() ) {
        return it->second;
    }

    if ( TheLocalFileSystem->Does_File_Exist(filename) ) {
        m_availableFiles[name_id] = true;

        return true;
    }

    if ( TheArchiveFileSystem->Does_File_Exist(filename) ) {
        m_availableFiles[name_id] = true;

        return true;
    }

    m_availableFiles[name_id] = false;

    return false;
}

void FileSystem::Get_File_List_From_Dir(AsciiString const &dir, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdirs)
{
    TheLocalFileSystem->Get_File_List_From_Dir("", dir, filter, filelist, search_subdirs);
    TheArchiveFileSystem->Get_File_List_From_Dir("", "", filter, filelist, search_subdirs);
}

bool FileSystem::Create_Dir(AsciiString name)
{
    if ( TheLocalFileSystem == nullptr ) {
        return false;
    }

    return TheLocalFileSystem->Create_Directory(name);
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
    //TODO Needs audio interface.
}
