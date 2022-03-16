/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Interface for file IO.
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
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

enum FileOpenType
{
    FM_CLOSED = 0,
    FM_READ = 1,
    FM_WRITE = 2,
    FM_READ_WRITE = FM_READ | FM_WRITE,
};

enum FileSeekType
{
    FS_SEEK_START = 0, // Seek to position relative to the start of file.
    FS_SEEK_CURRENT = 1, // Seek to position relative to the current offset.
    FS_SEEK_END = 2, // Seek to position relative to the end of file.
};

class FileClass
{
public:
    FileClass() {}
    virtual ~FileClass() {}

    virtual const char *File_Name() = 0;
    virtual const char *Set_Name(const char *filename) = 0;
    virtual bool Create() = 0;
    virtual bool Delete() = 0;
    virtual bool Is_Available(bool forced = false) = 0;
    virtual bool Is_Open() = 0;
    virtual bool Open(const char *filename, int rights = FM_READ) = 0;
    virtual bool Open(int rights = FM_READ) = 0;
    virtual int Read(void *buffer, int length) = 0;
    virtual off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT) = 0;
    virtual off_t Tell();
    virtual off_t Size() = 0;
    virtual int Write(void const *buffer, int size) = 0;
    virtual void Close() = 0;
    virtual time_t Get_Date_Time();
    virtual bool Set_Date_Time(time_t date_time);
#ifdef PLATFORM_WINDOWS
    virtual HANDLE Get_File_Handle() { return INVALID_HANDLE_VALUE; }
#else
    virtual int Get_File_Handle() { return -1; }
#endif
};
