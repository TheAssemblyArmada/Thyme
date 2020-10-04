/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Straw stream class implementation reading data from a file.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "filestraw.h"

FileStraw::~FileStraw()
{
    if (Valid_File() && m_hasOpened) {
        m_file->Close();
        m_hasOpened = false;
        m_file = nullptr;
    }
}

/**
 * @brief Reads data to the buffer from the underlying file class.
 */
int FileStraw::Get(void *buffer, int length)
{
    if (Valid_File() && buffer && length > 0) {
        if (!m_file->Is_Open()) {
            if (m_file->Is_Available()) {
                m_hasOpened = m_file->Open(FM_READ);
            }
        }

        return m_file->Read(buffer, length);
    }

    return 0;
}
