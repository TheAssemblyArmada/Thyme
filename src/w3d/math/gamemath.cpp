/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
 * @author feliwir
 *
 * @brief Floating point math functions. Based on Tiberian Technologies Renegade Scripts wwmath class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamemath.h"

template<typename T, int N>
inline constexpr Array<T, N> calculateFastTable(T (*func)(T), bool arc)
{
    Array<T, N> table;
    for (int a = 0; a < N; ++a) {
        float cv = arc ? (a - N / T(2)) * (T(1) / (N / T(2))) : T(a) * 2.0f * GAMEMATH_PI / SIN_TABLE_SIZE;

        table[a] = func(cv);
    }

    return table;
}

// calculate all tables at compile time
const Array<float, ARC_TABLE_SIZE> _FastAcosTable = calculateFastTable<float, ARC_TABLE_SIZE>(GameMath::Acos, true);
const Array<float, ARC_TABLE_SIZE> _FastAsinTable = calculateFastTable<float, ARC_TABLE_SIZE>(GameMath::Asin, true);
const Array<float, SIN_TABLE_SIZE> _FastSinTable = calculateFastTable<float, SIN_TABLE_SIZE>(GameMath::Sin, false);
const Array<float, SIN_TABLE_SIZE> _FastInvSinTable = calculateFastTable<float, SIN_TABLE_SIZE>(GameMath::Inv_Sin, false);

void GameMath::Init() {}

void GameMath::Shutdown() {}
