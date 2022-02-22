/**
 * @file
 *
 * @author feliwir
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
#include "alaudiomanager.h"
#include "filesystem.h"
#include "gamemath.h"
#include "globaldata.h"
#include "playingaudio.h"
#include "soundmanager.h"
#include "videoplayer.h"

using GameMath::Fast_To_Int_Truncate;

ALAudioManager::ALAudioManager() : m_binkPlayingAudio(nullptr), m_2dSampleCount(0), m_3dSampleCount(0), m_streamCount(0) {}

ALAudioManager::~ALAudioManager()
{
    if (m_binkPlayingAudio != nullptr) {
        Release_Playing_Audio(m_binkPlayingAudio);
        m_binkPlayingAudio = nullptr;
    }

    Unselect_Provider();
    g_theAudio = nullptr;
}

void ALAudioManager::Init()
{
    AudioManager::Init();
    Open_Device();
}

void ALAudioManager::Reset()
{
    // Worldbuilder is a debug build and has code to dump a list of audio assets here.
    AudioManager::Reset();
    // Stop_All_Audio_Immediately();
    Remove_All_Audio_Requests();
    Remove_Level_Specific_Audio_Event_Infos();
}

void ALAudioManager::Update()
{
    AudioManager::Update();
    Set_Device_Listener_Position();
    Process_Request_List();
    Process_Playing_List();
    Process_Fading_List();
    Process_Stopped_List();
}

void ALAudioManager::Stop_Audio(AudioAffect affect) {}

/**
 * Iterates playing audio lists and pauses the streams for the specified affects.
 */
void ALAudioManager::Pause_Audio(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_SOUND) {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr) {
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
                if (affect & AUDIOAFFECT_MUSIC) {
                }
            }
        }
    }

    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end();) {
        if (*it != nullptr && (*it)->m_requestType == AR_PLAY) {
            (*it)->Delete_Instance();
            it = m_audioRequestList.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Iterates playing audio lists and resumes the streams for the specified affects.
 */
void ALAudioManager::Resume_Audio(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_SOUND) {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr) {
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
            }
        }
    }
}

/**
 * Kills an event immediately based on the provided handle.
 */
void ALAudioManager::Kill_Event_Immediately(uintptr_t event)
{
    // Iterate the various lists until a matching handle is found.
    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end(); ++it) {
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
    }
}

/**
 * Starts the next song playing.
 */
void ALAudioManager::Next_Music_Track()
{
    Utf8String track_name;

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
    }

    // Original calls g_theAudio->Remove_Audio_Event(4) but result should be the same.
    Remove_Audio_Event(4); // 4 and 5 appear to relate to MusicManager.
    track_name = Next_Track_Name(track_name);
    AudioEventRTS rts_event(track_name);
    // Original uses global pointer for some reason for virtual call?
    // g_theAudio->Add_Audio_Event(&rts_event);
    Add_Audio_Event(&rts_event);
}

/**
 * Starts the previous song playing.
 */
void ALAudioManager::Prev_Music_Track()
{
    Utf8String track_name;

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
    }

    // Original calls g_theAudio->Remove_Audio_Event(4) but result should be the same.
    Remove_Audio_Event(4); // 4 and 5 appear to relate to MusicManager.
    track_name = Prev_Track_Name(track_name);
    AudioEventRTS rts_event(track_name);
    // g_theAudio->Add_Audio_Event(&rts_event);
    Add_Audio_Event(&rts_event);
}

/**
 * Checks if any currently playing audio events is a music track.
 */
bool ALAudioManager::Is_Music_Playing()
{
    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
    }

    return false;
}

/**
 * Checks if the named mustic track has finished playing a number of loops.
 */
bool ALAudioManager::Has_Music_Track_Completed(const Utf8String &name, int loops)
{
    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            // TODO: check for loopcount
            if ((*it)->openal.audio_event->Get_Event_Name() == name) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Gets the name of the currently queued or playing music track.
 */
Utf8String ALAudioManager::Music_Track_Name()
{
    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end(); ++it) {
        if ((*it)->Request_Type() == AR_PLAY && (*it)->Is_Adding()
            && (*it)->Event_Object()->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            return (*it)->Event_Object()->Get_Event_Name();
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            return (*it)->openal.audio_event->Get_Event_Name();
        }
    }

    return Utf8String();
}

/**
 * Checks if a given event handle refers to something that is currently playing.
 */
