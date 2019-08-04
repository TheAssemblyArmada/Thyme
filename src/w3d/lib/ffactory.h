////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: FFACTORY.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Base class for the W3dLib file io factory.
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
#pragma once

#include "fileclass.h"

class FileFactoryClass
{
public:
    virtual ~FileFactoryClass() {}
    virtual FileClass *Get_File(const char *filename) = 0;
    virtual void Return_File(FileClass *file) = 0;
};

// Pointer wrapper from RAII style use of file classes.
class auto_file_ptr
{
public:
    auto_file_ptr(FileFactoryClass *fact, const char *filename);
    ~auto_file_ptr() { m_factory->Return_File(m_file); }

    operator FileClass*() { return m_file; }
    FileClass &operator*() { return *m_file; }
    FileClass *operator->() { return m_file; }
    FileClass *Get() { return m_file; }

private:
    FileClass *m_file;
    FileFactoryClass *m_factory;
};

class RawFileFactoryClass
{
public:
    FileClass *Get_File(const char *filename);
    void Return_File(FileClass *file);

    static void Hook_Me();
};

#ifdef GAME_DLL
#include "hooker.h"

inline void RawFileFactoryClass::Hook_Me()
{

}

extern RawFileFactoryClass *&g_theWritingFileFactory;
extern FileFactoryClass *&g_theFileFactory;
#else
extern RawFileFactoryClass *g_theWritingFileFactory;
extern FileFactoryClass *g_theFileFactory;
#endif