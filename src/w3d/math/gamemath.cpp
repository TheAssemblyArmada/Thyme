/**
 * @file
 *
 * @author Tiberian Technologies
 * @author OmniBlade
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

float _FastAcosTable[ARC_TABLE_SIZE];
float _FastAsinTable[ARC_TABLE_SIZE];
float _FastSinTable[SIN_TABLE_SIZE];
float _FastInvSinTable[SIN_TABLE_SIZE];

void GameMath::Init()
{
    for (int a = 0; a < ARC_TABLE_SIZE; ++a) {
        float cv = float(a - ARC_TABLE_SIZE / 2) * (1.0f / (ARC_TABLE_SIZE / 2));
        _FastAcosTable[a] = Acos(cv);
        _FastAsinTable[a] = Asin(cv);
    }

    for (int a = 0; a < SIN_TABLE_SIZE; ++a) {
        float cv = (float)a * 2.0f * GAMEMATH_PI / SIN_TABLE_SIZE;
        _FastSinTable[a] = Sin(cv);

        if (a > 0) {
            _FastInvSinTable[a] = 1.0f / _FastSinTable[a];
        } else {
            _FastInvSinTable[a] = GAMEMATH_FLOAT_MAX;
        }
    }
}
void GameMath::Shutdown() {}
