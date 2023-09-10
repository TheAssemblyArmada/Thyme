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
#include "alaudiostream.h"
#include "alaudiomanager.h"

#include <captainslog.h>

namespace Thyme
{
ALAudioStream::ALAudioStream()
{
    alGenSources(1, &m_source);
    alGenBuffers(AL_STREAM_BUFFER_COUNT, m_buffers);

    // Make stream ignore positioning
    alSource3i(m_source, AL_POSITION, 0, 0, 0);
    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcei(m_source, AL_ROLLOFF_FACTOR, 0);
}

ALAudioStream::~ALAudioStream()
{
    // Unbind the buffers first
    alSourceStop(m_source);
    alSourcei(m_source, AL_BUFFER, 0);
    captainslog_dbgassert(ALAudioManager::Check_AL_Error(), "Failed to unbind buffers");
    alDeleteSources(1, &m_source);
    captainslog_dbgassert(ALAudioManager::Check_AL_Error(), "Failed to delete source");
    // Now delete the buffers
    alDeleteBuffers(AL_STREAM_BUFFER_COUNT, m_buffers);
    captainslog_dbgassert(ALAudioManager::Check_AL_Error(), "Failed to delete buffers");
}

bool ALAudioStream::BufferData(uint8_t *data, size_t data_size, ALenum format, int samplerate)
{
    ALint num_queued;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_queued);
    if (num_queued >= AL_STREAM_BUFFER_COUNT) {
        captainslog_error("Having too many buffers already queued: %i", num_queued);
        return false;
    }

    ALuint &current_buffer = m_buffers[m_current_buffer_idx];
    alBufferData(current_buffer, format, data, data_size, samplerate);
    alSourceQueueBuffers(m_source, 1, &current_buffer);
    m_current_buffer_idx++;

    if (m_current_buffer_idx >= AL_STREAM_BUFFER_COUNT)
        m_current_buffer_idx = 0;

    return true;
}

void ALAudioStream::Update()
{
    ALint processed;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
    while (processed > 0) {
        ALuint buffer;
        alSourceUnqueueBuffers(m_source, 1, &buffer);
        processed--;
    }

    ALint num_queued;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_queued);
    captainslog_info("Having %i buffers queued", num_queued);
    if (num_queued == 1) {
        // TODO: we could invoke a callback here, asking for more audio data
    }
}

void ALAudioStream::Reset()
{
    alSourceRewind(m_source);
    alSourcei(m_source, AL_BUFFER, 0);
}

bool ALAudioStream::IsPlaying()
{
    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

} // namespace Thyme
