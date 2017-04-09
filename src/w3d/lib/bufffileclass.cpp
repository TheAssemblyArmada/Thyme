////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: BUFFFILECLASS.H
//
//        Author:: CCHyper
//
//  Contributors:: OmniBlade
//
//   Description:: Buffer file IO class.
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
#include	"bufffileclass.h"
#include	"minmax.h"
#include	<string.h>

int BufferedFileClass::DesiredBufferSize = 1024;

BufferedFileClass::BufferedFileClass() :
    m_buffer(nullptr),
    m_bufferSize(0),
    m_bufferAvailable(0),
    m_bufferOffset(0)
{
    
}

BufferedFileClass::BufferedFileClass(char const *filename) :
    RawFileClass(filename),
    m_buffer(nullptr),
    m_bufferSize(0),
    m_bufferAvailable(0),
    m_bufferOffset(0)
{
    
}

BufferedFileClass::~BufferedFileClass()
{
    Reset_Buffer();
}

int BufferedFileClass::Write(void const *buffer, int size)
{
    //DEBUG_ASSERT(m_bufferSize > 0);
    
    return RawFileClass::Write(buffer, size);
}

int BufferedFileClass::Read(void *buffer, int size)
{
    int read = 0;

    if ( m_bufferAvailable > 0 ) {

        int nsize = MIN(m_bufferAvailable, size);

        memmove(buffer, m_buffer + m_bufferOffset, nsize);
        m_bufferAvailable -= nsize;
        m_bufferOffset += nsize;
        size -= nsize;

        buffer = static_cast<char *>(buffer) + nsize;

        read = nsize;

    }

    if ( size > 0 ) {
        int bsize = m_bufferSize;

        if ( !bsize ) {
            bsize = DesiredBufferSize;
        }

        if ( size > bsize ) {
            return read + RawFileClass::Read(buffer, size);
        }

        if ( !m_bufferSize ) {
            m_bufferSize = DesiredBufferSize;
            m_buffer = new char[m_bufferSize];
            m_bufferAvailable = 0;
            m_bufferOffset = 0;
        }

        if ( !m_buffer ) {
            return 0;
        }

        if ( !m_bufferAvailable ) {
            m_bufferAvailable = RawFileClass::Read(m_buffer, m_bufferSize);
            m_bufferOffset = 0;
        }

        if ( m_bufferAvailable > 0 ) {

            int nsize = MIN(m_bufferAvailable, size);

            if ( !m_buffer ) {
                return 0;
            }

            memmove(buffer, (static_cast<char *>(m_buffer) + m_bufferOffset), nsize);
            m_bufferAvailable -= nsize;
            m_bufferOffset += nsize;

            read += nsize;
        }
    }

    return read;
}

int BufferedFileClass::Seek(int pos, int whence)
{
    if ( whence != FS_SEEK_CURRENT || pos < 0 ) {
        Reset_Buffer();
    }

    if ( m_bufferAvailable ) {
        int left = m_bufferAvailable;
        
        if ( left > pos ) {
            left = pos;
        }

        m_bufferAvailable -= left;
        m_bufferOffset += left;

        return RawFileClass::Seek(pos - left, whence) - m_bufferAvailable;       
    }

    return RawFileClass::Seek(pos, whence);
}

void BufferedFileClass::Close()
{
    RawFileClass::Close();
    Reset_Buffer();
}

void BufferedFileClass::Reset_Buffer()
{
    if ( m_buffer != nullptr ) {
        delete[] m_buffer;
        m_buffer = nullptr;
        m_bufferSize = 0;
        m_bufferAvailable = 0;
        m_bufferOffset = 0;
    }
}
