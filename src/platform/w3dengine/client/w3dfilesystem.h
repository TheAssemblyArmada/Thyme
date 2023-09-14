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
#include "wwfile.h"

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
    virtual ~W3DFileSystem();

    virtual FileClass *Get_File(const char *filename) override;
    virtual void Return_File(FileClass *file) override;
};

class GameFileClass : public FileClass
{
public:
    GameFileClass(const char *filename = nullptr);
    virtual ~GameFileClass();

    virtual const char *File_Name() override;
    virtual const char *Set_Name(const char *filename) override;
    virtual bool Create() override;
    virtual bool Delete() override;
    virtual bool Is_Available(bool forced = false) override;
    virtual bool Is_Open() override;
    virtual bool Open(const char *filename, int rights = FM_READ) override;
    virtual bool Open(int rights = FM_READ) override;
    virtual int Read(void *buffer, int length) override;
    virtual off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT) override;
    virtual off_t Size() override;
    virtual int Write(void const *buffer, int size) override;
    virtual void Close() override;

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
