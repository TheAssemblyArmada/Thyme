/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for local file IO.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "localfile.h"
#include "ramfile.h"
#include <captainslog.h>

int LocalFile::s_totalOpen = 0;

void *LocalFile::Read_All_And_Close()
{
    int size = Size();
    uint8_t *data;

    if (size > 0) {
        captainslog_trace("Reading %s and closing.\n", m_filename.Str());
        data = new uint8_t[size];

        Read(data, size);
        Close();
    } else {
        // Calling function is responsible for delete so just alloc
        // 1 byte and return if no data.
        data = new uint8_t[1];
    }

    return data;
}

File *LocalFile::Convert_To_RAM()
{
    captainslog_trace("Converting %s to RAMFile.\n", m_filename.Str());
    RAMFile *ramfile = NEW_POOL_OBJ(RAMFile);

    if (ramfile->Open(this)) {
        if (m_deleteOnClose) {
            ramfile->Set_Del_On_Close(true);
            Close();

            return ramfile;
        } else {
            Close();
            this->Delete_Instance();

            return ramfile;
        }
    }

    ramfile->Close();
    ramfile->Delete_Instance();

    return this;
}
