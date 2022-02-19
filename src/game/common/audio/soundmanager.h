/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for managing loaded sounds.
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
#include "audioeventrts.h"
#include "coord.h"
#include "subsysteminterface.h"

class SoundManager : public SubsystemInterface
{
public:
    virtual void Init() override {}
    virtual void PostProcessLoad() override {}
    virtual void Reset() override;
    virtual void Update() override {}
    virtual void Lose_Focus() {}
    virtual void Gain_Focus() {}
    virtual void Set_Listener_Position(const Coord3D *pos) {}
    virtual void Set_View_Radius(float radius) {}
    virtual void Set_Camera_Audible_Distance(float distance) {}
    virtual float Get_Camera_Audible_Distance() { return 1.0f; }
    virtual void Add_Audio_Event(AudioEventRTS *event);
    virtual void Notify_Of_2D_Sample_Start() { ++m_2dSamplesPlaying; }
    virtual void Notify_Of_3D_Sample_Start() { ++m_3dSamplesPlaying; }
    virtual void Notify_Of_2D_Sample_Completion()
    {
        if (m_2dSamplesPlaying != 0) {
            --m_2dSamplesPlaying;
        }
    }
    virtual void Notify_Of_3D_Sample_Completion()
    {
        if (m_3dSamplesPlaying != 0) {
            --m_3dSamplesPlaying;
        }
    }
    virtual int32_t Get_Available_Samples() { return m_2dSampleSlotCount - m_2dSamplesPlaying; }
    virtual int32_t Get_Available_3D_Samples() { return m_3dSampleSlotCount - m_3dSamplesPlaying; }
    virtual Utf8String Get_Filename_For_Play_From_Audio_Event() { return Utf8String(); }
    virtual bool Can_Play_Now(AudioEventRTS *event);
    virtual bool Violates_Voice(AudioEventRTS *event);
    virtual bool Is_Interrupting(AudioEventRTS *event);

private:
    int32_t m_2dSampleSlotCount;
    int32_t m_3dSampleSlotCount;
    int32_t m_2dSamplesPlaying;
    int32_t m_3dSamplesPlaying;
};
