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
#include "alaudiomanager.h"
#include "alaudiostream.h"
#include "filesystem.h"
#include "gamemath.h"
#include "gamesounds.h"
#include "globaldata.h"
#include "playingaudio.h"
#include "videoplayer.h"

namespace Thyme
{
ALAudioManager::ALAudioManager() :
    m_alMaxDevicesIndex(0),
    m_speakerType(0),
#ifdef BUILD_WITH_FFMPEG
    m_audioFileCache(new FFmpegAudioFileCache),
#endif
    m_2dSampleCount(0),
    m_3dSampleCount(0),
    m_streamCount(0)
{
}

ALAudioManager::~ALAudioManager()
{
    Release_Bink_Handle();
    Close_Device();
    g_theAudio = nullptr;
    delete m_audioFileCache;
}

/**
 * Initialise the subsystem.
 */
void ALAudioManager::Init()
{
    AudioManager::Init();
    Open_Device();
#if BUILD_WITH_FFMPEG
    m_audioFileCache->Set_Max_Size(m_audioSettings->Get_Audio_Footprint());
#endif
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
}

/**
 * Reset the subsystem.
 */
void ALAudioManager::Reset()
{
    // Worldbuilder is a debug build and has code to dump a list of audio assets here.
    AudioManager::Reset();
    Stop_All_Audio_Immediately();
    Remove_All_Audio_Requests();
    Remove_Level_Specific_Audio_Event_Infos();
}

/**
 * Update the subsystem.
 */
void ALAudioManager::Update()
{
    AudioManager::Update();
    Set_Device_Listener_Position();
    Process_Request_List();
    Process_Playing_List();
    Process_Fading_List();
    Process_Stopped_List();
}

/**
 * Iterates playing audio lists and stops the streams for the specified affects.
 */
void ALAudioManager::Stop_Audio(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_SOUND) {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr) {
                alSourceStop((*it)->openal.source);
                (*it)->openal.stopped = 1;
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
                alSourceStop((*it)->openal.source);
                (*it)->openal.stopped = 1;
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
                if ((affect & AUDIOAFFECT_MUSIC)
                    || ((*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                        && (affect & AUDIOAFFECT_SPEECH))) {
                    alSourceStop((*it)->openal.source);
                    (*it)->openal.stopped = 1;
                }
            }
        }
    }
}

/**
 * Iterates playing audio lists and pauses the streams for the specified affects.
 */
void ALAudioManager::Pause_Audio(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_SOUND) {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr) {
                alSourcePause((*it)->openal.source);
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
                alSourcePause((*it)->openal.source);
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
                if ((affect & AUDIOAFFECT_MUSIC)
                    || ((*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                        && (affect & AUDIOAFFECT_SPEECH))) {
                    alSourcePause((*it)->openal.source);
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
                alSourcePlay((*it)->openal.source);
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
                alSourcePlay((*it)->openal.source);
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
                if ((affect & AUDIOAFFECT_MUSIC)
                    || ((*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                        && (affect & AUDIOAFFECT_SPEECH))) {
                    alSourcePlay((*it)->openal.source);
                }
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
        if (*it != nullptr && (*it)->Request_Type() == AR_PLAY && (*it)->Event_Handle() == event) {
            (*it)->Delete_Instance();
            m_audioRequestList.erase(it);

            return;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == event) {
            Release_Playing_Audio(*it);
            m_positionalAudioList.erase(it);

            return;
        }
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == event) {
            Release_Playing_Audio(*it);
            m_globalAudioList.erase(it);

            return;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == event) {
            Release_Playing_Audio(*it);
            m_streamList.erase(it);

            return;
        }
    }
}

/**
 * Starts the next song playing.
 */
void ALAudioManager::Next_Music_Track()
{
    Utf8String track_name;

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            track_name = (*it)->openal.audio_event->Get_Event_Name();
        }
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
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            track_name = (*it)->openal.audio_event->Get_Event_Name();
        }
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
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            return true;
        }
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
    if (g_theWriteableGlobalData && !g_theWriteableGlobalData->m_audioOn) {
        return;
    }

    m_speakerType = Translate_From_Speaker_Type(m_preferredSpeaker);

    // TODO: build devices list & set settings
    Enumerate_Devices();

    m_alcDevice = alcOpenDevice(NULL);
    if (m_alcDevice == nullptr) {
        captainslog_error("Failed to open ALC device");
        return;
    }
    captainslog_dbgassert(Check_ALC_Error(), "Failed to open ALC device");

    ALCint attributes[] = { ALC_FREQUENCY, m_audioSettings->Output_Rate(), 0 /* end-of-list */ };
    m_alcContext = alcCreateContext(m_alcDevice, attributes);
    if (m_alcContext == nullptr) {
        captainslog_error("Failed to create ALC context");
        return;
    }
    captainslog_dbgassert(Check_ALC_Error(), "Failed to create ALC context");

    if (!alcMakeContextCurrent(m_alcContext)) {
        captainslog_error("Failed to make ALC context current");
        return;
    }
    captainslog_dbgassert(Check_ALC_Error(), "Failed to create ALC context");

    Select_Provider(Get_Provider_Index(m_preferredProvider));
    Refresh_Cached_Variables();

    captainslog_dbgassert(Check_AL_Error(), "OpenAL error before starting");
    captainslog_dbgassert(Supports_Float_Samples(), "OpenAL implementation doesn't support float samples");
}

