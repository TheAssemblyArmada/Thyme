/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implements the ArchiveFileSystem interface on the BIG file format.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "win32bigfilesystem.h"
#include "asciistring.h"
#include "audiomanager.h"
#include "endiantype.h"
#include "file.h"
#include "localfilesystem.h"
#include "registryget.h"
#include "rtsutils.h"
#include "win32bigfile.h"

using rts::FourCC;

void Win32BIGFileSystem::Init()
{
    captainslog_dbgassert(
        g_theLocalFileSystem != nullptr, "TheLocalFileSystem must be initialized before TheArchiveFileSystem.");

    if (g_theLocalFileSystem != nullptr) {
        Load_Big_Files_From_Directory("", "*.big", false);

        Utf8String gen_path;

        Get_String_From_Generals_Registry("", "InstallPath", gen_path);
        captainslog_debug("Retrieved Generals path as '%s' from registry.", gen_path.Str());

        if (gen_path != "") {
            Load_Big_Files_From_Directory(gen_path, "*.big", false);
        }
    }
}

ArchiveFile *Win32BIGFileSystem::Open_Archive_File(const char *filename)
{
    File *file = g_theLocalFileSystem->Open_File(filename, File::READ | File::BINARY);
    Utf8String fullname = filename;
    fullname.To_Lower();
    Win32BIGFile *big = new Win32BIGFile;

    captainslog_debug("Win32BigFileSystem::Open_Archive_File - opening BIG file %s.", filename);

    if (file) {
        // Read and check Big file FourCC, make sure we opened the right thing.
        // BIGF is used in Generals games, BIG4 is used in BFME games.
        uint32_t idbuff;
        file->Read(&idbuff, sizeof(idbuff));

        if (idbuff == FourCC<'B', 'I', 'G', 'F'>::value || idbuff == FourCC<'B', 'I', 'G', '4'>::value) {
            uint32_t arch_size;
            uint32_t file_count;

            // Read information from header and convert to host integer format.
            file->Read(&arch_size, sizeof(arch_size));
            file->Read(&file_count, sizeof(file_count));
            arch_size = le32toh(arch_size);
            file_count = be32toh(file_count);
            captainslog_debug("Win32BigFileSystem::Open_Archive_File - size of archive file is %u bytes.", arch_size);
            captainslog_debug(
                "Win32BigFileSystem::Open_Archive_File - %u files are contained within the archive.", file_count);

            // Seek to first file information
            file->Seek(16, File::START);

            ArchivedFileInfo *info = new ArchivedFileInfo;

            // Process each file info found in the Big file header.
            for (unsigned int i = 0; i < file_count; ++i) {
                int32_t file_size = 0;
                int32_t file_pos = 0;
                char namebuf[BIG_PATH_MAX];

                // Read file size and position in the Big into host integer format.
                file->Read(&file_pos, sizeof(file_pos));
                file->Read(&file_size, sizeof(file_size));
                file_size = be32toh(file_size);
                file_pos = be32toh(file_pos);

                info->size = file_size;
                info->position = file_pos;
                info->archive_name = filename;

                int strlen = 0;
                char *putp = namebuf;

                for (; strlen < BIG_PATH_MAX; ++strlen) {
                    file->Read(putp, sizeof(*putp));

                    if (*putp == '\0') {
                        break;
                    }

                    ++putp;
                }

                captainslog_relassert(
                    strlen < BIG_PATH_MAX, 0xDEAD0002, "Filename string in BIG file header not null terminated");

                // Find the start of the file name
                int name_start = strlen;

                for (; name_start >= 0; --name_start) {
                    if (namebuf[name_start] == '\\' || namebuf[name_start] == '/') {
                        break;
                    }
                }

                // Store the file name in the info struct and then null first char so we
                // can recover the rest of the path.
                info->file_name = &namebuf[name_start + 1];
                info->file_name.To_Lower();
                // captainslog_trace("Base name is '%s'.", &namebuf[name_start + 1]);

                namebuf[name_start + 1] = '\0';

                // captainslog_trace("Path is '%s'.", namebuf);

                big->Add_File(namebuf, info);
            }

            big->Attach_File(file);
            delete info;
            return big;
        } else {
            captainslog_dbgassert(false, "Error reading BIG file identifier in file %s", filename);
            file->Close();
            delete big; // bugfix
            return nullptr;
        }
    } else {
        captainslog_dbgassert(false, "Could not open archive file %s for parsing", filename);
        delete big; // bugfix
        return nullptr;
    }
}

void Win32BIGFileSystem::Close_Archive_File(const char *filename)
{
    auto it = m_archiveFiles.find(filename);

    if (!(it == m_archiveFiles.end())) {
        // If we are removing the music big file, set audio engine accordingly
        if (strcasecmp(filename, "Music.Big") == 0) {
            // Do something with audio engine
            g_theAudio->Stop_Audio(AUDIOAFFECT_MUSIC);
        }

        captainslog_dbgassert(strcasecmp(filename, "Music.Big") == 0,
            "Attempting to close Archive file '%s', need to add code to handle its shutdown correctly.",
            filename);

        if (it->second != nullptr) {
            delete it->second;
        }

        m_archiveFiles.erase(it);
    }
}

bool Win32BIGFileSystem::Load_Big_Files_From_Directory(Utf8String dir, Utf8String filter, bool overwrite)
{
    std::set<Utf8String, rts::less_than_nocase<Utf8String>> file_list;

    g_theLocalFileSystem->Get_File_List_In_Directory(dir, "", filter, file_list, true);
    bool ret = false;

    for (auto it = file_list.begin(); it != file_list.end(); ++it) {
        captainslog_debug(
            "Win32BIGFileSystem::Load_Big_Files_From_Directory - loading %s into the directory tree.", (*it).Str());
        ArchiveFile *arch = Open_Archive_File((*it).Str());

        if (arch != nullptr) {
            Load_Into_Directory_Tree(arch, *it, overwrite);
            m_archiveFiles[*it] = arch;

            captainslog_debug(
                "Win32BIGFileSystem::Load_Big_Files_From_Directory - %s inserted into the archive file map.", (*it).Str());
            ret = true;
        }
    }

    return ret;
}
