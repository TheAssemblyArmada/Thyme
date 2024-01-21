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
#include "filesystem.h"
#include "gamemath.h"
#include "gamesounds.h"
#include "globaldata.h"
#include "playingaudio.h"
#include "videoplayer.h"
#include "view.h"

using GameMath::Fast_To_Int_Truncate;

/**
 * 0x0077C700
 */
MilesAudioManager::MilesAudioManager() :
    m_milesMaxProviderIndex(0),
    m_milesCurrentProvider(-1),
    m_milesLastProvider(-1),
    m_speakerType(0),
    m_milesDigitalDriver(nullptr),
    m_miles3DPositionObject(0),
    m_milesDelayFilter(0),
#ifdef BUILD_WITH_FFMPEG
    m_audioFileCache(new Thyme::FFmpegAudioFileCache),
#else
    m_audioFileCache(new MilesAudioFileCache),
#endif
    m_binkPlayingAudio(nullptr),
    m_2dSampleCount(0),
    m_3dSampleCount(0),
    m_streamCount(0)
{
}

/**
 * 0x0077CAD0
 */
MilesAudioManager::~MilesAudioManager()
{
    Release_Bink_Handle();
    Close_Device();
    g_theAudio = nullptr;
    delete m_audioFileCache;
}

/**
 * Initialise the subsystem.
 *
 * 0x0077CCE0
 */
void MilesAudioManager::Init()
{
    AudioManager::Init();
    Open_Device();
    m_audioFileCache->Set_Max_Size(m_audioSettings->Get_Audio_Footprint());
    AIL_set_file_callbacks(Streaming_File_Open, Streaming_File_Close, Streaming_File_Seek, Streaming_File_Read);
}

/**
 * Reset the subsystem.
 *
 * 0x0077CD30
 */
void MilesAudioManager::Reset()
{
    // Worldbuilder is a debug build and has code to dump a list of audio assets here.
    AudioManager::Reset();
    Stop_All_Audio_Immediately();
    Remove_All_Audio_Requests();
    Remove_Level_Specific_Audio_Event_Infos();
}

/**
 * Update the subsystem.
 *
 * 0x0077CD60
 */
void MilesAudioManager::Update()
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
 *
 * 0x0077CDA0
 */
void MilesAudioManager::Stop_Audio(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_SOUND) {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr) {
                AIL_register_EOS_callback((*it)->miles.sample, nullptr);
                AIL_stop_sample((*it)->miles.sample);
                (*it)->miles.stopped = 1;
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
                AIL_register_3D_EOS_callback((*it)->miles.sample_3d, nullptr);
                AIL_stop_3D_sample((*it)->miles.sample_3d);
                (*it)->miles.stopped = 1;
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
                if ((affect & AUDIOAFFECT_MUSIC)
                    || ((*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                        && (affect & AUDIOAFFECT_SPEECH))) {
                    AIL_register_stream_callback((*it)->miles.stream, nullptr);
                    AIL_pause_stream((*it)->miles.stream, 1);
                    (*it)->miles.stopped = 1;
                }
            }
        }
    }
}

/**
 * Iterates playing audio lists and pauses the streams for the specified affects.
 *
 * 0x0077CEB0
 */
void MilesAudioManager::Pause_Audio(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_SOUND) {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr) {
                AIL_stop_sample((*it)->miles.sample);
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
                AIL_stop_3D_sample((*it)->miles.sample_3d);
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
                if ((affect & AUDIOAFFECT_MUSIC)
                    || ((*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                        && (affect & AUDIOAFFECT_SPEECH))) {
                    AIL_pause_stream((*it)->miles.stream, 1);
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
 *
 * 0x0077CFC0
 */
void MilesAudioManager::Resume_Audio(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_SOUND) {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr) {
                AIL_resume_sample((*it)->miles.sample);
            }
        }
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr) {
                AIL_resume_3D_sample((*it)->miles.sample_3d);
            }
        }
    }

    if (affect & (AUDIOAFFECT_MUSIC | AUDIOAFFECT_SPEECH)) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if (*it != nullptr) {
                if ((affect & AUDIOAFFECT_MUSIC)
                    || ((*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                        && (affect & AUDIOAFFECT_SPEECH))) {
                    AIL_pause_stream((*it)->miles.stream, 0);
                }
            }
        }
    }
}

/**
 * Kills an event immediately based on the provided handle.
 *
 * 0x0077D780
 */
void MilesAudioManager::Kill_Event_Immediately(uintptr_t event)
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
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == event) {
            Release_Playing_Audio(*it);
            m_positionalAudioList.erase(it);

            return;
        }
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == event) {
            Release_Playing_Audio(*it);
            m_globalAudioList.erase(it);

            return;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == event) {
            Release_Playing_Audio(*it);
            m_streamList.erase(it);

            return;
        }
    }
}

/**
 * Starts the next song playing.
 *
 * 0x0077DE80
 */
void MilesAudioManager::Next_Music_Track()
{
    Utf8String track_name;

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            track_name = (*it)->miles.audio_event->Get_Event_Name();
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
 *
 * 0x0077E020
 */
void MilesAudioManager::Prev_Music_Track()
{
    Utf8String track_name;

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            track_name = (*it)->miles.audio_event->Get_Event_Name();
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
 *
 * 0x0077E1C0
 */
bool MilesAudioManager::Is_Music_Playing()
{
    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            return true;
        }
    }

    return false;
}

/**
 * Checks if the named mustic track has finished playing a number of loops.
 *
 * 0x0077E200
 */
bool MilesAudioManager::Has_Music_Track_Completed(const Utf8String &name, int loops)
{
    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            if ((*it)->miles.audio_event->Get_Event_Name() == name
                && (100000 - AIL_stream_loop_count((*it)->miles.stream)) >= loops) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Gets the name of the currently queued or playing music track.
 *
 * 0x0077E2D0
 */
Utf8String MilesAudioManager::Music_Track_Name()
{
    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end(); ++it) {
        if ((*it)->Request_Type() == AR_PLAY && (*it)->Is_Adding()
            && (*it)->Event_Object()->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            return (*it)->Event_Object()->Get_Event_Name();
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            return (*it)->miles.audio_event->Get_Event_Name();
        }
    }

    return Utf8String();
}

/**
 * Checks if a given event handle refers to something that is currently playing.
 *
 * 0x0077E5F0
 */