/**
 * Closes the audio device.
 */
void ALAudioManager::Close_Device()
{
    Unselect_Provider();

    alcMakeContextCurrent(nullptr);

    if (m_alcContext)
        alcDestroyContext(m_alcContext);

    if (m_alcDevice)
        alcCloseDevice(m_alcDevice);
}

/**
 * Handles audio completion.
 */
void ALAudioManager::Notify_Of_Audio_Completion(uintptr_t handle, unsigned type)
{
    PlayingAudio *playing = Find_Playing_Audio_From(handle, type);

    if (playing == nullptr) {
        return;
    }

    if (m_unkSpeech && playing->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_SPEECH) {
        m_unkSpeech = false;
    }

    if (playing->openal.audio_event->Get_Event_Info()->Get_Control() & CONTROL_LOOP) {
        if (playing->openal.audio_event->Get_Next_Play_Portion() == 0) {
            playing->openal.audio_event->Set_Next_Play_Portion(1);
        }
    }

    if (!(playing->openal.audio_event->Get_Event_Info()->Get_Control() & CONTROL_LOOP)
        || playing->openal.audio_event->Get_Next_Play_Portion() != 1
        || !(playing->openal.audio_event->Decrease_Loop_Count(), Start_Next_Loop(playing))) {
        playing->openal.audio_event->Advance_Next_Play_Portion();

        if (playing->openal.audio_event->Get_Next_Play_Portion() != 3) {
            switch (playing->openal.playing_type) {
                case PAT_2DSAMPLE:
                    Close_File(playing->openal.file_handle);
                    playing->openal.file_handle = Play_Sample2D(playing->openal.audio_event, playing);

                    if (playing->openal.file_handle != nullptr) {
                        return;
                    }
                    break;
                case PAT_3DSAMPLE:
                    Close_File(playing->openal.file_handle);
                    playing->openal.file_handle = Play_Sample3D(playing->openal.audio_event, playing);

                    if (playing->openal.file_handle != nullptr) {
                        return;
                    }
                    break;
                default:
                    break;
            }
        }

        if (playing->openal.playing_type == PAT_STREAM
            && playing->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            Play_Stream(playing->openal.audio_event, playing->openal.source);
        } else {
            playing->openal.stopped = 1;
        }
    }
}

/**
 * Gets the name of a provider from its index.
 */
Utf8String ALAudioManager::Get_Provider_Name(unsigned provider) const
{
    if (Is_On(AUDIOAFFECT_3DSOUND) && provider < (unsigned)m_alMaxDevicesIndex) {
        // TODO: return the provider
    }

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
            Adjust_Playing_Volume(*it);
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            (*it)->openal.audio_event->Set_Volume(adjust);
            Adjust_Playing_Volume(*it);
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Event_Name() == name) {
            (*it)->openal.audio_event->Set_Volume(adjust);
            Adjust_Playing_Volume(*it);
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
    if (m_streamList.empty()) {
        return false;
    }

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
 * For us each videostream holds it's own ALAudioStream
 */
BinkHandle ALAudioManager::Get_Bink_Handle()
{
    return nullptr;
}

/**
 * Releases the audio handle for use by the video player.
 * For us each videostream holds it's own ALAudioStream
 */
void ALAudioManager::Release_Bink_Handle() {}

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
    switch (event->Get_Event_Info()->Get_Event_Type()) {
        case EVENT_MUSIC:
            if (!Is_On(AUDIOAFFECT_MUSIC)) {
                return;
            }
            break;
        case EVENT_SPEECH:
            if (!Is_On(AUDIOAFFECT_SPEECH)) {
                return;
            }
            break;
        case EVENT_SOUND:
            if (!Is_On(AUDIOAFFECT_SOUND) || !Is_On(AUDIOAFFECT_3DSOUND)) {
                return;
            }
            break;
        default:
            break;
    }

    captainslog_warn("ALAudioManager just forwards Force_Play_Audio_Event to regular Play_Audio_Event");
    Play_Audio_Event(event);
}

