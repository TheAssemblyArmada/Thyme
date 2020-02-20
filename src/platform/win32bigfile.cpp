/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling .big file archives.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "win32bigfile.h"
#include "localfilesystem.h"
#include "ramfile.h"
#include "streamingarchivefile.h"

bool Win32BIGFile::Get_File_Info(Utf8String const &name, FileInfo *info)
{
    ArchivedFileInfo *arch_info = Get_Archived_File_Info(name);

    if (arch_info == nullptr) {
        return false;
    }

    g_theLocalFileSystem->Get_File_Info(m_backingFile->Get_File_Name(), info);
    info->file_size_high = 0;
    info->file_size_low = arch_info->size;

    return true;
}

File *Win32BIGFile::Open_File(const char *filename, int mode)
{
    ArchivedFileInfo *arch_info = Get_Archived_File_Info(filename);

    if (arch_info == nullptr) {
        captainslog_trace("Couldn't find info for the requested file.");
        return nullptr;
    }

    RAMFile *file = nullptr;

    if ((mode & File::READ) != 0) {
        file = new StreamingArchiveFile;
    } else {
        file = new RAMFile;
    }

    file->Set_Del_On_Close(true);

    if (!file->Open_From_Archive(m_backingFile, arch_info->file_name, arch_info->position, arch_info->size)) {
        file->Close();

        return nullptr;
    }

    if ((mode & File::WRITE) != 0) {
        File *localfile = g_theLocalFileSystem->Open_File(filename, mode);

        if (localfile != nullptr) {
            file->Copy_To_File(localfile);
        }

        file->Close();

        return localfile;
    }

    return file;
}
