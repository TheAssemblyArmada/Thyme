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
#include "audiomanager.h"
#include "filesystem.h"
#include "gamemusic.h"
#include "gamesounds.h"
#include "matrix3d.h"
#include "terrainlogic.h"
#include "w3dview.h"
#include <algorithm>
#include <captainslog.h>

#ifndef GAME_DLL
AudioManager *g_theAudio = nullptr;
#else
#include "hooker.h"
#endif

const char *AudioManager::s_speakerTypes[] = {
    "2 Speakers", "Headphones", "Surround Sound", "4 Speaker", "5.1 Surround", "7.1 Surround", nullptr
};

AudioManager::AudioManager() :
    m_audioSettings(new AudioSettings),
    m_miscAudio(new MiscAudio),
    m_musicManager(nullptr),
    m_soundManager(nullptr),
    m_audioHandleCounter(6),
    m_musicVolume(0.0f),
    m_soundVolume(0.0f),
    m_3dSoundVolume(0.0f),
    m_speechVolume(0.0f),
    m_musicVolumeAdjust(0.0f),
    m_soundVolumeAdjust(0.0f),
    m_3dSoundVolumeAdjust(0.0f),
    m_speechVolumeAdjust(0.0f),
    m_initialMusicVolume(0.0f),
    m_initialSoundVolume(0.0f),
    m_initial3DSoundVolume(0.0f),
    m_initialSpeechVolume(0.0f),
    m_zoomVolume(),
    m_emptyAudioEvent(new AudioEventRTS),
    m_savedVolumes(nullptr),
    m_cachedVariables(0xF)
{
    m_listenerPosition.Zero();
    m_listenerFacing.Set(0.0f, 1.0f, 0.0f);
}

AudioManager::~AudioManager()
{
    delete m_emptyAudioEvent;
    m_emptyAudioEvent = nullptr;
    delete m_musicManager;
    m_musicManager = nullptr;
    delete m_soundManager;
    m_soundManager = nullptr;
    delete m_miscAudio;
    m_miscAudio = nullptr;
    delete m_audioSettings;
    m_audioSettings = nullptr;
    delete m_savedVolumes;
}

/**
 * Initialises the subsystem.
 *
 * 0x00404C60
 */
void AudioManager::Init()
{
    INI ini;
    ini.Load("Data/INI/AudioSettings.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Default/Music.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Music.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Default/SoundEffects.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/SoundEffects.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Default/Speech.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Speech.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Default/Voice.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/Voice.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/MiscAudio.ini", INI_LOAD_OVERWRITE, nullptr);

    if (!Is_Music_Already_Loaded()) {
        m_fromCD = true;
        captainslog_dbgassert(false, "Music not detected as loaded, this shouldn't happen with released game data.");
        // Original code prompts to insert disc at this point, but it shouldn't be possible with the shipped
        // installer to install without the audio installed to the hard drive.
    }

    m_musicManager = new MusicManager;
    m_soundManager = new SoundManager;
    m_initialMusicVolume = m_audioSettings != nullptr ? m_audioSettings->Get_Music_Volume() : 0.55f;
    m_initialSoundVolume = m_audioSettings != nullptr ? m_audioSettings->Get_Sound_Volume() : 0.75f;
    m_initial3DSoundVolume = m_audioSettings != nullptr ? m_audioSettings->Get_3D_Sound_Volume() : 0.75f;
    m_initialSpeechVolume = m_audioSettings != nullptr ? m_audioSettings->Get_Speech_Volume() : 0.55f;
    m_musicVolumeAdjust = 1.0f;
    m_soundVolumeAdjust = 1.0f;
    m_3dSoundVolumeAdjust = 1.0f;
    m_speechVolumeAdjust = 1.0f;
    m_musicVolume = m_initialMusicVolume;
    m_soundVolume = m_initialSoundVolume;
    m_3dSoundVolume = m_initial3DSoundVolume;
    m_speechVolume = m_initialSpeechVolume;
}

