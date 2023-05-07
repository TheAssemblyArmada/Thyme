/**
 * @file
 *
 * @author feliwir
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
#include "audiofilecache.h"

namespace Thyme
{
class FFmpegAudioFileCache : public AudioFileCache
{
public:
    ~FFmpegAudioFileCache();

protected:
    void Release_Open_Audio(OpenAudioFile *open_audio) override;
    bool Load_File(File *file, OpenAudioFile &open_audio) override;

private:
    bool Open_FFmpeg_Contexts(OpenAudioFile *open_audio, File *file);
    bool Decode_FFmpeg(OpenAudioFile *open_audio);
    void Close_FFmpeg_Contexts(OpenAudioFile *open_audio);
    static int Read_FFmpeg_Packet(void *opaque, uint8_t *buf, int buf_size);

    void Fill_Wave_Data(OpenAudioFile *open_audio);
};
} // namespace Thyme
