/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for the W3dLib file io factory.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ffactory.h"
#include "bufffile.h"
#include "captainslog.h"
#include <cstring>

using std::strrchr;

RawFileFactoryClass defaultWritingFileFactory;
SimpleFileFactoryClass defaultFileFactory;

#ifndef GAME_DLL
RawFileFactoryClass *g_theWritingFileFactory = &defaultWritingFileFactory;
FileFactoryClass *g_theFileFactory = &defaultFileFactory;
#endif

SimpleFileFactoryClass *g_theSimpleFileFactory = &defaultFileFactory;

auto_file_ptr::auto_file_ptr(FileFactoryClass *fact, const char *filename) : m_file(nullptr), m_factory(fact)
{
    m_file = m_factory->Get_File(filename);

    if (m_file == nullptr) {
        m_file = new BufferedFileClass;
    }
}

FileClass *RawFileFactoryClass::Get_File(const char *filename)
{
    return new BufferedFileClass(filename);
}

void RawFileFactoryClass::Return_File(FileClass *file)
{
    delete file;
}

SimpleFileFactoryClass::SimpleFileFactoryClass() : m_isStripPath(false) {}

bool Is_Full_Path(const char *path)
{
    if (path == nullptr) {
        return false;
    }

    return path[1] == ':' || (path[0] == '\\' && path[1] == '\\');
}

FileClass *SimpleFileFactoryClass::Get_File(const char *filename)
{
    StringClass stripped_name(248, true);

    if (m_isStripPath) {
        const char *separator = strrchr(filename, '\\');

        if (separator != nullptr) {
            stripped_name = separator + 1;
        } else {
            stripped_name = filename;
        }
    } else {
        stripped_name = filename;
    }

    BufferedFileClass *file = new BufferedFileClass;
    captainslog_assert(file);

    StringClass new_name(stripped_name, true);

    if (!Is_Full_Path(stripped_name)) {
        CriticalSectionClass::LockClass lock(m_mutex);

        if (!m_subDirectory.Is_Empty()) {
            StringClass subdir(m_subDirectory, true);

            if (strchr(subdir, ';')) {
                for (const char *token = strtok(subdir.Peek_Buffer(), ";"); token; token = strtok(nullptr, ";")) {
                    new_name.Format("%s%s", token, stripped_name.Peek_Buffer());
                    file->Set_Name(new_name);

                    if (file->Open(FM_READ)) {
                        file->Close();
                        break;
                    }
                }
            } else {
                new_name.Format("%s%s", m_subDirectory.Peek_Buffer(), stripped_name.Peek_Buffer());
            }
        }
    }

    file->Set_Name(new_name);
    return file;
}

void SimpleFileFactoryClass::Return_File(FileClass *file)
{
    captainslog_assert(file);
    delete file;
}

void SimpleFileFactoryClass::Append_Sub_Directory(const char *sub_directory)
{
    char temp_sub_dir[1024];

    if (strlen(sub_directory) <= 1022 && strlen(sub_directory) >= 1) {
        strcpy(temp_sub_dir, sub_directory);

        if (temp_sub_dir[strlen(sub_directory)] != '\\') {
            temp_sub_dir[strlen(sub_directory)] = '\\';
            temp_sub_dir[strlen(sub_directory) + 1] = 0;
        }

        CriticalSectionClass::LockClass lock(m_mutex);

        if (m_subDirectory.Get_Length() && m_subDirectory[m_subDirectory.Get_Length() - 1] != ';') {
            m_subDirectory += ';';
        }

        m_subDirectory += temp_sub_dir;
    }
}