/**
 * Resets the subsystem.
 *
 * 0x00404FB0
 */
void AudioManager::Reset()
{
    m_eventVolumeList.clear();
    m_musicVolumeAdjust = 1.0f;
    m_soundVolumeAdjust = 1.0f;
    m_3dSoundVolumeAdjust = 1.0f;
    m_speechVolumeAdjust = 1.0f;
    m_musicVolume = m_initialMusicVolume;
    m_soundVolume = m_initialSoundVolume;
    m_3dSoundVolume = m_initial3DSoundVolume;
    m_speechVolume = m_initialSpeechVolume;
    m_unkSpeech = false;
}

/**
 * Updates the subsystem.
 *
 * 0x00404F30
 */
void AudioManager::Update()
{
    Coord3D view_pos = g_theTacticalView->Get_Position();
    float view_angle = g_theTacticalView->Get_Angle();
    Matrix3D tm(Matrix3D::Identity);
    tm.Rotate_Z(view_angle);
    Vector3 v(0.0f, 1.0f, 0.0f);
    v = tm * v;
    float height = m_audioSettings->Get_Microphone_Desired_Height_Above_Terrain();
    float max = m_audioSettings->Get_Microphone_Max_Percent_Between_Ground_And_Camera();

    Coord3D listener_dir;
    listener_dir.Set(v.X, v.Y, v.Z);

    Coord3D camera_pos = g_theTacticalView->Get_3D_Camera_Position();

    Coord3D view_to_camera;
    view_to_camera.Set(&camera_pos);
    view_to_camera.Sub(&view_pos);

    float scale;

    if (camera_pos.z <= height && view_to_camera.z > 0.0f) {
        scale = max;
    } else {
        float height_factor = height / view_to_camera.z;

        if (max < height_factor) {
            scale = max;
        } else {
            scale = height_factor;
        }
    }

    view_to_camera.Scale(scale);
    view_pos.z = g_theTerrainLogic->Get_Ground_Height(view_pos.x, view_pos.y, nullptr);

    Coord3D listener_pos;
    listener_pos.Set(&view_pos);
    listener_pos.Add(&view_to_camera);
    Set_Listener_Position(&listener_pos, &listener_dir);

    float zoom_percent = m_audioSettings->Get_Zoom_Sound_Volume_Percent_Amount();
    float zoom_min = m_audioSettings->Get_Zoom_Min_Distance();
    float zoom_max = m_audioSettings->Get_Zoom_Max_Distance();
    m_zoomVolume = 1.0f - zoom_percent;

    if (zoom_percent > 0.0f) {
        Coord3D zoom_pos = camera_pos;
        zoom_pos.Sub(&listener_pos);
        float zoom_len = zoom_pos.Length();

        if (zoom_len >= zoom_min) {
            if (zoom_len < zoom_max) {
                m_zoomVolume = 1.0f - (zoom_len - zoom_min) / (zoom_max - zoom_min) * zoom_percent;
            }
        } else {
            m_zoomVolume = 1.0f;
        }
    }

    Set_3D_Volume_Adjustment(m_zoomVolume);
}

/**
 * Sets the position of the listener for 3D audio.
 *
 * 0x00406050
 */
void AudioManager::Set_Listener_Position(const Coord3D *position, const Coord3D *direction)
{
    // TODO Should probably take references
    m_listenerPosition = *position;
    m_listenerFacing = *direction;
}

/**
 * Allocates a request object.
 *
 * 0x00406090
 */
AudioRequest *AudioManager::Allocate_Audio_Request(bool is_add_request)
{
    return NEW_POOL_OBJ(AudioRequest, is_add_request);
}

/**
 * Frees a request object.
 *
 * 0x004061A0
 */
void AudioManager::Release_Audio_Request(AudioRequest *request)
{
    if (request != nullptr) {
        request->Delete_Instance();
    }
}

