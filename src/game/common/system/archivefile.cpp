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

void ArchiveFile::Get_File_List_From_Dir(AsciiString const &a1, AsciiString const &filepath, AsciiString const &a3, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool a5)
{
    AsciiString path = filepath;
    AsciiString token;
    DetailedArchiveDirectoryInfo *dirp = &ArchiveInfo;


    // Lower case for matching and get first item of the path.
    path.To_Lower();

    for ( path.Next_Token(&token, "\\/"); token.Is_Not_Empty(); path.Next_Token(&token, "\\/") ) {
        // If an element of the path doesn't have a node for our next directory, return.
        if ( dirp->DirInfo.find(token) == dirp->DirInfo.end() ) {
            return;
        }

        dirp = &dirp->DirInfo[token];
    }

    Get_File_List_From_Dir(dirp, filepath, a3, filelist, a5);
}

void ArchiveFile::Get_File_List_From_Dir(DetailedArchiveDirectoryInfo const *dir_info, AsciiString const &filepath, AsciiString const &a3, std::set<AsciiString, rts::less_than_nocase<AsciiString> > &filelist, bool a5)
{
    //TODO
}
