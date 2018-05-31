/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief FileClass for reading files with raw OS API calls.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "fileclass.h"

class RawFileClass : public FileClass
{
public:
    RawFileClass();
    RawFileClass(const char *filename);

    virtual ~RawFileClass() { Reset(); };
    virtual const char *File_Name() { return m_filename; };
    virtual const char *Set_Name(const char *filename);
    virtual bool Create();
    virtual bool Delete();
    virtual bool Is_Available(bool forced = false);
    virtual bool const Is_Open() { return m_handle != -1; };
    virtual bool Open(const char *filename, int rights = FM_READ);
    virtual bool Open(int rights = FM_READ);
    virtual int Read(void *buffer, int length);
    virtual off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT);
    virtual off_t Tell() { return Seek(0, FS_SEEK_CURRENT); }
    virtual off_t Size();
    virtual int Write(void const *buffer, int length);
    virtual void Close();
    virtual time_t Get_Date_Time();
    virtual bool Set_Date_Time(time_t date_time);
    virtual int Get_File_Handle() { return m_handle; }

    virtual void Error(int error, bool can_retry = false, const char *filename = nullptr);
    virtual void Bias(int start, int length = -1);
    virtual void Attach(int handle, int rights);
    virtual void Detach();

private:
    void Reset();
    int Raw_Seek(int offset, int whence = FS_SEEK_CURRENT);

protected:
    int m_rights; // This is the current rights to this file.
    int m_biasStart; // Offset from true start to treat as start of file
    int m_biasLength; // Logical length of file.
    int m_handle;
    char *m_filename;
    time_t m_dateTime;
    bool m_isAllocated;
};