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
    RADAR_EVENT_BEACON,
    RADAR_EVENT_INFILTRATION,
    RADAR_EVENT_BATTLE_PLAN,
    RADAR_EVENT_STEALTH_DISCOVERED,
    RADAR_EVENT_STEALTH_NEUTRALIZED,
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
        COMMANDBUTTON,
        FONT,
        OBJECT_STATUS,
        TEAM_COMMANDBUTTON_ABILITY,
        SKIRMISH_APPROACH_PATH,
        COLOR,
        EMOTICON,
        OBJECT_FLAG,
        FACTION_NAME,
        OBJECT_TYPE_LIST,
        REVEAL_NAME,
        SCIENCE_AVAILABILITY,
        EVACUATE_CONTAINER_SIDE,
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
    Parameter(ParameterType type, int intval = 0);

    Parameter &operator=(const Parameter &that);

    void Qualify(const Utf8String &suffix, const Utf8String &side_test, const Utf8String &side_replacemet);

    Utf8String Get_UI_Text();
    void Get_Coord3D(Coord3D *coord);
    void Set_Coord3D(Coord3D *coord);
    void Write_Parameter(DataChunkOutput &output);

    static Parameter *Read_Parameter(DataChunkInput &input);

    ParameterType Get_Parameter_Type() const { return m_type; }
    const Utf8String &Get_String() const { return m_string; }
    int Get_Int() const { return m_int; }
    float Get_Real() const { return m_real; }
    BitFlags<OBJECT_STATUS_COUNT> Get_Status_Bits() const { return m_objStatus; }

    void Set_Status_Bits(BitFlags<OBJECT_STATUS_COUNT> bits) { m_objStatus.Set(bits); }
    void Set_String(Utf8String s) { m_string = s; }
    void Set_Int(int set) { m_int = set; }
    void Set_Real(float set) { m_real = set; }

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
