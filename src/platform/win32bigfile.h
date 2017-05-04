////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32BIGFILE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Base class for archive file handling.
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

#ifndef WIN32BIGFILE_H
#define WIN32BIGFILE_H

#include "archivefile.h"

class Win32BIGFile : public ArchiveFile
{
public:
    Win32BIGFile() {}
    virtual ~Win32BIGFile() {}

    virtual bool Get_File_Info(AsciiString const &name, FileInfo *info);
    virtual File *Open_File(char const *filename, int mode);
    virtual void Close_All_Files() {};
    virtual AsciiString Get_Name() { return FileName; }
    virtual AsciiString Get_Path() { return FilePath; }
    virtual void Set_Search_Priority(int priority) {}
    virtual void Close() {}

private:
    AsciiString FileName;
    AsciiString FilePath;
};

#endif // _WIN32BIGFILE_H
