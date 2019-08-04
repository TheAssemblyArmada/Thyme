/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for managing the audio engine.
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
#include "asciistring.h"
#include "audioeventinfo.h"
#include "audiomisc.h"
#include "audiorequest.h"
#include "audiosettings.h"
#include "coord.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include <list>
#include <vector>

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

enum AudioAffect
{
    AUDIOAFFECT_MUSIC = 1 << 0,
    AUDIOAFFECT_SOUND = 1 << 1,
    AUDIOAFFECT_3DSOUND = 1 << 2,
    AUDIOAFFECT_SPEECH = 1 << 3,
    AUDIOAFFECT_BASEVOL = 1 << 4,
};
DEFINE_ENUMERATION_BITWISE_OPERATORS(AudioAffect);

enum CachedVarsType
{
    CACHED_START,
    CACHED_SPEECH_ON = 1 << 0,
    CACHED_SOUND_ON = 1 << 1,
    CACHED_3DSOUND_ON = 1 << 2,
    CACHED_MUSIC_ON = 1 << 3,
    CACHED_VOL_SET = 1 << 4,
    CACHED_HW_ACCEL = 1 << 5,
    CACHED_SURROUND = 1 << 6,
    CACHED_FROM_CD = 1 << 7,
    CACHED_UNK8 = 1 << 8, // Speech related.
};

class AudioEventRTS;
struct PlayingAudio;
class MusicManager;
class SoundManager;

#ifdef THYME_USE_STLPORT
typedef std::hash_map<const Utf8String, AudioEventInfo *, rts::hash<Utf8String>, rts::equal_to<Utf8String>> audioinfomap_t;
#else
typedef std::unordered_map<const Utf8String, AudioEventInfo *, rts::hash<Utf8String>, rts::equal_to<Utf8String>>
    audioinfomap_t;
#endif

class AudioManager : public SubsystemInterface
{
public:
    AudioManager();
    virtual ~AudioManager();