bool ALAudioManager::Is_Currently_Playing(uintptr_t event)
{
    // Iterate the various lists until a matching handle is found.
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == event) {
            return true;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == event) {
            return true;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == event) {
            return true;
        }
    }

    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end(); ++it) {
        if (*it != nullptr && (*it)->Is_Adding() && (*it)->Event_Object()->Get_Playing_Handle() == event) {
            return true;
        }
    }

    return false;
}

/**
 * Opens an audio device for playing audio.
 */
void ALAudioManager::Open_Device()
{
    if (!g_theWriteableGlobalData->m_audioOn) {
        return;
    }

    m_speakerType = Translate_From_Speaker_Type(m_preferredSpeaker);

    // TODO: build devices list & set settings


    m_alcDevice = alcOpenDevice(NULL);
    if (!m_alcDevice) {
        captainslog_error("Failed to open ALC device");
        return;
    }

    Select_Provider(Get_Provider_Index(m_preferredProvider));
}

/**
 * Closes the audio device.
 */
void ALAudioManager::Close_Device()
{
    Unselect_Provider();
    if (m_alcDevice)
        alcCloseDevice(m_alcDevice);
}

/**
 * Handles audio completion.
 */
void ALAudioManager::Notify_Of_Audio_Completion(uintptr_t handle, unsigned unk2)
{
    PlayingAudio *playing = Find_Playing_Audio_From(handle, unk2);

    if (playing == nullptr) {
        return;
    }

    // TODO:
}

/**
 * Gets the name of a provider from its index.
 */
Utf8String ALAudioManager::Get_Provider_Name(unsigned provider) const
{
    return Utf8String::s_emptyString;
}

/**
 * Gets the index of a provider from its name.
 */
unsigned ALAudioManager::Get_Provider_Index(Utf8String name)
{
    for (int i = 0; i < m_alMaxDevicesIndex; ++i) {
        if (name == m_alDevicesList[i]) {
            return i;
        }
    }

    return UINT_MAX;
}

/**
 * Sets the provided provider as the current provider for the audio engine.
 */
void ALAudioManager::Select_Provider(unsigned provider) {}

/**
 * Invalidates the current provider.
 */
void ALAudioManager::Unselect_Provider()
{
    if (g_theVideoPlayer != nullptr) {
        g_theVideoPlayer->Notify_Player_Of_New_Provider(false);
    }
}

/**
 * Sets the speaker type.
 */
void ALAudioManager::Set_Speaker_Type(unsigned type)
{
    // TODO: set this
    m_speakerType = type;
}

/**
 * Gets the speaker type.
 */
unsigned ALAudioManager::Get_Speaker_Type()
{
    return m_speakerType;
}

/**
 * Checks if the number of instances of an event exceeds the limits for that event.
 */
bool ALAudioManager::Does_Violate_Limit(AudioEventRTS *event) const
{
    int limit;

    if ((limit = event->Get_Event_Info()->Get_Limit()) == 0) {
        return false;
    }

    int playing_matches = 0;
    int request_matches = 0;

    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (event->Get_Event_Name() == (*it)->openal.audio_event->Get_Event_Name()) {
                if (playing_matches == 0) {
                    event->Set_Handle_To_Kill((*it)->openal.audio_event->Get_Playing_Handle());
                }

                ++playing_matches;
            }
        }
    } else {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (event->Get_Event_Name() == (*it)->openal.audio_event->Get_Event_Name()) {
                if (playing_matches == 0) {
                    event->Set_Handle_To_Kill((*it)->openal.audio_event->Get_Playing_Handle());
                }

                ++playing_matches;
            }
        }
    }

    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end(); ++it) {
        if ((*it) != nullptr && (*it)->m_isAdding && event->Get_Event_Name() == (*it)->m_event.object->Get_Event_Name()) {
            ++playing_matches;
            ++request_matches;
        }
    }

    if ((event->Get_Event_Info()->Get_Control() & CONTROL_INTERRUPT) && request_matches < limit) {
        if (playing_matches < limit) {
            event->Set_Handle_To_Kill(0);
        }

        return false;
    }

    if (playing_matches < limit) {
        event->Set_Handle_To_Kill(0);

        return false;
    }

    return true;
}

/**
 * Checks if an event is playing at low priority.
 */
