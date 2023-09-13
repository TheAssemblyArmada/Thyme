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
#include "gamemusic.h"
#include "audiomanager.h"
#include "audiorequest.h"

void MusicManager::Add_Audio_Event(AudioEventRTS *event)
{
    Play_Track(event);
}

void MusicManager::Remove_Audio_Event(unsigned int handle)
{
    Stop_Track(handle);
}

void MusicManager::Play_Track(AudioEventRTS *event)
{
    AudioRequest *request = g_theAudio->Allocate_Audio_Request(true);
    request->Request_Play(event);
    g_theAudio->Append_Audio_Request(request);
}

void MusicManager::Stop_Track(unsigned int handle)
{
    AudioRequest *request = g_theAudio->Allocate_Audio_Request(false);
    request->Request_Stop(handle);
    g_theAudio->Append_Audio_Request(request);
}
