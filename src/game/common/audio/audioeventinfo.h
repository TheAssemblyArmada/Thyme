////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: AUDIOEVENTINFO.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Audio event information.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef AUDIOEVENTINFO_H
#define AUDIOEVENTINFO_H

#include "asciistring.h"
#include "ini.h"
#include "mempoolobj.h"
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
    CONTROL_IS_INTERRUPTING = 1 << 4,
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

    const AsciiString &Get_Event_Name() const { return m_eventName; }
    AudioType Get_Type() const { return m_type; }
    float Get_Volume() const { return m_volume; }
    int Get_Control() const { return m_control; }

    static void Parse_Audio_Event(INI *ini);

protected:
    AsciiString m_eventName;
    AsciiString m_filename;
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
    AudioType m_type;
    int m_control;
    std::vector<AsciiString> m_soundsMorning;
    std::vector<AsciiString> m_sounds;
    std::vector<AsciiString> m_soundsEvening;
    std::vector<AsciiString> m_soundsNight;
    std::vector<AsciiString> m_attack;
    std::vector<AsciiString> m_decay;
    float m_lowPassCutoff;
    float m_minRange;
    float m_maxRange;
    int m_eventType;

private:
    static FieldParse s_audioEventParseTable[];
};

#endif // AUDIOEVENTINFO_H