/**
 * Appends a request object to the internal processing queue.
 *
 * 0x004061D0
 */
void AudioManager::Append_Audio_Request(AudioRequest *request)
{
    m_audioRequestList.push_back(request);
}

/**
 * Find or allocate an AudioEventInfo object for the given name.
 *
 * 0x00406250
 */
AudioEventInfo *AudioManager::New_Audio_Event_Info(Utf8String name)
{
    AudioEventInfo *info = Find_Audio_Event_Info(name);

    if (info == nullptr) {
        info = NEW_POOL_OBJ(AudioEventInfo);

        m_allAudioEventInfo[name] = info;
    }

    return info;
}

/**
 * Adds an AudioEventInfo object to the internal directory.
 *
 * 0x004065D0
 */
void AudioManager::Add_Audio_Event_Info(AudioEventInfo *info)
{
    AudioEventInfo *found = Find_Audio_Event_Info(info->Get_Event_Name());

    if (found != nullptr) {
        *found = *info;
    } else {
        m_allAudioEventInfo[info->Get_Event_Name()] = info;
    }
}

/**
 * Finds an AudioEventInfo object in the internal directory.
 *
 * 0x004067B0
 */
AudioEventInfo *AudioManager::Find_Audio_Event_Info(Utf8String name) const
{
    auto it = m_allAudioEventInfo.find(name);

    if (it != m_allAudioEventInfo.end()) {
        return it->second;
    }

    return nullptr;
}

/**
 * Refresh boolean cached variables.
 *
 * 0x00406920
 */
void AudioManager::Refresh_Cached_Variables()
{
    AudioManager::Set_Hardware_Accelerated(Is_Current_Provider_Hardware_Accelerated());
    AudioManager::Set_Speaker_Surround(Is_Current_Speaker_Type_Surround_Sound());
}

/**
 * Gets the length of the audio file in milliseconds associated with an event.
 *
 * 0x00406970
 */
float AudioManager::Get_Audio_Length_MS(const AudioEventRTS *event)
{
    // If the event doesn't have info, try and populate it, if that fails return 0.
    if (event->Get_Event_Info() == nullptr) {
        Get_Info_For_Audio_Event(event);

        if (event->Get_Event_Info() == nullptr) {
            return 0.0f;
        }
    }

    AudioEventRTS new_event = *event;
    new_event.Generate_Filename();
    new_event.Generate_Play_Info();
    float attack_len = Get_File_Length_MS(new_event.Get_Attack_Name());
    float normal_len = Get_File_Length_MS(new_event.Get_File_Name());
    float decay_len = Get_File_Length_MS(new_event.Get_Decay_Name());

    return float(attack_len + normal_len) + decay_len;
}

/**
 * Checks if a music file is accessible.
 *
 * 0x00406A90
 */
bool AudioManager::Is_Music_Already_Loaded()
{
    AudioEventInfo *event_info = nullptr;

    for (auto it = m_allAudioEventInfo.begin(); it != m_allAudioEventInfo.end(); ++it) {
        if (it->second != nullptr && it->second->Get_Event_Type() == EVENT_MUSIC) {
            event_info = it->second;
        }
    }

    if (event_info == nullptr) {
        return false;
    }

    AudioEventRTS rts_event;
    rts_event.Set_Event_Info(event_info);
    rts_event.Generate_Filename();

    return g_theFileSystem->Does_File_Exist(rts_event.Get_File_Name().Str());
}

/**
 * Populates a container with all events of a given type.
 *
 * 0x00406C10
 */
void AudioManager::Find_All_Audio_Events_Of_Type(AudioType type, std::vector<AudioEventInfo *> &list)
{
    for (auto it = m_allAudioEventInfo.begin(); it != m_allAudioEventInfo.end(); ++it) {
        auto find_it = it;

        // From current position, find if an entry matches the type.
        for (; find_it != m_allAudioEventInfo.end(); ++find_it) {
            if (find_it->second->Get_Event_Type() == type) {
                break;
            }
        }

        it = find_it;

        if (it != m_allAudioEventInfo.end()) {
            list.push_back(it->second);
        } else {
            return;
        }
    }
}

