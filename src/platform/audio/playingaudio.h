/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for caching loaded audio samples to reduce file IO.
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
#include "audioeventinfo.h"

enum PlayingAudioType : int32_t
{
    PAT_2DSAMPLE,
    PAT_3DSAMPLE,
    PAT_STREAM,
    PAT_NONE,
};

#ifdef BUILD_WITH_MILES
#include <miles.h>

struct MilesPlayingAudio
{
    HSAMPLE sample;
    H3DSAMPLE sample_3d;
    HSTREAM stream;
    PlayingAudioType playing_type;
    int stopped;
    AudioEventRTS *audio_event;
    AudioDataHandle file_handle;
    bool disable_loops;
    bool release_event;
    int time_fading;
};

#endif

#ifdef BUILD_WITH_OPENAL
#include <AL/al.h>

struct OpenALPlayingAudio
{
    ALuint source;
    ALuint buffer;
    PlayingAudioType playing_type;
    int stopped;
    AudioEventRTS *audio_event;
    AudioDataHandle file_handle;
    bool disable_loops;
    bool release_event;
    int time_fading;
};
#endif

struct PlayingAudio
{
    union
    {
#ifdef BUILD_WITH_MILES
        MilesPlayingAudio miles;
#endif
        int unused;
#ifdef BUILD_WITH_OPENAL
        OpenALPlayingAudio openal;
#endif
    };
};
