/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief FileClass for reading files with raw OS API calls.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "rawfile.h"
#include <captainslog.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// Headers needed for posix open, close, read... etc.
#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <winbase.h>
// Make lseek 64bit on windows to match other platforms behaviour?
//#ifdef lseek
//    #undef lseek
//#endif

//#ifdef off_t
//    #undef off_t
//#endif

//#define lseek _lseeki64
// typedef __int64 off_t;
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_UTIME_H
#include <utime.h>
#elif defined HAVE_SYS_UTIME_H
#include <sys/utime.h>
#endif

RawFileClass::RawFileClass() :
    m_rights(FM_CLOSED),
    m_biasStart(0),
    m_biasLength(-1),
#ifdef PLATFORM_WINDOWS
    m_handle(INVALID_HANDLE_VALUE),
#else
    m_handle(-1),
#endif
    m_filename(),
    m_dateTime(0)
{
    Set_Name("");
};

RawFileClass::RawFileClass(const char *filename) :
    m_rights(FM_CLOSED),
    m_biasStart(0),
    m_biasLength(-1),
#ifdef PLATFORM_WINDOWS
    m_handle(INVALID_HANDLE_VALUE),
#else
    m_handle(-1),
#endif
    m_filename(),
    m_dateTime(0)
{
    Set_Name(filename);
};

void RawFileClass::Reset()
{
    // Close an open file handle.
    Close();

    // free the existing filename if it exists.
    m_filename = "";

    // clear the date/time.
    m_dateTime = 0;
}

bool RawFileClass::Create()
{
    // if file open, close it file before we can create it.
    Close();

    // open the file with write access.
    if (Open(FM_WRITE)) {
        if (m_biasLength != -1) {
            Seek(0, FS_SEEK_START);
        }

        Close();

        return true;
    }

    return false;
}

bool RawFileClass::Delete()
{
    // if file open, close it file before we delete it.
    Close();

    // is the filename valid and the file available?
    if (m_filename.Is_Empty()) {
        Error(2);
    } else if (Is_Available()) {
        // delete the file.
#ifdef PLATFORM_WINDOWS
        if (DeleteFileA(m_filename)) {
            return true;
        } else {
            Error(GetLastError(), 0, m_filename);
            return false;
        }
#else
        if (remove(m_filename)) {
            return true;
        } else {
            Error(errno, 0, m_filename);
            return false;
        }
#endif
    }

    return false;
}

bool RawFileClass::Open(const char *filename, int rights)
{
    Set_Name(filename);

    return Open(rights);
}