    // SubsystemInterface interface
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    // AudioManager interface
    virtual void Stop_Audio(AudioAffect affect) = 0;
    virtual void Pause_Audio(AudioAffect affect) = 0;
    virtual void Resume_Audio(AudioAffect affect) = 0;
    virtual void Pause_Ambient(AudioAffect affect) = 0;
    virtual void Lose_Focus();
    virtual void Regain_Focus();
    virtual int Add_Audio_Event(const AudioEventRTS *event);
    virtual void Remove_Audio_Event(unsigned event);
    virtual void Remove_Audio_Event(Utf8String event);
    virtual void Kill_Event_Immediately(uintptr_t event) = 0;
    virtual bool Is_Valid_Audio_Event(const AudioEventRTS *event) const;
    virtual bool Is_Valid_Audio_Event(AudioEventRTS *event) const;
    virtual void Next_Music_Track() = 0;
    virtual void Prev_Music_Track() = 0;
    virtual bool Is_Music_Playing() = 0;
    virtual bool Has_Music_Track_Completed(const Utf8String &name, int loops) = 0;
    virtual Utf8String Music_Track_Name() = 0;
    virtual void Set_Audio_Event_Enabled(Utf8String event, bool vol_override);
    virtual void Set_Audio_Event_Volume_Override(Utf8String event, float vol_override);
    virtual void Remove_Disabled_Events();
    virtual void Get_Info_For_Audio_Event(const AudioEventRTS *event) const;
    virtual bool Is_Currently_Playing(uintptr_t event) = 0;
    virtual void Open_Device() = 0;
    virtual void Close_Device() = 0;
    virtual void *Get_Device() = 0;
    virtual void Notify_Of_Audio_Completion(uintptr_t handle, unsigned unk2) = 0;
    virtual int Get_Provider_Count() = 0;
    virtual Utf8String Get_Provider_Name(unsigned provider) const = 0;
    virtual unsigned Get_Provider_Index(Utf8String name) = 0;
    virtual void Select_Provider(unsigned provider) = 0;
    virtual void Unselect_Provider() = 0;
    virtual unsigned Get_Selected_Provider() = 0;
    virtual void Set_Speaker_Type(unsigned type) = 0;
    virtual unsigned Get_Speaker_Type() = 0;
    virtual unsigned Translate_From_Speaker_Type(const Utf8String &type);
    virtual Utf8String Translate_To_Speaker_Type(unsigned type);
    virtual int Get_Num_2D_Samples() const = 0;
    virtual int Get_Num_3D_Samples() const = 0;
    virtual int Get_Num_Streams() const = 0;
    virtual bool Does_Violate_Limit(AudioEventRTS *event) const = 0;
    virtual bool Is_Playing_Lower_Priority(AudioEventRTS *event) const = 0;
    virtual bool Is_Playing_Already(AudioEventRTS *event) const = 0;
    virtual bool Is_Object_Playing_Voice(unsigned obj) const = 0;
    virtual void Adjust_Volume_Of_Playing_Audio(Utf8String name, float adjust) = 0;
    virtual void Remove_Playing_Audio(Utf8String name) = 0;
    virtual void Remove_All_Disabled_Audio() = 0;
    virtual bool Is_On(AudioAffect affect) const;
    virtual void Set_On(bool on, AudioAffect affect);
    virtual void Set_Volume(float volume, AudioAffect affect);
    virtual float Get_Volume(AudioAffect affect);
    virtual void Set_3D_Volume_Adjustment(float adj);
    virtual bool Has_3D_Sensitive_Streams_Playing() = 0;
    virtual void *Get_Bink_Handle() = 0;
    virtual void Release_Bink_Handle() = 0;
    virtual void friend_Force_Play_Audio_Event(AudioEventRTS *event) = 0;
    virtual void Set_Listener_Position(const Coord3D *position, const Coord3D *direction);
    virtual const Coord3D *Get_Listener_Position() const { return &m_listenerPosition; }
    virtual AudioRequest *Allocate_Audio_Request(bool is_add_request);
    virtual void Release_Audio_Request(AudioRequest *request);
    virtual void Append_Audio_Request(AudioRequest *request);
    virtual void Process_Request_List() = 0;
    virtual AudioEventInfo *New_Audio_Event_Info(Utf8String name);
    virtual void Add_Audio_Event_Info(AudioEventInfo *info);
    virtual AudioEventInfo *Find_Audio_Event_Info(Utf8String name) const;
    virtual void Release_Audio_Event_RTS(AudioEventRTS *event) { delete event; }
    virtual void Set_Hardware_Accelerated(bool accelerated) { m_hardwareAccelerated = accelerated; }
    virtual bool Get_Hardware_Accelerated() { return m_hardwareAccelerated; }
    virtual void Set_Speaker_Surround(bool surround) { m_surround = surround; }
    virtual bool Get_Speaker_Surround() { return m_surround; }
    virtual void Refresh_Cached_Variables();
    virtual void Set_Preferred_3D_Provider(Utf8String provider) = 0;
    virtual void Set_Preferred_Speaker(Utf8String speaker) = 0;
    virtual float Get_Audio_Length_MS(const AudioEventRTS *event);
    virtual float Get_File_Length_MS(Utf8String file_name) = 0;
    virtual void Close_Any_Sample_Using_File(const void *handle) = 0;
    virtual bool Is_Music_Already_Loaded();
    virtual bool Is_Music_Playing_From_CD() { return m_fromCD; }
    virtual void Find_All_Audio_Events_Of_Type(AudioType type, std::vector<AudioEventInfo *> &list);
    virtual const audioinfomap_t *Get_All_Audio_Events() const;
    virtual bool Is_Current_Provider_Hardware_Accelerated();
    virtual bool Is_Current_Speaker_Type_Surround();
    virtual bool Should_Play_Locally(const AudioEventRTS *event);
    virtual void Set_Device_Listener_Position() = 0;
    virtual int Allocate_New_Handle();
    virtual void Remove_Level_Specific_Audio_Event_Infos();
    virtual PlayingAudio *Find_Playing_Audio_From(uintptr_t handle, unsigned type) = 0;
    virtual void Process_Playing_List() = 0;
    virtual void Process_Fading_List() = 0;
    virtual void Process_Stopped_List() = 0;

    AudioSettings *Get_Audio_Settings() const { return m_audioSettings; }
    MiscAudio *Get_Misc_Audio() const { return m_miscAudio; }

#ifdef GAME_DLL
    static void Hook_Me();
    void Hook_Init() { AudioManager::Init(); }
    void Hook_Reset() { AudioManager::Reset(); }
    void Hook_Update() { AudioManager::Update(); }

