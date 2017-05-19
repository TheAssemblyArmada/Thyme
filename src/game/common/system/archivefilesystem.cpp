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
#include "globaldata.h"

ArchiveFileSystem::ArchiveFileSystem()
{
}

ArchiveFileSystem::~ArchiveFileSystem()
{
}

File *ArchiveFileSystem::Open_File(char const *filename, int mode)
{
    AsciiString archive = Get_Archive_Filename_For_File(filename);

    if ( archive.Is_Empty() ) {
        return nullptr;
    }

    ArchiveFile *file = ArchiveFiles[archive];

    DEBUG_ASSERT_PRINT(file != nullptr, "Did not find matching archive file.\n");

    return file->Open_File(filename, mode);
}

bool ArchiveFileSystem::Does_File_Exist(char const *filename)
{
    AsciiString path = filename;
    AsciiString token;
    ArchivedDirectoryInfo *dirp = &ArchiveDirInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while ( strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr ) {
        // If we ran out of directories, we don't have the directory that has the file.
        if ( dirp->Directories.find(token) == dirp->Directories.end() ) {
            return false;
        }

        dirp = &dirp->Directories[token];
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    if ( dirp->Files.find(token) == dirp->Files.end() ) {
        return false;
    }

    return true;
}

// Loads an archive file into the virtual directory tree. The over write option allows it to use this archive to
// replace the backing for a file name if it already has an entry in the tree.
void ArchiveFileSystem::Load_Into_Dir_Tree(ArchiveFile const *file, AsciiString const &archive_path, bool overwrite)
{
    std::set<AsciiString, rts::less_than_nocase<AsciiString>> file_list;
    
    // Retrieve a list of files in the archive
    file->Get_File_List_From_Dir("", "", "*", file_list, true);

    for ( auto it = file_list.begin(); it != file_list.end(); ++it ) {
        AsciiString path = *it;
        AsciiString token;
        //AsciiString fullname;
        ArchivedDirectoryInfo *dirp = &ArchiveDirInfo;

        // Lower case for matching.
        path.To_Lower();

        // Consider existence of '.' to indicate file as all should have .ext format
        // checks the remaining path does not contain one to catch directories in path
        // that do.
        while ( path.Next_Token(&token, "\\/") && (strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr) ) {
            // If we can't find the directory in our map, make it.
            if ( dirp->Directories.find(token) == dirp->Directories.end() ) {
                dirp->Directories[token].Name = token;
            }

            dirp = &dirp->Directories[token];
            //fullname += token;
            //fullname += "/";
        }

        if ( dirp->Files.find(token) == dirp->Files.end() || overwrite ) {
            dirp->Files[token] = archive_path;
        }
    }
}

bool ArchiveFileSystem::Get_File_Info(AsciiString const &name, FileInfo *info)
{
    if ( info == nullptr || name.Is_Empty() ) {
        return false;
    }

    // Find the archive that corresponds to this file name.
    AsciiString archive = Get_Archive_Filename_For_File(name);

    // Find the archive file pointer for the archive name we retrieved.
    if ( ArchiveFiles.find(archive) == ArchiveFiles.end() ) {
        return false;
    }

    return ArchiveFiles[archive]->Get_File_Info(name, info);
}

// Returns the filname of the archive file containing the passed in file name. 
AsciiString ArchiveFileSystem::Get_Archive_Filename_For_File(AsciiString const &filename)
{
    AsciiString path = filename;
    AsciiString token;
    ArchivedDirectoryInfo *dirp = &ArchiveDirInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while ( strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr ) {
        if ( dirp->Directories.find(token) == dirp->Directories.end() ) {
            return AsciiString();
        }

        dirp = &dirp->Directories[token];
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    if ( dirp->Files.find(token) == dirp->Files.end() ) {
        return AsciiString();
    }

    return dirp->Files[token];
}

// Populates a std::set of file paths based on the passed in filter and path to examine.
void ArchiveFileSystem::Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString>> &filelist, bool search_subdirs)
{
    // Get files from all archive files.
    for ( auto it = ArchiveFiles.begin(); it != ArchiveFiles.end(); ++it ) {
        it->second->Get_File_List_From_Dir(subdir, dirpath, filter, filelist, search_subdirs);
    }
}

// Load mods based on two path options set in the global data fields m_userModFile and 
// m_userModDirectory. These options are parsed from the -mod command line option.
// See GlobalData.h
void ArchiveFileSystem::Load_Mods()
{
    if ( !g_theWriteableGlobalData->m_userModFile.Is_Empty() ) {
        DEBUG_LOG("Loading mod file '%s'.\n", g_theWriteableGlobalData->m_userModFile.Str());
        ArchiveFile *file = Open_Archive_File(g_theWriteableGlobalData->m_userModFile.Str());

        if ( file != nullptr ) {
            Load_Into_Dir_Tree(file, g_theWriteableGlobalData->m_userModFile, true);
            ArchiveFiles[g_theWriteableGlobalData->m_userModFile] = file;
        }
    }

    if ( !g_theWriteableGlobalData->m_userModDirectory.Is_Empty() ) {
        DEBUG_LOG("Loading mod files from '%s'.\n", g_theWriteableGlobalData->m_userModDirectory.Str());
        Load_Archives_From_Dir(g_theWriteableGlobalData->m_userModDirectory, "*.big", true);
    }
}
