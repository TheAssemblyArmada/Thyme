////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: BITFLAGS.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Wrapper around std::bitsets.
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _BITFLAGS_H_
#define _BITFLAGS_H_

#include <bitset>

template <int bits>
class BitFlags
{
public:
    BitFlags() {}

    bool operator==(BitFlags &that) const { return m_bits == that.m_bits; }
    bool operator!=(BitFlags &that) const { return m_bits != that.m_bits; }

    static char const *s_bitNamesList[];
private:
    std::bitset<bits> m_bits;
};

#endif // _BITFLAGS_H_
