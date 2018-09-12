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
#include "audioeventrts.h"

AudioEventRTS::AudioEventRTS() :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_positionOfAudio{ 0.0f, 0.0f, 0.0f },
    m_objectID(),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_unkInt1(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
}

AudioEventRTS::AudioEventRTS(const AudioEventRTS &that) :
    m_filename(that.m_filename),
    m_eventInfo(that.m_eventInfo),
    m_playingHandle(that.m_playingHandle),
    m_handleToKill(that.m_handleToKill),
    m_eventName(that.m_eventName),
    m_filenameAttack(that.m_filenameAttack),
    m_filenameDecay(that.m_filenameDecay),
    m_priority(that.m_priority),
    m_volumeAdjustFactor(that.m_volumeAdjustFactor),
    m_timeOfDay(that.m_timeOfDay),
    m_positionOfAudio(that.m_positionOfAudio),
    m_objectID(that.m_objectID),
    m_eventType(that.m_eventType),
    m_shouldFade(that.m_shouldFade),
    m_isLogical(that.m_isLogical),
    m_shouldPlayLocally(that.m_shouldPlayLocally),
    m_pitchShift(that.m_pitchShift),
    m_volumeShift(that.m_volumeShift),
    m_delay(that.m_delay),
    m_loopCount(that.m_loopCount),
    m_currentSoundIndex(that.m_currentSoundIndex),
    m_unkInt1(that.m_unkInt1),
    m_playerIndex(that.m_playerIndex),
    m_nextPlayPortion(that.m_nextPlayPortion)
{
}

AudioEventRTS::AudioEventRTS(const Utf8String &name) :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(name),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_positionOfAudio{ 0.0f, 0.0f, 0.0f },
    m_objectID(),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_unkInt1(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
}

AudioEventRTS::AudioEventRTS(const Utf8String &name, ObjectID id) :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(name),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_positionOfAudio{ 0.0f, 0.0f, 0.0f },
    m_objectID(id),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_unkInt1(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
    if ( m_objectID != OBJECT_UNK ) {
        m_eventType = EVENT_SOUND;
    }
}

AudioEventRTS::AudioEventRTS(const Utf8String &name, const Coord3D *pos) :
    m_filename(),
    m_eventInfo(nullptr),
    m_playingHandle(0),
    m_handleToKill(0),
    m_eventName(name),
    m_filenameAttack(),
    m_filenameDecay(),
    m_priority(PRIORITY_NORMAL),
    m_volumeAdjustFactor(-1.0f),
    m_timeOfDay(TIME_OF_DAY_AFTERNOON),
    m_positionOfAudio(*pos),
    m_objectID(),
    m_eventType(EVENT_UNKVAL4),
    m_shouldFade(false),
    m_isLogical(false),
    m_shouldPlayLocally(false),
    m_pitchShift(1.0f),
    m_volumeShift(0.0f),
    m_delay(0.0f),
    m_loopCount(1),
    m_currentSoundIndex(-1),
    m_unkInt1(0),
    m_playerIndex(-1),
    m_nextPlayPortion()
{
}

AudioEventRTS &AudioEventRTS::operator=(const AudioEventRTS &that)
{
    // Original contitionally copied objectID and positionOFAudio
    // based on eventType, but I see no harm in just copying
    // everything if the conditional stuff isn't used on certain
    // eventTypes.
    if ( this != &that ) {
        m_filename = that.m_filename;
        m_eventInfo = that.m_eventInfo;
        m_playingHandle = that.m_playingHandle;
        m_handleToKill = that.m_handleToKill;
        m_eventName = that.m_eventName;
        m_filenameAttack = that.m_filenameAttack;
        m_filenameDecay = that.m_filenameDecay;
        m_priority = that.m_priority;
        m_volumeAdjustFactor = that.m_volumeAdjustFactor;
        m_timeOfDay = that.m_timeOfDay;
        m_positionOfAudio = that.m_positionOfAudio;
        m_objectID = that.m_objectID;
        m_eventType = that.m_eventType;
        m_shouldFade = that.m_shouldFade;
        m_isLogical = that.m_isLogical;
        m_shouldPlayLocally = that.m_shouldPlayLocally;
        m_pitchShift = that.m_pitchShift;
        m_volumeShift = that.m_volumeShift;
        m_delay = that.m_delay;
        m_loopCount = that.m_loopCount;
        m_currentSoundIndex = that.m_currentSoundIndex;
        m_unkInt1 = that.m_unkInt1;
        m_playerIndex = that.m_playerIndex;
        m_nextPlayPortion = that.m_nextPlayPortion;
    }

    return *this;
}

void AudioEventRTS::Generate_Filename()
{
}

void AudioEventRTS::Generate_Play_Info()
{
}

void AudioEventRTS::Set_Event_Name(Utf8String name)
{
    if (strcmp(name, m_eventName) != 0 && m_eventInfo != nullptr) {
        m_eventInfo = nullptr;
    }

    m_eventName = name;
}

float AudioEventRTS::Get_Volume() const
{
    if (m_volumeAdjustFactor == -1.0f) {
        if (m_eventInfo != nullptr) {
            return m_eventInfo->Get_Volume();
        }

        return 0.5f;
    }

    return m_volumeAdjustFactor;
}
