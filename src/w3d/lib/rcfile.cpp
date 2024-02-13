/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief RCFile
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "rcfile.h"

#ifdef PLATFORM_WINDOWS

#include <cstdlib>

RCFileClass::RCFileClass(HMODULE module, const char *name) :
    m_name(nullptr), m_unk(0), m_startPointer(nullptr), m_currentPointer(nullptr), m_endPointer(nullptr)
{
    if (name != nullptr) {
        m_name = strdup(name);
    }

    HRSRC handle = FindResource(module, m_name, "File");

    if (handle != nullptr) {
        HGLOBAL resource = LoadResource(module, handle);

        if (resource != nullptr) {
            m_startPointer = static_cast<char *>(LockResource(resource));

            if (m_startPointer != nullptr) {
                m_currentPointer = m_startPointer;
                m_endPointer = &m_startPointer[SizeofResource(module, handle)];
            }
        }
    }
}

RCFileClass::~RCFileClass()
{
    if (m_name != nullptr) {
        free(m_name);
    }
}

const char *RCFileClass::Set_Name(const char *filename)
{
    if (m_name != nullptr) {
        free(m_name);
        m_name = 0;
    }

    if (filename != nullptr) {
        m_name = strdup(filename);
    }

    return m_name;
}

int RCFileClass::Read(void *buffer, int length)
{
    if (m_currentPointer == nullptr) {
        return 0;
    }

    int size = length;

    if (&m_currentPointer[length] > m_endPointer) {
        size = m_endPointer - m_currentPointer;
    }

    memcpy(buffer, m_currentPointer, size);
    m_currentPointer += size;
    return size;
}

off_t RCFileClass::Seek(off_t offset, int whence)
{
    char *pointer;

    switch (whence) {
        case FS_SEEK_START:
            pointer = m_startPointer;
            break;
        case FS_SEEK_CURRENT:
            pointer = m_currentPointer;
            break;
        case FS_SEEK_END:
            pointer = m_endPointer;
            break;
    }

    m_currentPointer = &pointer[offset];

    if (m_currentPointer > m_endPointer) {
        m_currentPointer = m_endPointer;
    }

    if (m_currentPointer < m_startPointer) {
        m_currentPointer = m_startPointer;
    }

    return m_currentPointer - m_startPointer;
}

#endif