bool MilesAudioManager::Is_Currently_Playing(uintptr_t event)
{
    // Iterate the various lists until a matching handle is found.
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == event) {
            return true;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == event) {
            return true;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == event) {
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
 *
 * 0x0077E3B0
 */
void MilesAudioManager::Open_Device()
{
    if (!g_theWriteableGlobalData->m_audioOn) {
        return;
    }

    AIL_set_redist_directory("MSS\\");
    atexit(Exit_Shutdown); // Mac build doesn't set this.
    AIL_startup();
    m_speakerType = Translate_From_Speaker_Type(m_preferredSpeaker);
    bool quick_start = (bool)AIL_quick_startup(m_audioSettings->Use_Digital(),
        m_audioSettings->Use_Midi(),
        m_audioSettings->Output_Rate(),
        m_audioSettings->Output_Bits(),
        m_audioSettings->Output_Channels());
    AIL_quick_handles(&m_milesDigitalDriver, nullptr, nullptr);

    if (quick_start) {
        Build_Provider_List();
    } else {
        Set_On(false, AUDIOAFFECT_MUSIC | AUDIOAFFECT_SOUND | AUDIOAFFECT_3DSOUND | AUDIOAFFECT_SPEECH);
    }

    Select_Provider(Get_Provider_Index(m_preferredProvider));
    Refresh_Cached_Variables();

    if (Provider_Is_Valid()) {
        Init_Delay_Filter();
    }
}

/**
 * Closes the audio device.
 *
 * 0x0077E5D0
 */
void MilesAudioManager::Close_Device()
{
    Free_All_Miles_Handles();
    Unselect_Provider();
    AIL_shutdown();
}

/**
 * Handles audio completion.
 *
 * 0x0077E6C0
 */
void MilesAudioManager::Notify_Of_Audio_Completion(uintptr_t handle, unsigned type)
{
    PlayingAudio *playing = Find_Playing_Audio_From(handle, type);

    if (playing == nullptr) {
        return;
    }

    if (m_unkSpeech && playing->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_SPEECH) {
        m_unkSpeech = false;
    }

    if (playing->miles.audio_event->Get_Event_Info()->Get_Control() & CONTROL_LOOP) {
        if (playing->miles.audio_event->Get_Next_Play_Portion() == 0) {
            playing->miles.audio_event->Set_Next_Play_Portion(1);
        }
    }

    if (!(playing->miles.audio_event->Get_Event_Info()->Get_Control() & CONTROL_LOOP)
        || playing->miles.audio_event->Get_Next_Play_Portion() != 1
        || !(playing->miles.audio_event->Decrease_Loop_Count(), Start_Next_Loop(playing))) {
        playing->miles.audio_event->Advance_Next_Play_Portion();

        if (playing->miles.audio_event->Get_Next_Play_Portion() != 3) {
            switch (playing->miles.playing_type) {
                case PAT_2DSAMPLE:
                    Close_File(playing->miles.file_handle);
                    playing->miles.file_handle = Play_Sample(playing->miles.audio_event, playing->miles.sample);

                    if (playing->miles.file_handle != nullptr) {
                        return;
                    }
                    break;
                case PAT_3DSAMPLE:
                    Close_File(playing->miles.file_handle);
                    playing->miles.file_handle = Play_Sample3D(playing->miles.audio_event, playing->miles.sample_3d);

                    if (playing->miles.file_handle != nullptr) {
                        return;
                    }
                    break;
                default:
                    break;
            }
        }

        if (playing->miles.playing_type == PAT_STREAM
            && playing->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
            Play_Stream(playing->miles.audio_event, playing->miles.stream);
        } else {
            playing->miles.stopped = 1;
        }
    }
}

/**
 * Gets the name of a provider from its index.
 *
 * 0x0077E900
 */
Utf8String MilesAudioManager::Get_Provider_Name(unsigned provider) const
{
    if (Is_On(AUDIOAFFECT_3DSOUND) && provider < (unsigned)m_milesMaxProviderIndex) {
        return m_milesProviderList[provider].name;
    }

    return Utf8String::s_emptyString;
}

/**
 * Gets the index of a provider from its name.
 *
 * 0x0077E970
 */
unsigned MilesAudioManager::Get_Provider_Index(Utf8String name)
{
    for (int i = 0; i < m_milesMaxProviderIndex; ++i) {
        if (name == m_milesProviderList[i].name) {
            return i;
        }
    }

    return UINT_MAX;
}

/**
 * Sets the provided provider as the current provider for the audio engine.
 *
 * 0x0077EA60
 */
void MilesAudioManager::Select_Provider(unsigned provider)
{
    if (!Is_On(AUDIOAFFECT_3DSOUND) || provider == m_milesCurrentProvider) {
        return;
    }

    if (Provider_Is_Valid()) {
        Free_All_Miles_Handles();
        Unselect_Provider();
    }

    bool try_dolby = false;
    // Windows Original has a DirectSound check here to decide if Dolby provider is available.

    unsigned index;

    if (try_dolby) {
        index = Get_Provider_Index("Dolby Surround");
    } else {
        index = Get_Provider_Index("Miles Fast 2D Positional Audio");
    }

    int32_t ail_provider = AIL_open_3D_provider(m_milesProviderList[index].provider);

    if (ail_provider == 0) {
        // If we didn't get the provider we wanted, fallback to basic 2D audio.
        m_milesCurrentProvider = -1;
        index = Get_Provider_Index("Miles Fast 2D Positional Audio");
        ail_provider = AIL_open_3D_provider(m_milesProviderList[index].provider);
    }

    if (ail_provider != 0) {
        m_milesCurrentProvider = index;
        Init_Sample_Pools();
        Create_Listener();
        Set_Speaker_Type(m_speakerType);

        if (g_theVideoPlayer != nullptr) {
            g_theVideoPlayer->Notify_Player_Of_New_Provider(true);
        }
    }
}

/**
 * Invalidates the current provider.
 *
 * 0x0077ECB0
 */
void MilesAudioManager::Unselect_Provider()
{
    if (!Is_On(AUDIOAFFECT_3DSOUND) || m_milesCurrentProvider >= m_milesMaxProviderIndex) {
        return;
    }

    if (g_theVideoPlayer != nullptr) {
        g_theVideoPlayer->Notify_Player_Of_New_Provider(false);
    }

    AIL_close_3D_listener(m_miles3DPositionObject);
    m_miles3DPositionObject = nullptr;
    AIL_close_3D_provider(m_milesProviderList[m_milesCurrentProvider].provider);
    m_milesLastProvider = m_milesCurrentProvider;
    m_milesCurrentProvider = -1;
}

/**
 * Sets the speaker type.
 *
 * 0x0077ED40
 */
void MilesAudioManager::Set_Speaker_Type(unsigned type)
{
    if (Provider_Is_Valid()) {
        AIL_set_3D_speaker_type(m_milesProviderList[m_milesCurrentProvider].provider, type);
        m_speakerType = type;
    }
}

/**
 * Gets the speaker type.
 *
 * 0x0077ED80
 */
unsigned MilesAudioManager::Get_Speaker_Type()
{
    if (!Provider_Is_Valid()) {
        return 0;
    }

    return m_speakerType;
}

/**
 * Checks if the number of instances of an event exceeds the limits for that event.
 *
 * 0x0077EDD0
 */
bool MilesAudioManager::Does_Violate_Limit(AudioEventRTS *event) const
{
    int limit;

    if ((limit = event->Get_Event_Info()->Get_Limit()) == 0) {
        return false;
    }

    int playing_matches = 0;
    int request_matches = 0;

    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (event->Get_Event_Name() == (*it)->miles.audio_event->Get_Event_Name()) {
                if (playing_matches == 0) {
                    event->Set_Handle_To_Kill((*it)->miles.audio_event->Get_Playing_Handle());
                }

                ++playing_matches;
            }
        }
    } else {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (event->Get_Event_Name() == (*it)->miles.audio_event->Get_Event_Name()) {
                if (playing_matches == 0) {
                    event->Set_Handle_To_Kill((*it)->miles.audio_event->Get_Playing_Handle());
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
 *
 * 0x0077F2A0
 */
bool MilesAudioManager::Is_Playing_Lower_Priority(AudioEventRTS *event) const
{
    int priority;

    if ((priority = event->Get_Event_Info()->Get_Priority()) == 0) {
        return false;
    }

    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if ((*it)->miles.audio_event->Get_Event_Info()->Get_Priority() < priority) {
                return true;
            }
        }

        return false;
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if ((*it)->miles.audio_event->Get_Event_Info()->Get_Priority() < priority) {
            return true;
        }
    }

    return false;
}

/**
 * Checks if an event is playing already.
 *
 * 0x0077F020
 */
bool MilesAudioManager::Is_Playing_Already(AudioEventRTS *event) const
{
    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if ((*it)->miles.audio_event->Get_Event_Name() == event->Get_Event_Name()) {
                return true;
            }
        }

        return false;
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if ((*it)->miles.audio_event->Get_Event_Name() == event->Get_Event_Name()) {
            return true;
        }
    }

    return false;
}

