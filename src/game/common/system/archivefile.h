////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ARCHIVEFILE.H
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _ARCHIVEFILE_H_
#define _ARCHIVEFILE_H_

#include "asciistring.h"
#include <map>

struct FileInfo;
class File;
class StreamingArchiveFile;

struct ArchivedFileInfo
{
    AsciiString Name;
    AsciiString unk2;
    int Position;
    int Size;
};

struct DetailedArchiveDirectoryInfo
{
    AsciiString Name;
    std::map<AsciiString, DetailedArchiveDirectoryInfo> DirInfo;
    std::map<AsciiString, ArchivedFileInfo> FileInfo;
};

class ArchiveFile
{
    public:
        ArchiveFile();
        virtual ~ArchiveFile() {}

        virtual bool Get_File_Info(AsciiString const &name, FileInfo *info) = 0;
        virtual StreamingArchiveFile *Open_File(char const *filename, int mode) = 0;
        virtual void Close_All_Files() = 0;
        virtual AsciiString Get_Name() = 0;
        virtual AsciiString Get_Path() = 0;
        virtual void Set_Search_Priority(int priority) = 0;
        virtual void Close() = 0;

        ArchivedFileInfo *Get_Archived_File_Info(AsciiString const &filename);

    private:
        File *ArchiveName;
        DetailedArchiveDirectoryInfo ArchiveInfo;
};

#endif // _ARCHIVEFILE_H_
