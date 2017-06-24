////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: STREAMINGARCHIVEFILE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Archived file IO.
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
#include "streamingarchivefile.h"
#include "filesystem.h"
#include "minmax.h"

StreamingArchiveFile::StreamingArchiveFile() :
    ArchiveFile(nullptr),
    FileStart(0),
    FileSize(0),
    FilePos(0)
{
    
}

StreamingArchiveFile::~StreamingArchiveFile()
{
    File::Close();
}

bool StreamingArchiveFile::Open(const char *filename, int mode)
{
    File *basefile = g_theFileSystem->Open(filename, mode);

    if ( basefile == nullptr ) {
        return false;
    }

    return Open(basefile);
}

void StreamingArchiveFile::Close()
{
    File::Close();
}

int StreamingArchiveFile::Read(void *dst, int bytes)
{
    if ( ArchiveFile == nullptr ) {
        return 0;
    }
    
    ArchiveFile->Seek(FilePos + FileStart, START);
    
    if ( FilePos + bytes > FileSize ) {
        bytes = FileSize - FilePos;
    }

    int read_len = ArchiveFile->Read(dst, bytes);

    FilePos += read_len;

    return bytes;
}

int StreamingArchiveFile::Write(void const *src, int bytes)
{
    return -1;
}

int StreamingArchiveFile::Seek(int offset, File::SeekMode mode)
{
    switch ( mode ) {
        case START:
            FilePos = Clamp(offset, 0, FileSize);
            break;

        case CURRENT:
            FilePos = Clamp(FilePos + offset, 0, FileSize);
            break;

        case END:
            FilePos = Clamp(FileSize + offset, 0, FileSize);
            break;

        default:
            return -1;
    }

    return FilePos;
}

void StreamingArchiveFile::Next_Line(char *dst, int bytes)
{
    
}

bool StreamingArchiveFile::Scan_Int(int &integer)
{
    return false;
}

bool StreamingArchiveFile::Scan_Real(float &real)
{
    return false;
}

bool StreamingArchiveFile::Scan_String(AsciiString &string)
{
    return false;
}

void *StreamingArchiveFile::Read_All_And_Close()
{
    return nullptr;
}

bool StreamingArchiveFile::Open(File *file)
{
    return true;
}

bool StreamingArchiveFile::Open_From_Archive(File *file, AsciiString const &name, int pos, int size)
{
    if ( file == nullptr || !File::Open(name.Str(), READ | BINARY | STREAMING) ) {
        return false;
    }

    ArchiveFile = file;
    FileStart = pos;
    FileSize = size;
    FilePos = 0;

    // Check that the file start position and size are actually valid
    if ( ArchiveFile->Seek(FileStart, START) == FileStart ) {
        if ( ArchiveFile->Seek(FileSize, CURRENT) == FileStart + FileSize ) {
            ArchiveFile->Seek(FileStart, START);

            return true;
        }
    }

    return false;
}
