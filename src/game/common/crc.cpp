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
#include "always.h"
#include "crc.h"

void CRC::Add_CRC(uint8_t byte)
{
    m_crc = byte + (m_crc >> 31) + 2 * m_crc;
}

void CRC::Compute_CRC(void const *data, int bytes)
{
    if (data == nullptr) {
        return;
    }

    uint8_t const *buff = static_cast<uint8_t const *>(data);

    for (int i = 0; i < bytes; ++i) {
        Add_CRC(buff[i]);
    }
}