/**
 * Retrieves hash map of all audio event info.
 *
 * 0x005B9460
 */
const audioinfomap_t *AudioManager::Get_All_Audio_Events() const
{
    return &m_allAudioEventInfo;
}

/**
 * Checks if the current driver is hardware accelerated.
 *
 * 0x00406D00
 */
bool AudioManager::Is_Current_Provider_Hardware_Accelerated()
{
    // Search preferred providers list against current provider.
    for (int i = 0; i < DRIVER_SOFTWARE; ++i) {
        if (m_audioSettings->Get_Preferred_Driver(i).Str() == Get_Provider_Name(Get_Selected_Provider())) {
            return true;
        }
    }

    return false;
}

/**
 * Checks if the current speaker config is a surround config.
 *
 * 0x00406DE0
 */
bool AudioManager::Is_Current_Speaker_Type_Surround_Sound()
{
    return Get_Speaker_Type() == m_audioSettings->Get_Default_3D_Speaker_Type();
}

/**
 * Checks if an event should play locally.
 *
 * 0x00406E00
 */
bool AudioManager::Should_Play_Locally(const AudioEventRTS *event)
{
    // TODO Requires ControlBar
#ifdef GAME_DLL
    return Call_Method<bool, AudioManager, const AudioEventRTS *>(PICK_ADDRESS(0x00406E00, 0x006E5597), this, event);
#else
    return false;
#endif
}

/**
 * Allocates a unique handle value.
 *
 * 0x00406F00
 */
int AudioManager::Allocate_New_Handle()
{
    return m_audioHandleCounter++;
}

/**
 * Removed level specific event info from the all info hash map.
 *
 * 0x00406860
 */
void AudioManager::Remove_Level_Specific_Audio_Event_Infos()
{
    for (auto it = m_allAudioEventInfo.begin(); it != m_allAudioEventInfo.end();) {
        auto delete_it = it;
        ++it;

        if (delete_it->second->Is_Level_Specific()) {
            delete_it->second->Delete_Instance();
            m_allAudioEventInfo.erase(delete_it);
        }
    }
}

/**
 * Removes all audio requests from the request list.
 *
 * 0x00406200
 */
void AudioManager::Remove_All_Audio_Requests()
{
    for (auto it = m_audioRequestList.begin(); it != m_audioRequestList.end(); ++it) {
        Release_Audio_Request(*it);
    }

    m_audioRequestList.clear();
}

/**
 * Fetches the name of the track that follows the one provided.
 *
 * 0x00405700
 */
Utf8String AudioManager::Next_Track_Name(Utf8String track) const
{
    Utf8String next;
    auto it = m_trackList.begin();

    // Look for the track we want the next track from in the list
    while (it != m_trackList.end()) {
        if (*it == track) {
            break;
        }

        ++it;
    }

    // If we found the track, then return the next track if valid, else loop back to the start.
    if (it != m_trackList.end()) {
        ++it;

        if (it != m_trackList.end()) {
            next = *it;
        } else if (m_trackList.begin() != m_trackList.end()) {
            next = *m_trackList.begin();
        }
    }

    return next;
}

/**
 * Fetches the name of the track that preceeds the one provided.
 *
 * 0x004057D0
 */
Utf8String AudioManager::Prev_Track_Name(Utf8String track) const
{
    Utf8String next;
    auto it = m_trackList.rbegin();

    // Look for the track we want the previous track from in the list
    while (it != m_trackList.rend()) {
        if (*it == track) {
            break;
        }

        ++it;
    }

    // If we found the track, then return the next track if valid, else loop back to the start.
    if (it != m_trackList.rend()) {
        ++it;

        if (it != m_trackList.rend()) {
            next = *it;
        } else if (m_trackList.rbegin() != m_trackList.rend()) {
            next = *m_trackList.rbegin();
        }
    }

    return next;
}

