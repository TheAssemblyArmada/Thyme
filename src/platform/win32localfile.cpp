////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32LOCALFILE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Implementation of LocalFile file io interface. Despite the
//                 class name, the API used is Posix even on windows.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "win32localfile.h"
#include "hooker.h"
#include "hookcrt.h"
#include <fcntl.h>
#include <cstdlib>
#include <sys/stat.h>

// Headers needed for posix open, close, read... etc.
#ifdef PLATFORM_WINDOWS
#include <io.h>

// Make lseek 64bit on windows to match other platforms behaviour?
//#ifdef lseek
//    #undef lseek
//#endif

//#define lseek _lseeki64
#else
#include <unistd.h>
#endif

Win32LocalFile::Win32LocalFile() :
    FileHandle(INVALID_HANDLE)
{

}

Win32LocalFile::~Win32LocalFile()
{
    if ( FileHandle != INVALID_HANDLE ) {
        close(FileHandle);
        FileHandle = -1;
        --TotalOpen;
    }

    File::Close();
}

bool Win32LocalFile::Open(char const *filename, int mode)
{
    if ( !File::Open(filename, mode) ) {
        return false;
    }

    int openmode = O_RDONLY;

    if ( (OpenMode & CREATE) != 0 ) {
        openmode |= O_CREAT;
    }

    if ( (OpenMode & TRUNCATE) != 0 ) {
        openmode |= O_TRUNC;
    }

    if ( (OpenMode & APPEND) != 0 ) {
        openmode |= O_APPEND;
    }

    if ( (OpenMode & TEXT) != 0 ) {
        openmode |= O_TEXT;
    }

    if ( (OpenMode & BINARY) != 0 ) {
        openmode |= O_BINARY;
    }

    if ( (OpenMode & (READ | WRITE)) == (READ | WRITE) ) {
        openmode |= O_RDWR;
    } else if ( (OpenMode & 2) != 0 ) {
        openmode |= O_WRONLY | O_CREAT;
    }

    FileHandle = open(filename, openmode, S_IREAD | S_IWRITE);

    if ( FileHandle == -1 ) {
        return false;
    }

    ++TotalOpen;

    if ( (OpenMode & APPEND) != 0 && Seek(0, END) < 0 ) {
        Close();

        return false;
    }

    return true;
}

int Win32LocalFile::Read(void *dst, int bytes)
{
    if ( !Access ) {
        return -1;
    }

    if ( dst != nullptr ) {
        return read(FileHandle, dst, bytes);
    } else {
        lseek(FileHandle, bytes, CURRENT);
    }

    return bytes;
}

int Win32LocalFile::Write(void const *src, int bytes)
{
    if ( !Access || src == nullptr ) {
        return -1;
    }

    return write(FileHandle, src, bytes);
}

int Win32LocalFile::Seek(int offset, File::SeekMode mode)
{
    switch ( mode ) {
        case START:
        case CURRENT:
        case END:
            break;
        default:
            return -1;
    }

    return lseek(FileHandle, offset, mode);
}

void Win32LocalFile::Next_Line(char *dst, int bytes)
{
    DEBUG_LOG("Seeking getting next line from Win32LocalFile %s.\n", FileName.Str());

    int i;

    for ( i = 0; i < bytes - 1; ++i ) {
        char tmp;
        int ret = read(FileHandle, &tmp, sizeof(tmp));

        if ( ret <= 0 || tmp == '\n' ) {
            break;
        }

        if ( dst != nullptr ) {
            dst[i] = tmp;
        }
    }

    if ( dst != nullptr ) {
        dst[i] = '\0';
    }
}

bool Win32LocalFile::Scan_Int(int &integer)
{
    DEBUG_LOG("Scanning Int from Win32LocalFile %s.\n", FileName.Str());
    char tmp;
    AsciiString number;

    integer = 0;

    // Loop to find the first numeric character.
    do {
        if ( read(FileHandle, &tmp, sizeof(tmp)) == 0 ) {
            return false;
        }
    } while ( !isdigit(tmp) && tmp != '-' );

    // Build up our string of numeric characters
    while ( true ) {
        number.Concat(tmp);

        int bytes = read(FileHandle, &tmp, sizeof(tmp));

        if ( bytes == 0 ) {
            break;
        }

        if ( !isdigit(tmp) ) {
            // If we read a byte, seek back that byte for the next read
            // as we are done with the current number.
            if ( bytes ) {
                lseek(FileHandle, -1, CURRENT);
            }

            break;
        }
    }

    integer = atoi(number.Str());

    return true;
}

bool Win32LocalFile::Scan_Real(float &real)
{
    DEBUG_LOG("Scanning Real from Win32LocalFile %s.\n", FileName.Str());
    char tmp;
    AsciiString number;

    real = 0.0f;

    // Loop to find the first numeric character.
    do {
        if ( read(FileHandle, &tmp, sizeof(tmp)) == 0 ) {
            return false;
        }
    } while ( !isdigit(tmp) && tmp != '-' && tmp != '.' );

    // Build up our string of numeric characters
    bool have_point = false;

    while ( true ) {
        number.Concat(tmp);

        if ( tmp == '.' ) {
            have_point = true;
        }

        int bytes = read(FileHandle, &tmp, sizeof(tmp));

        if ( bytes == 0 ) {
            break;
        }

        if ( !isdigit(tmp) && (tmp != '.' || have_point) ) {
            // If we read a byte, seek back that byte for the next read
            // as we are done with the current number.
            if ( bytes ) {
                lseek(FileHandle, -1, CURRENT);
            }

            break;
        }
    }

    real = atof(number.Str());

    return true;
}

bool Win32LocalFile::Scan_String(AsciiString &string)
{
    DEBUG_LOG("Scanning String from Win32LocalFile %s.\n", FileName.Str());
    char tmp;
    string.Clear();

    // Loop to find the none space character.
    do {
        if ( read(FileHandle, &tmp, sizeof(tmp)) == 0 ) {
            return false;
        }
    } while ( isspace(tmp) );

    while ( true ) {
        string.Concat(tmp);

        int bytes = read(FileHandle, &tmp, sizeof(tmp));

        if ( bytes == 0 ) {
            break;
        }

        if ( isspace(tmp) ) {
            // If we read a byte, seek back that byte for the next read
            // as we are done with the current number.
            if ( bytes ) {
                lseek(FileHandle, -1, CURRENT);
            }

            break;
        }
    }

    return true;
}
