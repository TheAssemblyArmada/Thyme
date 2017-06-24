////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: CRC.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Class for generating Cyclic Redundancy Checks.
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

#ifndef CRC_H
#define CRC_H

#include "bittype.h"

class CRC
{
public:
    CRC() : m_crc(0) {}
    void Compute_CRC(void const *data, int bytes);
    uint32_t Get_CRC() { return m_crc; }

    static uint32_t Memory(void const *data, size_t bytes, uint32_t crc);
    static uint32_t String(const char *string, uint32_t crc);
private:
    void Add_CRC(uint8_t byte);

    static uint32_t m_table[256];
    uint32_t m_crc;
};

#endif // _CRC_H
