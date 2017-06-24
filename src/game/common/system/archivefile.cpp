////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ARCHIVEFILE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Base class for archive file handling.
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
#include "archivefile.h"
#include "file.h"

ArchiveFile::ArchiveFile() :
    BackingFile(nullptr),
    ArchiveInfo()
{

}

ArchivedFileInfo *ArchiveFile::Get_Archived_File_Info(AsciiString const &filename)
{
    AsciiString path = filename;
    AsciiString token;
    DetailedArchiveDirectoryInfo *dirp = &ArchiveInfo;

    // Lower case for matching and get first item of the path.
    path.To_Lower();
    path.Next_Token(&token, "\\/");

    // Consider existence of '.' to indicate file as all should have .ext format
    // checks the remaining path does not contain one to catch directories in path
    // that do.
    while ( strchr(token.Str(), '.') == nullptr || strchr(path.Str(), '.') != nullptr ) {
        if ( dirp->Directories.find(token) == dirp->Directories.end() ) {
            return nullptr;
        }
        
        dirp = &dirp->Directories[token];
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    auto file_it = dirp->Files.find(token);

    if ( file_it == dirp->Files.end() ) {
        return nullptr;
    }

    return &file_it->second;
}

void ArchiveFile::Add_File(AsciiString const &filepath, ArchivedFileInfo const *info)
{
    //DEBUG_LOG("Adding '%s' to interal archive for '%s'.\n", filepath.Str(), info->ArchiveName.Str());
    AsciiString path = filepath;
    AsciiString token;
    DetailedArchiveDirectoryInfo *dirp = &ArchiveInfo;


    // Lower case for matching and get first item of the path.
    path.To_Lower();

    for ( path.Next_Token(&token, "\\/"); !token.Is_Empty(); path.Next_Token(&token, "\\/") ) {
        // If an element of the path doesn't have a directory node, add it.
        //DEBUG_LOG("Searching for path element '%s'.\n", token.Str());
        if ( dirp->Directories.find(token) == dirp->Directories.end() ) {
            //DEBUG_LOG("Adding path element '%s'.\n", token.Str());
            dirp->Directories[token].Name = token;
        }

        dirp = &dirp->Directories[token];
    }

    dirp->Files[info->FileName] = *info;
}

void ArchiveFile::Attach_File(File *file)
{
    if ( BackingFile != nullptr ) {
        BackingFile->Close();
        BackingFile = nullptr;
    }

    BackingFile = file;
}

void ArchiveFile::Get_File_List_From_Dir(AsciiString const &subdir, AsciiString const &dirpath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdir) const
{
    AsciiString path = dirpath;
    AsciiString token;
    DetailedArchiveDirectoryInfo const *dirp = &ArchiveInfo;


    // Lower case for matching and get first item of the path.
    path.To_Lower();

    // Go to the last InfoNode in the path to extract file contents from.
    for ( path.Next_Token(&token, "\\/"); token.Is_Not_Empty(); path.Next_Token(&token, "\\/") ) {
        // If an element of the path doesn't have a node for our next directory, return.
        if ( dirp->Directories.find(token) == dirp->Directories.end() ) {
            return;
        }

        dirp = &dirp->Directories[token];
    }

    Get_File_List_From_Dir(dirp, dirpath, filter, filelist, search_subdir);
}

void ArchiveFile::Get_File_List_From_Dir(DetailedArchiveDirectoryInfo const *dir_info, AsciiString const &dirpath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdir) const
{
    // Add the files from any subdirectories, recursive call.
    for ( auto it = dir_info->Directories.begin(); it != dir_info->Directories.end(); ++it ) {
        AsciiString path = dirpath;

        if ( !path.Is_Empty() && !path.Ends_With("\\") && !path.Ends_With("/") ) {
            path += "/";
        }

        path += it->second.Name;
        Get_File_List_From_Dir(&(it->second), path, filter, filelist, search_subdir);
    }

    // Add all the files that match the search pattern.
    for ( auto it = dir_info->Files.begin(); it != dir_info->Files.end(); ++it ) {
        if ( Search_String_Matches(it->second.FileName, filter) ) {
            AsciiString path = dirpath;

            if ( !path.Is_Empty() && !path.Ends_With("\\") && !path.Ends_With("/") ) {
                path += "/";
            }
            
            path += it->second.FileName;
            filelist.insert(path);
        }
    }
}

// Helper funtion to check if a string matches the search string.
bool Search_String_Matches(AsciiString string, AsciiString search)
{
    // Trivial case if first string is empty.
    if ( string.Is_Empty() ) {
        return search.Is_Empty();
    }

    // If there is no seach string, there cannot be a match.
    if ( search.Is_Empty() ) {
        return false;
    }

    const char *cstring = string.Peek();
    const char *csearch = search.Peek();

    // ? is wildcard for a single character.
    // * is wildcard for a run of characters.
    while ( true ) {
        if ( *cstring != *csearch  && *csearch != '?' && *csearch != '*' ) {
            return false;
        }

        // Move on to the next character to check
        if ( *cstring == *csearch || *csearch == '?' ) {
            ++cstring;
            ++csearch;
        // Move on to the rest of the search string and
        // recursively check to see if it matches.
        } else if ( *csearch == '*' ) {
            ++csearch;

            if ( *csearch == '\0' ) {
                return true;
            }

            while ( *cstring != '\0' ) {
                if ( Search_String_Matches(cstring, csearch) ) {
                    return true;
                }

                ++cstring;
            }
        }

        if ( *cstring == '\0' ) {
            break;
        }

        if ( *csearch == '\0' ) {
            return false;
        }
    }

    return *csearch == '\0';
}
