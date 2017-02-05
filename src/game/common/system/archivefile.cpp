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
        auto dir_it = dirp->DirInfo.find(token);

        if ( dir_it == dirp->DirInfo.end() ) {
            return nullptr;
        }
        
        dirp = &dir_it->second;
        path.Next_Token(&token, "\\/");
    }

    // Assuming we didn't run out of directories to try, find the file
    // in the reached directory.
    auto file_it = dirp->FileInfo.find(token);

    if ( file_it == dirp->FileInfo.end() ) {
        return nullptr;
    }

    return &file_it->second;
}

void ArchiveFile::Add_File(AsciiString const &filepath, ArchivedFileInfo const *info)
{
    AsciiString path = filepath;
    AsciiString token;
    DetailedArchiveDirectoryInfo *dirp = &ArchiveInfo;


    // Lower case for matching and get first item of the path.
    path.To_Lower();

    for ( path.Next_Token(&token, "\\/"); token.Is_Not_Empty(); path.Next_Token(&token, "\\/") ) {
        // If an element of the path doesn't have a directory node, add it.
        if ( dirp->DirInfo.find(token) == dirp->DirInfo.end() ) {
            dirp->DirInfo[token].Name = token;
        }

        dirp = &dirp->DirInfo[token];
    }

    dirp->FileInfo[info->Name] = *info;
}

void ArchiveFile::Attach_File(File *file)
{
    if ( BackingFile != nullptr ) {
        BackingFile->Close();
        BackingFile = nullptr;
    }

    BackingFile = file;
}

void ArchiveFile::Get_File_List_From_Dir(AsciiString const &a1, AsciiString const &filepath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdir)
{
    AsciiString path = filepath;
    AsciiString token;
    DetailedArchiveDirectoryInfo *dirp = &ArchiveInfo;


    // Lower case for matching and get first item of the path.
    path.To_Lower();

    // Go to the last InfoNode in the path to extract file contents from.
    for ( path.Next_Token(&token, "\\/"); token.Is_Not_Empty(); path.Next_Token(&token, "\\/") ) {
        // If an element of the path doesn't have a node for our next directory, return.
        if ( dirp->DirInfo.find(token) == dirp->DirInfo.end() ) {
            return;
        }

        dirp = &dirp->DirInfo[token];
    }

    Get_File_List_From_Dir(dirp, filepath, filter, filelist, search_subdir);
}

void ArchiveFile::Get_File_List_From_Dir(DetailedArchiveDirectoryInfo const *dir_info, AsciiString const &filepath, AsciiString const &filter, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool search_subdir)
{
    //TODO
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

    char const *cstring = string.Peek();
    char const *csearch = search.Peek();

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
