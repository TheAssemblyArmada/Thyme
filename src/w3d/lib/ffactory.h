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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _FFACTORY_H_
#define _FFACTORY_H_

#include "fileclass.h"
#include "hooker.h"

class FileFactoryClass
{
public:
    virtual ~FileFactoryClass() {}
    virtual FileClass *Get_File(char const *filename) = 0;
    virtual void Return_File(FileClass *file) = 0;
};

// Pointer wrapper from RAII style use of file classes.
class auto_file_ptr
{
public:
    auto_file_ptr(FileFactoryClass *fact, char const *filename);
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
    FileClass *Get_File(char const *filename);
    void Return_File(FileClass *file);

    static void Hook_Me();
};

inline void RawFileFactoryClass::Hook_Me()
{

}

#define TheWritingFileFactory (Make_Global<RawFileFactoryClass*>(0x00A1EEB8))
//extern RawFileFactoryClass *TheWritingFileFactory;

#define TheFileFactory (Make_Global<FileFactoryClass*>(0x008A99EC))
//extern FileFactoryClass *TheFileFactory;

#endif
