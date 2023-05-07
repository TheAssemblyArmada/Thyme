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
#include "audiofilecache.h"

class MilesAudioFileCache : public Thyme::AudioFileCache
{
    ALLOW_HOOKING
public:
    ~MilesAudioFileCache();

protected:
    void Release_Open_Audio(OpenAudioFile *open_audio) override;
    bool Load_File(File *file, OpenAudioFile &open_audio) override;
};