/**
 * Handles the loss of focus event for the audio engine.
 *
 * 0x00406F10
 */
void AudioManager::Lose_Focus()
{
    if (m_savedVolumes != nullptr) {
        captainslog_warn("deleting m_savedVolumes to prevent leaking memory!");
        // #BUGFIX clean up the memory leak
        // WB debug printed when it occurred but didn't clean up
        delete[] m_savedVolumes;
    }

    m_savedVolumes = new float[4];
    m_savedVolumes[0] = m_initialMusicVolume;
    m_savedVolumes[1] = m_initialSoundVolume;
    m_savedVolumes[2] = m_initial3DSoundVolume;
    m_savedVolumes[3] = m_initialSpeechVolume;
    Set_Volume(0.0f, AUDIOAFFECT_MUSIC | AUDIOAFFECT_SOUND | AUDIOAFFECT_3DSOUND | AUDIOAFFECT_SPEECH | AUDIOAFFECT_BASEVOL);
}

/**
 * Handles the regaining of focus event for the audio engine.
 *
 * 0x00406F70
 */
void AudioManager::Regain_Focus()
{
    if (m_savedVolumes != nullptr) {
        Set_Volume(m_savedVolumes[0], AUDIOAFFECT_MUSIC | AUDIOAFFECT_BASEVOL);
        Set_Volume(m_savedVolumes[1], AUDIOAFFECT_SOUND | AUDIOAFFECT_BASEVOL);
        Set_Volume(m_savedVolumes[2], AUDIOAFFECT_3DSOUND | AUDIOAFFECT_BASEVOL);
        Set_Volume(m_savedVolumes[3], AUDIOAFFECT_SPEECH | AUDIOAFFECT_BASEVOL);
        delete[] m_savedVolumes;
        m_savedVolumes = nullptr;
    }
}

/**
 * Adds an audio event.
 *
 * 0x00405390
 */
int AudioManager::Add_Audio_Event(const AudioEventRTS *event)
{
    if (event->Get_Event_Name().Is_Empty()) {
        return 1;
    }

    if (event->Get_Event_Name() == "NoSound") {
        return 1;
    }

    if (event->Get_Event_Info() == nullptr) {
        Get_Info_For_Audio_Event(event);

        if (event->Get_Event_Info() == nullptr) {
            captainslog_debug("No info for requested audio event '%s'", event->Get_Event_Name().Str());
            return 0;
        }
    }

    switch (event->Get_Event_Type()) {
        case EVENT_MUSIC:
            if (!Is_On(AUDIOAFFECT_MUSIC)) {
                return 1;
            }

            break;
        case EVENT_SPEECH:
            if (!Is_On(AUDIOAFFECT_SPEECH)) {
                return 1;
            }

            break;
        case EVENT_SOUND:
            if (!Is_On(AUDIOAFFECT_SOUND)) {
                return 1;
            }

            if (!Is_On(AUDIOAFFECT_3DSOUND)) {
                return 1;
            }

            break;
        default:
            break;
    }

    if (m_unkSpeech && event->Get_Event_Type() == EVENT_SPEECH) {
        return 1;
    }

    AudioEventRTS *new_event = new AudioEventRTS(*event);
    new_event->Set_Playing_Handle(Allocate_New_Handle());
    new_event->Generate_Filename();
    event->Set_Current_Sound_Index(new_event->Get_Current_Sound_Index());
    new_event->Generate_Play_Info();

    for (auto it = m_eventVolumeList.begin(); it != m_eventVolumeList.end(); ++it) {
        if (it->first == new_event->Get_Event_Name()) {
            new_event->Set_Volume(it->second);
            break;
        }
    }

    if (!new_event->Should_Play_Locally() && !Should_Play_Locally(new_event)) {
        Release_Audio_Event_RTS(new_event);

        return 3; // Don't play locally?
    }

    if (new_event->Get_Volume() < m_audioSettings->Get_Min_Sample_Vol()) {
        Release_Audio_Event_RTS(new_event);

        return 2; // Volume too low?
    }

    if (event->Get_Event_Info()->Get_Event_Type() == EVENT_MUSIC) {
        m_musicManager->Add_Audio_Event(new_event);
    } else {
        m_soundManager->Add_Audio_Event(new_event);
    }

    if (new_event != nullptr) {
        return new_event->Get_Playing_Handle();
    }

    return 1;
}

