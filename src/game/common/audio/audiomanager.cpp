////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: AUDIOMANAGER.CPP
//
//        Author:: OmniBlade
//
//  Contributors::
//
//   Description:: Base class for managing the audio engine.
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
#include "audiomanager.h"

#ifdef THYME_STANDALONE
AudioManager *g_theAudio = nullptr;
#endif

AudioManager::AudioManager() {}

AudioManager::~AudioManager() {}

void AudioManager::Init() {}

void AudioManager::Reset()
{
    m_unkList1.clear();
    m_musicVolumeAdjust = 1.0f;
    m_soundVolumeAdjust = 1.0f;
    m_3dSoundVolumeAdjust = 1.0f;
    m_speechVolumeAdjust = 1.0f;
    m_musicVolume = m_initialMusicVolume;
    m_soundVolume = m_initiaSoundVolume;
    m_3dSoundVolume = m_initial3DSoundVolume;
    m_speechVolume = m_initialSpeechVolume;
    m_cachedVariables &= ~CACHED_UNK1;
}

void AudioManager::Update()
{
    // TODO Requires g_theTacticalView.
    Call_Method<void, AudioManager>(0x00404FB0, this);
}

void AudioManager::Set_Listener_Position(const Coord3D *position, const Coord3D *direction) 
{
    // TODO Should probably take references
    m_listenerPosition = *position;
    m_listenerFacing = *direction;
}

const Coord3D *AudioManager::Get_Listener_Position() const
{
    return &m_listenerPosition;
}

AudioRequest *AudioManager::Allocate_Audio_Request(bool is_add_request)
{
    return new AudioRequest(is_add_request);
}

void AudioManager::Release_Audio_Request(AudioRequest *request)
{
    Delete_Instance(request);
}

void AudioManager::Append_Audio_Request(AudioRequest *request)
{
    m_audioRequestList.push_back(request);
}

AudioEventInfo *AudioManager::New_Audio_Event_Info(AsciiString name)
{
    AudioEventInfo *info = Find_Audio_Event_Info(name);

    if (info == nullptr) {
        info = new AudioEventInfo;

        m_audioInfoHashMap[name] = info;
    }

    return info;
}

void AudioManager::Add_Audio_Event_Info(AudioEventInfo *info)
{
    AudioEventInfo *found = Find_Audio_Event_Info(info->Get_Event_name());

    if (found != nullptr) {
        *found = *info;
    } else {
        m_audioInfoHashMap[info->Get_Event_name()] = info;
    }
}

AudioEventInfo *AudioManager::Find_Audio_Event_Info(AsciiString name) const
{
    auto it = m_audioInfoHashMap.find(name);

    if (it != m_audioInfoHashMap.end()) {
        return it->second;
    }

    return nullptr;
}

void AudioManager::Release_Audio_Event_RTS(AudioEventRTS *event)
{
    delete event;
}

void AudioManager::Set_Hardware_Accelerated(bool accelerated)
{
    if (accelerated) {
        m_cachedVariables |= CACHED_HW_ACCEL;
    } else {
        m_cachedVariables &= ~CACHED_HW_ACCEL;
    }
}

bool AudioManager::Get_Hardware_Accelerated()
{
    return (m_cachedVariables & CACHED_HW_ACCEL) != 0;
}

void AudioManager::Set_Speaker_Surround(bool surround)
{
    if (surround) {
        m_cachedVariables |= CACHED_SURROUND;
    } else {
        m_cachedVariables &= ~CACHED_SURROUND;
    }
}

bool AudioManager::Get_Speaker_Surround()
{
    return (m_cachedVariables & CACHED_SURROUND) != 0;
}

void AudioManager::Refresh_Cached_Variables()
{
    AudioManager::Set_Hardware_Accelerated(Is_Current_Provider_Hardware_Accelerated());
    AudioManager::Set_Speaker_Surround(Is_Current_Speaker_Type_Surround());
}

float AudioManager::Get_Audio_Length_MS(const AudioEventRTS *event)
{
    // TODO Needs more of AudioEventRTS implementing.
    return Call_Method<float, AudioManager, const AudioEventRTS *>(0x00406970, this, event);
}

bool AudioManager::Is_Music_Already_Loaded()
{
    // TODO Needs more of AudioEventRTS implementing.
    return Call_Method<bool, AudioManager>(0x00406A90, this);
}

bool AudioManager::Is_Music_Playing_From_CD()
{
    return (m_cachedVariables & CACHED_FROM_CD) != 0;
}

void AudioManager::Find_All_Audio_Events_Of_Type(AudioType type, std::vector<AudioEventInfo *> &list)
{
    for (auto it = m_audioInfoHashMap.begin(); it != m_audioInfoHashMap.end(); ++it) {
        auto find_it = it;

        // From current position, find if an entry matches the type.
        for (; find_it != m_audioInfoHashMap.end(); ++find_it) {
            if (find_it->second->Get_Type() == type) {
                break;
            }
        }

        it = find_it;

        if (it != m_audioInfoHashMap.end()) {
            list.push_back(it->second);
        } else {
            return;
        }
    }
}

const audioinfomap_t *AudioManager::Get_All_Audio_Events() const
{
    return &m_audioInfoHashMap;
}

bool AudioManager::Is_Current_Provider_Hardware_Accelerated()
{
    return false;
}

bool AudioManager::Is_Current_Speaker_Type_Surround()
{
    return false;
}

bool AudioManager::Should_Play_Locally(const AudioEventRTS *event)
{
    return false;
}

int AudioManager::Allocate_New_Handle()
{
    return 0;
}

void AudioManager::Remove_Level_Specific_Audio_Event_Infos() {}

void AudioManager::Lose_Focus() {}

void AudioManager::Regain_Focus() {}

int AudioManager::Add_Audio_Event(const AudioEventRTS *event)
{
    return 0;
}

void AudioManager::Remove_Audio_Event(unsigned int event) {}

void AudioManager::Remove_Audio_Event(AsciiString event) {}

bool AudioManager::Is_Valid_Audio_Event(const AudioEventRTS *event)
{
    return false;
}

bool AudioManager::Is_Valid_Audio_Event(AudioEventRTS *event)
{
    return false;
}

void AudioManager::Set_Audio_Event_Enabled(AsciiString event, bool vol_override) {}

void AudioManager::Set_Audio_Event_Volume_Override(AsciiString event, float vol_override) {}

void AudioManager::Remove_Disabled_Events() {}

void AudioManager::Get_Info_For_Audio_Event(const AudioEventRTS *event) {}

unsigned int AudioManager::Translate_From_Speaker_Type(const AsciiString &type)
{
    return 0;
}

AsciiString AudioManager::Translate_To_Speaker_Type(unsigned int type)
{
    return AsciiString();
}

bool AudioManager::Is_On(AudioAffect affect) const
{
    return false;
}

void AudioManager::Set_On(bool on, AudioAffect affect) {}

void AudioManager::Set_Volume(float volume, AudioAffect affect) {}

float AudioManager::Get_Volume(AudioAffect affect)
{
    return 0.0f;
}

void AudioManager::Set_3D_Volume_Adjustment(float adj) {}
