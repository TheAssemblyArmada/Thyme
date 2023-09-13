/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief RCFile
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

#ifdef PLATFORM_WINDOWS
#include <Windows.h>

class RCFileClass : public FileClass
{
private:
    char *m_name;
    int m_unk;
    char *m_startPointer;
    char *m_currentPointer;
    char *m_endPointer;

public:
    RCFileClass(HMODULE module, const char *name);
    virtual ~RCFileClass() override;
    virtual const char *File_Name() override { return m_name; }
    virtual const char *Set_Name(const char *filename) override;
    virtual bool Create() override { return false; }
    virtual bool Delete() override { return false; }
    virtual bool Is_Available(bool forced = false) override { return Is_Open(); }
    virtual bool Is_Open() override { return m_startPointer != nullptr; }
    virtual bool Open(const char *filename, int rights = FM_READ) override { return Is_Open(); }
    virtual bool Open(int rights = FM_READ) override { return Is_Open(); }
    virtual int Read(void *buffer, int length) override;
    virtual off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT) override;
    virtual off_t Size() override { return m_endPointer - m_startPointer; }
    virtual int Write(void const *buffer, int size) override { return 0; }
    virtual void Close() override {}
};
#endif
