////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32BIGFILE.H
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
#include "win32bigfile.h"
#include "localfilesystem.h"
#include "ramfile.h"
#include "streamingarchivefile.h"

bool Win32BIGFile::Get_File_Info(AsciiString const &name, FileInfo *info)
{
    ArchivedFileInfo *arch_info = Get_Archived_File_Info(name);

    if ( arch_info == nullptr ) {
        return false;
    }

    TheLocalFileSystem->Get_File_Info(BackingFile->Get_File_Name(), info);
    info->FileSizeHigh = 0;
    info->FileSizeLow = arch_info->Size;

    return true;
}

File *Win32BIGFile::Open_File(char const *filename, int mode)
{
    ArchivedFileInfo *arch_info = Get_Archived_File_Info(filename);

    if ( arch_info == nullptr ) {
        DEBUG_LOG("Couldn't find info for the requested file.\n");
        return nullptr;
    }

    RAMFile *file = nullptr;

    if ( (mode & File::READ) != 0 ) {
        file = new StreamingArchiveFile;
    } else {
        file = new RAMFile;
    }

    file->Set_Del_On_Close(true);

    if ( !file->Open_From_Archive(BackingFile, arch_info->FileName, arch_info->Position, arch_info->Size) ) {
        file->Close();

        return nullptr;
    }

    if ( (mode & File::WRITE) != 0 ) {
        File *localfile = TheLocalFileSystem->Open_File(filename, mode);

        if ( localfile != nullptr ) {
            file->Copy_To_File(localfile);
        }

        file->Close();

        return localfile;
    }

    return file;
}
