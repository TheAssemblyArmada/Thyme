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
#pragma once

#include "always.h"
#include "videoplayer.h"

namespace Thyme
{
#ifdef BUILD_WITH_OPENAL
class ALAudioStream;
#endif
class FFmpegVideoPlayer final : public VideoPlayer
{
public:
    FFmpegVideoPlayer();
    virtual ~FFmpegVideoPlayer();

    // Subsystem interface methods
    virtual void Init() override;
    virtual void Reset() override { VideoPlayer::Reset(); }
    virtual void Update() override { VideoPlayer::Update(); }

    // Video player methods
    virtual void Deinit() override;
    virtual VideoStream *Open(Utf8String title) override;
    virtual VideoStream *Load(Utf8String title) override;
    virtual void Notify_Player_Of_New_Provider(bool initialise) override;

    void Initialise_FFmpeg_With_OpenAL();

    VideoStream *Create_Stream(File *file);
};
} // namespace Thyme
