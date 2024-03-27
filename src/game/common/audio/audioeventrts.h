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

#include "always.h"
#include "audioeventinfo.h"
#include "coord.h"
#include "gametype.h"
#include "ini.h"

class AudioEventRTS
{
    ALLOW_HOOKING
public:
    AudioEventRTS();
    AudioEventRTS(const AudioEventRTS &that);
    AudioEventRTS(const Utf8String &name);
    AudioEventRTS(const Utf8String &name, ObjectID id);
    AudioEventRTS(const Utf8String &name, DrawableID id);
    AudioEventRTS(const Utf8String &name, const Coord3D *pos);
    virtual ~AudioEventRTS() {}

    AudioEventRTS &operator=(const AudioEventRTS &that);

    void Generate_Filename();
    void Generate_Play_Info();
    void Decrease_Loop_Count();
    void Advance_Next_Play_Portion();

    void Set_Event_Name(Utf8String name);
    void Set_Playing_Handle(int handle) { m_playingHandle = handle; }
    void Set_Current_Sound_Index(int index) const { m_currentSoundIndex = index; }
    void Set_Volume(float volume) { m_volumeAdjustFactor = volume; }
    void Set_Event_Info(AudioEventInfo *info) const { m_eventInfo = info; }
    // Thyme specific: Directly apply the filename from the info without any extra processing
    void Set_Event_Info_With_Filename(AudioEventInfo *info)
    {
        Set_Event_Info(info);
        m_filename = m_eventInfo->Get_File_Name();
    }
    void Set_Player_Index(int index) { m_playerIndex = index; }
    void Set_Next_Play_Portion(int portion) { m_nextPlayPortion = portion; }
    void Set_Handle_To_Kill(int handle) { m_handleToKill = handle; }
    void Set_Time_Of_Day(TimeOfDayType tod) { m_timeOfDay = tod; }
    void Set_Position(Coord3D const *position)
    {
        if (position != nullptr && (m_eventType == AudioType::EVENT_3D || m_eventType == AudioType::EVENT_UNKVAL4)) {
            m_positionOfAudio = *position;
            m_eventType = AudioType::EVENT_3D;
        }
    }

    const Utf8String &Get_File_Name() const { return m_filename; }
    const Utf8String &Get_Attack_Name() const { return m_filenameAttack; }
    const Utf8String &Get_Decay_Name() const { return m_filenameDecay; }
    const Utf8String &Get_Event_Name() const { return m_eventName; }
    const AudioEventInfo *Get_Event_Info() const
    {
        if (m_eventInfo) {
            if (m_eventInfo->m_eventName == m_eventName) {
                return m_eventInfo;
            }

            m_eventInfo = nullptr;
        }

        return m_eventInfo;
    }
    uintptr_t Get_Handle_To_Kill() const { return m_handleToKill; }
    Coord3D *Get_Current_Pos();
    AudioType Get_Event_Type() const { return m_eventType; }
    int Get_Current_Sound_Index() const { return m_currentSoundIndex; }
    bool Should_Play_Locally() const { return m_shouldPlayLocally; }
    bool Is_Positional_Audio() const;
    float Get_Volume() const;
    float Get_Pitch_Shift() const { return m_pitchShift; }
    float Get_Volume_Shift() const { return m_volumeShift; }
    float Get_Delay() const { return m_delay; }
    uintptr_t Get_Playing_Handle() const { return m_playingHandle; }
    int Get_Next_Play_Portion() const { return m_nextPlayPortion; }
    bool Has_More_Loops() const { return m_loopCount >= 0; }
    ObjectID Get_Object_ID() const
    {
        if (m_eventType == EVENT_3D_OBJECT) {
            return m_objectID;
        } else {
            return INVALID_OBJECT_ID;
        }
    }
    DrawableID Get_Drawable_ID() const
    {
        if (m_eventType == EVENT_3D_DRAWABLE) {
            return m_drawableID;
        } else {
            return INVALID_DRAWABLE_ID;
        }
    }
    void Decrement_Delay(float amount) { m_delay -= amount; }
    bool Is_Currently_Playing();
    void Set_Object_ID(ObjectID id);
    void Set_Drawable_ID(DrawableID id);

private:
    Utf8String Generate_Filename_Prefix(AudioType type, bool localize);
    Utf8String Generate_Filename_Extension(AudioType type);
    void Adjust_For_Localization(Utf8String &filename);

private:
    Utf8String m_filename;
    mutable AudioEventInfo *m_eventInfo;
    uintptr_t m_playingHandle;
    uintptr_t m_handleToKill;
    Utf8String m_eventName;
    Utf8String m_filenameAttack;
    Utf8String m_filenameDecay;
    PriorityType m_priority;
    float m_volumeAdjustFactor;
    TimeOfDayType m_timeOfDay;
    Coord3D m_positionOfAudio;
    union
    {
        ObjectID m_objectID;
        DrawableID m_drawableID;
    };
    AudioType m_eventType;
    bool m_shouldFade;
    bool m_isLogical;
    bool m_shouldPlayLocally;
    float m_pitchShift;
    float m_volumeShift;
    float m_delay;
    int m_loopCount;
    mutable int m_currentSoundIndex;
    unsigned m_soundListPos;
    int m_playerIndex;
    int m_nextPlayPortion;
};

class DynamicAudioEventRTS : public MemoryPoolObject
{
    IMPLEMENT_POOL(DynamicAudioEventRTS);

public:
    AudioEventRTS m_event;
    DynamicAudioEventRTS() {}
    DynamicAudioEventRTS(const DynamicAudioEventRTS &that) : m_event(that.m_event) {}
    virtual ~DynamicAudioEventRTS() override {}
    DynamicAudioEventRTS &operator=(const DynamicAudioEventRTS &that)
    {
        MemoryPoolObject::operator=(that);
        m_event = that.m_event;
        return *this;
    }
};
