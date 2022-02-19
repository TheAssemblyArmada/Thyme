/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Random Number Generation classes
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"

class RandomClass
{
public:
    RandomClass(unsigned seed = 0);

    operator int32_t() { return (operator()()); };
    int32_t operator()();
    int32_t operator()(int32_t minval, int32_t maxval);

    enum
    {
        SIGNIFICANT_BITS = 15
    };

protected:
    unsigned long m_seed;

    /*
    **	Internal working constants that are used to generate the next
    **	random number.
    */
    enum
    {
        MULT_CONSTANT = 0x41C64E6D,
        ADD_CONSTANT = 0x00003039,
        THROW_AWAY_BITS = 10
    };
};

class Random2Class
{
public:
    Random2Class(unsigned seed = 0);

    operator int32_t() { return (operator()()); };
    int32_t operator()();
    int32_t operator()(int32_t minval, int32_t maxval);

    enum
    {
        SIGNIFICANT_BITS = 32
    };

protected:
    int32_t m_index1;
    int32_t m_index2;
    int32_t m_table[250];
};

class Random3Class
{
public:
    Random3Class(unsigned seed1 = 0, unsigned seed2 = 0);

    operator int32_t() { return (operator()()); };
    int32_t operator()();
    int32_t operator()(int32_t minval, int32_t maxval);

    enum
    {
        SIGNIFICANT_BITS = 32
    };

protected:
    static uint32_t s_mix1[20];
    static uint32_t s_mix2[20];
    int32_t m_seed;
    int32_t m_index;
};

class Random4Class
{
public:
    Random4Class(uint32_t seed = 4357);

    operator int32_t() { return (operator()()); };
    int32_t operator()();
    int32_t operator()(int32_t minval, int32_t maxval);
    float Get_Float();

    enum
    {
        SIGNIFICANT_BITS = 32
    };

protected:
    uint32_t m_mt[624];
    int32_t m_mti;
};

template<class T> int32_t Pick_Random_Number(T &generator, int32_t minval, int32_t maxval)
{
    if (minval == maxval) {
        return (minval);
    }

    if (minval > maxval) {
        int32_t temp = minval;
        minval = maxval;
        maxval = temp;
    }

    int32_t magnitude = maxval - minval;
    int32_t highbit = T::SIGNIFICANT_BITS - 1;

    while ((magnitude & (1 << highbit)) == 0 && highbit > 0) {
        highbit--;
    }

    int32_t mask = ~((~0L) << (highbit + 1));
    int32_t pick = magnitude + 1;

    while (pick > magnitude) {
        pick = generator() & mask;
    }

    return (pick + minval);
}
