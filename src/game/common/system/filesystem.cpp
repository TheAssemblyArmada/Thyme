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
#include "localfilesystem.h"
#include "archivefilesystem.h"

FileSystem::FileSystem() :
    AvailableFiles()
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
        file = TheArchiveFileSystem->Open_File(filename, mode);
    }

    return file;
}

bool FileSystem::Does_File_Exist(char const *filename)
{
    return Call_Method<bool, FileSystem, char const*>(0x00446610, this, filename);
}

void FileSystem::Get_File_List_From_Dir(AsciiString const &dir, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdirs)
{
    Call_Method<void, FileSystem, AsciiString const &, AsciiString const &, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &, bool>(0x00446770, this, dir, filter, filelist, search_subdirs);
}
