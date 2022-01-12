/**
 * @file
 *
 * @author xezon
 *
 * @brief File utility functions. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "common/system/file.h"
#include "stringutil.h"

namespace rts
{

// Read from a file until the specified end character is reached. Will not stop reading at escaped end character. Writes to
// destination string until size minus 1. Destination string does include the read end character. Always writes null
// terminator. Returns true, unless no line was read and the end of the file was reached.
template<typename CharType>
bool Read_Line(File *file, CharType *dest, std::size_t size, const CharType *eol_chars, std::size_t *num_copied = nullptr)
{
    const CharType *writer_end = dest + size - 1;
    CharType *writer = dest;

    int total_bytes_read = 0;
    bool escaped = false;

    while (writer != writer_end) {
        const int bytes_read = file->Read(writer, sizeof(CharType));
        total_bytes_read += bytes_read;

        if (bytes_read != sizeof(CharType)) {
            break;
        }

        // Stop at end character.
        if (!escaped) {
            if (Is_Search_Character(*writer, eol_chars)) {
                ++writer;
                break;
            }
        }

        // End escaping.
        escaped = false;

        // Begin escaping.
        if (*writer == Get_Char<CharType>('\\')) {
            escaped = !escaped;
        }

        ++writer;
    }

    *writer = Get_Char<CharType>('\0');

    if (num_copied != nullptr) {
        *num_copied = (writer - dest);
    }

    return total_bytes_read != 0;
}

// Read any type from file.
template<typename T> bool Read_Any(File *file, T &value)
{
    return file->Read(&value, sizeof(T)) == sizeof(T);
}

// Write any type to file.
template<typename T> bool Write_Any(File *file, const T &value)
{
    return file->Write(&value, sizeof(T)) == sizeof(T);
}

// Read string buffer with given size from file.
template<typename StringType> bool Read_Str(File *file, StringType &string)
{
    using CharType = typename StringType::value_type;
    using SizeType = typename StringType::size_type;

    const SizeType size = string.size();
    void *data = string.data();
    const int bytes = static_cast<int>(size) * sizeof(CharType);

    return file->Read(data, bytes) == bytes;
}

// Write string buffer with given size to file.
template<typename StringType> bool Write_Str(File *file, const StringType &string)
{
    using CharType = typename StringType::value_type;
    using SizeType = typename StringType::size_type;

    const SizeType size = string.size();
    const void *data = string.data();
    const int bytes = static_cast<int>(size) * sizeof(CharType);

    return file->Write(data, bytes) == bytes;
}

// Read Bytes from file.
inline bool Read(File *file, void *data, int len)
{
    return file->Read(data, len) == len;
}

// Write Bytes to file.
inline bool Write(File *file, const void *data, int len)
{
    return file->Write(data, len) == len;
}

} // namespace rts
