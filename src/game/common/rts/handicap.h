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
#pragma once

#include "always.h"
#include "thingtemplate.h"

class Dict;

class Handicap
{
public:
    enum HandicapType
    {
        BUILDCOST,
        BUILDTIME,
        HANDICAP_TYPE_COUNT,
    };

    enum ThingType
    {
        GENERIC,
        BUILDINGS,
        THING_TYPE_COUNT,
    };

    Handicap();

    void Init();
    void Read_From_Dict(const Dict *dict);
    static ThingType Get_Best_Thing_Type(const ThingTemplate *thing)
    {
        return thing->Is_KindOf(KINDOF_STRUCTURE) ? BUILDINGS : GENERIC;
    }
    float Get_Handicap(HandicapType ht, const ThingTemplate *thing) const;

private:
    float m_handicaps[HANDICAP_TYPE_COUNT][THING_TYPE_COUNT];
};