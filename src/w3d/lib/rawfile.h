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

#include "always.h"
#include "wwfile.h"
#include "wwstring.h"

class RawFileClass : public FileClass
{
public:
    RawFileClass();
    RawFileClass(const char *filename);

    virtual ~RawFileClass() override { Reset(); };
    virtual const char *File_Name() override { return m_filename; };
    virtual const char *Set_Name(const char *filename) override;
    virtual bool Create() override;
    virtual bool Delete() override;
    virtual bool Is_Available(bool forced = false) override;
#ifdef PLATFORM_WINDOWS
    virtual bool Is_Open() override { return m_handle != (HANDLE)(LONG_PTR)-1; };
#else
    virtual bool Is_Open() override { return m_handle != -1; };
#endif
    virtual bool Open(const char *filename, int rights = FM_READ) override;
    virtual bool Open(int rights = FM_READ) override;
    virtual int Read(void *buffer, int length) override;
    virtual off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT) override;
    virtual off_t Tell() override { return Seek(0, FS_SEEK_CURRENT); }
    virtual off_t Size() override;
    virtual int Write(void const *buffer, int length) override;
    virtual void Close() override;
    virtual time_t Get_Date_Time() override;
    virtual bool Set_Date_Time(time_t date_time) override;
    virtual void Error(int error, bool can_retry = false, const char *filename = nullptr);
#ifdef PLATFORM_WINDOWS
    virtual HANDLE Get_File_Handle() override { return m_handle; }
#else
    virtual int Get_File_Handle() override { return m_handle; }
#endif
    virtual void Bias(int start, int length = -1);
#ifdef PLATFORM_WINDOWS
    virtual void Attach(HANDLE handle, int rights);
#else
    virtual void Attach(int handle, int rights);
#endif
    virtual void Detach();

private:
    void Reset();
    off_t Raw_Seek(off_t offset, int whence = FS_SEEK_CURRENT);

protected:
    int m_rights; // This is the current rights to this file.
    int m_biasStart; // Offset from true start to treat as start of file
    int m_biasLength; // Logical length of file.
#ifdef PLATFORM_WINDOWS
    HANDLE m_handle;
#else
    int m_handle;
#endif
    StringClass m_filename;
    time_t m_dateTime;
};
