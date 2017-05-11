////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32BIGFILESYSTEM.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Implements the ArchiveFileSystem interface on the BIG file
//                 format.
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
#include "win32bigfilesystem.h"
#include "asciistring.h"
#include "endiantype.h"
#include "file.h"
#include "localfilesystem.h"
#include "hooker.h"
#include "rtsutils.h"
#include "win32bigfile.h"

using rts::FourCC;

#define GetStringFromGeneralsRegistry(x,y,z) Call_Function<void, AsciiString, AsciiString, AsciiString const &>(0x004988A0, x, y, z)

void Win32BIGFileSystem::Init()
{
    DEBUG_LOG("Initialising BIG file system.\n");
    if ( TheLocalFileSystem != nullptr ) {
        Load_Archives_From_Dir("", "*.big", false);

        AsciiString gen_path;

        GetStringFromGeneralsRegistry("", "InstallPath", gen_path);

        DEBUG_LOG("Retrieved Generals path as '%s' from registry.\n", gen_path.Str());

        if ( !gen_path.Is_Empty() ) {
            Load_Archives_From_Dir(gen_path, "*.big", false);
        }
    }
}

ArchiveFile *Win32BIGFileSystem::Open_Archive_File(char const *filename)
{
    uint32_t idbuff;

    File *file = TheLocalFileSystem->Open_File(filename, File::READ | File::BINARY);
    Win32BIGFile *big = new Win32BIGFile;
    
    AsciiString fullname = filename;
    fullname.To_Lower();

    if ( file == nullptr ) {
        DEBUG_LOG("Couldn't open local archive file '%s'.\n", filename);

        return nullptr;
    }

    // Read and check Big file FourCC, make sure we opened the right thing.
    // BIGF is used in Generals games, BIG4 is used in BFME games.
    file->Read(&idbuff, sizeof(idbuff));

    if ( idbuff != FourCC<'B', 'I', 'G', 'F'>::value && idbuff != FourCC<'B', 'I', 'G', '4'>::value ) {
        DEBUG_LOG("Opened file '%s' does not have correct Big File FourCC, closing.\n", filename);
        file->Close();

        return nullptr;
    }

    uint32_t arch_size;
    uint32_t file_count;
    //uint32_t first_offset;

    // Read information from header and convert to host integer format.
    file->Read(&arch_size, sizeof(arch_size));
    file->Read(&file_count, sizeof(file_count));
    arch_size = le32toh(arch_size);
    file_count = be32toh(file_count);
    
    //DEBUG_LOG("Big file is '%u' bytes long and contains '%u' files.\n", arch_size, file_count);

    // Seek to first file information
    file->Seek(16, File::START);

    ArchivedFileInfo *info = new ArchivedFileInfo;

    // Process each file info found in the Big file header.
    for ( unsigned int i = 0; i < file_count; ++i ) {
        int32_t file_size = 0;
        int32_t file_pos = 0;
        char namebuf[PATH_MAX];

        // Read file size and position in the Big into host integer format.
        file->Read(&file_pos, sizeof(file_pos));
        file->Read(&file_size, sizeof(file_size));
        file_size = be32toh(file_size);
        file_pos = be32toh(file_pos);

        info->Size = file_size;
        info->Position = file_pos;
        info->ArchiveName = filename;

        int strlen = 0;
        char *putp = namebuf;

        for ( ; strlen < PATH_MAX; ++strlen ) {
            file->Read(putp, sizeof(*putp));

            if ( *putp == '\0' ) {
                break;
            }

            ++putp;
        }

        ASSERT_THROW_PRINT(strlen < PATH_MAX, 0xDEAD0002, "Filename string in BIG file header not null terminated");

        //DEBUG_LOG("Recovered a file path of '%s' with size '%d' and position '%d'.\n", namebuf, file_size, file_pos);

        // Find the start of the file name
        int name_start = strlen;

        for ( ; name_start >= 0; --name_start ) {
            if ( namebuf[name_start] == '\\' || namebuf[name_start] == '/' ) {
                break;
            }
        }

        // Store the file name in the info struct and then null first char so we
        // can recover the rest of the path.
        info->FileName = &namebuf[name_start + 1];
        info->FileName.To_Lower();
        //DEBUG_LOG("Base name is '%s'.\n", &namebuf[name_start + 1]);

        namebuf[name_start + 1] = '\0';

        //DEBUG_LOG("Path is '%s'.\n", namebuf);

        AsciiString file_path = namebuf;
        file_path += info->FileName;
        big->Add_File(namebuf, info);
    }

    big->Attach_File(file);

    delete info;

    return big;
}

void Win32BIGFileSystem::Close_Archive_File(char const *filename)
{
    auto it = ArchiveFiles.find(filename);

    if ( it != ArchiveFiles.end() ) {
        // If we are removing the music big file, set audio engine accordingly
        if ( strcasecmp(filename, "Music.Big") == 0 ) {
            DEBUG_LOG("Something is supposed to happen to audio engine here.\n");
            //Do something with audio engine
        }

        if ( it->second != nullptr ) {
            delete it->second;
        }

        ArchiveFiles.erase(it);
    }
}

void Win32BIGFileSystem::Load_Archives_From_Dir(AsciiString dir, AsciiString filter, bool read_subdirs)
{
    std::set<AsciiString, rts::less_than_nocase<AsciiString>> file_list;

    TheLocalFileSystem->Get_File_List_From_Dir(dir, "", filter, file_list, read_subdirs);

    for ( auto it = file_list.begin(); it != file_list.end(); ++it ) {
        ArchiveFile *arch = Open_Archive_File((*it).Str());

        if ( arch != nullptr ) {
            Load_Into_Dir_Tree(arch, *it, read_subdirs);
            ArchiveFiles[*it] = arch;
        }
    }
}
