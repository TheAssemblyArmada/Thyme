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

#ifndef AUDIOEVENTRTS_H
#define AUDIOEVENTRTS_H

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

    void Set_Event_Name(AsciiString name);

private:
    AsciiString m_filename;
    AudioEventInfo *m_eventInfo;
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
    int m_eventType;
    bool m_shouldFade;
    bool m_isLogical;
    bool m_unkBool1;
    float m_pitchShift;
    float m_volumeShift;
    float m_delay;
    int m_loopCount;
    int m_currentSoundIndex;
    int m_unkInt1;
    int m_playerIndex;
    int m_nextPlayPortion;
};

#endif // AUDIOEVENTRTS_H
