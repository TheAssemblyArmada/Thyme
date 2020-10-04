/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "pot.h"

int Find_POT(int i)
{
    unsigned val = i;
    int shift_bits = 0;
    int total_sig_bits = 0;
    int set_bits = 0;

    while (val != 0) {
        if (val & 1) {
            shift_bits = total_sig_bits;
            ++set_bits;
        }

        val >>= 1;
        ++total_sig_bits;
    }

    if (set_bits >= 2) {
        ++shift_bits;
    }

    return 1 << shift_bits;
}

unsigned Find_POT_Log2(unsigned i)
{
    unsigned val = i;
    unsigned shift_bits = 0;
    unsigned total_sig_bits = 0;
    unsigned set_bits = 0;

    while (val != 0) {
        if (val & 1) {
            shift_bits = total_sig_bits;
            ++set_bits;
        }

        val >>= 1;
        ++total_sig_bits;
    }

    if (set_bits >= 2) {
        ++shift_bits;
    }

    return shift_bits;
}
