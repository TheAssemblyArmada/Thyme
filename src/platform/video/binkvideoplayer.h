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
#pragma once

#include "always.h"
#include "videoplayer.h"

struct Bink;

class BinkVideoPlayer final : public VideoPlayer
{
    ALLOW_HOOKING
public:
    BinkVideoPlayer();
    virtual ~BinkVideoPlayer();

    // Subsystem interface methods
    virtual void Init() override;
    virtual void Reset() override { VideoPlayer::Reset(); }
    virtual void Update() override { VideoPlayer::Update(); }

    // Video player methods
    virtual void Deinit() override;
    virtual VideoStream *Open(Utf8String title) override;
    virtual VideoStream *Load(Utf8String title) override;
    virtual void Notify_Player_Of_New_Provider(bool initialise) override;
    virtual void Initialise_Bink_With_Miles(); // Original has this virtual, unclear why though.

private:
    VideoStream *Create_Stream(Bink *handle);
};
