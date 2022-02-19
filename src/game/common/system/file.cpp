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
#include <algorithm>
#include <cstdio>

namespace Thyme
{
static_assert(File::FileMode::LASTMODE <= 0x10000, "Not enough room to encode buffer size");

// Buffer size is encoded into the upper 16 bits of target integer value.
// 0xFFFF0000
//   ^^^^ buffer size
// The encoded value itself is scaled up/down by 64 (6 bit shift),
// therefore giving an effective range of 1 * 64 to 0xFFFF * 64.

constexpr uint32_t LOCATOR = 16;
constexpr uint32_t SCALER = 6;

int32_t Encode_Buffered_File_Mode(int32_t mode, int32_t buffer_size)
{
    buffer_size = std::clamp(buffer_size, (1 << SCALER), (0xFFFF << SCALER));

    return mode | File::FileMode::BUFFERED | (static_cast<uint32_t>(buffer_size) >> SCALER << LOCATOR);
}

bool Decode_Buffered_File_Mode(int32_t mode, int32_t &buffer_size)
{
    if (mode & File::FileMode::BUFFERED) {
        buffer_size = static_cast<uint32_t>(mode) >> LOCATOR << SCALER;
        return true;
    }
    return false;
}
} // namespace Thyme

File::~File()
{
    File::Close();
}

bool File::Open(const char *filename, int32_t mode)
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

    int32_t open_mode = mode;

    // If read/write mode not specified assume read.
    if ((mode & (READ | WRITE)) == 0) {
        open_mode = mode | READ;
    }

    // Clear file if not read or appended.
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
        this->Delete_Instance();
    }
}

bool File::Print(const char *format, ...)
{
    bool success = false;
    va_list va;
    char buffer[10240];

    va_start(va, format);

    if ((m_openMode & TEXT) != 0) {
        // Format our message to be written out
        int32_t length = vsnprintf(buffer, sizeof(buffer), format, va);

        // Only write if we didn't truncate due to buffer overrun.
        if (length < sizeof(buffer)) {
            success = (Write(buffer, length) == length);
        }
    }

    va_end(va);

    return success;
}

int32_t File::Size()
{
    int32_t current = Seek(0, CURRENT);
    int32_t end = Seek(0, END);
    Seek(current, START);

    return end < 0 ? 0 : end;
}

int32_t File::Position()
{
    return Seek(0, CURRENT);
}
