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
}

/**
 * Uninitilise the class.
 */
void FFmpegVideoPlayer::Deinit() {}

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

    // Check for a language specific directoy Data/%language%/Movies.
    path.Format("Data/%s/Movies/%s.%s", Get_Registry_Language().Str(), vid->file_name.Str(), "bik");
    // Load the file from disk
    file = g_theFileSystem->Open_File(path.Str(), File::READ | File::BINARY | File::BUFFERED);

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
        // Initialise_Bink_With_Miles();
    }

    Deinit();
    // BinkSetSoundTrack(0, nullptr);
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

    // This takes ownership of FFmpegFile
    FFmpegVideoStream *stream = new FFmpegVideoStream(ffmpegFile);
    stream->m_next = m_firstStream;
    stream->m_player = this;
    m_firstStream = stream;

    // float vol = g_theAudio->Get_Volume(AUDIOAFFECT_SPEECH);

    return stream;
}
} // namespace Thyme
