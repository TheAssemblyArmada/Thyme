/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief W3DLib style wrapper around filesystem.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef W3DFILESYSTEM_H
#define W3DFILESYSTEM_H

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
    virtual ~W3DFileSystem();

    virtual FileClass *Get_File(const char *filename);
    virtual void Return_File(FileClass *file);

#ifndef THYME_STANDALONE
    FileClass *Get_File_NV(const char *filename);
    void Return_File_NV(FileClass *file);

    static void Hook_Me();
#endif
};

class GameFileClass : public FileClass
{
public:
    GameFileClass(const char *filename = nullptr);
    virtual ~GameFileClass();

    virtual const char *File_Name();
    virtual const char *Set_Name(const char *filename);
    virtual bool Create();
    virtual bool Delete();
    virtual bool Is_Available(bool forced = false);
    virtual bool const Is_Open();
    virtual bool Open(const char *filename, int rights = FM_READ);
    virtual bool Open(int rights = FM_READ);
    virtual int Read(void *buffer, int length);
    virtual off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT);
    virtual off_t Size();
    virtual int Write(void const *buffer, int size);
    virtual void Close();

private:
    File *m_theFile;
    bool m_fileExists;
    char m_filePath[PATH_MAX];
    char m_filename[PATH_MAX];
};

#ifndef THYME_STANDALONE
inline void W3DFileSystem::Hook_Me()
{
    Hook_Method(0x00763880, &Get_File_NV);
    Hook_Method(0x00815370, &Return_File_NV);
}
#endif

#endif // W3DFILESYSTEM_H
