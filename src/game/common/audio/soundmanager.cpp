/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for managing loaded sounds.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "soundmanager.h"
#include "audiomanager.h"

/**
 * Reset the subsystem.
 *
 * 0x00445FF0
 */
void SoundManager::Reset()
{
    m_2dSamplesPlaying = 0;
    m_3dSamplesPlaying = 0;
}

/**
 * Add an event to the manager.
 *
 * 0x00446010
 */
void SoundManager::Add_Audio_Event(AudioEventRTS *event)
{
    if (m_2dSampleSlotCount == 0 && m_3dSampleSlotCount == 0) {
        m_2dSampleSlotCount = g_theAudio->Get_Num_2D_Samples();
        m_3dSampleSlotCount = g_theAudio->Get_Num_3D_Samples();
    }

    if (!Can_Play_Now(event)) {
        g_theAudio->Release_Audio_Event_RTS(event);

        return;
    }

    AudioRequest *request = g_theAudio->Allocate_Audio_Request(true);
    request->Request_Play(event);
    g_theAudio->Append_Audio_Request(request);
}

/**
 * Determine if an event can be played.
 *
 * 0x00446120
 */
bool SoundManager::Can_Play_Now(AudioEventRTS *event)
{
    // Requires PlayerList and PartitionManager classes.
#ifdef GAME_DLL
    return Call_Method<bool, SoundManager, AudioEventRTS *>(PICK_ADDRESS(0x00446120, 0x008AF28A), this, event);
#else
    return false;
#endif
}

/**
 * Determines if an event will conflict with voice events perhaps?
 *
 * 0x004462F0
 */
bool SoundManager::Violates_Voice(AudioEventRTS *event)
{
    if (!(event->Get_Event_Info()->Get_Visibility() & VISIBILITY_VOICE)) {
        return false;
    }

    if (event->Get_Object_ID() != OBJECT_UNK) {
        if (g_theAudio->Is_Object_Playing_Voice(event->Get_Object_ID())) {
            return true;
        }
    }

    return false;
}

/**
 * Determines if an event is an interrupting event.
 *
 * 0x00446350
 */
bool SoundManager::Is_Interrupting(AudioEventRTS *event)
{
    return (event->Get_Event_Info()->Get_Control() & CONTROL_INTERRUPT) != 0;
}
