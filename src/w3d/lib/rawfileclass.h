////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: RAWFILECLASS.H
//
//        Author:: CCHyper, OmniBlade
//
//  Contributors:: 
//
//   Description:: FileClass for reading files with raw OS API calls.
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

#ifndef _RAWFILECLASS_H_
#define _RAWFILECLASS_H_

#include	"fileclass.h"

class RawFileClass : public FileClass
{
public:
    RawFileClass() : FileClass(), m_rights(FM_CLOSED), m_biasStart(0), m_biasLength(-1), m_handle(-1), m_filename(nullptr), m_isAllocated(false), m_dateTime(0) { Set_Name(""); };
    RawFileClass(char const *filename) : FileClass(), m_rights(FM_CLOSED), m_biasStart(0), m_biasLength(-1), m_handle(-1), m_filename(nullptr), m_isAllocated(false), m_dateTime(0) { Set_Name(filename); };
    
    virtual ~RawFileClass() { Reset(); };
    virtual char const *File_Name() { return m_filename; };
    virtual char const *Set_Name(char const *filename);
    virtual bool Create();
    virtual bool Delete();
    virtual bool Is_Available(bool forced = false);
    virtual bool const Is_Open() { return m_handle != -1; };
    virtual bool Open(char const *filename, int rights = FM_READ);
    virtual bool Open(int rights = FM_READ);
    virtual int Read(void *buffer, int length);
    virtual int Seek(int offset, int whence = FS_SEEK_CURRENT);
    virtual int Tell() { return Seek(0, FS_SEEK_CURRENT); }
    virtual int Size();
    virtual int Write(void const *buffer, int length);
    virtual void Close();
    virtual time_t Get_Date_Time();
    virtual bool Set_Date_Time(time_t date_time);
    virtual int Get_File_Handle() { return m_handle; }

    virtual void Error(int error, bool can_retry = false, char const *filename = nullptr);
    virtual void Bias(int start, int length = -1);
    virtual void Attach(int handle, int rights);
    virtual void Detach();

private:
    void Reset();
    int Raw_Seek(int offset, int whence = FS_SEEK_CURRENT);

protected:
    int m_rights;     // This is the current rights to this file.
    int m_biasStart;  // Offset from true start to treat as start of file
    int m_biasLength; // Logical length of file.
    int m_handle;
    char *m_filename;
    time_t m_dateTime;
    bool m_isAllocated;
};

#endif // _RAWFILECLASS_H_
