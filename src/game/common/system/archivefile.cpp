#include "archivefile.h"

ArchiveFile::ArchiveFile() :
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
