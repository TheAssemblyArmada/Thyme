/**
 * @file
 *
 * @author xezon
 *
 * @brief Map view helper to extract information from DOS executable.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <windows.h>

class MapViewOfFileClass
{
public:
    explicit MapViewOfFileClass(const wchar_t *fileName);
    ~MapViewOfFileClass();

    LPVOID GetMapViewOfFile() const { return m_lpFileBase; }
    PIMAGE_DOS_HEADER GetDosHeader() const { return m_dosHeader; }
    PIMAGE_NT_HEADERS GetNtHeader() const { return m_ntHeader; }
    PIMAGE_OPTIONAL_HEADER GetOptionalHeader() const { return m_optionalHeader; }
    PIMAGE_SECTION_HEADER GetSectionHeaders() const { return m_sectionHeaders; }
    WORD GetSectionHeaderCount() const { return m_ntHeader ? m_ntHeader->FileHeader.NumberOfSections : 0; }

private:
    HANDLE m_hFile;
    HANDLE m_hFileMapping;
    LPVOID m_lpFileBase;
    PIMAGE_DOS_HEADER m_dosHeader;
    PIMAGE_NT_HEADERS m_ntHeader;
    PIMAGE_OPTIONAL_HEADER m_optionalHeader;
    PIMAGE_SECTION_HEADER m_sectionHeaders;
};

struct ImageSectionInfo
{
    LPVOID BaseOfCode;
    LPVOID BaseOfData;
    SIZE_T SizeOfCode;
    SIZE_T SizeOfData;
};

bool GetModuleSectionInfo(ImageSectionInfo &info);