/**
 * Processes the request list for audio waiting to be played.
 */
void ALAudioManager::Process_Request_List()
{
    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end();) {
        if (*it != nullptr) {
            if (Process_Request_This_Frame(*it)) {
                if (!(*it)->m_isProcessed || Check_For_Sample(*it)) {
                    Process_Request(*it);
                }

                (*it)->Delete_Instance();
                it = m_audioRequestList.erase(it);
            } else {
                Adjust_Request(*it);
                ++it;
            }
        }
    }
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

    float length = 0.0f;

#ifdef BUILD_WITH_FFMPEG
    AudioDataHandle handle = m_audioFileCache->Open_File(file_name);
    length = m_audioFileCache->Get_File_Length_MS(handle);
    m_audioFileCache->Close_File(handle);
#endif

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
void ALAudioManager::Set_Device_Listener_Position()
{
    ALfloat listenerOri[] = { m_listenerFacing.x, m_listenerFacing.y, m_listenerFacing.z, 0.0f, 0.0f, -1.0f };
    alListener3f(AL_POSITION, m_listenerPosition.x, m_listenerPosition.y, m_listenerPosition.z);
    alListenerfv(AL_ORIENTATION, listenerOri);
}

/**
 * Finds a playing audio that uses a given handle and is a given type.
 */
