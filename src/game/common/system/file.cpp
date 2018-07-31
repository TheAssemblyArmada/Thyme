/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for file IO.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "file.h"
#include <cstdio>

File::~File()
{
    File::Close();
}

bool File::Open(const char *filename, int mode)
{
    if (m_access) {
        return false;
    }

    m_filename = filename;

    // Write and streaming mutually exclusive.
    if ((mode & (WRITE | STREAMING)) == (WRITE | STREAMING)) {
        return false;
    }

    // Text and binary modes are mutually exclusive.
    if ((mode & (TEXT | BINARY)) == (TEXT | BINARY)) {
        return false;
    }

    int open_mode = mode;

    // If read/write mode not specified assume read.
    if ((mode & (READ | WRITE)) == 0) {
        open_mode = mode | READ;
    }

    if ((mode & (READ | APPEND)) == 0) {
        open_mode |= TRUNCATE;
    }

    // If neither text nor binary specified, assume binary.
    if ((mode & (TEXT | BINARY)) == 0) {
        open_mode |= BINARY;
    }

    m_openMode = open_mode;
    m_access = true;

    return true;
}

void File::Close()
{
    if (!m_access) {
        return;
    }

    m_filename = "<no file>";

    m_access = false;

    if (m_deleteOnClose) {
        Delete_Instance(this);
    }
}

bool File::Print(const char *format, ...)
{
    va_list va;
    char buffer[10240];

    va_start(va, format);

    if ((m_openMode & TEXT) != 0) {
        // Format our message to be written out
        int length = vsnprintf(buffer, sizeof(buffer), format, va);

        // Only write if we didn't truncate due to buffer overrun.
        if (length < sizeof(buffer)) {
            return Write(buffer, length) == length;
        }
    }

    return false;
}

int File::Size()
{
    int current = Seek(0, CURRENT);
    int end = Seek(0, END);
    Seek(current, START);

    return end < 0 ? 0 : end;
}

int File::Position()
{
    return Seek(0, CURRENT);
}
