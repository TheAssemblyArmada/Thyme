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
public:
    BinkVideoPlayer();

#ifdef BUILD_WITH_BINK
    virtual ~BinkVideoPlayer();

    // Subsystem interface methods
    virtual void Init() override;
    virtual void Reset() override { VideoPlayer::Reset(); }
    virtual void Update() override { VideoPlayer::Update(); }

    // Video player methods
    virtual void Deinit() override;
    virtual VideoStream *Open(Utf8String title) override;
    virtual VideoStream *Load(Utf8String title) override;
    virtual void Notify_Player_Of_New_Provider(bool unk) override;
    virtual void Initialise_Bink_With_Miles(); // Original has this virtual, unclear why though.

#ifndef THYME_STANDALONE
    static void Hook_Me();
    void Hook_Init() { BinkVideoPlayer::Init(); }
    void Hook_Deinit() { BinkVideoPlayer::Deinit(); }
    VideoStream *Hook_Open(Utf8String title) { return BinkVideoPlayer::Open(title); }
    VideoStream *Hook_Load(Utf8String title) { return BinkVideoPlayer::Load(title); }
    void Hook_Notify_Player_Of_New_Provider(bool unk) { BinkVideoPlayer::Notify_Player_Of_New_Provider(unk); };
    void Hook_Initialise_Bink_With_Miles() { BinkVideoPlayer::Initialise_Bink_With_Miles(); }
#endif
#endif // BUILD_WITH_BINK

private:
    VideoStream *Create_Stream(Bink *handle);
};

#ifdef BUILD_WITH_BINK
#ifndef THYME_STANDALONE
#include "hooker.h"

inline void BinkVideoPlayer::Hook_Me()
{
    Hook_Method(0x007AA550, &Hook_Init);
    Hook_Method(0x007AA570, &Hook_Deinit);
    Hook_Method(0x007AA6A0, &Hook_Open);
    Hook_Method(0x007AA8E0, &Hook_Load);
    Hook_Method(0x007AA970, &Hook_Notify_Player_Of_New_Provider);
    Hook_Method(0x007AA9A0, &Hook_Initialise_Bink_With_Miles);
    Hook_Method(0x007AA5B0, &Create_Stream);
}
#endif
#endif // BUILD_WITH_BINK
