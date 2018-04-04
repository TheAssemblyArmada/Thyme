////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: AUDIOEVENTRTS.H
//
//        Author:: OmniBlade
//
//  Contributors::
//
//   Description:: Audio event class.
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

#include "audioeventinfo.h"
#include "coord.h"
#include "gametype.h"
#include "ini.h"

class AudioEventRTS
{
public:
    AudioEventRTS();
    AudioEventRTS(const AudioEventRTS &that);
    AudioEventRTS(const AsciiString &name);
    AudioEventRTS(const AsciiString &name, ObjectID id);
    AudioEventRTS(const AsciiString &name, const Coord3D *pos);
    virtual ~AudioEventRTS() {}

    AudioEventRTS &operator=(const AudioEventRTS &that);

    void Generate_Filename();
    void Generate_Play_Info();

    void Set_Event_Name(AsciiString name);
    void Set_Playing_Handle(int handle) { m_playingHandle = handle; }
    void Set_Current_Sound_Index(int index) const { m_currentSoundIndex = index; }
    void Set_Volume(float volume) { m_volumeAdjustFactor = volume; }
    void Set_Event_Info(AudioEventInfo *info) const { m_eventInfo = info; }

    const AsciiString &Get_Event_Name() const { return m_filename; }
    const AudioEventInfo *Get_Event_Info() const { return m_eventInfo; }
    AudioType Get_Event_Type() const { return m_eventType; }
    int Get_Current_Sound_Index() const { return m_currentSoundIndex; }
    bool Should_Play_Locally() const { return m_shouldPlayLocally; }
    float Get_Volume() const;
    int Get_Playing_Handle() const { return m_playingHandle; }

private:
    AsciiString m_filename;
    mutable AudioEventInfo *m_eventInfo;
    int m_playingHandle;
    int m_handleToKill;
    AsciiString m_eventName;
    AsciiString m_filenameAttack;
    AsciiString m_filenameDecay;
    PriorityType m_priority;
    float m_volumeAdjustFactor;
    TimeOfDayType m_timeOfDay;
    Coord3D m_positionOfAudio;
    ObjectID m_objectID;
    AudioType m_eventType;
    bool m_shouldFade;
    bool m_isLogical;
    bool m_shouldPlayLocally;
    float m_pitchShift;
    float m_volumeShift;
    float m_delay;
    int m_loopCount;
    mutable int m_currentSoundIndex;
    int m_unkInt1;
    int m_playerIndex;
    int m_nextPlayPortion;
};