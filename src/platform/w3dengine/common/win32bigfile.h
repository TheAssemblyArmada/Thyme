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

#include "always.h"
#include "archivefile.h"

class Win32BIGFile : public ArchiveFile
{
public:
    virtual ~Win32BIGFile() override {}

    virtual bool Get_File_Info(Utf8String const &name, FileInfo *info) const override;
    virtual File *Open_File(const char *filename, int mode) override;
    virtual void Close_All_Files() override {}
    virtual Utf8String Get_Name() override { return m_fileName; }
    virtual Utf8String Get_Path() override { return m_filePath; }
    virtual void Set_Search_Priority(int priority) override {}
    virtual void Close() override {}

private:
    Utf8String m_fileName;
    Utf8String m_filePath;
};