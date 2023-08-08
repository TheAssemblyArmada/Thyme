/**
 * @file
 *
 * @author feliwir
 *
 * @brief Audio manager implementation built on top of OpenAL (with ALSoft as reference implementation)
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
#include "audiomanager.h"
#include <new>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#define MSEC_PER_LOGICFRAME_REAL (1000.0f / 30.0f)
#define AL_MAX_PLAYBACK_DEVICES 64

struct PlayingAudio;

namespace Thyme
{
class ALAudioManager final : public AudioManager
{
    enum
    {
        MILES_PROVIDER_COUNT = 64,
    };

public:
    ALAudioManager();
    virtual ~ALAudioManager();

    // SubsystemInterface interface
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    // AudioManager interface
    virtual void Stop_Audio(AudioAffect affect) override;
    virtual void Pause_Audio(AudioAffect affect) override;
    virtual void Resume_Audio(AudioAffect affect) override;
    virtual void Pause_Ambient(AudioAffect affect) override {}
    virtual void Kill_Event_Immediately(uintptr_t event) override;
    virtual void Next_Music_Track() override;
    virtual void Prev_Music_Track() override;
    virtual bool Is_Music_Playing() override;
    virtual bool Has_Music_Track_Completed(const Utf8String &name, int loops) override;
    virtual Utf8String Music_Track_Name() override;
    virtual bool Is_Currently_Playing(uintptr_t event) override;
    virtual void Open_Device() override;
    virtual void Close_Device() override;
    virtual void *Get_Device() override { return nullptr; }
    virtual void Notify_Of_Audio_Completion(uintptr_t handle, unsigned unk2) override;
    virtual int Get_Provider_Count() override { return 0; }
    virtual Utf8String Get_Provider_Name(unsigned provider) const override;
    virtual unsigned Get_Provider_Index(Utf8String name) override;
    virtual void Select_Provider(unsigned provider) override;
    virtual void Unselect_Provider() override;
    virtual unsigned Get_Selected_Provider() override { return 0; }
    virtual void Set_Speaker_Type(unsigned type) override;
    virtual unsigned Get_Speaker_Type() override;
    virtual int Get_Num_2D_Samples() const override { return m_2dSampleCount; }
    virtual int Get_Num_3D_Samples() const override { return m_3dSampleCount; }
    virtual int Get_Num_Streams() const override { return m_streamCount; }
    virtual bool Does_Violate_Limit(AudioEventRTS *event) const override;
    virtual bool Is_Playing_Lower_Priority(AudioEventRTS *event) const override;
    virtual bool Is_Playing_Already(AudioEventRTS *event) const override;
    virtual bool Is_Object_Playing_Voice(unsigned obj) const override;
    virtual void Adjust_Volume_Of_Playing_Audio(Utf8String name, float adjust) override;
    virtual void Remove_Playing_Audio(Utf8String name) override;
    virtual void Remove_All_Disabled_Audio() override;
    virtual bool Has_3D_Sensitive_Streams_Playing() override;
    virtual void *Get_Bink_Handle() override;
    virtual void Release_Bink_Handle() override;
    virtual void friend_Force_Play_Audio_Event(AudioEventRTS *event) override;
    virtual void Process_Request_List() override;
    virtual void Set_Hardware_Accelerated(bool accelerated) override;
    virtual void Set_Speaker_Surround(bool surround) override;
    virtual void Set_Preferred_3D_Provider(Utf8String provider) override;
    virtual void Set_Preferred_Speaker(Utf8String speaker) override;
    virtual float Get_File_Length_MS(Utf8String file_name) override;
    virtual void Close_Any_Sample_Using_File(const AudioDataHandle handle) override;
    virtual void Set_Device_Listener_Position() override;
    virtual PlayingAudio *Find_Playing_Audio_From(uintptr_t handle, unsigned type) override;
    virtual void Process_Playing_List() override;
    virtual void Process_Fading_List() override;
    virtual void Process_Stopped_List() override;

private:
    void Release_Playing_Audio(PlayingAudio *audio);
    void Stop_All_Audio_Immediately();
    void Play_Stream(AudioEventRTS *event, ALuint source);
    void *Play_Sample3D(AudioEventRTS *event, ALuint source);
    void *Play_Sample(AudioEventRTS *event, ALuint source);
    void Adjust_Playing_Volume(PlayingAudio *audio);
    bool Start_Next_Loop(PlayingAudio *audio);
    void Play_Audio_Event(AudioEventRTS *event);
    void Pause_Audio_Event(uintptr_t handle);
    void Stop_Audio_Event(uintptr_t handle);
    void Process_Request(AudioRequest *request);
    void Stop_All_Speech();
    Coord3D *Get_Current_Position_From_Event(AudioEventRTS *event);
    // TODO: Open_File & Close_File
    void *Open_File(AudioEventRTS *event) { return nullptr; }
    void Close_File(void *handle) {}
    bool Process_Request_This_Frame(AudioRequest *request);
    void Adjust_Request(AudioRequest *request);
    bool Check_For_Sample(AudioRequest *request);

    static void Init_Playing_Audio(PlayingAudio *audio);

private:
    Utf8String m_alDevicesList[AL_MAX_PLAYBACK_DEVICES];
    int m_alMaxDevicesIndex;

    unsigned m_speakerType;
    Utf8String m_preferredProvider;
    Utf8String m_preferredSpeaker;
    std::list<PlayingAudio *> m_globalAudioList;
    std::list<PlayingAudio *> m_positionalAudioList;
    std::list<PlayingAudio *> m_streamList;
    std::list<PlayingAudio *> m_stoppedList;
    PlayingAudio *m_binkPlayingAudio;
    int m_2dSampleCount;
    int m_3dSampleCount;
    int m_streamCount;

    ALCdevice *m_alcDevice = nullptr;
    ALCcontext *m_alcContext = nullptr;
};
} // namespace Thyme