/**
 * Removes an audio event.
 *
 * 0x004058A0
 */
void AudioManager::Remove_Audio_Event(unsigned handle)
{
    if (handle == 4 || handle == 5) {
        m_musicManager->Remove_Audio_Event(handle);
    } else if (handle >= 6) {
        AudioRequest *request = Allocate_Audio_Request(false);
        request->Request_Stop(handle);
        Append_Audio_Request(request);
    }
}

/**
 * Removes an audio event.
 *
 * 0x00405C30
 */
void AudioManager::Remove_Audio_Event(Utf8String event)
{
    Remove_Playing_Audio(event);
}

/**
 * Checks if an event is valid.
 *
 * 0x00405680
 */
bool AudioManager::Is_Valid_Audio_Event(const AudioEventRTS *event) const
{
    if (event->Get_Event_Name().Is_Empty()) {
        return false;
    }

    Get_Info_For_Audio_Event(event);

    return event->Get_Event_Info() != nullptr;
}

/**
 * Checks if an event is valid.
 *
 * 0x00405680
 */
// Windows calls same function for both locations in vtable.
bool AudioManager::Is_Valid_Audio_Event(AudioEventRTS *event) const
{
    if (event->Get_Event_Name().Is_Empty()) {
        return false;
    }

    Get_Info_For_Audio_Event(event);

    return event->Get_Event_Info() != nullptr;
}

/**
 * Sets an event as enabled.
 *
 * 0x004058F0
 */
void AudioManager::Set_Audio_Event_Enabled(Utf8String event, bool vol_override)
{
    Set_Audio_Event_Volume_Override(event, vol_override ? -1.0f : 0.0f);
}

/**
 * Sets an override volume.
 *
 * 0x00405990
 */
void AudioManager::Set_Audio_Event_Volume_Override(Utf8String event, float vol_override)
{
    if (event == Utf8String::s_emptyString) {
        m_eventVolumeList.clear();

        return;
    }

    // -1.0f indicates no adjustment is to be made.

    if (vol_override != -1.0f) {
        Adjust_Volume_Of_Playing_Audio(event, vol_override);
    }

    for (auto it = m_eventVolumeList.begin(); it != m_eventVolumeList.end(); ++it) {
        if (event == it->first) {

            if (vol_override == -1.0f) {
                m_eventVolumeList.erase(it);
            } else {
                it->second = vol_override;
            }
            return;
        }
    }

    if (vol_override != -1.0f) {
        m_eventVolumeList.push_front(std::pair<Utf8String, float>(event, vol_override));
    }
}

/**
 * Removes events that have been disabled.
 *
 * 0x00405CC0
 */
void AudioManager::Remove_Disabled_Events()
{
    Remove_All_Disabled_Audio();
}

/**
 * Gets the audio info for an audio event.
 *
 * 0x00405340
 */
void AudioManager::Get_Info_For_Audio_Event(const AudioEventRTS *event) const
{
    if (event == nullptr || event->Get_Event_Info() != nullptr) {
        return;
    }

    event->Set_Event_Info(Find_Audio_Event_Info(event->Get_Event_Name()));
}

/**
 * Converts a speaker type name to an index.
 *
 * 0x00405CD0
 */
unsigned AudioManager::Translate_From_Speaker_Type(const Utf8String &type)
{
    unsigned i = 0;

    while (s_speakerTypes[i] != nullptr) {
        if (type == s_speakerTypes[i]) {
            break;
        }

        ++i;
    }

    return i;
}

