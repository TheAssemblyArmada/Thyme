/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Audio manager implementation built on top of Mile Sound System mss32.dll.
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

class MilesAudioFileCache;
class PlayingAudio;

struct MilesProviderStruct
{
    Utf8String name;
    int provider;
    int unknown_int;
};

class MilesAudioManager final : public AudioManager
{
    enum
    {
        MILES_PROVIDER_COUNT = 64,
    };

public:
    MilesAudioManager();
    virtual ~MilesAudioManager();

    // SubsystemInterface interface
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    // AudioManager interface
    virtual void Stop_Audio(AudioAffect affect) override;
    virtual void Pause_Audio(AudioAffect affect) override;
    virtual void Resume_Audio(AudioAffect affect) override;
    virtual void Pause_Ambient(AudioAffect affect) override {}
    virtual void Kill_Event_Immediately(unsigned int event) override;
    virtual void Next_Music_Track() override;
    virtual void Prev_Music_Track() override;
    virtual bool Is_Music_Playing() override;
    virtual bool Has_Music_Track_Completed(const Utf8String name) override;
    virtual Utf8String Music_Track_Name() override;
    virtual bool Is_Currently_Playing() override;
    virtual void Open_Device() override;
    virtual void Close_Device() override;
    virtual void *Get_Device() override;
    virtual void Notify_Of_Audio_Completion(unsigned int unk1, unsigned int unk2) override;
    virtual int Get_Provider_Count() override;
    virtual Utf8String Get_Provider_Name(unsigned int index) const override;
    virtual unsigned int Get_Provider_Index(Utf8String name) override;
    virtual void Select_Provider(unsigned int provider) override;
    virtual void Unselect_Provider() override;
    virtual unsigned int Get_Selected_Provider() override;
    virtual void Set_Speaker_Type(unsigned int type) override;
    virtual unsigned int Get_Speaker_Type() override;
    virtual int Get_Num_2D_Samples() const override;
    virtual int Get_Num_3D_Samples() const override;
    virtual int Get_Num_Streams() const override;
    virtual bool Does_Violate_Limit(AudioEventRTS *event) const override;
    virtual bool Is_Playing_Lower_Priority(AudioEventRTS *event) const override;
    virtual bool Is_Playing_Already(AudioEventRTS *event) const override;
    virtual bool Is_Object_Playing_Void(unsigned int obj) const override;
    virtual void Adjust_Volume_Of_Playing_Audio(Utf8String name, float adjust) override;
    virtual void Remove_Playing_Audio(Utf8String name) override;
    virtual void Remove_All_Disabled_Audio() override;
    virtual bool Has_3D_Sensitive_Streams_Playing() override;
    virtual void *Get_Bink_Handle() override;
    virtual void Release_Bink_Handle() override;
    virtual void friend_Force_Play_Audio_Event(AudioEventRTS *event) override;
    virtual void Process_Request_List() override;
    virtual void Release_Audio_Event_RTS(AudioEventRTS *event);
    virtual void Set_Hardware_Accelerated(bool accelerated) override;
    virtual void Set_Speaker_Surround(bool surround) override;
    virtual void Set_Preferred_3D_Provider(Utf8String provider) override;
    virtual void Set_Preferred_Speaker(Utf8String speaker) override;
    virtual float Get_File_Length_MS(Utf8String file_name) override;
    virtual void Close_Any_Sample_Using_File(const void *handle) override;
    virtual void Set_Device_Listener_Position() override;
    virtual PlayingAudio *Find_Playing_Audio_From(unsigned int unk1, unsigned int unk2) override;
    virtual void Process_Playing_List() override;
    virtual void Process_Fading_List() override;
    virtual void Process_Stopped_List() override;

private:
    MilesProviderStruct m_milesProviderList[MILES_PROVIDER_COUNT];
    int m_milesMaxProviderIndex;
    int m_milesCurrentProvider;
    int m_milesLastProvider_maybe;
    int m_speakerType;
    Utf8String m_preferredProvider;
    Utf8String m_preferredSpeaker;
    void *m_milesDigitalDriver; // DIG_DRIVER
    int m_miles3DPositionObject;
    int m_milesMonoDelayFilter;
    std::list<void *> m_unknownMilesList; // These first three are different lists, likely miles specific.
    std::list<void *> m_sampleHandleList_maybe;
    std::list<void *> m_3dSampleHandleList_maybe;
    std::list<PlayingAudio *> m_notPositionalAudioList_maybe;
    std::list<PlayingAudio *> m_positionalAudioList_maybe;
    std::list<PlayingAudio *> m_playingList_maybe;
    std::list<PlayingAudio *> m_fadingList_maybe;
    std::list<PlayingAudio *> m_stoppedList_maybe;
    MilesAudioFileCache *m_audioFileCache;
    PlayingAudio *m_binkPlayingAudio;
    int m_2dSampleCount;
    int m_3dSampleCount;
    int m_streamCount;
};
