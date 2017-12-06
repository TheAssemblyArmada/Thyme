/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper around std::bitset.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef BITFLAGS_H
#define BITFLAGS_H

#include <bitset>

template <int bits>
class BitFlags
{
public:
    BitFlags() {}
    bool operator==(BitFlags &that) const { return m_bits == that.m_bits; }
    bool operator!=(BitFlags &that) const { return m_bits != that.m_bits; }

    void Set(unsigned bit) { m_bits.set(bit); }

    static const char *s_bitNamesList[];
private:
    std::bitset<bits> m_bits;
};

#endif // _BITFLAGS_H
