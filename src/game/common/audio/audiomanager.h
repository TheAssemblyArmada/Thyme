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

using AudioDataHandle = void *;
using AudioDevice = void *;
using BinkHandle = void *;

class AudioEventRTS;
struct PlayingAudio;
class MusicManager;
class SoundManager;

#ifdef THYME_USE_STLPORT
typedef std::hash_map<const Utf8String, AudioEventInfo *, rts::hash<Utf8String>, std::equal_to<Utf8String>> audioinfomap_t;
#else
typedef std::unordered_map<const Utf8String, AudioEventInfo *, rts::hash<Utf8String>, std::equal_to<Utf8String>>
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
    virtual void Remove_Audio_Event(unsigned handle);
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
    virtual bool Is_Currently_Playing(uintptr_t event) { return true; }
    virtual void Open_Device() = 0;
    virtual void Close_Device() = 0;
    virtual AudioDevice Get_Device() = 0;
    virtual void Notify_Of_Audio_Completion(uintptr_t handle, unsigned type) = 0;
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
    virtual BinkHandle Get_Bink_Handle() = 0;
    virtual void Release_Bink_Handle() = 0;
    virtual void friend_Force_Play_Audio_Event(AudioEventRTS *event) = 0;
    virtual void Set_Listener_Position(const Coord3D *position, const Coord3D *direction);
    virtual const Coord3D *Get_Listener_Position() const { return &m_listenerPosition; }
    virtual AudioRequest *Allocate_Audio_Request(bool is_add_request);
    virtual void Release_Audio_Request(AudioRequest *request);
    virtual void Append_Audio_Request(AudioRequest *request);
    virtual void Process_Request_List() {}
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
    virtual void Close_Any_Sample_Using_File(const AudioDataHandle handle) = 0;
    virtual bool Is_Music_Already_Loaded();
    virtual bool Is_Music_Playing_From_CD() { return m_fromCD; }
    virtual void Find_All_Audio_Events_Of_Type(AudioType type, std::vector<AudioEventInfo *> &list);
    virtual const audioinfomap_t *Get_All_Audio_Events() const;
    virtual bool Is_Current_Provider_Hardware_Accelerated();
    virtual bool Is_Current_Speaker_Type_Surround_Sound();
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
    AudioEventRTS *Get_Empty_Audio_Event() const { return m_emptyAudioEvent; }
    void Add_Track_Name(Utf8String const &track) { m_trackList.push_back(track); }

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
    audioinfomap_t m_allAudioEventInfo;
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
    AudioEventRTS *m_emptyAudioEvent; // TODO work out use of this var
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
extern AudioManager *&g_theAudio;
#else
extern AudioManager *g_theAudio;
#endif