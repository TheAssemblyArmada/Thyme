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
#include "random.h"

RandomClass::RandomClass(unsigned seed) : m_seed(seed) {}

int RandomClass::operator()()
{
    m_seed = (m_seed * MULT_CONSTANT) + ADD_CONSTANT;
    return (m_seed >> THROW_AWAY_BITS) & (~((~0) << SIGNIFICANT_BITS));
}

int RandomClass::operator()(int minval, int maxval)
{
    return Pick_Random_Number(*this, minval, maxval);
}

Random2Class::Random2Class(unsigned seed) : m_index1(0), m_index2(103)
{
    Random3Class random(seed);

    for (int index = 0; index < ARRAY_SIZE(m_table); index++) {
        m_table[index] = random;
    }
}

int Random2Class::operator()()
{
    m_table[m_index1] ^= m_table[m_index2];
    int val = m_table[m_index1];
    m_index1++;
    m_index2++;

    if (m_index1 >= ARRAY_SIZE(m_table)) {
        m_index1 = 0;
    }

    if (m_index2 >= ARRAY_SIZE(m_table)) {
        m_index2 = 0;
    }

    return val;
}

int Random2Class::operator()(int minval, int maxval)
{
    return Pick_Random_Number(*this, minval, maxval);
}

// clang-format off
unsigned int Random3Class::s_mix1[20] = {
    0x0baa96887, 0x01e17d32c, 0x003bcdc3c, 0x00f33d1b2,
    0x076a6491d, 0x0c570d85d, 0x0e382b1e3, 0x078db4362,
    0x07439a9d4, 0x09cea8ac5, 0x089537c5c, 0x02588f55d,
    0x0415b5e1d, 0x0216e3d95, 0x085c662e7, 0x05e8ab368,
    0x03ea5cc8c, 0x0d26a0f74, 0x0f3a9222b, 0x048aad7e4
};

unsigned int Random3Class::s_mix2[20] = {
    0x04b0f3b58, 0x0e874f0c3, 0x06955c5a6, 0x055a7ca46,
    0x04d9a9d86, 0x0fe28a195, 0x0b1ca7865, 0x06b235751,
    0x09a997a61, 0x0aa6e95c8, 0x0aaa98ee1, 0x05af9154c,
    0x0fc8e2263, 0x0390f5e8c, 0x058ffd802, 0x0ac0a5eba,
    0x0ac4874f6, 0x0a9df0913, 0x086be4c74, 0x0ed2c123b
};
// clang-format on

Random3Class::Random3Class(unsigned seed1, unsigned seed2) : m_seed(seed1), m_index(seed2) {}

int Random3Class::operator()()
{
    int loword = m_seed;
    int hiword = m_index++;

    for (int i = 0; i < 4; i++) {
        int hihold = hiword;
        int temp = hihold ^ s_mix1[i];
        int itmpl = temp & 0xffff;
        int itmph = temp >> 16;
        temp = itmpl * itmpl + ~(itmph * itmph);
        temp = (temp >> 16) | (temp << 16);
        hiword = loword ^ ((temp ^ s_mix2[i]) + itmpl * itmph);
        loword = hihold;
    }

    return hiword;
}

int Random3Class::operator()(int minval, int maxval)
{
    return Pick_Random_Number(*this, minval, maxval);
}

#define N 624
#define M 397
#define MATRIX_A 0x9908b0df
#define UPPER_MASK 0x80000000
#define LOWER_MASK 0x7fffffff

#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y) (y >> 11)
#define TEMPERING_SHIFT_S(y) (y << 7)
#define TEMPERING_SHIFT_T(y) (y << 15)
#define TEMPERING_SHIFT_L(y) (y >> 18)

Random4Class::Random4Class(unsigned int seed)
{
    if (!seed) {
        seed = 4375;
    }

    m_mt[0] = seed & 0xffffffff;

    for (m_mti = 1; m_mti < N; m_mti++) {
        m_mt[m_mti] = (69069 * m_mt[m_mti - 1]) & 0xffffffff;
    }
}

int Random4Class::operator()()
{
    unsigned int x;
    static unsigned int mag01[2] = { 0x0, MATRIX_A };

    if (m_mti >= N) {
        int kk;

        for (kk = 0; kk < N - M; kk++) {
            x = (m_mt[kk] & UPPER_MASK) | (m_mt[kk + 1] & LOWER_MASK);
            m_mt[kk] = m_mt[kk + M] ^ (x >> 1) ^ mag01[x & 0x1];
        }
        for (; kk < N - 1; kk++) {
            x = (m_mt[kk] & UPPER_MASK) | (m_mt[kk + 1] & LOWER_MASK);
            m_mt[kk] = m_mt[kk + (M - N)] ^ (x >> 1) ^ mag01[x & 0x1];
        }
        x = (m_mt[N - 1] & UPPER_MASK) | (m_mt[0] & LOWER_MASK);
        m_mt[N - 1] = m_mt[M - 1] ^ (x >> 1) ^ mag01[x & 0x1];

        m_mti = 0;
    }

    x = m_mt[m_mti++];
    x ^= TEMPERING_SHIFT_U(x);
    x ^= TEMPERING_SHIFT_S(x) & TEMPERING_MASK_B;
    x ^= TEMPERING_SHIFT_T(x) & TEMPERING_MASK_C;
    x ^= TEMPERING_SHIFT_L(x);

    return static_cast<int>(x);
}

int Random4Class::operator()(int minval, int maxval)
{
    return Pick_Random_Number(*this, minval, maxval);
}

float Random4Class::Get_Float()
{
    int x = (*this)();
    return static_cast<unsigned int>(x) * 2.3283064370807973754314699618685e-10f;
}