/**
 * Checks if an object is playing a voice.
 *
 * 0x0077F130
 */
bool MilesAudioManager::Is_Object_Playing_Voice(unsigned obj) const
{
    if (obj == 0) {
        return false;
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if ((*it)->miles.audio_event->Get_Object_ID() == obj
            && ((*it)->miles.audio_event->Get_Event_Info()->Get_Visibility() & VISIBILITY_VOICE)) {
            return true;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if ((*it)->miles.audio_event->Get_Object_ID() == obj
            && ((*it)->miles.audio_event->Get_Event_Info()->Get_Visibility() & VISIBILITY_VOICE)) {
            return true;
        }
    }

    return false;
}

/**
 * Changes the volume of playing audio.
 *
 * 0x0077F400
 */
void MilesAudioManager::Adjust_Volume_Of_Playing_Audio(Utf8String name, float adjust)
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Name() == name) {
            (*it)->miles.audio_event->Set_Volume(adjust);
            float pan;
            AIL_sample_volume_pan((*it)->miles.sample, nullptr, &pan);
            AIL_set_sample_volume_pan((*it)->miles.sample,
                (*it)->miles.audio_event->Get_Volume() * (*it)->miles.audio_event->Get_Volume_Shift(),
                pan);
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Name() == name) {
            (*it)->miles.audio_event->Set_Volume(adjust);
            AIL_set_3D_sample_volume((*it)->miles.sample_3d,
                (*it)->miles.audio_event->Get_Volume() * (*it)->miles.audio_event->Get_Volume_Shift());
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Name() == name) {
            (*it)->miles.audio_event->Set_Volume(adjust);
            float pan;
            AIL_stream_volume_pan((*it)->miles.stream, nullptr, &pan);
            AIL_set_stream_volume_pan((*it)->miles.stream,
                (*it)->miles.audio_event->Get_Volume() * (*it)->miles.audio_event->Get_Volume_Shift(),
                pan);
        }
    }
}

/**
 * Removes playing audio.
 *
 * 0x0077F6C0
 */
void MilesAudioManager::Remove_Playing_Audio(Utf8String name)
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Name() == name) {
            Release_Playing_Audio(*it);
            it = m_globalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end();) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Name() == name) {
            Release_Playing_Audio(*it);
            it = m_positionalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end();) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Event_Name() == name) {
            Release_Playing_Audio(*it);
            it = m_streamList.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Removes all audio with a volume of 0.
 *
 * 0x0077F8F0
 */
void MilesAudioManager::Remove_All_Disabled_Audio()
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Volume() == 0.0f) {
            Release_Playing_Audio(*it);
            it = m_globalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end();) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Volume() == 0.0f) {
            Release_Playing_Audio(*it);
            it = m_positionalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end();) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Volume() == 0.0f) {
            Release_Playing_Audio(*it);
            it = m_streamList.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Checks if any streams have properties suggesting they are position sensitive.
 *
 * 0x0077FD80
 */
bool MilesAudioManager::Has_3D_Sensitive_Streams_Playing()
{
    if (m_streamList.empty()) {
        return false;
    }

    for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
        if (*it != nullptr
            && ((*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() != EVENT_MUSIC
                || !(*it)->miles.audio_event->Get_Event_Name().Starts_With("Game_"))) {
            return true;
        }
    }

    return false;
}

/**
 * Gets an audio handle for use by the video player.
 *
 * 0x00780820
 */
BinkHandle MilesAudioManager::Get_Bink_Handle()
{
    // If we don't already have a playing audio for bink, make one.
    if (m_binkPlayingAudio == nullptr) {
        PlayingAudio *pap = new PlayingAudio;
        Init_Playing_Audio(pap);
        pap->miles.stopped = false;
        pap->miles.audio_event = new AudioEventRTS("BinkHandle");
        Get_Info_For_Audio_Event(pap->miles.audio_event);

        if (!m_sampleHandleList.empty()) {
            pap->miles.sample = m_sampleHandleList.front();
            m_sampleHandleList.pop_front();
        }

        pap->miles.playing_type = PAT_2DSAMPLE;

        if (pap->miles.sample == 0) {
            Release_Playing_Audio(pap);
            return nullptr;
        }

        m_binkPlayingAudio = pap;
    }

    AILLPDIRECTSOUND info;
    AIL_get_DirectSound_info(m_binkPlayingAudio->miles.sample, &info, nullptr);

    return static_cast<BinkHandle>(info);
}

/**
 * Releases the audio handle for use by the video player.
 *
 * 0x007809C0
 */
void MilesAudioManager::Release_Bink_Handle()
{
    if (m_binkPlayingAudio != nullptr) {
        Release_Playing_Audio(m_binkPlayingAudio);
        m_binkPlayingAudio = nullptr;
    }
}

/**
 * Forces an audio event to be played.
 *
 * 0x007809E0
 */
void MilesAudioManager::friend_Force_Play_Audio_Event(AudioEventRTS *event)
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

    AudioEventRTS tmp_event(*event);
    tmp_event.Generate_Filename();
    tmp_event.Generate_Play_Info();

    for (auto it = m_eventVolumeList.begin(); it != m_eventVolumeList.end(); ++it) {
        if (it->first == tmp_event.Get_Event_Name()) {
            tmp_event.Set_Volume(it->second);
            break;
        }
    }

    HAUDIO quick_aud = AIL_quick_load_and_play(tmp_event.Get_File_Name().Str(), 1, 0);
    AIL_quick_set_volume(quick_aud, tmp_event.Get_Volume() * Get_Volume(AUDIOAFFECT_SPEECH), 0.5f);
    m_forcePlayedAudioList.push_back(quick_aud);
}

