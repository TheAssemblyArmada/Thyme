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

#include "always.h"
#include "array.h"
#include "bittype.h"

class CRC
{
public:
    static uint32_t Memory(void const *data, size_t bytes, uint32_t crc);
    static uint32_t String(const char *string, uint32_t crc);

private:
    static const Array<uint32_t, 256> m_table;
};