PlayingAudio *ALAudioManager::Find_Playing_Audio_From(uintptr_t handle, unsigned type)
{
    switch (type) {
        case PAT_2DSAMPLE:
            for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
                if (*it != nullptr && (*it)->openal.source == (ALuint)handle) {
                    return *it;
                }
            }

            break;
        case PAT_3DSAMPLE:
            for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
                if (*it != nullptr && (*it)->openal.source == (ALuint)handle) {
                    return *it;
                }
            }

            break;
        case PAT_STREAM:
            for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
                if (*it != nullptr && (*it)->openal.source == (ALuint)handle) {
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
    ALint source_state;
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if (*it != nullptr) {
            // Check if the audio has finished playing
            if ((*it)->openal.source) {
                alGetSourcei((*it)->openal.source, AL_SOURCE_STATE, &source_state);
                if (source_state == AL_STOPPED)
                    Notify_Of_Audio_Completion((*it)->openal.source, PAT_2DSAMPLE);
            }

            if ((*it)->openal.stopped == 1) {
                Release_Playing_Audio(*it);
                it = m_globalAudioList.erase(it);
            } else {
                if (m_volumeSet) {
                    Adjust_Playing_Volume(*it);
                }

                ++it;
            }
        } else {
            it = m_globalAudioList.erase(it);
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end();) {
        if (*it != nullptr) {
            // Check if the audio has finished playing
            if ((*it)->openal.source) {
                alGetSourcei((*it)->openal.source, AL_SOURCE_STATE, &source_state);
                if (source_state == AL_STOPPED)
                    Notify_Of_Audio_Completion((*it)->openal.source, PAT_3DSAMPLE);
            }

            if ((*it)->openal.stopped == 1) {
                Release_Playing_Audio(*it);
                it = m_positionalAudioList.erase(it);
            } else {
                if (m_volumeSet) {
                    Adjust_Playing_Volume(*it);
                }

                Coord3D *current_pos = (*it)->openal.audio_event->Get_Current_Pos();

                if (current_pos != nullptr) {
                    if ((*it)->openal.audio_event->Get_Event_Type() == EVENT_UNKVAL3) {
                        Stop_Audio_Event((*it)->openal.audio_event->Get_Playing_Handle());
                        ++it;
                    } else {
                        float sample_vol = Get_Effective_Volume((*it)->openal.audio_event);
                        // Is this conditional check incorrect? Original does this but makes no sense.
                        // BUGFIX TODO should be m3dSoundVolume for the result as well?
                        sample_vol /= m_3dSoundVolume > 0.0f ? m_soundVolume : 1.0f;

                        if (sample_vol < m_audioSettings->Get_Min_Sample_Vol()
                            && !(((*it)->openal.audio_event->Get_Event_Info()->Get_Visibility() & VISIBILITY_GLOBAL)
                                || ((*it)->openal.audio_event->Get_Event_Info()->Get_Priority() == 4))) {
                            Release_Playing_Audio(*it);
                            it = m_positionalAudioList.erase(it);
                        } else {
                            alSource3f((*it)->openal.source, AL_POSITION, current_pos->x, current_pos->y, current_pos->z);
                            ++it;
                        }
                    }
                } else {
                    Release_Playing_Audio(*it);
                    it = m_positionalAudioList.erase(it);
                }
            }
        } else {
            it = m_positionalAudioList.erase(it);
        }
    }

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
    for (auto it = m_fadingList.begin(); it != m_fadingList.end();) {
        if (*it == nullptr) {
            // TODO BUGFIX This appears to be an infinite loop if any entry is null.
            continue;
        }

        if ((*it)->openal.time_fading < Get_Audio_Settings()->Get_Time_To_Fade()) {
            ++(*it)->openal.time_fading;
            float adjustment = (float)(*it)->openal.time_fading / (float)Get_Audio_Settings()->Get_Time_To_Fade();
            float effective_vol = (float)(1.0f - adjustment) * Get_Effective_Volume((*it)->openal.audio_event);
            alSourcef((*it)->openal.source, AL_GAIN, effective_vol);
            ++it;
        } else {
            (*it)->openal.stopped = 1;
            (*it)->openal.disable_loops = true;
            Release_Playing_Audio(*it);
            it = m_fadingList.erase(it);
        }
    }
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
        switch (audio->openal.playing_type) {
            case PAT_2DSAMPLE:
                if (m_soundManager)
                    m_soundManager->Notify_Of_2D_Sample_Completion();
                break;
            case PAT_3DSAMPLE:
                if (m_soundManager)
                    m_soundManager->Notify_Of_3D_Sample_Completion();
                break;
            default:
                break;
        }
    }

    Release_OpenAL_Handles(audio);
    Close_File(audio->openal.file_handle);

    if (audio->openal.release_event) {
        Release_Audio_Event_RTS(audio->openal.audio_event);
    }

    delete audio;
}

/**
 * Stops the playing audio sample.
 */
void ALAudioManager::Release_OpenAL_Handles(PlayingAudio *audio)
{
    if (audio->openal.buffer)
        alDeleteBuffers(1, &audio->openal.buffer);
    if (audio->openal.source)
        alDeleteSources(1, &audio->openal.source);

    audio->openal.playing_type = PAT_NONE;
}

/**
 * Frees memory holding audio samples.
 */
void ALAudioManager::Free_All_OpenAL_Handles()
{
    Stop_All_Audio_Immediately();

    m_2dSampleCount = 0;
    m_3dSampleCount = 0;
    m_streamCount = 0;
}

/**
 * Releases all playing audio samples.
 */
void ALAudioManager::Stop_All_Audio_Immediately()
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); it = m_globalAudioList.erase(it)) {
        if (*it != nullptr) {
            Release_Playing_Audio(*it);
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); it = m_positionalAudioList.erase(it)) {
        if (*it != nullptr) {
            Release_Playing_Audio(*it);
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); it = m_streamList.erase(it)) {
        if (*it != nullptr) {
            Release_Playing_Audio(*it);
        }
    }

    for (auto it = m_fadingList.begin(); it != m_fadingList.end(); it = m_fadingList.erase(it)) {
        if (*it != nullptr) {
            Release_Playing_Audio(*it);
        }
    }
}

/**
 * Loads a miles stream for an audio event.
 */
void ALAudioManager::Play_Stream(AudioEventRTS *event, ALuint source)
{
    if (event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
        alSourcei(source, AL_LOOPING, AL_TRUE);
    }

    // TODO: stream file
    alSourcePlay(source);
}