/**
 * Processes the request list for audio waiting to be played.
 *
 * 0x0077FA00
 */
void MilesAudioManager::Process_Request_List()
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
 *
 * 0x0077FF30
 */
void MilesAudioManager::Set_Hardware_Accelerated(bool accelerated)
{
    bool changed = m_hardwareAccelerated != accelerated;
    AudioManager::Set_Hardware_Accelerated(accelerated);

    // Handle a change of state.
    if (changed) {
        // If we are accelerated, try and choose best/available driver.
        // These calls call g_theAudio global rather than calling on this instance in original.
        if (m_hardwareAccelerated) {
            for (int i = 0; i < DRIVER_SOFTWARE; ++i) {
                unsigned index = Get_Provider_Index(Get_Audio_Settings()->Get_Preferred_Driver(i));
                Select_Provider(index);

                if (index == Get_Selected_Provider()) {
                    break;
                }
            }
        } else {
            Select_Provider(Get_Provider_Index(Get_Audio_Settings()->Get_Preferred_Driver(DRIVER_SOFTWARE)));
        }
    }
}

/**
 * Sets if the audio engine should use surround sound if available.
 *
 * 0x00780020
 */
void MilesAudioManager::Set_Speaker_Surround(bool surround)
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
 *
 * 0x0077C970
 */
void MilesAudioManager::Set_Preferred_3D_Provider(Utf8String provider)
{
    m_preferredProvider = provider;
}

/**
 * Sets the preferred speaker arrangement.
 *
 * 0x0077CA10
 */
void MilesAudioManager::Set_Preferred_Speaker(Utf8String speaker)
{
    m_preferredSpeaker = speaker;
}

/**
 * Gets the files length in milliseconds.
 *
 * 0x00780090
 */
float MilesAudioManager::Get_File_Length_MS(Utf8String file_name)
{
    if (file_name.Is_Empty()) {
        return 0.0f;
    }

    HSTREAM handle = AIL_open_stream(m_milesDigitalDriver, file_name.Str(), 0);

    if (handle == nullptr) {
        return 0.0f;
    }

    int32_t length;
    AIL_stream_ms_position(handle, &length, nullptr);
    AIL_close_stream(handle);

    return length;
}

/**
 * Close all samples using a given file handle.
 *
 * 0x00780190
 */
