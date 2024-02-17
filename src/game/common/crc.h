/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for generating Cyclic Redundancy Checks.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "array.h"
#include "bittype.h"

class CRC
{
public:
    CRC() : m_crc(0) {}
    void Compute_CRC(void const *data, int bytes);
    uint32_t Get_CRC() { return m_crc; }
    void Clear() { m_crc = 0; }

private:
    void Add_CRC(uint8_t byte);

    uint32_t m_crc;
};