/**
 * Loads a file and attach it to an OpenAL source. Returns a pointer to the file data.
 */
AudioDataHandle ALAudioManager::Play_Sample3D(AudioEventRTS *event, PlayingAudio *audio)
{
    Coord3D *pos = Get_Current_Position_From_Event(event);

    if (pos == nullptr) {
        return nullptr;
    }

    // We can just reuse our existing function
    AudioDataHandle handle = Play_Sample(event, audio);

    if (handle != nullptr) {
        auto openal = audio->openal;
        if (event->Get_Event_Info()->Get_Visibility() & VISIBILITY_GLOBAL) {
            alSourcef(openal.source, AL_REFERENCE_DISTANCE, m_audioSettings->Global_Min_Range());
            alSourcef(openal.source, AL_MAX_DISTANCE, m_audioSettings->Global_Max_Range());
        } else {
            alSourcef(openal.source, AL_REFERENCE_DISTANCE, event->Get_Event_Info()->Min_Range());
            alSourcef(openal.source, AL_MAX_DISTANCE, event->Get_Event_Info()->Max_Range());
        }
        alSource3f(openal.source, AL_POSITION, pos->x, pos->y, pos->z);
    }

    return handle;
}

/**
 * Loads a file and attach it to an OpenAL source. Returns a pointer to the file data.
 */
