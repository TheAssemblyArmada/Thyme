/**
 * @file
 *
 * @author OmniBlade
 * @author xezon
 *
 * @brief Same as Win32LocalFile, but with buffered read and write. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "standardfile.h"
#include <cctype>
#include <fcntl.h>

namespace Thyme
{

StandardFile::StandardFile() : m_file(nullptr) {}

StandardFile::~StandardFile()
{
    if (m_file != nullptr) {
        fclose(m_file);
        m_file = nullptr;
        --s_totalOpen;
    }

    File::Close();
}

bool StandardFile::Open(const char *filename, int mode)
{
    int buffer_size;

    if (!Decode_Buffered_File_Mode(mode, buffer_size)) {
        return false;
    }

    if (!File::Open(filename, mode)) {
        return false;
    }

    // r    open for reading (The file must exist)
    // w    open for writing (creates file if it doesn't exist). Deletes content and overwrites the file.
    // a    open for appending (creates file if it doesn't exist).
    // r+   open for reading and writing (The file must exist).
    // w+   open for reading and writing.
    //      If file exists deletes content and overwrites the file, otherwise creates an empty new file.
    // a+   open for reading and writing (append if file exists).

    Utf8String modestr;

    if ((m_access & FileMode::APPEND) != 0) {
        modestr.Concat('a');
        if ((m_access & FileMode::READ) != 0) {
            modestr.Concat('+');
        }
    } else if ((m_access & FileMode::CREATE) != 0) {
        if ((m_access & FileMode::READ) != 0) {
            modestr.Concat("w+");
        } else {
            modestr.Concat('w');
        }
    } else if ((m_access & FileMode::WRITE) != 0) {
        if ((m_access & FileMode::READ) != 0) {
            modestr.Concat("r+");
        } else {
            modestr.Concat('w');
        }
    } else if ((m_access & FileMode::READ) != 0) {
        modestr.Concat('r');
    } else {
        return false;
    }

    if ((m_access & FileMode::BINARY) != 0) {
        modestr.Concat('b');
    }

    // #TODO Treat filename as UTF8 and call wide char version of fopen?

    m_file = fopen(filename, modestr.Str());

    if (m_file == nullptr) {
        return false;
    }

    ++s_totalOpen;

    if ((m_access & FileMode::APPEND) != 0 && Seek(0, SeekMode::END) < 0) {
        Close();
        return false;
    }

    if (buffer_size != 0) {
        if (0 != setvbuf(m_file, nullptr, _IOFBF, buffer_size)) {
            captainslog_dbgassert(0, "File buffer could not be set");
        }
    } else {
        // Uses default buffer size - likely 512.
    }

    return true;
}

int StandardFile::Read(void *dst, int bytes)
{
    if (!m_open) {
        return -1;
    }

    if (dst != nullptr) {
        return fread(dst, 1, bytes, m_file);
    } else {
        Seek(bytes, SeekMode::CURRENT);
    }

    return bytes;
}

int StandardFile::Write(void const *src, int bytes)
{
    if (!m_open || src == nullptr) {
        return -1;
    }

    return fwrite(src, 1, bytes, m_file);
}

int StandardFile::Seek(int offset, File::SeekMode mode)
{
    int fmode;
    switch (mode) {
        case SeekMode::START:
            fmode = SEEK_SET;
            break;
        case SeekMode::CURRENT:
            fmode = SEEK_CUR;
            break;
        case SeekMode::END:
            fmode = SEEK_END;
            break;
        default:
            return -1;
    }

    return fseek(m_file, (long)offset, fmode);
}

void StandardFile::Next_Line(char *dst, int bytes)
{
    captainslog_trace("Seeking getting next line from StandardFile %s.", m_name.Str());

    int i;

    for (i = 0; i < bytes - 1; ++i) {
        char tmp;
        size_t ret = fread(&tmp, sizeof(tmp), 1, m_file);

        if (ret == 0 || tmp == '\n') {
            break;
        }

        if (dst != nullptr) {
            dst[i] = tmp;
        }
    }

    if (dst != nullptr) {
        dst[i] = '\0';
    }
}

bool StandardFile::Scan_Int(int &integer)
{
    captainslog_trace("Scanning Int from StandardFile %s.", m_name.Str());
    char tmp;
    Utf8String number;

    integer = 0;

    // Loop to find the first numeric character.
    do {
        if (fread(&tmp, sizeof(tmp), 1, m_file) == 0) {
            return false;
        }
    } while (!isdigit(tmp) && tmp != '-');

    // Build up our string of numeric characters
    while (true) {
        number.Concat(tmp);

        if (fread(&tmp, sizeof(tmp), 1, m_file) == 0) {
            break;
        }

        if (!isdigit(tmp)) {
            // If we read a byte, seek back that byte for the next read
            // as we are done with the current number.
            Seek(-1, SeekMode::CURRENT);

            break;
        }
    }

    integer = atoi(number.Str());

    return true;
}

bool StandardFile::Scan_Real(float &real)
{
    captainslog_trace("Scanning Real from StandardFile %s.", m_name.Str());
    char tmp;
    Utf8String number;

    real = 0.0f;

    // Loop to find the first numeric character.
    do {
        if (fread(&tmp, sizeof(tmp), 1, m_file) == 0) {
            return false;
        }
    } while (!isdigit(tmp) && tmp != '-' && tmp != '.');

    // Build up our string of numeric characters
    bool have_point = false;

    while (true) {
        number.Concat(tmp);

        if (tmp == '.') {
            have_point = true;
        }

        if (fread(&tmp, sizeof(tmp), 1, m_file) == 0) {
            break;
        }

        if (!isdigit(tmp) && (tmp != '.' || have_point)) {
            // If we read a byte, seek back that byte for the next read
            // as we are done with the current number.
            Seek(-1, SeekMode::CURRENT);

            break;
        }
    }

    real = atof(number.Str());

    return true;
}

bool StandardFile::Scan_String(Utf8String &string)
{
    captainslog_trace("Scanning String from StandardFile %s.", m_name.Str());
    char tmp;
    string.Clear();

    // Loop to find the non-space character.
    do {
        if (fread(&tmp, sizeof(tmp), 1, m_file) == 0) {
            return false;
        }
    } while (isspace(tmp));

    while (true) {
        string.Concat(tmp);

        if (fread(&tmp, sizeof(tmp), 1, m_file) == 0) {
            break;
        }

        if (isspace(tmp)) {
            // If we read a byte, seek back that byte for the next read
            // as we are done with the current number.
            Seek(-1, SeekMode::CURRENT);

            break;
        }
    }

    return true;
}

} // namespace Thyme