void MilesAudioManager::Close_Any_Sample_Using_File(const AudioDataHandle handle)
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if ((*it)->miles.file_handle == handle) {
            Release_Playing_Audio(*it);
            it = m_globalAudioList.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end();) {
        if ((*it)->miles.file_handle == handle) {
            Release_Playing_Audio(*it);
            it = m_positionalAudioList.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * Sets the position of the listener for 3D sounds.
 *
 * 0x00780230
 */
void MilesAudioManager::Set_Device_Listener_Position()
{
    if (m_miles3DPositionObject != nullptr) {
        AIL_set_3D_orientation(
            m_miles3DPositionObject, m_listenerFacing.x, m_listenerFacing.y, m_listenerFacing.z, 0.0f, 0.0f, -1.0f);
        AIL_set_3D_position(m_miles3DPositionObject, m_listenerPosition.x, m_listenerPosition.y, m_listenerPosition.z);
    }
}

/**
 * Finds a playing audio that uses a given handle and is a given type.
 *
 * 0x0077E860
 */
PlayingAudio *MilesAudioManager::Find_Playing_Audio_From(uintptr_t handle, unsigned type)
{
    switch (type) {
        case PAT_2DSAMPLE:
            for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
                if (*it != nullptr && (*it)->miles.sample == (HSAMPLE)handle) {
                    return *it;
                }
            }

            break;
        case PAT_3DSAMPLE:
            for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
                if (*it != nullptr && (*it)->miles.sample_3d == (H3DSAMPLE)handle) {
                    return *it;
                }
            }

            break;
        case PAT_STREAM:
            for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
                if (*it != nullptr && (*it)->miles.stream == (HSTREAM)handle) {
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
 *
 * 0x0077FB10
 */
void MilesAudioManager::Process_Playing_List()
{
    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end();) {
        if (*it != nullptr) {
            if ((*it)->miles.stopped == 1) {
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
            if ((*it)->miles.stopped == 1) {
                Release_Playing_Audio(*it);
                it = m_positionalAudioList.erase(it);
            } else {
                if (m_volumeSet) {
                    Adjust_Playing_Volume(*it);
                }

                Coord3D *current_pos = (*it)->miles.audio_event->Get_Current_Pos();

                if (current_pos != nullptr) {
                    if ((*it)->miles.audio_event->Get_Event_Type() == EVENT_UNKVAL3) {
                        Stop_Audio_Event((*it)->miles.audio_event->Get_Playing_Handle());
                        ++it;
                    } else {
                        float sample_vol = Get_Effective_Volume((*it)->miles.audio_event);
                        // Is this conditional check incorrect? Original does this but makes no sense.
                        // BUGFIX TODO should be m3dSoundVolume for the result as well?
                        sample_vol /= m_3dSoundVolume > 0.0f ? m_soundVolume : 1.0f;

                        if (sample_vol < m_audioSettings->Get_Min_Sample_Vol()
                            && !(((*it)->miles.audio_event->Get_Event_Info()->Get_Visibility() & VISIBILITY_GLOBAL)
                                || ((*it)->miles.audio_event->Get_Event_Info()->Get_Priority() == 4))) {
                            AIL_register_3D_EOS_callback((*it)->miles.sample_3d, nullptr);
                            Release_Playing_Audio(*it);
                            it = m_positionalAudioList.erase(it);
                        } else {
                            AIL_set_3D_position((*it)->miles.sample_3d, current_pos->x, current_pos->y, current_pos->z);
                            ++it;
                        }
                    }
                } else {
                    AIL_register_3D_EOS_callback((*it)->miles.sample_3d, nullptr);
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
            if ((*it)->miles.stopped == 1) {
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
 *
 * 0x0077FDE0
 */
void MilesAudioManager::Process_Fading_List()
{
    for (auto it = m_fadingList.begin(); it != m_fadingList.end();) {
        if (*it == nullptr) {
            // TODO BUGFIX This appears to be an infinite loop if any entry is null.
            continue;
        }

        if ((*it)->miles.time_fading < Get_Audio_Settings()->Get_Time_To_Fade()) {
            ++(*it)->miles.time_fading;
            float adjustment = (float)(*it)->miles.time_fading / (float)Get_Audio_Settings()->Get_Time_To_Fade();
            float effective_vol = (float)(1.0f - adjustment) * Get_Effective_Volume((*it)->miles.audio_event);

            switch ((*it)->miles.playing_type) {
                case PAT_2DSAMPLE:
                    AIL_set_sample_volume_pan((*it)->miles.sample, effective_vol, 0.5f);
                    break;
                case PAT_3DSAMPLE:
                    AIL_set_3D_sample_volume((*it)->miles.sample_3d, effective_vol);
                    break;
                case PAT_STREAM:
                    AIL_set_stream_volume_pan((*it)->miles.stream, effective_vol, 0.5f);
                    break;
                default:
                    break;
            }

            ++it;
        } else {
            (*it)->miles.stopped = 1;
            (*it)->miles.disable_loops = true;
            Release_Playing_Audio(*it);
            it = m_fadingList.erase(it);
        }
    }
}

/**
 * Processes the stopped audio lists.
 *
 * 0x0077FEE0
 */
void MilesAudioManager::Process_Stopped_List()
{
    for (auto it = m_stoppedList.begin(); it != m_stoppedList.end(); it = m_stoppedList.erase(it)) {
        if (*it != nullptr) {
            Release_Playing_Audio(*it);
        }
    }
}

/**
 * Stops the playing audio sample.
 *
 * 0x0077D9B0
 */
void MilesAudioManager::Release_Playing_Audio(PlayingAudio *audio)
{
    if (audio->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_SOUND) {

        switch (audio->miles.playing_type) {
            case PAT_2DSAMPLE:
                if (audio->miles.sample != nullptr) {
                    m_soundManager->Notify_Of_2D_Sample_Completion();
                }
                break;
            case PAT_3DSAMPLE:
                if (audio->miles.sample_3d != nullptr) {
                    m_soundManager->Notify_Of_3D_Sample_Completion();
                }
                break;
            default:
                break;
        }
    }

    Release_Miles_Handles(audio);
    Close_File(audio->miles.file_handle);

    if (audio->miles.release_event) {
        Release_Audio_Event_RTS(audio->miles.audio_event);
    }

    delete audio;
}

/**
 * Stops the playing audio sample.
 *
 * 0x0077D8E0
 */
void MilesAudioManager::Release_Miles_Handles(PlayingAudio *audio)
{
    switch (audio->miles.playing_type) {
        case PAT_2DSAMPLE:
            if (audio->miles.sample != nullptr) {
                AIL_register_EOS_callback(audio->miles.sample, nullptr);
                AIL_stop_sample(audio->miles.sample);
                m_sampleHandleList.push_back(audio->miles.sample);
            }

            break;
        case PAT_3DSAMPLE:
            if (audio->miles.sample_3d != nullptr) {
                AIL_register_3D_EOS_callback(audio->miles.sample_3d, nullptr);
                AIL_stop_3D_sample(audio->miles.sample_3d);
                m_3dSampleHandleList.push_back(audio->miles.sample_3d);
            }

            break;
        case PAT_STREAM:
            if (audio->miles.stream != nullptr) {
                AIL_register_stream_callback(audio->miles.stream, nullptr);
                AIL_close_stream(audio->miles.stream);
            }

            break;
        default:
            break;
    }

    audio->miles.playing_type = PAT_NONE;
}

/**
 * Frees memory holding audio samples.
 *
 * 0x0077DB80
 */
void MilesAudioManager::Free_All_Miles_Handles()
{
    Stop_All_Audio_Immediately();

    for (auto it = m_sampleHandleList.begin(); it != m_sampleHandleList.end(); it = m_sampleHandleList.erase(it)) {
        AIL_release_sample_handle(*it);
    }

    m_2dSampleCount = 0;

    for (auto it = m_3dSampleHandleList.begin(); it != m_3dSampleHandleList.end(); it = m_3dSampleHandleList.erase(it)) {
        AIL_release_3D_sample_handle(*it);
    }

    m_3dSampleCount = 0;
    m_streamCount = 0;
}

/**
 * Releases all playing audio samples.
 *
 * 0x0077DA30
 */
void MilesAudioManager::Stop_All_Audio_Immediately()
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

    for (auto it = m_forcePlayedAudioList.begin(); it != m_forcePlayedAudioList.end(); ++it) {
        if (*it != nullptr) {
            AIL_quick_unload(*it);
        }
    }

    m_forcePlayedAudioList.clear();
}

/**
 * Loads a miles stream for an audio event.
 *
 * Inlined
 */
void MilesAudioManager::Play_Stream(AudioEventRTS *event, HSAMPLE stream)
{
    if (event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
        AIL_set_stream_loop_count(stream, 1000000);
    }

    AIL_register_stream_callback(stream, Set_Stream_Complete);
    AIL_start_stream(stream);
    // AudioEventRTS::getAudioEventInfo(v13); // Unclear why original has this call here.
}

/**
 * Loads a miles 3D sample for an audio event and returns a pointer to the file data.
 *
 * 0x00780520
 */
AudioDataHandle MilesAudioManager::Play_Sample3D(AudioEventRTS *event, H3DSAMPLE sample)
{
    Coord3D *pos = Get_Current_Position_From_Event(event);

    if (pos == nullptr) {
        return nullptr;
    }

    AudioDataHandle handle = Open_File(event);

    if (handle != nullptr) {
        AIL_set_3D_sample_file(sample, handle);
        AIL_register_3D_EOS_callback(sample, Set_3DSample_Complete);

        if (event->Get_Event_Info()->Get_Visibility() & VISIBILITY_GLOBAL) {
            AIL_set_3D_sample_distances(sample, m_audioSettings->Global_Min_Range(), m_audioSettings->Global_Max_Range());
        } else {
            AIL_set_3D_sample_distances(sample, event->Get_Event_Info()->Min_Range(), event->Get_Event_Info()->Max_Range());
        }

        AIL_set_3D_position(sample, pos->x, pos->y, pos->z);
        Init_Filters3D(sample, event, pos);
        AIL_start_3D_sample(sample);
    }

    return handle;
}

/**
 * Loads a miles sample for an audio event and returns a pointer to the file data.
 *
 * Inlined
 */
AudioDataHandle MilesAudioManager::Play_Sample(AudioEventRTS *event, HSAMPLE sample)
{
    AIL_init_sample(sample);
    AIL_register_EOS_callback(sample, Set_Sample_Complete);
    Init_Filters(sample, event);
    AudioDataHandle handle = Open_File(event);

    if (handle != nullptr) {
        AIL_set_sample_file(sample, handle, 0);
        AIL_start_sample(sample);
    }

    return handle;
}

/**
 * Attempts to start another loop of a playing audio file, return indicates success.
 *
 * 0x00780400
 */
bool MilesAudioManager::Start_Next_Loop(PlayingAudio *audio)
{
    Close_File(audio->miles.file_handle);
    audio->miles.file_handle = nullptr;

    if (audio->miles.disable_loops || !audio->miles.audio_event->Has_More_Loops()) {
        return false;
    }

    audio->miles.audio_event->Generate_Filename();

    if (audio->miles.audio_event->Get_Delay() > MSEC_PER_LOGICFRAME_REAL) {
        audio->miles.release_event = false;
        audio->miles.disable_loops = true;
        audio->miles.stopped = 1;
        AudioRequest *request = Allocate_Audio_Request(true);
        request->m_event.object = audio->miles.audio_event;
        request->m_isProcessed = true;
        Append_Audio_Request(request);

        return true;
    }

    if (audio->miles.playing_type == PAT_3DSAMPLE) {
        audio->miles.file_handle = Play_Sample3D(audio->miles.audio_event, audio->miles.sample_3d);
    } else {
        audio->miles.file_handle = Play_Sample(audio->miles.audio_event, audio->miles.sample);
    }

    return audio->miles.file_handle != nullptr;
}

/**
 * Initialises some Miles audio filters.
 *
 * 0x0077DD80
 */
void MilesAudioManager::Init_Filters(HSAMPLE sample, AudioEventRTS *event)
{
    AIL_set_sample_volume_pan(sample, event->Get_Volume_Shift() * event->Get_Volume() * m_soundVolume, 0.5f);

    captainslog_dbgassert(
        event->Get_Pitch_Shift() != 0.0f, "Invalid Pitch shift in sound: '%s'", event->Get_Event_Name().Str());

    if (event->Get_Pitch_Shift() != 0.0f) {
        AIL_set_sample_playback_rate(
            sample, Fast_To_Int_Truncate(AIL_sample_playback_rate(sample) * event->Get_Pitch_Shift()));
    }

    if (event->Get_Delay() > 0.0f) {
        float delay = event->Get_Delay();
        AIL_set_sample_processor(sample, 1, m_milesDelayFilter);
        AIL_set_filter_sample_preference(sample, "Mono Delay Time", &delay);
        // In macOS release, these next two are not set.
        delay = 0.0f;
        AIL_set_filter_sample_preference(sample, "Mono Delay", &delay);
        AIL_set_filter_sample_preference(sample, "Mono Delay Mix", &delay);
    }
}

/**
 * Initialises some Miles 3d audio filters.
 *
 * Inlined
 */
void MilesAudioManager::Init_Filters3D(H3DSAMPLE sample, AudioEventRTS *event, Coord3D *coord)
{
    AIL_set_3D_sample_volume(sample, event->Get_Volume() * event->Get_Volume_Shift() * m_3dSoundVolume);

    captainslog_dbgassert(
        event->Get_Pitch_Shift() != 0.0f, "Invalid Pitch shift in sound: '%s'", event->Get_Event_Name().Str());

    if (event->Get_Pitch_Shift() != 0.0f) {
        AIL_set_3D_sample_playback_rate(
            sample, Fast_To_Int_Truncate(AIL_3D_sample_playback_rate(sample) * event->Get_Pitch_Shift()));
    }

    if (event->Get_Event_Info()->Low_Pass_Cutoff() > 0.0f && !Is_On_Screen(coord)) {
        AIL_set_3D_sample_occlusion(sample, 1.0f - event->Get_Event_Info()->Low_Pass_Cutoff());
    }
}

/**
 * Initialises pools of sample handles for Miles.
 *
 * 0x007806F0
 */
void MilesAudioManager::Init_Sample_Pools()
{
    if (!Is_On(AUDIOAFFECT_3DSOUND) || !Provider_Is_Valid()) {
        return;
    }

    for (int i = 0; i < Get_Audio_Settings()->Get_2D_Sample_Count(); ++i) {
        HSAMPLE handle = AIL_allocate_sample_handle(m_milesDigitalDriver);
        captainslog_dbgassert(handle != nullptr, "Couldn't get %d 2D samples", i + 1);

        if (handle != nullptr) {
            AIL_init_sample(handle);
            AIL_set_sample_user_data(handle, 0, i + 1);
            m_sampleHandleList.push_back(handle);
            ++m_2dSampleCount;
        }
    }

    for (int i = 0; i < Get_Audio_Settings()->Get_3D_Sample_Count(); ++i) {
        H3DSAMPLE handle = AIL_allocate_3D_sample_handle(m_milesProviderList[m_milesCurrentProvider].provider);
        captainslog_dbgassert(handle != nullptr, "Couldn't get %d 3D samples", i + 1);

        if (handle != nullptr) {
            AIL_set_3D_user_data(handle, 0, i + 1);
            m_3dSampleHandleList.push_back(handle);
            ++m_3dSampleCount;
        }
    }

    m_streamCount = Get_Audio_Settings()->Get_Stream_Count();
}

/**
 * Plays an audio event.
 *
 * 0x0077D080
 */
void MilesAudioManager::Play_Audio_Event(AudioEventRTS *event)
{
    const AudioEventInfo *aud_info = event->Get_Event_Info();

    if (aud_info == nullptr) {
        return;
    }

    uintptr_t kill_handle = event->Get_Handle_To_Kill();
    HSTREAM stream_handle = 0;
    Utf8String name = event->Get_File_Name();
    PlayingAudio *pa = new PlayingAudio;
    Init_Playing_Audio(pa);
    pa->miles.stopped = false;

    switch (aud_info->Get_Event_Type()) {
        case EVENT_SPEECH:
            if (event->Should_Play_Locally()) {
                Stop_All_Speech();
            }
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
                    if (*it != nullptr && (*it)->miles.audio_event != nullptr
                        && (*it)->miles.audio_event->Get_Playing_Handle() == kill_handle) {
                        Release_Playing_Audio(*it);
                        m_streamList.erase(it);
                        killed = true;
                        break;
                    }
                }
            }

            if (kill_handle != 0 && !killed) {
                stream_handle = 0;
            } else {
                stream_handle = AIL_open_stream(m_milesDigitalDriver, name.Str(), 0);
            }

            pa->miles.audio_event = event;
            pa->miles.stream = stream_handle;
            pa->miles.playing_type = PAT_STREAM;

            if (stream_handle != nullptr) {
                if (aud_info->Get_Event_Type() == EVENT_SPEECH && event->Should_Play_Locally()) {
                    m_unkSpeech = true;
                }

                AIL_set_stream_volume_pan(stream_handle, volume, 0.5f);
                Play_Stream(event, stream_handle);
                m_streamList.push_back(pa);
                pa = nullptr;
            }
            break;
        }
        case EVENT_SOUND:
            if (event->Is_Positional_Audio()) {
                H3DSAMPLE sample_handle;
                bool killed = false;

                if (kill_handle != 0) {
                    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
                        if ((*it)->miles.audio_event->Get_Playing_Handle() == kill_handle) {
                            Release_Playing_Audio(*it);
                            m_positionalAudioList.erase(it);
                            killed = true;
                            break;
                        }
                    }
                }

                if (kill_handle != 0 && !killed) {
                    sample_handle = nullptr;
                } else {
                    sample_handle = Get_First_3D_Sample();

                    if (sample_handle == nullptr && Kill_Lowest_Priority_Sound_Immediately(event)) {
                        sample_handle = Get_First_3D_Sample();
                    }
                }

                pa->miles.audio_event = event;
                pa->miles.sample_3d = sample_handle;
                pa->miles.playing_type = PAT_3DSAMPLE;
                pa->miles.file_handle = nullptr;
                m_positionalAudioList.push_back(pa);

                if (sample_handle != nullptr) {
                    pa->miles.file_handle = Play_Sample3D(event, sample_handle);
                    m_soundManager->Notify_Of_3D_Sample_Start();
                }

                if (pa->miles.file_handle == nullptr) {
                    m_positionalAudioList.pop_back();
                } else {
                    pa = nullptr;
                }
            } else {
                HSAMPLE sample_handle;
                bool killed = false;

                if (kill_handle != 0) {
                    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
                        if ((*it)->miles.audio_event->Get_Playing_Handle() == kill_handle) {
                            Release_Playing_Audio(*it);
                            m_globalAudioList.erase(it);
                            killed = true;
                            break;
                        }
                    }
                }

                if (kill_handle != 0 && !killed) {
                    sample_handle = nullptr;
                } else {
                    sample_handle = Get_First_2D_Sample();

                    if (sample_handle == nullptr && Kill_Lowest_Priority_Sound_Immediately(event)) {
                        sample_handle = Get_First_2D_Sample();
                    }
                }

                pa->miles.audio_event = event;
                pa->miles.sample = sample_handle;
                pa->miles.playing_type = PAT_2DSAMPLE;
                pa->miles.file_handle = nullptr;
                m_globalAudioList.push_back(pa);

                if (sample_handle != nullptr) {
                    pa->miles.file_handle = Play_Sample(event, sample_handle);
                    m_soundManager->Notify_Of_2D_Sample_Start();
                }

                if (pa->miles.file_handle == nullptr) {
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

void MilesAudioManager::Pause_Audio_Event(uintptr_t handle)
{
    // Unimplemented
}

/**
 * Stops an audio event from a handle.
 *
 * 0x0077D630
 */
void MilesAudioManager::Stop_Audio_Event(uintptr_t handle)
{
    if (handle == 4 || handle == 5) {
        for (auto it = m_streamList.begin(); it != m_streamList.end(); ++it) {
            if ((*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
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
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == handle) {
            (*it)->miles.disable_loops = true;
            // Looks like it passes the miles stream pointer as a handle... not sure though.
            Notify_Of_Audio_Completion((uintptr_t)(*it)->miles.stream, 2);
            break;
        }
    }

    for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == handle) {
            (*it)->miles.disable_loops = true;
            break;
        }
    }

    for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
        if (*it != nullptr && (*it)->miles.audio_event->Get_Playing_Handle() == handle) {
            (*it)->miles.disable_loops = true;
            break;
        }
    }
}

/**
 * Process an audio request.
 *
 * Inlined
 */
void MilesAudioManager::Process_Request(AudioRequest *request)
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
 *
 * 0x0077DD20
 */
void MilesAudioManager::Stop_All_Speech()
{
    auto it = m_streamList.begin();

    while (it != m_streamList.end()) {
        if (*it != nullptr) {
            if ((*it)->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_SPEECH) {
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
 *
 * 0x0077DC30
 */
void MilesAudioManager::Adjust_Playing_Volume(PlayingAudio *audio)
{
    float adjusted_vol = audio->miles.audio_event->Get_Volume() * audio->miles.audio_event->Get_Volume_Shift();

    switch (audio->miles.playing_type) {
        case PAT_2DSAMPLE: {
            float pan;
            AIL_sample_volume_pan(audio->miles.sample, nullptr, &pan);
            AIL_set_sample_volume_pan(audio->miles.sample, adjusted_vol * m_soundVolume, pan);
        } break;
        case PAT_3DSAMPLE:
            AIL_set_3D_sample_volume(audio->miles.sample_3d, adjusted_vol * m_3dSoundVolume);
            break;
        case PAT_STREAM: {
            float pan;
            float vol;
            AIL_stream_volume_pan(audio->miles.stream, nullptr, &pan);

            if (audio->miles.audio_event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
                vol = adjusted_vol * m_musicVolume;
            } else {
                vol = adjusted_vol * m_speechVolume;
            }

            AIL_set_stream_volume_pan(audio->miles.stream, vol, pan);
        } break;
        default:
            break;
    }
}

/**
 * Gets the first sample from the 2d sample handles list.
 *
 * Inlined
 */
HSAMPLE MilesAudioManager::Get_First_2D_Sample()
{
    HSAMPLE retval = nullptr;

    if (!m_sampleHandleList.empty()) {
        retval = m_sampleHandleList.front();
        m_sampleHandleList.pop_front();
    }

    return retval;
}

/**
 * Gets the first sample from the 3d sample handles list.
 *
 * Inlined
 */
H3DSAMPLE MilesAudioManager::Get_First_3D_Sample()
{
    H3DSAMPLE retval = nullptr;

    if (!m_3dSampleHandleList.empty()) {
        retval = m_3dSampleHandleList.front();
        m_3dSampleHandleList.pop_front();
    }

    return retval;
}

/**
 * Gets the effective volume of the event.
 *
 * 0x00780280
 */
float MilesAudioManager::Get_Effective_Volume(AudioEventRTS *event) const
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
 * Kills the lowest priority sound that is lower than the provided event.
 *
 * 0x0077F330
 */
bool MilesAudioManager::Kill_Lowest_Priority_Sound_Immediately(AudioEventRTS *event)
{
    AudioEventRTS *sound = Find_Lowest_Priority_Sound(event);

    if (sound == nullptr) {
        return false;
    }

    // Search different lists for the lowest priority sounds to stop depending on audio type.
    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            if (*it != nullptr && (*it)->miles.audio_event != nullptr && (*it)->miles.audio_event == sound) {
                Release_Playing_Audio(*it);
                m_positionalAudioList.erase(it);

                return true;
            }
        }
    } else {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            if (*it != nullptr && (*it)->miles.audio_event != nullptr && (*it)->miles.audio_event == sound) {
                Release_Playing_Audio(*it);
                // BUGFIX The original appears to do m_positionalAudioList.erase(it),
                // possible copy/paste error in original code?
                m_globalAudioList.erase(it);

                return true;
            }
        }
    }

    return false;
}

/**
 * Finds the lowest priority sound that is lower than the provided event.
 *
 * 0x0077F1C0
 */
AudioEventRTS *MilesAudioManager::Find_Lowest_Priority_Sound(AudioEventRTS *event)
{
    captainslog_assert(event != nullptr);
    int target_priority = event->Get_Event_Info()->Get_Priority();

    // Nothing can be lower than this so can't return anything.
    if (target_priority == 0) {
        return nullptr;
    }

    AudioEventRTS *lowest_event = nullptr;

    if (event->Is_Positional_Audio()) {
        for (auto it = m_positionalAudioList.begin(); it != m_positionalAudioList.end(); ++it) {
            int list_priority = (*it)->miles.audio_event->Get_Event_Info()->Get_Priority();
            int lowest_priority = target_priority;

            if (list_priority < target_priority && (lowest_event == nullptr || lowest_priority > list_priority)) {
                lowest_event = (*it)->miles.audio_event;
                lowest_priority = list_priority;

                if (lowest_priority == 0) {
                    break;
                }
            }
        }
    } else {
        for (auto it = m_globalAudioList.begin(); it != m_globalAudioList.end(); ++it) {
            int list_priority = (*it)->miles.audio_event->Get_Event_Info()->Get_Priority();
            int lowest_priority = target_priority;

            if (list_priority < target_priority && (lowest_event == nullptr || lowest_priority > list_priority)) {
                lowest_event = (*it)->miles.audio_event;
                lowest_priority = list_priority;

                if (lowest_priority == 0) {
                    break;
                }
            }
        }
    }

    return lowest_event;
}

/**
 * Gets the position the event is supposed to play at currently.
 *
 * Inlined.
 */
Coord3D *MilesAudioManager::Get_Current_Position_From_Event(AudioEventRTS *event)
{
    if (!event->Is_Positional_Audio()) {
        return nullptr;
    }

    return event->Get_Current_Pos();
}

/**
 * Helper function to clear a PlayingAudio object.
 */
void MilesAudioManager::Init_Playing_Audio(PlayingAudio *audio)
{
    if (audio != nullptr) {
        audio->miles.sample = 0;
        audio->miles.sample_3d = 0;
        audio->miles.stream = 0;
        audio->miles.playing_type = PAT_NONE;
        audio->miles.audio_event = nullptr;
        audio->miles.disable_loops = false;
        audio->miles.release_event = true;
        audio->miles.time_fading = 0;
    }
}

/**
 * Check if a request should be processed this frame.
 *
 * Inlined.
 */
bool MilesAudioManager::Process_Request_This_Frame(AudioRequest *request)
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
void MilesAudioManager::Adjust_Request(AudioRequest *request)
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
bool MilesAudioManager::Check_For_Sample(AudioRequest *request)
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
 *
 *
 * Inlined.
 */
bool MilesAudioManager::Is_On_Screen(const Coord3D *coord) const
{
    static ICoord2D _dummy;
    return g_theTacticalView->World_To_Screen_Tri(coord, &_dummy);
}

/**
 * Builds the 3D provider list by enumerating over the available providers.
 *
 * Inlined.
 */
void MilesAudioManager::Build_Provider_List()
{
    int max_provider = 0;
    uint32_t provider_type = 0;
    char *provider_name = nullptr;

    while (max_provider < MILES_PROVIDER_COUNT) {
        if (!AIL_enumerate_3D_providers(&provider_type, &m_milesProviderList[max_provider].provider, &provider_name)) {
            break;
        }

        m_milesProviderList[max_provider].name = provider_name;
        ++max_provider;
    }

    m_milesMaxProviderIndex = max_provider;
}

/**
 * Initialises the delay filter, empty in the original macOS port.
 *
 * Inlined.
 */
void MilesAudioManager::Init_Delay_Filter()
{
    if (m_milesDelayFilter == 0) {
        uint32_t next = 0;
        uint32_t filter;
        char *name;

        do {
            if (!AIL_enumerate_filters(&next, &filter, &name)) {
                return;
            }
        } while (strcmp(name, "Mono Delay Filter") != 0);

        m_milesDelayFilter = filter;
    }
}

/**
 * Creates the 3D listener.
 *
 * Inlined
 */
void MilesAudioManager::Create_Listener()
{
    if (Is_On(AUDIOAFFECT_3DSOUND) && Provider_Is_Valid()) {
        m_miles3DPositionObject = AIL_open_3D_listener(m_milesProviderList[m_milesCurrentProvider].provider);
    }
}

/**
 * Wrapper for Miles open file callback.
 *
 * 0x00780C90
 */
int __stdcall MilesAudioManager::Streaming_File_Open(const char *name, uintptr_t *handle)
{
    *handle = reinterpret_cast<uintptr_t>(g_theFileSystem->Open_File(name, File::READ | File::STREAMING));

    return *handle != 0;
}

/**
 * Wrapper for Miles close file callback.
 *
 * 0x00780CC0
 */
void __stdcall MilesAudioManager::Streaming_File_Close(uintptr_t handle)
{
    reinterpret_cast<File *>(handle)->Close();
}

/**
 * Wrapper for Miles seek file callback.
 *
 * 0x00780CD0
 */
int32_t __stdcall MilesAudioManager::Streaming_File_Seek(uintptr_t handle, int32_t pos, uint32_t whence)
{
    return reinterpret_cast<File *>(handle)->Seek(pos, (File::SeekMode)whence);
}

/**
 * Wrapper for Miles read file callback.
 *
 * 0x00780CF0
 */
uint32_t __stdcall MilesAudioManager::Streaming_File_Read(uintptr_t handle, void *dst, uint32_t size)
{
    return reinterpret_cast<File *>(handle)->Read(dst, size);
}

/**
 * Wrapper for Miles stream complete callback.
 *
 * 0x00780C70
 */
void __stdcall MilesAudioManager::Set_Stream_Complete(void *info)
{
    g_theAudio->Notify_Of_Audio_Completion((uintptr_t)info, PAT_STREAM);
}

/**
 * Wrapper for Miles sample complete callback.
 *
 * 0x00780C30
 */
void __stdcall MilesAudioManager::Set_Sample_Complete(void *info)
{
    g_theAudio->Notify_Of_Audio_Completion((uintptr_t)info, PAT_2DSAMPLE);
}

/**
 * Wrapper for Miles 3d sample complete callback.
 *
 * 0x00780C50
 */
void __stdcall MilesAudioManager::Set_3DSample_Complete(void *info)
{
    g_theAudio->Notify_Of_Audio_Completion((uintptr_t)info, PAT_3DSAMPLE);
}
