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
#include <cctype>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>

// Headers needed for posix open, close, read... etc.
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#define O_TEXT 0
#define O_BINARY 0
#elif defined PLATFORM_WINDOWS
#include <io.h>
#include <utf.h>
// Wraps the wide string call with an adapter from utf8.
#undef open
#define open(filename, oflags, ...) _wopen(UTF8To16(filename), oflags, ##__VA_ARGS__)

// Make lseek 64bit on windows to match other platforms behaviour?
//#ifdef lseek
//    #undef lseek
//#endif

//#ifdef off_t
//    #undef off_t
//#endif

//#define lseek _lseeki64
// typedef __int64 off_t;
#else
#error Headers for posix IO needed in localfile.cpp
#endif

int LocalFile::s_totalOpen = 0;

LocalFile::LocalFile() : m_fileHandle(INVALID_HANDLE) {}

LocalFile::~LocalFile()
{
    if (m_fileHandle != INVALID_HANDLE) {
        close(m_fileHandle);
        m_fileHandle = INVALID_HANDLE;
        --s_totalOpen;
    }

    File::Close();
}

bool LocalFile::Open(const char *filename, int mode)
{
    if (!File::Open(filename, mode)) {
        return false;
    }

    int openmode = O_RDONLY;

    if ((m_access & CREATE) != 0) {
        openmode |= O_CREAT;
    }

    if ((m_access & TRUNCATE) != 0) {
        openmode |= O_TRUNC;
    }

    if ((m_access & APPEND) != 0) {
        openmode |= O_APPEND;
    }

    if ((m_access & TEXT) != 0) {
        openmode |= O_TEXT;
    }

    if ((m_access & BINARY) != 0) {
        openmode |= O_BINARY;
    }

    if ((m_access & (READ | WRITE)) == (READ | WRITE)) {
        openmode |= O_RDWR;
    } else if ((m_access & WRITE) != 0) {
        openmode |= O_WRONLY | O_CREAT;
    }

    m_fileHandle = open(filename, openmode, S_IREAD | S_IWRITE);

    if (m_fileHandle != INVALID_HANDLE) {
        ++s_totalOpen;
        if ((m_access & APPEND) == 0 || Seek(0, END) >= 0) {
            return true;
        }
    }

    Close();
    return false;
}

int LocalFile::Read(void *dst, int bytes)
{
    if (!m_open) {
        return -1;
    }

    if (dst != nullptr) {
        return read(m_fileHandle, dst, bytes);
    }

    lseek(m_fileHandle, bytes, CURRENT);
    return bytes;
}

int LocalFile::Write(void const *src, int bytes)
{
    if (!m_open || src == nullptr) {
        return -1;
    }

    return write(m_fileHandle, src, bytes);
}

int LocalFile::Seek(int offset, SeekMode mode)
{
    switch (mode) {
        case START:
            return lseek(m_fileHandle, offset, START);
        case CURRENT:
            return lseek(m_fileHandle, offset, CURRENT);
        case END:
            captainslog_dbgassert(
                offset <= 0, "LocalFile::Seek - pos should be <= 0 for a seek starting at the end of the file");
            return lseek(m_fileHandle, offset, END);
        default:
            return -1;
    }
}

void LocalFile::Next_Line(char *dst, int bytes)
{
    char tmp = '\0';
    int pos = 0;
    int count;

    do {
        if (dst == nullptr || pos >= bytes - 1) {
            count = read(m_fileHandle, &tmp, sizeof(tmp));
        } else {
            count = read(m_fileHandle, &dst[pos], sizeof(tmp));
            tmp = dst[pos];
        }

        pos++;
    } while (count != 0 && tmp != '\n');

    if (dst != nullptr) {
        if (pos < bytes) {
            dst[pos] = '\0';
        } else {
            dst[bytes] = '\0';
        }
    }
}

bool LocalFile::Scan_Int(int &integer)
{
    integer = 0;
    Utf8String number;
    int count;
    char tmp;

    do {
        count = read(m_fileHandle, &tmp, sizeof(tmp));
    } while (count != 0 && (tmp < '0' || tmp > '9') && tmp != '-');

    if (count == 0) {
        return false;
    } else {
        do {
            number.Concat(tmp);
            count = read(m_fileHandle, &tmp, sizeof(tmp));
        } while (count != 0 && tmp >= '0' && tmp <= '9');

        if (count != 0) {
            lseek(m_fileHandle, -1, CURRENT);
        }

        integer = atoi(number.Str());
        return true;
    }
}

bool LocalFile::Scan_Real(float &real)
{
    real = 0.0f;
    Utf8String number;
    int count;
    char tmp;
    bool has_point = false;

    do {
        count = read(m_fileHandle, &tmp, sizeof(tmp));
    } while (count != 0 && (tmp < '0' || tmp > '9') && tmp != '-' && tmp != '.');

    if (count == 0) {
        return false;
    } else {
        do {
            number.Concat(tmp);

            if (tmp == '.') {
                has_point = true;
            }

            count = read(m_fileHandle, &tmp, sizeof(tmp));
        } while (count != 0 && tmp >= '0' && tmp <= '9' || tmp == '.' && !has_point);

        if (count != 0) {
            lseek(m_fileHandle, -1, CURRENT);
        }

        real = atof(number.Str());
        return true;
    }
}

bool LocalFile::Scan_String(Utf8String &string)
{
    char tmp;
    int count;
    string.Clear();

    do {
        count = read(m_fileHandle, &tmp, sizeof(tmp));
    } while (count != 0 && isspace(tmp));

    if (count == 0) {
        return false;
    }

    do {
        count = read(m_fileHandle, &tmp, sizeof(tmp));
    } while (count != 0 && !isspace(tmp));

    if (count != 0) {
        lseek(m_fileHandle, -1, CURRENT);
    }

    return true;
}

void *LocalFile::Read_Entire_And_Close()
{
    int size = Size();
    uint8_t *data = new uint8_t[size];
    Read(data, size);
    Close();
    return data;
}

File *LocalFile::Convert_To_RAM_File()
{
    RAMFile *ramfile = NEW_POOL_OBJ(RAMFile);

    if (ramfile->Open(this)) {
        if (m_deleteOnClose) {
            ramfile->Delete_On_Close();
            Close();
        } else {
            Close();
            Delete_Instance();
        }

        return ramfile;
    } else {
        ramfile->Close();
        ramfile->Delete_Instance();
        return this;
    }
}

void LocalFile::Close()
{
    File::Close();
}
