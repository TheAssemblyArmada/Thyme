////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ARCHIVEFILESYSTEM.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Interface for an archive file system implementations.
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
#include "archivefilesystem.h"
#include "archivefile.h"

ArchiveFileSystem::ArchiveFileSystem()
{
}

ArchiveFileSystem::~ArchiveFileSystem()
{
}

File *ArchiveFileSystem::Open_File(char const *filename, int mode)
{
    return nullptr;
}

bool ArchiveFileSystem::Does_File_Exist(char const *filename)
{
    return false;
}

void ArchiveFileSystem::Load_Archives_From_Dir(AsciiString dir, AsciiString filter, bool read_subdirs)
{
}

void ArchiveFileSystem::Load_Into_Dir_Tree(ArchiveFile const *file, AsciiString const &dir, bool unk)
{
}

bool ArchiveFileSystem::Get_File_Info()
{
    return false;
}

AsciiString ArchiveFileSystem::Get_Archive_Filename_For_File(AsciiString const &filename)
{
    return AsciiString();
}

void ArchiveFileSystem::Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString>>& filelist, bool search_subdirs)
{
    for ( auto it = ArchiveFiles.begin(); it != ArchiveFiles.end(); ++it ) {
        it->second->Get_File_List_From_Dir(subdir, dirpath, filter, filelist, search_subdirs);
    }
}
