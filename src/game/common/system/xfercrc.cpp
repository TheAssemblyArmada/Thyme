////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: XFERCRC.CPP
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
#include "xfercrc.h"
#include "endiantype.h"

// \brief Adds val to rotl m_crc
void XferCRC::Add_CRC(uint32_t val)
{
    m_crc = htobe32(val) + (m_crc >> 31) + (m_crc << 1);
}

void XferCRC::Open(AsciiString filename)
{
    Xfer::Open(filename);
    m_crc = 0;
}

void XferCRC::xferSnapshot(SnapShot *thing)
{
    if ( thing != nullptr ) {
        thing->CRC_Snapshot(this);
    }
}

void XferCRC::xferImplementation(void *thing, int size)
{
    uint32_a *data = static_cast<uint32_a*>(thing);

    if ( thing == nullptr || size < 1 ) {
        return;
    }

    // Use up all the multiples of 4 data.
    for ( int i = size / 4; i > 0; --i ) {
        Add_CRC(*data++);
    }

    // Use remaining bytes padded with 0
    if ( size % 4 > 0 ) {
        uint32_t tmp = 0;
        uint8_t *cdata = reinterpret_cast<uint8_t*>(data);
        int shift = 0;

        for ( int i = 0; i < size % 4; ++i ) {
            tmp |= cdata[i] << shift;
            shift += 8;
        }

        Add_CRC(tmp);
    }
}
