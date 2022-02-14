/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implements RAM file IO.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ramfile.h"
#include "filesystem.h"
#include <algorithm>
#include <cctype>

RAMFile::RAMFile() : m_data(nullptr), m_pos(0), m_size(0) {}

RAMFile::~RAMFile()
{
    if (m_data != nullptr) {
        delete[] m_data;
    }
}

bool RAMFile::Open(const char *filename, int mode)
{
    File *basefile = g_theFileSystem->Open(filename, mode);

    if (basefile == nullptr) {
        return false;
    }

    bool retval = Open(basefile);
    basefile->Close();

    return retval;
}

void RAMFile::Close()
{
    if (m_data != nullptr) {
        delete[] m_data;
        m_data = nullptr;
    }

    File::Close();
}

int RAMFile::Read(void *dst, int bytes)
{
    if (dst == nullptr) {
        return -1;
    }

    // Clip the amount to read to be within the data remaining.
    bytes = std::min(bytes, m_size - m_pos);

    if (bytes > 0) {
        memcpy(dst, m_data + m_pos, bytes);
    }

    m_pos += bytes;

    return bytes;
}

int RAMFile::Write(void const *src, int bytes)
{
    // No writing to RAM files.
    return -1;
}

int RAMFile::Seek(int offset, File::SeekMode mode)
{
    switch (mode) {
        case START:
            m_pos = offset;
            break;

        case CURRENT:
            m_pos += offset;
            break;

        case END:
            m_pos = offset + m_size;
            break;

        default:
            return -1;
    }

    // Don't seek to outside the file.
    m_pos = std::clamp(m_pos, 0, m_size); // std::min(0, std::min(m_pos, m_size));

    return m_pos;
}

void RAMFile::Next_Line(char *dst, int bytes)
{
    int i = 0;

    // Copy data until newline.
    for (; m_pos < m_size; ++m_pos) {
        char tmp = m_data[m_pos];

        if (tmp == '\n') {
            break;
        }

        if (dst != nullptr && i < bytes - 1) {
            dst[i++] = tmp;
        }
    }

    // If we broke on newline, increment past it.
    if (m_pos < m_size) {
        // Copy newline to destination if not full?
        if (dst != nullptr && i < bytes) {
            dst[i++] = m_data[m_pos];
        }

        ++m_pos;
    }

    // If we have a data pointer to copy to, make sure its null terminiated
    if (dst != nullptr) {
        if (i >= bytes) {
            dst[bytes] = '\0';
        } else {
            dst[i] = '\0';
        }
    }

    // Make sure our position is still within data.
    m_pos = std::min(m_pos, m_size);
}

bool RAMFile::Scan_Int(int &integer)
{
    char tmp;
    Utf8String number;

    integer = 0;

    if (m_pos >= m_size) {
        m_pos = m_size;

        return false;
    }

    // Find first digit or '-' symbol.
    do {
        tmp = m_data[m_pos];

        if (isdigit(tmp) || tmp == '-') {
            break;
        }

    } while (++m_pos < m_size);

    if (m_pos >= m_size) {
        m_pos = m_size;

        return false;
    }

    for (tmp = m_data[m_pos]; m_pos < m_size && isdigit(tmp); tmp = m_data[++m_pos]) {
        number.Concat(tmp);
    }

    integer = atoi(number.Str());

    return true;
}

bool RAMFile::Scan_Real(float &real)
{
    char tmp;
    Utf8String number;

    real = 0.0f;

    if (m_pos >= m_size) {
        m_pos = m_size;

        return false;
    }

    // Find first digit, '-' or '.' symbol.
    do {
        tmp = m_data[m_pos];

        if (isdigit(tmp) || tmp == '-' || tmp == '.') {
            break;
        }

    } while (++m_pos < m_size);

    // Check we are still in bounds
    if (m_pos >= m_size) {
        m_pos = m_size;

        return false;
    }

    bool have_point = false;

    for (tmp = m_data[m_pos]; m_pos < m_size && (isdigit(tmp) || (tmp == '.' && !have_point)); tmp = m_data[++m_pos]) {
        number.Concat(tmp);

        if (tmp == '.') {
            have_point = true;
        }
    }

    real = atof(number.Str());

    return true;
}

bool RAMFile::Scan_String(Utf8String &string)
{
    string.Clear();

    // Find first none space.
    for (; m_pos < m_size; ++m_pos) {
        if (!isspace(m_data[m_pos])) {
            break;
        }
    }

    // Check we are still in bounds
    if (m_pos >= m_size) {
        m_pos = m_size;

        return false;
    }

    // Read into Utf8String
    for (; m_pos < m_size; ++m_pos) {
        if (isspace(m_data[m_pos])) {
            break;
        }

        string.Concat(m_data[m_pos]);
    }

    return true;
}

void *RAMFile::Read_All_And_Close()
{
    char *data;

    if (m_data != nullptr) {
        data = m_data;
        m_data = nullptr;
        Close();
    } else {
        // Valid pointer return seems expected.
        data = new char[1];
    }

    return data;
}

bool RAMFile::Open(File *file)
{
    if (file == nullptr) {
        return false;
    }

    if (File::Open(file->Get_File_Name().Str(), file->Get_File_Mode())) {
        m_size = file->Size();
        m_data = new char[m_size];

        if (m_data != nullptr) {
            // Read the entire file into our buffer.
            m_size = file->Read(m_data, m_size);

            // If we didn't read any data into our buffer, abort.
            if (m_size >= 0) {
                m_pos = 0;

                return true;
            } else {
                delete[] m_data;
                m_data = nullptr;
            }
        }
    }

    return false;
}

bool RAMFile::Open_From_Archive(File *file, Utf8String const &name, int pos, int size)
{
    if (file == nullptr) {
        return false;
    }

    if (File::Open(name.Str(), READ | BINARY)) {
        if (m_data != nullptr) {
            delete[] m_data;
            m_data = nullptr;
        }

        m_size = size;
        m_data = new char[size];

        if (file->Seek(pos, START) == pos) {
            if (file->Read(m_data, m_size) == size) {
                m_filename = file->Get_File_Name();

                return true;
            }
        }
    }

    return false;
}

bool RAMFile::Copy_To_File(File *file)
{
    return file != nullptr && file->Write(m_data, m_size) == m_size;
}
