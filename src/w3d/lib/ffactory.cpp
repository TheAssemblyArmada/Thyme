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
#include "bufffileclass.h"

RawFileFactoryClass defaultWritingFileFactory;

#ifndef GAME_DLL
RawFileFactoryClass *g_theWritingFileFactory = &defaultWritingFileFactory;
FileFactoryClass *g_theFileFactory = nullptr;
#endif

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
