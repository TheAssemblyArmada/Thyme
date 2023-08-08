/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief W3DLib style wrapper around filesystem.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "ffactory.h"
#include "file.h"
#include "fileclass.h"

enum GameFileType
{
    GAME_FILE_UNK,
    GAME_FILE_W3D,
    GAME_FILE_TGA,
    GAME_FILE_DDS,
};

class W3DFileSystem : public FileFactoryClass
{
public:
    W3DFileSystem();
    ~W3DFileSystem() override;

    FileClass *Get_File(const char *filename) override;
    void Return_File(FileClass *file) override;
};

class GameFileClass : public FileClass
{
public:
    GameFileClass(const char *filename = nullptr);
    ~GameFileClass() override;

    const char *File_Name() override;
    const char *Set_Name(const char *filename) override;
    bool Create() override;
    bool Delete() override;
    bool Is_Available(bool forced = false) override;
    bool Is_Open() override;
    bool Open(const char *filename, int rights = FM_READ) override;
    bool Open(int rights = FM_READ) override;
    int Read(void *buffer, int length) override;
    off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT) override;
    off_t Size() override;
    int Write(void const *buffer, int size) override;
    void Close() override;

private:
    File *m_theFile;
    bool m_fileExists;
    char m_filePath[PATH_MAX];
    char m_filename[PATH_MAX];
};

#ifdef GAME_DLL
extern W3DFileSystem *&g_theW3DFileSystem;
#else
extern W3DFileSystem *g_theW3DFileSystem;
#endif
