////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: W3DFILESYSTEM.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: W3DLib style wrapper around filesystem.
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _W3DFILESYSTEM_H_
#define _W3DFILESYSTEM_H_

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

    virtual FileClass *Get_File(char const *filename);
    virtual void Return_File(FileClass *file);
};

class GameFileClass : public FileClass
{
public:
    GameFileClass(char const *filename = nullptr);
    virtual ~GameFileClass();

    virtual char const *File_Name();
    virtual char const *Set_Name(char const *filename);
    virtual bool Create();
    virtual bool Delete();
    virtual bool Is_Available(bool forced = false);
    virtual bool const Is_Open();
    virtual bool Open(char const *filename, int rights = FM_READ);
    virtual bool Open(int rights = FM_READ);
    virtual int Read(void *buffer, int length);
    virtual int Seek(int offset, int whence = FS_SEEK_CURRENT);
    virtual int Size();
    virtual int Write(void const *buffer, int size);
    virtual void Close();

private:
    File *m_theFile;
    bool m_fileExists;
    char m_filePath[PATH_MAX];
    char m_filename[PATH_MAX];
};

#endif // _W3DFILESYSTEM_H_