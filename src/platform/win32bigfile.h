/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling .big file archives.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "archivefile.h"

class Win32BIGFile : public ArchiveFile
{
public:
    Win32BIGFile() {}
    virtual ~Win32BIGFile() {}

    virtual bool Get_File_Info(AsciiString const &name, FileInfo *info) override;
    virtual File *Open_File(const char *filename, int mode) override;
    virtual void Close_All_Files() override {};
    virtual AsciiString Get_Name() override { return FileName; }
    virtual AsciiString Get_Path() override { return FilePath; }
    virtual void Set_Search_Priority(int priority) override {}
    virtual void Close() override {}

private:
    AsciiString FileName;
    AsciiString FilePath;
};