    void Hook_Lose_Focus() { AudioManager::Lose_Focus(); }
    void Hook_Regain_Focus() { AudioManager::Regain_Focus(); }
    int Hook_Add_Audio_Event(const AudioEventRTS *event) { return AudioManager::Add_Audio_Event(event); }
    void Hook_Remove_Audio_Event(unsigned event) { AudioManager::Remove_Audio_Event(event); }
    void Hook_Remove_Audio_Event_String(Utf8String event) { AudioManager::Remove_Audio_Event(event); }
    bool Hook_Is_Valid_Const_Audio_Event(const AudioEventRTS *event) { return AudioManager::Is_Valid_Audio_Event(event); }
    void Hook_Set_Audio_Event_Enabled(Utf8String event, bool vol_override)
    {
        AudioManager::Set_Audio_Event_Enabled(event, vol_override);
    }
    void Hook_Set_Audio_Event_Volume_Override(Utf8String event, float vol_override)
    {
        AudioManager::Set_Audio_Event_Volume_Override(event, vol_override);
    }
    void Hook_Remove_Disabled_Events() { AudioManager::Remove_Disabled_Events(); }
    void Hook_Get_Info_For_Audio_Event(const AudioEventRTS *event) { AudioManager::Get_Info_For_Audio_Event(event); }
    unsigned Hook_Translate_From_Speaker_Type(const Utf8String &type)
    {
        return AudioManager::Translate_From_Speaker_Type(type);
    }
    Utf8String Hook_Translate_To_Speaker_Type(unsigned type) { return AudioManager::Translate_To_Speaker_Type(type); }
    bool Hook_Is_On(AudioAffect affect) { return AudioManager::Is_On(affect); }
    void Hook_Set_On(bool on, AudioAffect affect) { AudioManager::Set_On(on, affect); }
    void Hook_Set_Volume(float volume, AudioAffect affect) { AudioManager::Set_Volume(volume, affect); }
    float Hook_Get_Volume(AudioAffect affect) { return AudioManager::Get_Volume(affect); }
    void Hook_Set_3D_Volume_Adjustment(float adj) { AudioManager::Set_3D_Volume_Adjustment(adj); }
    void Hook_Set_Listener_Position(const Coord3D *position, const Coord3D *direction)
    {
        AudioManager::Set_Listener_Position(position, direction);
    }
    AudioRequest *Hook_Allocate_Audio_Request(bool is_add_request)
    {
        return AudioManager::Allocate_Audio_Request(is_add_request);
    }
    void Hook_Release_Audio_Request(AudioRequest *request) { AudioManager::Release_Audio_Request(request); }
    void Hook_Append_Audio_Request(AudioRequest *request) { AudioManager::Append_Audio_Request(request); }
    AudioEventInfo *Hook_New_Audio_Event_Info(Utf8String name) { return AudioManager::New_Audio_Event_Info(name); }
    void Hook_Add_Audio_Event_Info(AudioEventInfo *info) { AudioManager::Add_Audio_Event_Info(info); }
    AudioEventInfo *Hook_Find_Audio_Event_Info(Utf8String name) { return AudioManager::Find_Audio_Event_Info(name); }
    void Hook_Refresh_Cached_Variables() { AudioManager::Refresh_Cached_Variables(); }
    float Hook_Get_Audio_Length_MS(const AudioEventRTS *event) { return AudioManager::Get_Audio_Length_MS(event); }
    bool Hook_Is_Music_Already_Loaded() { return AudioManager::Is_Music_Already_Loaded(); }
    void Hook_Find_All_Audio_Events_Of_Type(AudioType type, std::vector<AudioEventInfo *> &list)
    {
        AudioManager::Find_All_Audio_Events_Of_Type(type, list);
    }
    const audioinfomap_t *Hook_Get_All_Audio_Events() { return AudioManager::Get_All_Audio_Events(); }
    bool Hook_Is_Current_Provider_Hardware_Accelerated() { return AudioManager::Is_Current_Provider_Hardware_Accelerated(); }
    bool Hook_Is_Current_Speaker_Type_Surround() { return AudioManager::Is_Current_Speaker_Type_Surround(); }
    bool Hook_Should_Play_Locally(const AudioEventRTS *event) { return AudioManager::Should_Play_Locally(event); }
    int Hook_Allocate_New_Handle() { return AudioManager::Allocate_New_Handle(); }
    void Hook_Remove_Level_Specific_Audio_Event_Infos() { AudioManager::Remove_Level_Specific_Audio_Event_Infos(); }
#endif

protected:
    Utf8String Next_Track_Name(Utf8String track) const;
    Utf8String Prev_Track_Name(Utf8String track) const;
    void Remove_All_Audio_Requests();

protected:
    AudioSettings *m_audioSettings;
    MiscAudio *m_miscAudio;
    MusicManager *m_musicManager;
    SoundManager *m_soundManager;
    Coord3D m_listenerPosition;
    Coord3D m_listenerFacing;
    std::list<AudioRequest *> m_audioRequestList;
    std::vector<Utf8String> m_trackList;
    audioinfomap_t m_audioInfoHashMap;
    int m_audioHandleCounter;
    std::list<std::pair<Utf8String, float>>
        m_eventVolumeList; // TODO workout what list this actually is, some kind of volume list
    float m_musicVolume;
    float m_soundVolume;
    float m_3dSoundVolume;
    float m_speechVolume;
    float m_musicVolumeAdjust;
    float m_soundVolumeAdjust;
    float m_3dSoundVolumeAdjust;
    float m_speechVolumeAdjust;
    float m_initialMusicVolume;
    float m_initialSoundVolume;
    float m_initial3DSoundVolume;
    float m_initialSpeechVolume;
    float m_zoomVolume;
    AudioEventRTS *m_unkAudioEventRTS; // TODO work out use of this var
    float *m_savedVolumes; // Used during focus loss to preserve volume settings.

