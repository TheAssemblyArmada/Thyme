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
#include "dict.h"
#include "namekeygenerator.h"

Handicap::Handicap()
{
    Init();
}

/**
 * @brief Initialises all handicap multipliers to 1.0.
 *
 * 0x005773A0
 */
void Handicap::Init()
{
    m_handicaps[BUILDCOST][GENERIC] = 1.0f;
    m_handicaps[BUILDCOST][BUILDINGS] = 1.0f;
    m_handicaps[BUILDTIME][GENERIC] = 1.0f;
    m_handicaps[BUILDTIME][BUILDINGS] = 1.0f;
}

/**
 * @brief Read handicap settings from a dict object.
 *
 * 0x005773C0
 */
void Handicap::Read_From_Dict(const Dict *dict)
{
    static const char *_ht_names[] = { "BUILDCOST", "BUILDTIME" };
    static const char *_tt_names[] = { "GENERIC", "BUILDINGS" };
    bool exists = false;

    for (int i = 0; i < HANDICAP_TYPE_COUNT; ++i) {
        for (int j = 0; j < THING_TYPE_COUNT; ++j) {
            Utf8String key_name = "HANDICAP_";
            key_name += _ht_names[i];
            key_name += "_";
            key_name += _tt_names[j];

            float value = dict->Get_Real(g_theNameKeyGenerator->Name_To_Key(key_name.Str()), &exists);

            if (exists) {
                m_handicaps[i][j] = value;
            }
        }
    }
}

/**
 * @brief Gets the handicap of the requested type for a thing.
 *
 * 0x00577640
 */
float Handicap::Get_Handicap(HandicapType ht, const ThingTemplate *thing) const
{
    return m_handicaps[ht][Get_Best_Thing_Type(thing)];
}
