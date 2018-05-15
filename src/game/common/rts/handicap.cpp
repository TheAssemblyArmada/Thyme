/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Tracks handicap modifiers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "handicap.h"

Handicap::Handicap()
{
    m_handicaps[0][0] = 1.0f;
    m_handicaps[0][1] = 1.0f;
    m_handicaps[1][0] = 1.0f;
    m_handicaps[1][1] = 1.0f;
}

void Handicap::Init()
{
}

void Handicap::Read_From_Dict(const Dict * dict)
{
}

Handicap::ThingType Handicap::Get_Best_Thing_Type(const ThingTemplate * thing)
{
    return ThingType();
}

float Handicap::Get_Handicap(HandicapType ht, const ThingTemplate * thing)
{
    return 0.0f;
}