bool ALAudioManager::Is_Playing_Lower_Priority(AudioEventRTS *event) const
{
    int priority;

    if ((priority = event->Get_Event_Info()->Get_Priority()) == 0) {
        return false;
    }

    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if ((*it)->openal.audio_event->Get_Event_Info()->Get_Priority() < priority) {
                return true;
            }
        }

        return false;
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if ((*it)->openal.audio_event->Get_Event_Info()->Get_Priority() < priority) {
            return true;
        }
    }

    return false;
}

/**
 * Checks if an event is playing already.
 */
bool ALAudioManager::Is_Playing_Already(AudioEventRTS *event) const
{
    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if ((*it)->openal.audio_event->Get_Event_Name() == event->Get_Event_Name()) {
                return true;
            }
        }

        return false;
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if ((*it)->openal.audio_event->Get_Event_Name() == event->Get_Event_Name()) {
            return true;
        }
    }

    return false;
}

/**
 * Checks if an object is playing a voice.
 */
bool ALAudioManager::Is_Object_Playing_Voice(unsigned obj) const
{
    if (obj == 0) {
        return false;
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if ((*it)->openal.audio_event->Get_Object_ID() == obj
            && ((*it)->openal.audio_event->Get_Event_Info()->Get_Visibility() & VISIBILITY_VOICE)) {
            return true;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if ((*it)->openal.audio_event->Get_Object_ID() == obj
            && ((*it)->openal.audio_event->Get_Event_Info()->Get_Visibility() & VISIBILITY_VOICE)) {
            return true;
        }
    }

    return false;
}

/**
 * Changes the volume of playing audio.
 */
void ALAudioManager::Adjust_Volume_Of_Playing_Audio(Utf8String name, float adjust)
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            (*it)->openal.audio_event->Set_Volume(adjust);
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            (*it)->openal.audio_event->Set_Volume(adjust);
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            (*it)->openal.audio_event->Set_Volume(adjust);
        }
    }
}

/**
 * Removes playing audio.
 */
void ALAudioManager::Remove_Playing_Audio(Utf8String name)
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            Release_Playing_Audio(*it);
            it = m_globalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end();) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            Release_Playing_Audio(*it);
            it = m_positionalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end();) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            Release_Playing_Audio(*it);
            it = m_streamList.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Removes all audio with a volume of 0.
 */
void ALAudioManager::Remove_All_Disabled_Audio()
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Volume() == 0.0f) {
            Release_Playing_Audio(*it);
            it = m_globalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end();) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Volume() == 0.0f) {
            Release_Playing_Audio(*it);
            it = m_positionalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end();) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Volume() == 0.0f) {
            Release_Playing_Audio(*it);
            it = m_streamList.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Checks if any streams have properties suggesting they are position sensitive.
 */
bool ALAudioManager::Has_3D_Sensitive_Streams_Playing()
{
    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr
            && ((*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                || !(*it)->openal.audio_event->Get_Event_Name().Starts_With("Game_"))) {
            return true;
        }
    }

    return false;
}

/**
 * Gets an audio handle for use by the video player.
 */
void *ALAudioManager::Get_Bink_Handle()
{
    // If we don't already have a playing audio for bink, make one.
    if (m_binkPlayingAudio == nullptr) {
        PlayingAudio *pap = new PlayingAudio;
        pap->openal.stopped = false;
        pap->openal.audio_event = new AudioEventRTS("BinkHandle");
        Get_Info_For_Audio_Event(pap->openal.audio_event);
        pap->openal.playing_type = 0;

        m_binkPlayingAudio = pap;
    }

    return reinterpret_cast<void *>(m_binkPlayingAudio->openal.source);
}

/**
 * Releases the audio handle for use by the video player.
 */
void ALAudioManager::Release_Bink_Handle()
{
    if (m_binkPlayingAudio != nullptr) {
        Release_Playing_Audio(m_binkPlayingAudio);
        m_binkPlayingAudio = nullptr;
    }
}

/**
 * Forces an audio event to be played.
 */
void ALAudioManager::friend_Force_Play_Audio_Event(AudioEventRTS *event)
{
    if (event->Get_Event_Info() == nullptr) {
        Get_Info_For_Audio_Event(event);
        if (event->Get_Event_Info() == nullptr) {
            captainslog_warn("No info for forced audio event '%s'", event->Get_Event_Name().Str());
            return;
        }
    }

    // TODO:
}

/**
 * Processes the request list for audio waiting to be played.
 */
void ALAudioManager::Process_Request_List()
{
    // TODO
}

