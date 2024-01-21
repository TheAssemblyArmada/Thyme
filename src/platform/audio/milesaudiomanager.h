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
#include <miles.h>
#include <new>

#ifdef BUILD_WITH_FFMPEG
#include "ffmpegaudiofilecache.h"
#else
#include "milesaudiofilecache.h"
#endif

#define MSEC_PER_LOGICFRAME_REAL (1000.0f / 30.0f)

class MilesAudioFileCache;
struct PlayingAudio;

struct MilesProviderStruct
{
    Utf8String name;
    uint32_t provider;
    uint32_t unknown_int;
};

class MilesAudioManager final : public AudioManager
{
    ALLOW_HOOKING
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
    virtual void Kill_Event_Immediately(uintptr_t event) override;
    virtual void Next_Music_Track() override;
    virtual void Prev_Music_Track() override;
    virtual bool Is_Music_Playing() override;
    virtual bool Has_Music_Track_Completed(const Utf8String &name, int loops) override;
    virtual Utf8String Music_Track_Name() override;
    virtual bool Is_Currently_Playing(uintptr_t event) override;
    virtual void Open_Device() override;
    virtual void Close_Device() override;
    virtual AudioDevice Get_Device() override { return m_milesDigitalDriver; }
    virtual void Notify_Of_Audio_Completion(uintptr_t handle, unsigned type) override;
    virtual int Get_Provider_Count() override { return m_milesMaxProviderIndex; }
    virtual Utf8String Get_Provider_Name(unsigned provider) const override;
    virtual unsigned Get_Provider_Index(Utf8String name) override;
    virtual void Select_Provider(unsigned provider) override;
    virtual void Unselect_Provider() override;
    virtual unsigned Get_Selected_Provider() override { return m_milesCurrentProvider; }
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
    virtual BinkHandle Get_Bink_Handle() override;
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

#ifdef GAME_DLL
    MilesAudioManager *Hook_Ctor() { return new (this) MilesAudioManager; }
    void Hook_Dtor() { MilesAudioManager::~MilesAudioManager(); }
#endif

private:
    void Release_Playing_Audio(PlayingAudio *audio);
    void Release_Miles_Handles(PlayingAudio *audio);
    void Free_All_Miles_Handles();
    void Stop_All_Audio_Immediately();
    void Play_Stream(AudioEventRTS *event, HSAMPLE stream);
    AudioDataHandle Play_Sample3D(AudioEventRTS *event, H3DSAMPLE stream);
    AudioDataHandle Play_Sample(AudioEventRTS *event, HSAMPLE stream);
    bool Start_Next_Loop(PlayingAudio *audio);
    void Init_Filters(HSAMPLE sample, AudioEventRTS *event);
    void Init_Filters3D(H3DSAMPLE sample, AudioEventRTS *event, Coord3D *coord);
    void Init_Sample_Pools();
    void Play_Audio_Event(AudioEventRTS *event);
    void Pause_Audio_Event(uintptr_t handle);
    void Stop_Audio_Event(uintptr_t handle);
    void Process_Request(AudioRequest *request);
    void Stop_All_Speech();
    void Adjust_Playing_Volume(PlayingAudio *audio);
    HSAMPLE Get_First_2D_Sample();
    H3DSAMPLE Get_First_3D_Sample();
    float Get_Effective_Volume(AudioEventRTS *event) const;
    bool Kill_Lowest_Priority_Sound_Immediately(AudioEventRTS *event);
    AudioEventRTS *Find_Lowest_Priority_Sound(AudioEventRTS *event);
    Coord3D *Get_Current_Position_From_Event(AudioEventRTS *event);
    AudioDataHandle Open_File(AudioEventRTS *event) { return m_audioFileCache->Open_File(event); }
    void Close_File(AudioDataHandle handle) { m_audioFileCache->Close_File(handle); }
    bool Provider_Is_Valid() const { return m_milesCurrentProvider < m_milesMaxProviderIndex; }
    void Build_Provider_List();
    void Init_Delay_Filter();
    void Create_Listener();
    bool Process_Request_This_Frame(AudioRequest *request);
    void Adjust_Request(AudioRequest *request);
    bool Check_For_Sample(AudioRequest *request);
    bool Is_On_Screen(const Coord3D *coord) const;

    // Callbacks for file access
    static int __stdcall Streaming_File_Open(const char *name, uintptr_t *handle);
    static void __stdcall Streaming_File_Close(uintptr_t handle);
    static int32_t __stdcall Streaming_File_Seek(uintptr_t handle, int32_t pos, uint32_t whence);
    static uint32_t __stdcall Streaming_File_Read(uintptr_t handle, void *dst, uint32_t size);
    static void __stdcall Set_Stream_Complete(void *info);
    static void __stdcall Set_Sample_Complete(void *info);
    static void __stdcall Set_3DSample_Complete(void *info);
    static void Init_Playing_Audio(PlayingAudio *audio);
    static void Exit_Shutdown() { AIL_shutdown(); }

private:
    MilesProviderStruct m_milesProviderList[MILES_PROVIDER_COUNT];
    int m_milesMaxProviderIndex;
    int m_milesCurrentProvider;
    int m_milesLastProvider;
    unsigned m_speakerType;
    Utf8String m_preferredProvider;
    Utf8String m_preferredSpeaker;
    HDIGDRIVER m_milesDigitalDriver;
    H3DPOBJECT m_miles3DPositionObject;
    int m_milesDelayFilter;
    std::list<HAUDIO> m_forcePlayedAudioList; // Possibly stream audio list?
    std::list<HSAMPLE> m_sampleHandleList;
    std::list<H3DSAMPLE> m_3dSampleHandleList;
    std::list<PlayingAudio *> m_globalAudioList;
    std::list<PlayingAudio *> m_positionalAudioList;
    std::list<PlayingAudio *> m_streamList;
    std::list<PlayingAudio *> m_fadingList;
    std::list<PlayingAudio *> m_stoppedList;
#ifdef BUILD_WITH_FFMPEG
    Thyme::FFmpegAudioFileCache *m_audioFileCache;
#else
    MilesAudioFileCache *m_audioFileCache;
#endif
    PlayingAudio *m_binkPlayingAudio;
    int m_2dSampleCount;
    int m_3dSampleCount;
    int m_streamCount;
};