bool RawFileClass::Open(int rights)
{
    // close the file if it is already open.
    RawFileClass::Close();

    // make sure we have a valid filename set.
    if (m_filename.Get_Length() == 0) {
        Error(2);
    }

    // set the file rights.
    m_rights = rights;

#ifdef PLATFORM_WINDOWS
    switch (rights) {
        case FM_READ:
            m_handle = CreateFileA(m_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            break;

        case FM_WRITE:
            m_handle = CreateFileA(m_filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
            break;

        // Based on the RA code, this is actually READ/WRITE
        case FM_READ | FM_WRITE:
            m_handle = CreateFileA(m_filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
            break;

        default:
            break;
    }
#else // _WIN32

    switch (rights) {
        case FM_READ:
            m_handle = open(m_filename, O_RDONLY, S_IREAD | S_IWRITE);
            break;
        case FM_WRITE:
            m_handle = open(m_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
            break;
        case FM_READ_WRITE:
            m_handle = open(m_filename, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
            break;
        default:
            break;
    }
#endif

    if (m_biasStart || m_biasLength != -1) {
        RawFileClass::Seek(0, FS_SEEK_START);
    }

#ifdef PLATFORM_WINDOWS
    if (m_handle == INVALID_HANDLE_VALUE) {
        return false;
    }
#else
    if (m_handle == -1) {
        return false;
    }
#endif

    return true;
}

bool RawFileClass::Is_Available(bool forced)
{
    // if the filename is invalid, the file is not available.
    if (m_filename.Get_Length() == 0) {
        return false;
    }

    // if the file is already open, it is available.
    if (Is_Open()) {
        return true;
    }

    // Mode appears to indicated if we want to keep the current value of m_handle or not
    if (forced) {
        RawFileClass::Open(FM_READ);
        RawFileClass::Close();

        return true;
    }

#ifdef PLATFORM_WINDOWS
    m_handle = CreateFileA(m_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (m_handle != INVALID_HANDLE_VALUE) {
        // close the file handle/stream.
        if (!CloseHandle(m_handle)) {
            Error(GetLastError(), 0, m_filename);
        }

        m_handle = INVALID_HANDLE_VALUE;

        return true;
    }
#else
    m_handle = open(m_filename, O_RDONLY, S_IREAD);

    if (m_handle != -1) {
        // close the file handle/stream.
        if (close(m_handle) == -1) {
            Error(errno, 0, m_filename);
        }

        m_handle = -1;

        return true;
    }
#endif

    return false;
}

void RawFileClass::Close()
{
    // if the file is NOT open, we can return.
    if (Is_Open()) {
#ifdef PLATFORM_WINDOWS
        if (!CloseHandle(m_handle)) {
            Error(GetLastError(), 0, m_filename);
        }

        m_handle = INVALID_HANDLE_VALUE;
#else

        // close the file handle/stream.
        if (close(m_handle) == -1) {
            Error(errno, 0, m_filename);
        }

        m_handle = -1;
#endif
    }
}

int RawFileClass::Read(void *buffer, int length)
{
    bool opened = false; // have we opened the file to allow us to read?

    if (!Is_Open()) {
        if (!Open(FM_READ)) {
            return 0;
        }

        opened = true;
    }

    if (m_biasLength != -1) {
        int tmplen = m_biasLength - Seek(0, FS_SEEK_CURRENT);

        if (tmplen > length) {
            tmplen = length;
        }

        length = tmplen;
    }

    int totalread = 0;

    while (length > 0) {
#ifdef PLATFORM_WINDOWS
        int readlen;
        SetErrorMode(SEM_FAILCRITICALERRORS);

        if (ReadFile(m_handle, buffer, length, reinterpret_cast<LPDWORD>(&readlen), 0)) {
            SetErrorMode(0);
            length -= readlen;
            totalread += readlen;

            if (!readlen) {
                break;
            }
        } else {
            length -= readlen;
            totalread += readlen;

            Error(GetLastError(), 0, m_filename);
            SetErrorMode(0);
            break;
        }
#else
        int readlen = read(m_handle, buffer, length);

        if (readlen >= 0) {
            length -= readlen;
            totalread += readlen;

            if (!readlen) {
                break;
            }
        } else {
            Error(errno, 0, m_filename);
            break;
        }
#endif
    }

    if (opened) {
        Close();
    }

    return totalread;
}

off_t RawFileClass::Seek(off_t offset, int whence)
{
    if (m_biasLength == -1) {
        return Raw_Seek(offset, whence);
    }

    switch (whence) {
        case FS_SEEK_START:
            if (offset > m_biasLength) {
                offset = m_biasLength;
            }

            offset += m_biasStart;

            break;

        case FS_SEEK_CURRENT:
            break;

        case FS_SEEK_END:
            whence = FS_SEEK_START;
            offset += m_biasLength + m_biasStart;
            break;

        default:
            break;
    }

    off_t seekval = Raw_Seek(offset, whence) - m_biasStart;

    if (seekval < 0) {
        return Raw_Seek(m_biasStart, FS_SEEK_START) - m_biasStart;
    }

    if (seekval > m_biasLength) {
        return Raw_Seek(m_biasLength + m_biasStart, FS_SEEK_START) - m_biasStart;
    }

    return seekval;
}

off_t RawFileClass::Size()
{
    int size = 0;

    if (m_biasLength == -1) {
        if (Is_Open()) {
#ifdef PLATFORM_WINDOWS
            size = GetFileSize(m_handle, 0);
            if (size == -1) {
                Error(GetLastError(), 0, m_filename);
            }
#else

            off_t cur = lseek(m_handle, 0, FS_SEEK_CURRENT);

            size = lseek(m_handle, 0, FS_SEEK_END);

            if (size == -1) {
                Error(errno, 0, m_filename);
            }

            // reset our pos in the file
            lseek(m_handle, cur, FS_SEEK_START);
#endif
        } else if (Open(FM_READ)) {
            size = Size();
            Close();
        }

        m_biasLength = size - m_biasStart;
    }

    return m_biasLength;
}

int RawFileClass::Write(void const *buffer, int length)
{
    int writelen = 0; // total bytes written to file.
    bool opened = false; // have we opened the file to allow us to write?

    if (!Is_Open()) {
        if (!Open(FM_WRITE)) {
            return 0;
        }

        opened = true;
    }

    // write the data in the buffer to the file, and
    // store the total bytes written.
#ifdef PLATFORM_WINDOWS
    if (!WriteFile(m_handle, buffer, length, reinterpret_cast<LPDWORD>(&writelen), 0)) {
        Error(GetLastError(), 0, m_filename);
    }
#else

    writelen = write(m_handle, buffer, length);

    if (writelen < 0) {
        Error(errno, 0, m_filename);
    }
#endif

    if (m_biasLength != -1 && Raw_Seek(0, FS_SEEK_CURRENT) > (int)(m_biasLength + m_biasStart)) {
        m_biasLength = Raw_Seek(0, FS_SEEK_CURRENT) - m_biasStart;
    }

    // close the file if we opened it.
    if (opened) {
        Close();
    }

    return writelen;
}

const char *RawFileClass::Set_Name(const char *filename)
{
    // reset the file bias.
    Bias(0, -1);

    if (filename == nullptr) {
        return m_filename;
    }

    m_filename = filename;

    return m_filename;
}

time_t RawFileClass::Get_Date_Time()
{
    struct stat attrib;

    // get stats
    if (stat(m_filename, &attrib) == 0) {
        return attrib.st_mtime;
    }

    return 0;
}

bool RawFileClass::Set_Date_Time(time_t datetime)
{
    struct utimbuf tstamp;

    // assumes datetime is time_t format
    tstamp.modtime = datetime;
    tstamp.actime = datetime;

    if (Is_Open()) {
        // set the file time and return if it succeeded
        return utime(m_filename, &tstamp) != 0;
    }

    return false;
}

void RawFileClass::Error(int error, bool can_retry, const char *filename)
{
    // Nothing in SAGE
    captainslog_error("Triggered error %d for file '%s'.", error, filename);
}

void RawFileClass::Bias(int offset, int length)
{
    if (offset) {
        int fsize = RawFileClass::Size();

        m_biasLength = fsize;
        m_biasStart += offset;

        if (length != -1) {
            if (fsize >= length) {
                fsize = length;
            }

            m_biasLength = fsize;
        }

        m_biasLength &= (m_biasLength >> 31) - 1;

        // if the file is open, reset seek position.
        if (Is_Open()) {
            RawFileClass::Seek(0, FS_SEEK_START);
        }
    } else {
        m_biasStart = 0;
        m_biasLength = -1;
    }
}

off_t RawFileClass::Raw_Seek(off_t offset, int whence)
{
    // if the file is not open, raise an error.
    if (!Is_Open()) {
        Error(9, 0, m_filename);
        return 0;
    }
#ifdef PLATFORM_WINDOWS
    int retval = 0;

    // seek to the offset specified, return the position.
    // and if fseek returned with a error, raise an error too.
    retval = SetFilePointer(m_handle, offset, 0, whence);
    if (retval == -1) {
        Error(GetLastError(), 0, m_filename);
        return 0;
    }
#else
    int retval = lseek(m_handle, offset, whence);

    // seek to the offset specified, return the position.
    // and if lseek returned with a error, raise an error too.
    if (retval == -1) {
        Error(errno, 0, m_filename);
        return 0;
    }
#endif
    return retval;
}

#ifdef PLATFORM_WINDOWS
void RawFileClass::Attach(HANDLE handle, int rights)
#else
void RawFileClass::Attach(int handle, int rights)
#endif
{
    Reset();

    m_handle = handle;
    m_rights = rights;
    m_biasStart = 0;
    m_biasLength = -1;
    m_dateTime = 0;
}

void RawFileClass::Detach()
{
#ifdef PLATFORM_WINDOWS
    m_handle = INVALID_HANDLE_VALUE;
#else
    m_handle = -1;
#endif
    m_rights = FM_CLOSED;
    m_biasStart = 0;
    m_biasLength = -1;
    m_dateTime = 0;
}
