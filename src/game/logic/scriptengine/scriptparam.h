/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing action and condition parameters.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef SCRIPTPARAM_H
#define SCRIPTPARAM_H

#include "always.h"
#include "asciistring.h"
#include "coord.h"
#include "mempoolobj.h"

class Parameter : public MemoryPoolObject
{
    IMPLEMENT_POOL(Parameter);

public:
    enum ParameterType : int32_t
    {
        INT,
        REAL,
        SCRIPT,
        TEAM,
        COUNTER,
        FLAG,
        COMPARISON,
        WAYPOINT,
        BOOLEAN,
        TRIGGER_AREA,
        TEXT_STRING,
        SIDE,
        SOUND,
        SCRIPT_SUBROUTINE,
        UNIT,
        OBJECT_TYPE,
        COORD3D,
        ANGLE,
        TEAM_STATE,
        RELATION,
        AI_MOOD,
        DIALOG,
        MUSIC,
        MOVIE,
        WAYPOINT_PATH,
        LOCALIZED_TEXT,
        BRIDGE,
        KIND_OF_PARAM,
        ATTACK_PRIORITY_SET,
        RADAR_EVENT_TYPE,
        SPECIAL_POWER,
        SCIENCE,
        UPGRADE,
        COMMANDBUTTON_ABILITY,
        BOUNDARY,
        BUILDABLE,
        SURFACES_ALLOWED,
        SHAKE_INTENSITY,
        UNK_COMMANDBUTTON_RELATED,
        FONT,
        OBJECT_STATUS,
        UNK_ABILITY_RELATED, //Object ability perhaps?
        UNK_STRING_RELATED, // Message Popup perhaps?
        COLOR,
        EMOTICON,
        OBJECT_FLAG,
        FACTION_NAME,
        UNK_INT_VALUE_RELATED,
        REVEAL_NAME,
        SCIENCE_AVAILABILITY,
        UNK_POSITON_REALATED, // Vacate/Unload perhaps? right, left, normal(burst from center), unspecified
        PERCENT,
        NUM_ITEMS,
    };

public:
    Parameter(ParameterType type);
    virtual ~Parameter() {}

private:
    ParameterType m_type;
    bool m_initialized;
    int m_int;
    float m_real;
    AsciiString m_string;
    Coord3D m_coord;
    int m_unkInt1;
    int m_unkInt2;
};

#endif // SCRIPTPARAM_H
