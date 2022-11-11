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

    File::Close();
}

bool RAMFile::Open(const char *filename, int mode)
{
    File *basefile = g_theFileSystem->Open_File(filename, mode);

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
    if (m_data == nullptr) {
        return -1;
    }

    if (bytes > m_size - m_pos) {
        bytes = m_size - m_pos;
    }

    if (bytes > 0) {
        if (dst != nullptr) {
            memcpy(dst, m_data + m_pos, bytes);
        }
    }

    m_pos += bytes;

    return bytes;
}

int RAMFile::Write(void const *src, int bytes)
{
    // No writing to RAM files.
    return -1;
}

int RAMFile::Seek(int offset, SeekMode mode)
{
    int pos;

    switch (mode) {
        case START:
            pos = offset;
            break;

        case CURRENT:
            pos = offset + m_pos;
            break;

        case END:
            captainslog_dbgassert(offset <= 0, "RAMFile::Seek - position should be <= 0 for a seek starting from the end.");
            pos = offset + m_size;
            break;

        default:
            return -1;
    }

    if (pos < 0) {
        pos = 0;
    } else if (pos > m_size) {
        pos = m_size;
    }

    m_pos = pos;
    return m_pos;
}

void RAMFile::Next_Line(char *dst, int bytes)
{
    int i = 0;

    while (m_pos < m_size && m_data[m_pos] != '\n') {
        if (dst != nullptr) {
            if (i < bytes - 1) {
                dst[i++] = m_data[m_pos];
            }
        }

        m_pos++;
    }

    if (m_pos < m_size) {
        if (dst != nullptr && i < bytes) {
            dst[i++] = m_data[m_pos];
        }

        m_pos++;
    }

    if (dst != nullptr) {
        if (i >= bytes) {
            dst[bytes] = '\0';
        } else {
            dst[i] = '\0';
        }
    }
    if (m_pos >= m_size) {
        m_pos = m_size;
    }
}

bool RAMFile::Scan_Int(int &integer)
{
    integer = 0;
    Utf8String number;

    while (m_pos < m_size && (m_data[m_pos] < '0' || m_data[m_pos] > '9') && m_data[m_pos] != '-') {
        m_pos++;
    }

    if (m_pos < m_size) {
        do {
            number.Concat(m_data[m_pos]);
            m_pos++;
        } while (m_pos < m_size && m_data[m_pos] >= '0' && m_data[m_pos] <= '9');

        integer = atoi(number.Str());
        return true;
    } else {
        m_pos = m_size;
        return false;
    }
}

bool RAMFile::Scan_Real(float &real)
{
    real = 0.0f;
    Utf8String number;
    bool has_point = false;

    while (m_pos < m_size && (m_data[m_pos] < '0' || m_data[m_pos] > '9') && m_data[m_pos] != '-' && m_data[m_pos] != '.') {
        m_pos++;
    }

    if (m_pos < m_size) {
        do {
            number.Concat(m_data[m_pos]);

            if (m_data[m_pos] == '.') {
                has_point = true;
            }

            m_pos++;
        } while (m_pos < m_size && ((m_data[m_pos] >= '0' && m_data[m_pos] <= '9') || (m_data[m_pos] == '.' && !has_point)));

        real = atof(number.Str());
        return true;
    } else {
        m_pos = m_size;
        return false;
    }
}

bool RAMFile::Scan_String(Utf8String &string)
{
    string.Clear();

    while (m_pos < m_size && isspace(m_data[m_pos])) {
        m_pos++;
    }

    if (m_pos < m_size) {
        do {
            string.Concat(m_data[m_pos]);
            m_pos++;
        } while (m_pos < m_size && !isspace(m_data[m_pos]));

        return true;
    } else {
        m_pos = m_size;
        return false;
    }
}

void *RAMFile::Read_Entire_And_Close()
{
    if (m_data != nullptr) {
        char *data = m_data;
        m_data = nullptr;
        Close();
        return data;
    } else {
        captainslog_dbgassert(false, "m_data is NULL in RAMFile::Read_Entire_And_Close -- should not happen!");
        return new char[1];
    }
}

bool RAMFile::Open(File *file)
{
    if (file == nullptr) {
        return false;
    }

    if (!File::Open(file->Get_Name(), file->Get_Access())) {
        return false;
    }

    m_size = file->Size();
    m_data = new char[m_size];

    if (m_data == nullptr) {
        return false;
    }

    // Read the entire file into our buffer.
    m_size = file->Read(m_data, m_size);

    // If we didn't read any data into our buffer, abort.
    if (m_size >= 0) {
        m_pos = 0;

        return true;
    } else {
        delete[] m_data;
        m_data = nullptr;
        return false;
    }
}

bool RAMFile::Open_From_Archive(File *file, Utf8String const &name, int pos, int size)
{
    if (file == nullptr) {
        return false;
    }

    if (!File::Open(name.Str(), READ | BINARY)) {
        return false;
    }

    if (m_data != nullptr) {
        delete[] m_data;
        m_data = nullptr;
    }

    if (file->Seek(pos, START) != pos) {
        return false;
    }

    m_data = new char[size];
    m_size = size;

    // #BUGFIX allocation check for robustness.
    if (m_data == nullptr) {
        return false;
    }

    if (file->Read(m_data, m_size) != size) {
        // #BUGFIX clean up on failure to prevent garbage read.
        delete[] m_data;
        m_data = nullptr;
        return false;
    }

    m_name = name;
    return true;
}

bool RAMFile::Copy_Data_To_File(File *file)
{
    return file != nullptr && file->Write(m_data, m_size) == m_size;
}
