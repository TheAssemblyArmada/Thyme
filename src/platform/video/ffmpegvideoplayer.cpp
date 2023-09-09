/**
 * @file
 *
 * @author feliwir
 *
 * @brief Video player implementation using FFmpeg.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ffmpegvideoplayer.h"
#include "audiomanager.h"
#include "ffmpegfile.h"
#include "ffmpegvideostream.h"
#include "globaldata.h"
#include "registry.h"

#ifdef BUILD_WITH_OPENAL
#include "alaudiostream.h"
#endif

namespace Thyme
{
FFmpegVideoPlayer::FFmpegVideoPlayer() {}

FFmpegVideoPlayer::~FFmpegVideoPlayer()
{
    Deinit();
}

/**
 * Initilise the class.
 */
void FFmpegVideoPlayer::Init()
{
    VideoPlayer::Init();
    Initialise_FFmpeg_With_OpenAL();
}

/**
 * Uninitilise the class.
 */
void FFmpegVideoPlayer::Deinit()
{
    VideoPlayer::Deinit();
}

/**
 * Opens a video stream from a video title.
 */
VideoStream *FFmpegVideoPlayer::Open(Utf8String title)
{
    Video *vid = Get_Video(title);

    if (vid == nullptr) {
        return nullptr;
    }

    File *file = nullptr;
    Utf8String path;

    // First check the custom user "mod" directory if set.
    if (g_theWriteableGlobalData && g_theWriteableGlobalData->m_userModDirectory.Is_Not_Empty()) {
        path.Format(
            "%s%s/%s.%s", g_theWriteableGlobalData->m_userModDirectory.Str(), "Data/Movies", vid->file_name.Str(), "bik");
        // Load the file from disk
        file = g_theFileSystem->Open_File(path.Str(), File::READ | File::BINARY | File::BUFFERED);
    }

    // Check for a language specific directoy Data/%language%/Movies.
    if (file == nullptr) {
        path.Format("Data/%s/Movies/%s.%s", Get_Registry_Language().Str(), vid->file_name.Str(), "bik");
        // Load the file from disk
        file = g_theFileSystem->Open_File(path.Str(), File::READ | File::BINARY | File::BUFFERED);
    }

    // Finally check Data/Movies.
    if (file == nullptr) {
        path.Format("Data/Movies/%s.%s", vid->file_name.Str(), "bik");
        // Load the file from disk
        file = g_theFileSystem->Open_File(path.Str(), File::READ | File::BINARY | File::BUFFERED);
    }

    return Create_Stream(file);
}

/**
 * Loads a video stream from a video title.
 */
VideoStream *FFmpegVideoPlayer::Load(Utf8String title)
{
    return Open(title);
}

/**
 * Appears to do a more thorough deinitialisation.
 */
void FFmpegVideoPlayer::Notify_Player_Of_New_Provider(bool initialise)
{
    if (initialise) {
        Initialise_FFmpeg_With_OpenAL();
    }

    Deinit();
}

/**
 * Performs a specific initilisation assuming OpenALA is the audio sound system.
 */
void FFmpegVideoPlayer::Initialise_FFmpeg_With_OpenAL()
{
#ifdef BUILD_WITH_OPENAL
    BinkHandle handle = g_theAudio->Get_Bink_Handle();

    // If we don't have a miles handle or we fail to set the sound system from it, set to have no audio tracks.
    if (handle == nullptr) {
        return;
    }

    m_audio_stream = static_cast<ALAudioStream *>(handle);
#endif
}

/**
 * Internal function for creating new FFmpeg specific video stream objects.
 */
VideoStream *FFmpegVideoPlayer::Create_Stream(File *file)
{
    if (file == nullptr) {
        return nullptr;
    }

    FFmpegFile *ffmpegFile = new FFmpegFile();
    if (!ffmpegFile->Open(file)) {
        delete ffmpegFile;
        return nullptr;
    }

#ifdef BUILD_WITH_OPENAL
    m_audio_stream->Reset();
#endif

    // This takes ownership of FFmpegFile
    FFmpegVideoStream *stream = new FFmpegVideoStream(this, m_firstStream, ffmpegFile);
    m_firstStream = stream;

#ifdef BUILD_WITH_OPENAL
    float vol = g_theAudio->Get_Volume(AUDIOAFFECT_SPEECH);
    m_audio_stream->SetVolume(vol);
#endif

    return stream;
}
} // namespace Thyme