AudioDataHandle ALAudioManager::Play_Sample2D(AudioEventRTS *event, PlayingAudio *audio)
{
    // We can just reuse our existing function
    AudioDataHandle handle = Play_Sample(event, audio);

    if (handle != nullptr) {
        auto openal = audio->openal;
        alSourcei(openal.source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(openal.source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    }
    return handle;
}

/**
 * Loads a file and attach it to an OpenAL source. Returns a pointer to the file data.
 */
AudioDataHandle ALAudioManager::Play_Sample(AudioEventRTS *event, PlayingAudio *audio)
{
    AudioDataHandle handle = Open_File(event);

    if (handle != nullptr) {
        uint8_t *data = nullptr;
        uint32_t size = 0;
        uint32_t freq = 0;
        uint8_t channels = 1;
        uint8_t bits_per_sample = 16;
#if BUILD_WITH_FFMPEG
        FFmpegAudioFileCache::Get_Wave_Data(handle, data, size, freq, channels, bits_per_sample);
#endif
        auto openal = audio->openal;
        alGenBuffers(1, &openal.buffer);
        captainslog_dbgassert(Check_AL_Error(), "Failed to generate buffer");
        alBufferData(openal.buffer, Get_AL_Format(channels, bits_per_sample), data, size, freq);
        captainslog_dbgassert(Check_AL_Error(), "Failed to buffer data");
        alSourcei(openal.source, AL_BUFFER, openal.buffer);
        alSourcePlay(openal.source);
        captainslog_dbgassert(Check_AL_Error(), "Failed to play source");
    }

    return handle;
}

/**
 * Attempts to start another loop of a playing audio file, return indicates success.
 */
bool ALAudioManager::Start_Next_Loop(PlayingAudio *audio)
{
    Close_File(audio->openal.file_handle);
    audio->openal.file_handle = nullptr;

    if (audio->openal.disable_loops || !audio->openal.audio_event->Has_More_Loops()) {
        return false;
    }

    audio->openal.audio_event->Generate_Filename();

    if (audio->openal.audio_event->Get_Delay() > MSEC_PER_LOGICFRAME_REAL) {
        audio->openal.release_event = false;
        audio->openal.disable_loops = true;
        audio->openal.stopped = 1;
        AudioRequest *request = Allocate_Audio_Request(true);
        request->m_event.object = audio->openal.audio_event;
        request->m_isProcessed = true;
        Append_Audio_Request(request);

        return true;
    }

    if (audio->openal.playing_type == PAT_3DSAMPLE) {
        audio->openal.file_handle = Play_Sample3D(audio->openal.audio_event, audio);
    } else {
        audio->openal.file_handle = Play_Sample2D(audio->openal.audio_event, audio);
    }

    return audio->openal.file_handle != nullptr;
}

/**
 * Plays an audio event.
 */
void ALAudioManager::Play_Audio_Event(AudioEventRTS *event)
{
    const AudioEventInfo *aud_info = event->Get_Event_Info();

    if (aud_info == nullptr) {
        return;
    }

    uintptr_t kill_handle = event->Get_Handle_To_Kill();
    ALuint source_handle = 0;
    Utf8String name = event->Get_File_Name();
    PlayingAudio *pa = new PlayingAudio;
    Init_Playing_Audio(pa);
    pa->openal.stopped = false;

    switch (aud_info->Get_Event_Type()) {
        case EVENT_SPEECH:
            if (event->Should_Play_Locally()) {
                Stop_All_Speech();
            }
            [[fallthrough]];
            // Fallthrough
        case EVENT_MUSIC: {
            float volume = 1.0f;

            if (aud_info->Get_Event_Type() == EVENT_SPEECH) {
                volume = m_speechVolume;
            } else {
                volume = m_musicVolume;
            }

            volume *= event->Get_Volume();
            bool killed = false;

            if (kill_handle != 0) {
                for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
                    if (*it != nullptr && (*it)->openal.audio_event != nullptr
                        && (*it)->openal.audio_event->Get_Playing_Handle() == kill_handle) {
                        Release_Playing_Audio(*it);
                        m_streamList.erase(it);
                        killed = true;
                        break;
                    }
                }
            }

            if (kill_handle != 0 && !killed) {
                source_handle = 0;
            } else {
                alGenSources(1, &source_handle);
            }

            pa->openal.audio_event = event;
            pa->openal.source = source_handle;
            pa->openal.playing_type = PAT_STREAM;

            if (source_handle != 0) {
                if (aud_info->Get_Event_Type() == EVENT_SPEECH && event->Should_Play_Locally()) {
                    m_unkSpeech = true;
                }

                Play_Stream(event, source_handle);
                m_streamList.push_back(pa);
                pa = nullptr;
            }
            break;
        }
        case EVENT_SOUND:
            if (event->Is_Positional_Audio()) {
                bool killed = false;

                if (kill_handle != 0) {
                    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
                        if ((*it)->openal.audio_event->Get_Playing_Handle() == kill_handle) {
                            Release_Playing_Audio(*it);
                            m_positionalAudioList.erase(it);
                            killed = true;
                            break;
                        }
                    }
                }

                if (kill_handle != 0 && !killed) {
                    source_handle = 0;
                } else {
                    alGenSources(1, &source_handle);
                }

                pa->openal.audio_event = event;
                pa->openal.source = source_handle;
                pa->openal.playing_type = PAT_3DSAMPLE;
                pa->openal.file_handle = nullptr;
                m_positionalAudioList.push_back(pa);

                if (source_handle != 0) {
                    pa->openal.file_handle = Play_Sample3D(event, pa);
                    m_soundManager->Notify_Of_3D_Sample_Start();
                }

                if (pa->openal.file_handle == nullptr) {
                    m_positionalAudioList.pop_back();
                } else {
                    pa = nullptr;
                }
            } else {
                bool killed = false;

                if (kill_handle != 0) {
                    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
                        if ((*it)->openal.audio_event->Get_Playing_Handle() == kill_handle) {
                            Release_Playing_Audio(*it);
                            m_globalAudioList.erase(it);
                            killed = true;
                            break;
                        }
                    }
                }

                if (kill_handle != 0 && !killed) {
                    source_handle = 0;
                } else {
                    alGenSources(1, &source_handle);
                    captainslog_dbgassert(Check_AL_Error(), "Failed to generate source");
                }

                pa->openal.audio_event = event;
                pa->openal.source = source_handle;
                pa->openal.playing_type = PAT_2DSAMPLE;
                pa->openal.file_handle = nullptr;
                m_globalAudioList.push_back(pa);

                if (source_handle != 0) {
                    pa->openal.file_handle = Play_Sample2D(event, pa);
                    if (m_soundManager)
                        m_soundManager->Notify_Of_2D_Sample_Start();
                }

                if (pa->openal.file_handle == nullptr) {
                    m_globalAudioList.pop_back();
                } else {
                    pa = nullptr;
                }
            }
            break;
        default:
            break;
    }

    if (pa != nullptr) {
        Release_Playing_Audio(pa);
    }
}

/**
 * Pauses an audio event from a handle.
 */
void ALAudioManager::Pause_Audio_Event(uintptr_t handle)
{
    // Unimplemented
}

/**
 * Stops an audio event from a handle.
 */