/**
 * Sets if the audio engine should use hardware acceleration if available.
 */
void ALAudioManager::Set_Hardware_Accelerated(bool accelerated)
{
    // do nothing i guess
}

/**
 * Sets if the audio engine should use surround sound if available.
 */
void ALAudioManager::Set_Speaker_Surround(bool surround)
{
    bool changed = m_surround != surround;
    AudioManager::Set_Speaker_Surround(surround);

    if (changed) {
        // These calls call g_theAudio global rather than calling on this instance in original.
        if (m_surround) {
            Set_Speaker_Type(Get_Audio_Settings()->Get_Default_3D_Speaker_Type());
        } else {
            Set_Speaker_Type(Get_Audio_Settings()->Get_Default_2D_Speaker_Type());
        }
    }
}

/**
 * Sets the preferred provider for 3D sounds.
 */
void ALAudioManager::Set_Preferred_3D_Provider(Utf8String provider)
{
    m_preferredProvider = provider;
}

/**
 * Sets the preferred speaker arrangement.
 */
void ALAudioManager::Set_Preferred_Speaker(Utf8String speaker)
{
    m_preferredSpeaker = speaker;
}

/**
 * Gets the files length in milliseconds.
 */
float ALAudioManager::Get_File_Length_MS(Utf8String file_name)
{
    if (file_name.Is_Empty()) {
        return 0.0f;
    }

    // TODO:
    int32_t length;

    return length;
}

/**
 * Close all samples using a given file handle.
 */
void ALAudioManager::Close_Any_Sample_Using_File(const AudioDataHandle handle)
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if ((*it)->openal.file_handle == handle) {
            Release_Playing_Audio(*it);
            it = m_globalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end();) {
        if ((*it)->openal.file_handle == handle) {
            Release_Playing_Audio(*it);
            it = m_positionalAudioList.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Sets the position of the listener for 3D sounds.
 */
void ALAudioManager::Set_Device_Listener_Position() {}

/**
 * Finds a playing audio that uses a given handle and is a given type.
 */
PlayingAudio *ALAudioManager::Find_Playing_Audio_From(uintptr_t handle, unsigned type)
{
    switch (type) {
        case 0:
            for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
                if (*it != nullptr && (*it)->openal.source == (int)handle) {
                    return *it;
                }
            }

            break;
        case 1:
            for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
                if (*it != nullptr && (*it)->openal.source == (int)handle) {
                    return *it;
                }
            }

            break;
        case 2:
            for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
                if (*it != nullptr && (*it)->openal.source == (int)handle) {
                    return *it;
                }
            }

            break;
        default:
            break;
    }

    return nullptr;
}

/**
 * Processes the playing audio lists.
 */
void ALAudioManager::Process_Playing_List()
{
    for (auto it = m_streamList.begin(); it != m_streamList.end();) {
        if (*it != nullptr) {
            if ((*it)->openal.stopped == 1) {
                Release_Playing_Audio(*it);
                it = m_streamList.erase(it);
            } else {
                if (m_volumeSet) {
                    Adjust_Playing_Volume(*it);
                }

                ++it;
            }
        } else {
            it = m_streamList.erase(it);
        }
    }

    if (m_volumeSet) {
        m_volumeSet = false;
    }
}

/**
 * Processes the fading audio lists.
 */
void ALAudioManager::Process_Fading_List()
{
    // TODO:
}

/**
 * Processes the stopped audio lists.
 */
void ALAudioManager::Process_Stopped_List()
{
    for (auto it = m_stoppedList.begin(); it != m_stoppedList.end(); it = m_stoppedList.erase(it)) {
        if (*it != nullptr) {
            Release_Playing_Audio(*it);
        }
    }
}

/**
 * Stops the playing audio sample.
 */
void ALAudioManager::Release_Playing_Audio(PlayingAudio *audio)
{
    if (audio->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_SOUND) {
        if (audio->openal.playing_type != 0) {
            m_soundManager->Notify_Of_3D_Sample_Completion();
        } else {
            m_soundManager->Notify_Of_2D_Sample_Completion();
        }
    }

    // TODO: free source

    if (audio->openal.release_event) {
        Release_Audio_Event_RTS(audio->openal.audio_event);
    }

    delete audio;
}

/**
 * Adjusts the volume of a playing audio object at the Miles audio level.
 */
void ALAudioManager::Adjust_Playing_Volume(PlayingAudio *audio) {}