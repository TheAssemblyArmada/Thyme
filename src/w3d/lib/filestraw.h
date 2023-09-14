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
#pragma once

#include "always.h"
#include "straw.h"
#include "wwfile.h"

class FileStraw : public Straw
{
public:
    FileStraw() : m_file(nullptr), m_hasOpened(false) {}
    FileStraw(FileClass &file) : m_file(&file), m_hasOpened(false) {}
    FileStraw(FileClass *file) : m_file(file), m_hasOpened(false) {}
    virtual ~FileStraw();

    virtual int Get(void *buffer, int length) override;

private:
    bool Valid_File() { return m_file != nullptr; }

private:
    FileClass *m_file;
    bool m_hasOpened;
};
