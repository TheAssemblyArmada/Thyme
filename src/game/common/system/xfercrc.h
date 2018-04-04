////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: XFERCRC.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Some transfer thing interface that CRCs data.
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

#include "xfer.h"

class XferCRC : public Xfer
{
public:
    XferCRC() : m_crc(0) { m_type = XFER_CRC; }
    virtual ~XferCRC() {}

    virtual void Open(AsciiString filename);
    virtual void Close() {}
    virtual int Begin_Block() { return 0; }
    virtual void End_Block() {}
    virtual void Skip(int offset) {}

    virtual void xferSnapshot(SnapShot *thing);
    virtual void xferImplementation(void *thing, int size);
    virtual uint32_t Get_CRC() { return m_crc; }

protected:
    void Add_CRC(uint32_t val);

    uint32_t m_crc;
};