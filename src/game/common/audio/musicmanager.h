/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Music management class.
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
#include "audioeventrts.h"

class MusicManager
{
public:
    virtual ~MusicManager() {}
    virtual void Add_Audio_Event(AudioEventRTS *event);
    virtual void Remove_Audio_Event(unsigned int handle);

private:
    void Play_Track(AudioEventRTS *event);
    void Stop_Track(unsigned int handle);
};