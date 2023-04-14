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

#include "always.h"
#include "asciistring.h"
#include "bitflags.h"
#include "coord.h"
#include "datachunk.h"
#include "mempoolobj.h"

struct BorderColor
{
    const char *color_name;
    uint32_t border_color;
};

enum AIMood
{
    AI_MOOD_SLEEP = -2,
    AI_MOOD_PASSIVE,
    AI_MOOD_NORMAL,
    AI_MOOD_ALERT,
    AI_MOOD_AGGRESSIVE,
};

enum RadarEventType
{
    RADAR_EVENT_INVALID,
    RADAR_EVENT_CONSTRUCTION,
    RADAR_EVENT_UPGRADE,
    RADAR_EVENT_UNDER_ATTACK,
    RADAR_EVENT_INFORMATION,
    RADAR_EVENT_UNK,
    RADAR_EVENT_INFILTRATION,
    RADAR_EVENT_UNK2,
    RADAR_EVENT_UNK3,
    RADAR_EVENT_UNK4,
    RADAR_EVENT_UNIT_LOST,
};

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
        UNK_ABILITY_RELATED, // Object ability perhaps?
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

private:
    enum
    {
        LESS_THAN,
        LESS_THAN_EQUAL,
        EQUAL,
        GREATER_THAN_EQUAL,
        GREATER_THAN,
        NOT_EQUAL,
    };

protected:
    virtual ~Parameter() override {}

public:
    Parameter(ParameterType type);

    Parameter &operator=(const Parameter &that);

    void Qualify(const Utf8String &suffix, const Utf8String &side_test, const Utf8String &side_replacemet);

    Utf8String Get_UI_Text();
    void Get_Coord3D(Coord3D *coord);
    void Set_Coord3D(Coord3D *coord);
    void Set_Status_Bits(BitFlags<OBJECT_STATUS_COUNT> bits) { m_objStatus.Set(bits); }

    static Parameter *Read_Parameter(DataChunkInput &input);

private:
    ParameterType m_type;
    bool m_initialized;
    int m_int;
    float m_real;
    Utf8String m_string;
    Coord3D m_coord;
    BitFlags<OBJECT_STATUS_COUNT> m_objStatus;
};

inline Parameter &Parameter::operator=(const Parameter &that)
{
    if (this != &that) {
        m_type = that.m_type;
        m_initialized = that.m_initialized;
        m_int = that.m_int;
        m_real = that.m_real;
        m_string = that.m_string;
        m_coord = that.m_coord;
        m_objStatus = that.m_objStatus;
    }

    return *this;
}
