/**
 * @file
 *
 * @author feliwir
 *
 * @brief Audio stream implementation built on top of OpenAL (with ALSoft as reference implementation)
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
#include <AL/al.h>

#define AL_STREAM_BUFFER_COUNT 32

namespace Thyme
{
class ALAudioStream final
{
public:
    ALAudioStream();
    ~ALAudioStream();

    bool BufferData(uint8_t *data, size_t data_size, ALenum format, int samplerate);
    bool IsPlaying();
    void Update();
    void Reset();

    void Play() { alSourcePlay(m_source); }
    void Pause() { alSourcePause(m_source); }
    void Stop() { alSourceStop(m_source); }

    void SetVolume(float vol) { alSourcef(m_source, AL_GAIN, vol); }

protected:
    ALuint m_source = 0;
    ALuint m_buffers[AL_STREAM_BUFFER_COUNT] = {};
    unsigned int m_current_buffer_idx = 0;
};
} // namespace Thyme