/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Audio event information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "ini.h"
#include "mempoolobj.h"
#include "randomvalue.h"
#include <vector>

enum AudioType : int32_t
{
    EVENT_MUSIC,
    EVENT_SPEECH,
    EVENT_SOUND,
    EVENT_UNKVAL3,
    EVENT_UNKVAL4,
};

enum AudioControlType : int32_t
{
    CONTROL_LOOP = 1 << 0,
    CONTROL_RANDOM = 1 << 1,
    CONTROL_ALL = 1 << 2,
    CONTROL_POST_DELAY = 1 << 3,
    CONTROL_INTERRUPT = 1 << 4,
};

enum AudioVisibilityType : int32_t
{
    VISIBILITY_UI = 1 << 0,
    VISIBILITY_WORLD = 1 << 1,
    VISIBILITY_SHROUDED = 1 << 2,
    VISIBILITY_GLOBAL = 1 << 3,
    VISIBILITY_VOICE = 1 << 4,
    VISIBILITY_PLAYER = 1 << 5,
    VISIBILITY_ALLIES = 1 << 6,
    VISIBILITY_ENEMIES = 1 << 7,
    VISIBILITY_EVERYONE = 1 << 8,
};

class DynamicAudioEventInfo;

class AudioEventInfo : public MemoryPoolObject
{
    IMPLEMENT_POOL(AudioEventInfo);
public:
    virtual ~AudioEventInfo() {}

    virtual bool Is_Level_Specific() { return false; }
    virtual DynamicAudioEventInfo *Get_Dynamic_Event_Info() { return nullptr; }
    virtual const DynamicAudioEventInfo *Get_Dynamic_Event_Info() const { return nullptr; }

    const Utf8String &Get_Event_Name() const { return m_eventName; }
    const Utf8String &Get_File_Name() const { return m_filename; }
    int Get_Visibility() const { return m_visibility; }
    AudioType Get_Event_Type() const { return m_eventType; }
    float Get_Volume() const { return m_volume; }
    int Get_Loop_Count() const { return m_loopCount; }
    int Get_Control() const { return m_control; }
    int Get_Priority() const { return m_priority; }
    int Get_Limit() const { return m_limit; }
    float Get_Pitch_Shift() const { return Get_Audio_Random_Value_Real(m_pitchShiftLow, m_pitchShiftHigh); }
    float Get_Volume_Shift() const { return Get_Audio_Random_Value_Real(m_volumeShift + 1.0f, 1.0f); }
    float Get_Delay() const { return Get_Audio_Random_Value_Real(m_delayLow, m_delayHigh); }

    size_t Sound_Count() const { return m_sounds.size(); }
    const Utf8String &Get_Sound(int index) const  { return m_sounds[index]; }
    size_t Attack_Count() const { return m_attack.size(); }
    const Utf8String &Get_Attack(int index) const { return m_attack[index]; }
    size_t Decay_Count() const { return m_decay.size(); }
    const Utf8String &Get_Decay(int index) const { return m_decay[index]; }
    float Min_Range() const { return m_minRange; }
    float Max_Range() const { return m_maxRange; }

    static void Parse_Audio_Event(INI *ini);

private:
    static void Parse_Pitch_Shift(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Delay(INI *ini, void *formal, void *store, const void *user_data);

protected:
    Utf8String m_eventName;
    Utf8String m_filename;
    float m_volume;
    float m_volumeShift;
    float m_minVolume;
    float m_pitchShiftLow;
    float m_pitchShiftHigh;
    int m_delayLow;
    int m_delayHigh;
    int m_limit;
    int m_loopCount;
    int m_priority;
    int32_t m_visibility;
    int32_t m_control;
    std::vector<Utf8String> m_soundsMorning;
    std::vector<Utf8String> m_sounds;
    std::vector<Utf8String> m_soundsNight;
    std::vector<Utf8String> m_soundsEvening;
    std::vector<Utf8String> m_attack;
    std::vector<Utf8String> m_decay;
    float m_lowPassCutoff;
    float m_minRange;
    float m_maxRange;
    AudioType m_eventType;

private:
    static FieldParse s_audioEventParseTable[];
};
