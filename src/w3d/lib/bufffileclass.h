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
#pragma once

#ifndef BUFFFILECLASS_H
#define BUFFFILECLASS_H

#include	"rawfileclass.h"

class BufferedFileClass : public RawFileClass
{
public:
    BufferedFileClass();
    BufferedFileClass(char const *filename);
    virtual ~BufferedFileClass();

    virtual int Write(void const *buffer, int size);
    virtual int Read(void *buffer, int size);
    virtual int Seek(int pos, int whence);
    virtual void Close();

    void Reset_Buffer();
        
    static void Set_Desired_Buffer_Size(int size) { DesiredBufferSize = size; }

protected:
    char *m_buffer;
    int m_bufferSize;
    int m_bufferAvailable;
    int m_bufferOffset;

private:
    static int DesiredBufferSize;
};

#endif // _BUFFFILECLASS_H
