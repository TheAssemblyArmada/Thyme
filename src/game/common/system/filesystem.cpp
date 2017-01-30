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

FileSystem::FileSystem() :
    AvailableFiles()
{

}

FileSystem::~FileSystem()
{

}

void FileSystem::Init()
{
}

void FileSystem::Reset()
{
}

void FileSystem::Update()
{
}

File *FileSystem::Open(char const *filename, int mode)
{
    return nullptr;
}

bool FileSystem::Does_File_Exist(char const *filename)
{
    return false;
}

void FileSystem::Get_File_List_From_Dir(AsciiString const &dir, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool a5)
{
}