    // Use a bitfield union to make bit twiddling code simpler.
    union
    {
        struct
        {
            bool m_speechOn : 1;
            bool m_soundOn : 1;
            bool m_3dSoundOn : 1;
            bool m_musicOn : 1;
            bool m_volumeSet : 1;
            bool m_hardwareAccelerated : 1;
            bool m_surround : 1;
            bool m_fromCD : 1;
            bool m_unkSpeech : 1;
        };
        unsigned m_cachedVariables;
    };

private:
    static const char *s_speakerTypes[];
};

#ifdef GAME_DLL
#include "hooker.h"
extern AudioManager *&g_theAudio;

inline void AudioManager::Hook_Me()
{
    Hook_Method(0x00404C60, &AudioManager::Hook_Init);
    Hook_Method(0x00404F30, &AudioManager::Hook_Reset);
    // Hook_Method(0x00404F30, &AudioManager::Hook_Update); // Not Implemented.
    Hook_Method(0x00406050, &AudioManager::Hook_Set_Listener_Position);
    Hook_Method(0x00406090, &AudioManager::Hook_Allocate_Audio_Request);
    Hook_Method(0x004061A0, &AudioManager::Hook_Release_Audio_Request);
    Hook_Method(0x004061D0, &AudioManager::Hook_Append_Audio_Request);
    Hook_Method(0x00406250, &AudioManager::Hook_New_Audio_Event_Info);
    Hook_Method(0x004065D0, &AudioManager::Hook_Add_Audio_Event_Info);
    Hook_Method(0x004067B0, &AudioManager::Hook_Find_Audio_Event_Info);
    Hook_Method(0x00406920, &AudioManager::Hook_Refresh_Cached_Variables);
    Hook_Method(0x00406970, &AudioManager::Hook_Get_Audio_Length_MS);
    Hook_Method(0x00406A90, &AudioManager::Hook_Is_Music_Already_Loaded);
    Hook_Method(0x00406C10, &AudioManager::Hook_Find_All_Audio_Events_Of_Type);
    Hook_Method(0x005B9460, &AudioManager::Hook_Get_All_Audio_Events);
    Hook_Method(0x00406D00, &AudioManager::Hook_Is_Current_Provider_Hardware_Accelerated);
    Hook_Method(0x00406DE0, &AudioManager::Hook_Is_Current_Speaker_Type_Surround);
    // Hook_Method(0x00406E00, &AudioManager::Hook_Should_Play_Locally); // Not Implemented.
    Hook_Method(0x00406F00, &AudioManager::Hook_Allocate_New_Handle);
    Hook_Method(0x00406860, &AudioManager::Hook_Remove_Level_Specific_Audio_Event_Infos);
    Hook_Method(0x00406200, &AudioManager::Remove_All_Audio_Requests);
    Hook_Method(0x00405700, &AudioManager::Next_Track_Name);
    Hook_Method(0x004057D0, &AudioManager::Prev_Track_Name);
    Hook_Method(0x00406F10, &AudioManager::Hook_Lose_Focus);
    Hook_Method(0x00406F70, &AudioManager::Hook_Regain_Focus);
    Hook_Method(0x00405390, &AudioManager::Hook_Add_Audio_Event);
    Hook_Method(0x004058A0, &AudioManager::Hook_Remove_Audio_Event);
    Hook_Method(0x00405C30, &AudioManager::Hook_Remove_Audio_Event_String);
    Hook_Method(0x00405680, &AudioManager::Hook_Is_Valid_Const_Audio_Event);
    Hook_Method(0x004058F0, &AudioManager::Hook_Set_Audio_Event_Enabled);
    Hook_Method(0x00405990, &AudioManager::Hook_Set_Audio_Event_Volume_Override);
    Hook_Method(0x00405CC0, &AudioManager::Hook_Remove_Disabled_Events);
    Hook_Method(0x00405340, &AudioManager::Hook_Get_Info_For_Audio_Event);
    Hook_Method(0x00405CD0, &AudioManager::Hook_Translate_From_Speaker_Type);
    Hook_Method(0x00405DD0, &AudioManager::Hook_Translate_To_Speaker_Type);
    Hook_Method(0x00405E50, &AudioManager::Hook_Is_On);
    Hook_Method(0x00405E90, &AudioManager::Hook_Set_On);
    Hook_Method(0x00405F20, &AudioManager::Hook_Set_Volume);
    Hook_Method(0x00405FC0, &AudioManager::Hook_Get_Volume);
    Hook_Method(0x00405FF0, &AudioManager::Hook_Set_3D_Volume_Adjustment);
}
#else
extern AudioManager *g_theAudio;
#endif