void ALAudioManager::Stop_Audio_Event(uintptr_t handle)
{
    if (handle == 4 || handle == 5) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if ((*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
                if (handle == 5) {
                    m_fadingList.push_back(*it);
                } else {
                    Release_Playing_Audio(*it);
                }

                m_streamList.erase(it);
                break;
            }
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == handle) {
            (*it)->openal.disable_loops = true;
            Notify_Of_Audio_Completion((uintptr_t)(*it)->openal.source, 2);
            break;
        }
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == handle) {
            (*it)->openal.disable_loops = true;
            break;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->openal.audio_event->Get_Playing_Handle() == handle) {
            (*it)->openal.disable_loops = true;
            break;
        }
    }
}

/**
 * Process an audio request.
 */
void ALAudioManager::Process_Request(AudioRequest *request)
{
    switch (request->m_requestType) {
        case AR_PLAY:
            Play_Audio_Event(request->m_event.object);
            break;
        case AR_PAUSE:
            Pause_Audio_Event(request->m_event.handle);
            break;
        case AR_STOP:
            Stop_Audio_Event(request->m_event.handle);
            break;
        default:
            break;
    }
}

/**
 * Stops all speech playing audio.
 */
void ALAudioManager::Stop_All_Speech()
{
    auto it = m_streamList.begin();

    while (it != m_streamList.end()) {
        if (*it != nullptr) {
            if ((*it)->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_SPEECH) {
                Release_Playing_Audio(*it);
                it = m_streamList.erase(it);
            } else {
                ++it;
            }
        }
    }
}

/**
 * Adjusts the volume of a playing audio object at the Miles audio level.
 */
void ALAudioManager::Adjust_Playing_Volume(PlayingAudio *audio)
{
    float adjusted_vol = audio->openal.audio_event->Get_Volume() * audio->openal.audio_event->Get_Volume_Shift();
    switch (audio->openal.playing_type) {
        case PAT_2DSAMPLE: {
            alSourcef(audio->openal.source, AL_GAIN, adjusted_vol * m_soundVolume);
        } break;
        case PAT_3DSAMPLE:
            alSourcef(audio->openal.source, AL_GAIN, adjusted_vol * m_3dSoundVolume);
            break;
        case PAT_STREAM: {
            float vol;

            if (audio->openal.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
                vol = adjusted_vol * m_musicVolume;
            } else {
                vol = adjusted_vol * m_speechVolume;
            }

            alSourcef(audio->openal.source, AL_GAIN, vol * m_3dSoundVolume);
        } break;
        default:
            break;
    }
}

/**
 * Gets the effective volume of the event.s
 */
float ALAudioManager::Get_Effective_Volume(AudioEventRTS *event) const
{
    float vol = event->Get_Volume() * event->Get_Volume_Shift() * 1.0f;

    // Handle easy cases for none positional stuff.
    if (event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
        return vol * m_musicVolume;
    } else if (event->Get_Event_Info()->Get_Event_Type() == EVENT_SPEECH) {
        return vol * m_speechVolume;
    } else if (!event->Is_Positional_Audio()) {
        return vol * m_soundVolume;
    }

    // Coord3D being used as Vector3 essentially here.
    Coord3D *event_pos = event->Get_Current_Pos();
    Coord3D difference = m_listenerPosition;
    vol *= m_3dSoundVolume;

    if (event_pos == nullptr) {
        return vol;
    }

    // Get a vector representing the difference between listerner and sound.
    difference.Sub(event_pos);
    float min_dist;
    float max_dist;

    if (event->Get_Event_Info()->Get_Visibility() & VISIBILITY_GLOBAL) {
        min_dist = Get_Audio_Settings()->Global_Min_Range();
        max_dist = Get_Audio_Settings()->Global_Max_Range();
    } else {
        min_dist = event->Get_Event_Info()->Min_Range();
        max_dist = event->Get_Event_Info()->Max_Range();
    }

    // Calculate distance from our vector between source and listener.
    float dist = difference.Length();

    // Based on distance and range limits, calculate volume.
    if (dist > min_dist) {
        vol *= 1.0f / (float)(dist / min_dist);
    }

    if (dist >= max_dist) {
        vol = 0.0f;
    }

    return vol;
}

/**
 * Utility method to get the OpenAL format.
 */
