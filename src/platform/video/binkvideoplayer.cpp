/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Video player implementation using Bink video.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "binkvideoplayer.h"
#include "audiomanager.h"
#include "binkvideostream.h"
#include "globaldata.h"
#include "registryget.h"
#include <bink.h>

/**
 * 0x007AA4B0
 */
BinkVideoPlayer::BinkVideoPlayer() {}

/**
 * 0x007AA4F0
 */
BinkVideoPlayer::~BinkVideoPlayer()
{
    BinkVideoPlayer::Deinit();
}

/**
 * Initilise the class.
 *
 * 0x007AA4F0
 */
void BinkVideoPlayer::Init()
{
    VideoPlayer::Init();
    Initialise_Bink_With_Miles();
}

/**
 * Uninitilise the class.
 *
 * 0x007AA4F0
 */
void BinkVideoPlayer::Deinit()
{
    g_theAudio->Release_Bink_Handle();
    VideoPlayer::Deinit();
}

/**
 * Opens a video stream from a video title.
 *
 * 0x007AA6A0
 */
VideoStream *BinkVideoPlayer::Open(Utf8String title)
{
    Video *vid = Get_Video(title);

    if (vid == nullptr) {
        return nullptr;
    }

    Bink *handle = nullptr;
    Utf8String path;

    // First check the custom user "mod" directory if set.
    if (g_theWriteableGlobalData->m_userModDirectory.Is_Not_Empty()) {
        path.Format(
            "%s%s/%s.%s", g_theWriteableGlobalData->m_userModDirectory.Str(), "Data/Movies", vid->file_name.Str(), "bik");
        handle = BinkOpen(path.Str(), 0x2000);
    }

    // Next check for a language specific directoy Data/%language%/Movies.
    if (handle == nullptr) {
        path.Format("Data/%s/Movies/%s.%s", Get_Registry_Language().Str(), vid->file_name.Str(), "bik");
        handle = BinkOpen(path.Str(), 0x2000);
    }

    // Finally check Data/Movies.
    if (handle == nullptr) {
        path.Format("Data/Movies/%s.%s", vid->file_name.Str(), "bik");
        handle = BinkOpen(path.Str(), 0x2000);
    }

    return Create_Stream(handle);
}

/**
 * Loads a video stream from a video title.
 *
 * 0x007AA8E0
 */
VideoStream *BinkVideoPlayer::Load(Utf8String title)
{
    return Open(title);
}

/**
 * Appears to do a more thorough deinitialisation.
 *
 * 0x007AA970
 */
void BinkVideoPlayer::Notify_Player_Of_New_Provider(bool initialise)
{
    if (initialise) {
        Initialise_Bink_With_Miles();
    }

    Deinit();
    BinkSetSoundTrack(0, nullptr);
}

/**
 * Performs a specific initilisation assuming Miles is the audio sound system.
 *
 * 0x007AA9A0
 */
void BinkVideoPlayer::Initialise_Bink_With_Miles()
{
    BinkHandle handle = g_theAudio->Get_Bink_Handle();

    // If we don't have a miles handle or we fail to set the sound system from it, set to have no audio tracks.
    if (handle == nullptr || BinkSetSoundSystem(BinkOpenDirectSound, (uintptr_t)handle) == 0) {
        BinkSetSoundTrack(0, nullptr);
    }
}

/**
 * Internal function for creating new Bink specific video stream objects.
 *
 * 0x007AA5B0
 */
VideoStream *BinkVideoPlayer::Create_Stream(Bink *handle)
{
    if (handle == nullptr) {
        return nullptr;
    }

    BinkVideoStream *stream = new BinkVideoStream;
    stream->m_binkHandle = handle;
    stream->m_next = m_firstStream;
    stream->m_player = this;
    m_firstStream = stream;

    float vol = g_theAudio->Get_Volume(AUDIOAFFECT_SPEECH);
    BinkSetVolume(stream->m_binkHandle, 0, (((int((vol / 1.25f) * 100.0f)) + 1) << 0xF) / 100);

    return stream;
}
