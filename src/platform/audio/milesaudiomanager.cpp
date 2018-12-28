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
#include "milesaudiomanager.h"

MilesAudioManager::MilesAudioManager() :
    m_milesMaxProviderIndex(0),
    m_milesCurrentProvider(-1),
    m_milesLastProvider_maybe(-1),
    m_speakerType(0),
    m_milesDigitalDriver(nullptr),
    m_miles3DPositionObject(0),
    m_milesMonoDelayFilter(0),
    m_audioFileCache(nullptr /*new AudioFileCache*/),
    m_binkPlayingAudio(nullptr),
    m_2dSampleCount(0),
    m_3dSampleCount(0),
    m_streamCount(0)
{
}

MilesAudioManager::~MilesAudioManager() {}

void MilesAudioManager::Init() {}

void MilesAudioManager::Reset() {}

void MilesAudioManager::Update() {}

void MilesAudioManager::Stop_Audio(AudioAffect affect) {}

void MilesAudioManager::Pause_Audio(AudioAffect affect) {}

void MilesAudioManager::Resume_Audio(AudioAffect affect) {}

void MilesAudioManager::Kill_Event_Immediately(unsigned int event) {}

void MilesAudioManager::Next_Music_Track() {}

void MilesAudioManager::Prev_Music_Track() {}

bool MilesAudioManager::Is_Music_Playing()
{
    return false;
}

bool MilesAudioManager::Has_Music_Track_Completed(const Utf8String name)
{
    return false;
}

Utf8String MilesAudioManager::Music_Track_Name()
{
    return Utf8String();
}

bool MilesAudioManager::Is_Currently_Playing()
{
    return false;
}

void MilesAudioManager::Open_Device() {}

void MilesAudioManager::Close_Device() {}

void *MilesAudioManager::Get_Device()
{
    return nullptr;
}

void MilesAudioManager::Notify_Of_Audio_Completion(unsigned int unk1, unsigned int unk2) {}

int MilesAudioManager::Get_Provider_Count()
{
    return 0;
}

Utf8String MilesAudioManager::Get_Provider_Name(unsigned int index) const
{
    return Utf8String();
}

unsigned int MilesAudioManager::Get_Provider_Index(Utf8String name)
{
    return 0;
}

void MilesAudioManager::Select_Provider(unsigned int provider) {}

void MilesAudioManager::Unselect_Provider() {}

unsigned int MilesAudioManager::Get_Selected_Provider()
{
    return 0;
}

void MilesAudioManager::Set_Speaker_Type(unsigned int type) {}

unsigned int MilesAudioManager::Get_Speaker_Type()
{
    return 0;
}

int MilesAudioManager::Get_Num_2D_Samples() const
{
    return 0;
}

int MilesAudioManager::Get_Num_3D_Samples() const
{
    return 0;
}

int MilesAudioManager::Get_Num_Streams() const
{
    return 0;
}

bool MilesAudioManager::Does_Violate_Limit(AudioEventRTS *event) const
{
    return false;
}

bool MilesAudioManager::Is_Playing_Lower_Priority(AudioEventRTS *event) const
{
    return false;
}

bool MilesAudioManager::Is_Playing_Already(AudioEventRTS *event) const
{
    return false;
}

bool MilesAudioManager::Is_Object_Playing_Void(unsigned int obj) const
{
    return false;
}

void MilesAudioManager::Adjust_Volume_Of_Playing_Audio(Utf8String name, float adjust) {}

void MilesAudioManager::Remove_Playing_Audio(Utf8String name) {}

void MilesAudioManager::Remove_All_Disabled_Audio() {}

bool MilesAudioManager::Has_3D_Sensitive_Streams_Playing()
{
    return false;
}

void *MilesAudioManager::Get_Bink_Handle()
{
    return nullptr;
}

void MilesAudioManager::Release_Bink_Handle() {}

void MilesAudioManager::friend_Force_Play_Audio_Event(AudioEventRTS *event) {}

void MilesAudioManager::Process_Request_List() {}

void MilesAudioManager::Release_Audio_Event_RTS(AudioEventRTS *event) {}

void MilesAudioManager::Set_Hardware_Accelerated(bool accelerated) {}

void MilesAudioManager::Set_Speaker_Surround(bool surround) {}

void MilesAudioManager::Set_Preferred_3D_Provider(Utf8String provider) {}

void MilesAudioManager::Set_Preferred_Speaker(Utf8String speaker) {}

float MilesAudioManager::Get_File_Length_MS(Utf8String file_name)
{
    return 0.0f;
}

void MilesAudioManager::Close_Any_Sample_Using_File(const void *handle) {}

void MilesAudioManager::Set_Device_Listener_Position() {}

PlayingAudio *MilesAudioManager::Find_Playing_Audio_From(unsigned int unk1, unsigned int unk2)
{
    return nullptr;
}

void MilesAudioManager::Process_Playing_List() {}

void MilesAudioManager::Process_Fading_List() {}

void MilesAudioManager::Process_Stopped_List() {}