ALenum ALAudioManager::Get_AL_Format(uint8_t channels, uint8_t bits_per_sample)
{
    if (channels == 1 && bits_per_sample == 8)
        return AL_FORMAT_MONO8;
    if (channels == 1 && bits_per_sample == 16)
        return AL_FORMAT_MONO16;
    if (channels == 1 && bits_per_sample == 32)
        return AL_FORMAT_MONO_FLOAT32;
    if (channels == 2 && bits_per_sample == 8)
        return AL_FORMAT_STEREO8;
    if (channels == 2 && bits_per_sample == 16)
        return AL_FORMAT_STEREO16;
    if (channels == 2 && bits_per_sample == 32)
        return AL_FORMAT_STEREO_FLOAT32;

    captainslog_warn("Unknown OpenAL format: %i channels, %i bits per sample", channels, bits_per_sample);
    return AL_FORMAT_MONO8;
}

/**
 * Gets the position the event is supposed to play at currently.
 */
Coord3D *ALAudioManager::Get_Current_Position_From_Event(AudioEventRTS *event)
{
    if (!event->Is_Positional_Audio()) {
        return nullptr;
    }

    return event->Get_Current_Pos();
}

/**
 * Helper function to clear a PlayingAudio object.
 */
void ALAudioManager::Init_Playing_Audio(PlayingAudio *audio)
{
    if (audio != nullptr) {
        audio->openal.source = 0;
        audio->openal.buffer = 0;
        audio->openal.playing_type = PAT_NONE;
        audio->openal.audio_event = nullptr;
        audio->openal.disable_loops = false;
        audio->openal.release_event = true;
        audio->openal.time_fading = 0;
    }
}

/**
 * Check if a request should be processed this frame.
 *
 * Inlined.
 */
bool ALAudioManager::Process_Request_This_Frame(AudioRequest *request)
{
    if (request->m_isAdding) {
        return request->m_event.object->Get_Delay() < MSEC_PER_LOGICFRAME_REAL;
    }

    return true;
}

/**
 * Reduced the delay until this request should be played by one frame.
 *
 * Inlined.
 */
void ALAudioManager::Adjust_Request(AudioRequest *request)
{
    if (request->m_isAdding) {
        request->m_event.object->Decrement_Delay(MSEC_PER_LOGICFRAME_REAL);
        request->m_isProcessed = true;
    }
}

/**
 * Checks the status of a request to determine if it should be processed.
 *
 * Inlined.
 */
bool ALAudioManager::Check_For_Sample(AudioRequest *request)
{
    if (request->m_isAdding) {
        return true;
    }

    // If the object doesn't have info, try and retrieve it.
    if (request->m_event.object->Get_Event_Info() == nullptr) {
        Get_Info_For_Audio_Event(request->m_event.object);
    }

    if (request->m_event.object->Get_Event_Info()->Get_Visibility() == VISIBILITY_WORLD) {
        return m_soundManager->Can_Play_Now(request->m_event.object);
    }

    return true;
}

/**
 * Check for OpenAL errors
 */
bool ALAudioManager::Check_AL_Error()
{
    ALenum error_code = alGetError();
    if (error_code != 0) {
        auto error_msg = alGetString(error_code);
        captainslog_error("OpenAL error: %s", error_msg);
        return false;
    }
    return true;
}

/**
 * Check for OpenAL errors
 */
bool ALAudioManager::Check_ALC_Error()
{
    ALCenum error_code = alcGetError(m_alcDevice);
    if (error_code != 0) {
        auto error_msg = alcGetString(m_alcDevice, error_code);
        captainslog_error("ALC error: %s", error_msg);
        return false;
    }
    return true;
}

/**
 * Fill list of all supported devices
 */
void ALAudioManager::Enumerate_Devices()
{
    const ALCchar *devices = NULL;
    if (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE) {
        devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
        if ((devices == nullptr || *devices == '\0') && alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE) {
            devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
        }
    }

    if (devices == nullptr) {
        captainslog_warn("Enumerating OpenAL devices is not supported");
        return;
    }

    const ALCchar *device = devices;
    const ALCchar *next = devices + 1;
    size_t len = 0;
    size_t idx = 0;
    while (device && *device != '\0' && next && *next != '\0' && idx < AL_MAX_PLAYBACK_DEVICES) {
        m_alDevicesList[idx++] = device;
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }

    m_alMaxDevicesIndex = idx;
}

} // namespace Thyme
