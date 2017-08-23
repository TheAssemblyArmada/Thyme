////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: SOUNDMANAGER.H
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

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "asciistring.h"
#include "audioeventrts.h"
#include "subsysteminterface.h"
#include "coord.h"

class SoundManager : public SubsystemInterface
{
public:
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}
    virtual void Lose_Focus() {}
    virtual void Gain_Focus() {}
    virtual void Set_Listener_Position(const Coord3D *pos) {}
    virtual void Set_View_Radius(float radius) {}
    virtual void Set_Camera_Audiable_Distance(float distance) {}
    virtual float Get_Camera_Audiable_Distance() { return 1.0f; }
    virtual void Add_Audio_Event(AudioEventRTS *event);
    virtual void Notify_Of_2D_Sample_Start() { ++m_2dSamplesPlaying; }
    virtual void Notify_Of_3D_Sample_Start() { ++m_3dSamplesPlaying; }
    virtual void Notify_Of_2D_Sample_Completion() { if (m_2dSamplesPlaying != 0) { --m_2dSamplesPlaying; } }
    virtual void Notify_Of_3D_Sample_Completion() { if (m_3dSamplesPlaying != 0) { --m_3dSamplesPlaying; } }
    virtual int Get_Available_Samples() { return m_2dSampleSlotCount - m_2dSamplesPlaying; }
    virtual int Get_Available_3D_Samples() { return m_3dSampleSlotCount - m_3dSamplesPlaying; }
    virtual AsciiString Get_Filename_For_Play_From_Audio_Event() { return AsciiString(); }
    virtual bool Can_Play_Now(AudioEventRTS *event);
    virtual bool Violates_Voice(AudioEventRTS *event);
    virtual bool Is_Interrupting(AudioEventRTS *event);

private:
    int m_2dSampleSlotCount;
    int m_3dSampleSlotCount;
    int m_2dSamplesPlaying;
    int m_3dSamplesPlaying;
};

#endif
