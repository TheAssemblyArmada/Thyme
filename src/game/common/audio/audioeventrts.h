/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Audio event class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
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
    AudioEventRTS(const Utf8String &name);
    AudioEventRTS(const Utf8String &name, ObjectID id);
    AudioEventRTS(const Utf8String &name, const Coord3D *pos);
    virtual ~AudioEventRTS() {}

    AudioEventRTS &operator=(const AudioEventRTS &that);

    void Generate_Filename();
    void Generate_Play_Info();

    void Set_Event_Name(Utf8String name);
    void Set_Playing_Handle(int handle) { m_playingHandle = handle; }
    void Set_Current_Sound_Index(int index) const { m_currentSoundIndex = index; }
    void Set_Volume(float volume) { m_volumeAdjustFactor = volume; }
    void Set_Event_Info(AudioEventInfo *info) const { m_eventInfo = info; }
    void Set_Player_Index(int index) { m_playerIndex = index; }

    const Utf8String &Get_File_Name() const { return m_filename; }
    const Utf8String &Get_Attack_Name() const { return m_filenameAttack; }
    const Utf8String &Get_Decay_Name() const { return m_filenameDecay; }
    const AudioEventInfo *Get_Event_Info() const { return m_eventInfo; }
    AudioType Get_Event_Type() const { return m_eventType; }
    int Get_Current_Sound_Index() const { return m_currentSoundIndex; }
    bool Should_Play_Locally() const { return m_shouldPlayLocally; }
    bool Is_Positional_Audio();
    float Get_Volume() const;
    int Get_Playing_Handle() const { return m_playingHandle; }
    int Get_Next_Play_Portion() const { return m_nextPlayPortion; }

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    Utf8String Generate_Filename_Prefix(AudioType type, bool localize);
    Utf8String Generate_Filename_Extension(AudioType type);
    void Adjust_For_Localization(Utf8String &filename);

private:
    Utf8String m_filename;
    mutable AudioEventInfo *m_eventInfo;
    int m_playingHandle;
    int m_handleToKill;
    Utf8String m_eventName;
    Utf8String m_filenameAttack;
    Utf8String m_filenameDecay;
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

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void AudioEventRTS::Hook_Me()
{
    Hook_Method(0x00445080, &Generate_Filename);
    Hook_Method(0x00445380, &Generate_Play_Info);
    Hook_Method(0x00445A20, &Generate_Filename_Prefix);
    Hook_Method(0x00445C30, &Generate_Filename_Extension);
    Hook_Method(0x00445D80, &Adjust_For_Localization);
    Hook_Method(0x00444F90, &Set_Event_Name);
    Hook_Method(0x00445900, &Is_Positional_Audio);
    Hook_Method(0x00445950, &Get_Volume);
}
#endif