/**
 * Converts a speaker index to a type name.
 *
 * 0x00405DD0
 */
Utf8String AudioManager::Translate_To_Speaker_Type(unsigned type)
{
    if (type < ARRAY_SIZE(s_speakerTypes)) {
        return s_speakerTypes[type];
    }

    return s_speakerTypes[0];
}

/**
 * Checks if a class of sounds is enabled.
 *
 * 0x00405E50
 */
bool AudioManager::Is_On(AudioAffect affect) const
{
    if (affect & AUDIOAFFECT_MUSIC) {
        return m_musicOn;
    } else if (affect & AUDIOAFFECT_SOUND) {
        return m_soundOn;
    } else if (affect & AUDIOAFFECT_3DSOUND) {
        return m_3dSoundOn;
    }

    return m_speechOn;
}

/**
 * Sets the state of a class of sounds.
 *
 * 0x00405E90
 */
void AudioManager::Set_On(bool on, AudioAffect affect)
{
    if (affect & AUDIOAFFECT_MUSIC) {
        m_musicOn = on;
    }

    if (affect & AUDIOAFFECT_SOUND) {
        m_soundOn = on;
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        m_3dSoundOn = on;
    }

    if (affect & AUDIOAFFECT_SPEECH) {
        m_speechOn = on;
    }
}

/**
 * Set a class of sounds volume.
 *
 * 0x00405F20
 */
void AudioManager::Set_Volume(float volume, AudioAffect affect)
{
    if (affect & AUDIOAFFECT_MUSIC) {
        if (affect & AUDIOAFFECT_BASEVOL) {
            m_initialMusicVolume = volume;
        } else {
            m_musicVolumeAdjust = volume;
        }

        m_musicVolume = m_initialMusicVolume * m_musicVolumeAdjust;
    }

    if (affect & AUDIOAFFECT_SOUND) {
        if (affect & AUDIOAFFECT_BASEVOL) {
            m_initialSoundVolume = volume;
        } else {
            m_soundVolumeAdjust = volume;
        }

        m_soundVolume = m_initialSoundVolume * m_soundVolumeAdjust;
    }

    if (affect & AUDIOAFFECT_3DSOUND) {
        if (affect & AUDIOAFFECT_BASEVOL) {
            m_initial3DSoundVolume = volume;
        } else {
            m_3dSoundVolumeAdjust = volume;
        }

        m_3dSoundVolume = m_initial3DSoundVolume * m_3dSoundVolumeAdjust;
    }

    if (affect & AUDIOAFFECT_SPEECH) {
        if (affect & AUDIOAFFECT_BASEVOL) {
            m_initialSpeechVolume = volume;
        } else {
            m_speechVolumeAdjust = volume;
        }

        m_speechVolume = m_initialSpeechVolume * m_speechVolumeAdjust;
    }

    m_volumeSet = true;
}

/**
 * Gets the volume for the specified class of sounds.
 *
 * 0x00405FC0
 */
float AudioManager::Get_Volume(AudioAffect affect)
{
    if (affect & AUDIOAFFECT_MUSIC) {
        return m_musicVolume;
    } else if (affect & AUDIOAFFECT_SOUND) {
        return m_soundVolume;
    } else if (affect & AUDIOAFFECT_3DSOUND) {
        return m_3dSoundVolume;
    }

    return m_speechVolume;
}

/**
 * Sets an adjustment for 3D sounds.
 *
 * 0x00405FF0
 */
void AudioManager::Set_3D_Volume_Adjustment(float adj)
{
    m_3dSoundVolume = std::clamp(m_initial3DSoundVolume * m_3dSoundVolumeAdjust * adj, 0.0f, 1.0f);

    if (Has_3D_Sensitive_Streams_Playing()) {
        m_volumeSet = true;
    }
}
