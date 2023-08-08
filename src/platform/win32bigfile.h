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
    ~Win32BIGFile() override {}

    bool Get_File_Info(Utf8String const &name, FileInfo *info) const override;
    File *Open_File(const char *filename, int mode) override;
    void Close_All_Files() override {}
    Utf8String Get_Name() override { return m_fileName; }
    Utf8String Get_Path() override { return m_filePath; }
    void Set_Search_Priority(int priority) override {}
    void Close() override {}

private:
    Utf8String m_fileName;
    Utf8String m_filePath;
};