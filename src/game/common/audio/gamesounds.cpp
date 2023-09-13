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
#include "gamesounds.h"
#include "audiomanager.h"
#include "partitionmanager.h"
#include "playerlist.h"

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

    if (Can_Play_Now(event)) {
        AudioRequest *request = g_theAudio->Allocate_Audio_Request(true);
        request->Request_Play(event);
        g_theAudio->Append_Audio_Request(request);
        return;
    }

    g_theAudio->Release_Audio_Event_RTS(event);
}

/**
 * Determine if an event can be played.
 *
 * 0x00446120
 */
bool SoundManager::Can_Play_Now(AudioEventRTS *event)
{
    if (event->Is_Positional_Audio() && (event->Get_Event_Info()->Get_Visibility() & VISIBILITY_GLOBAL) == 0
        && event->Get_Event_Info()->Get_Priority() != 4) {
        Coord3D listener_pos = *g_theAudio->Get_Listener_Position();
        Coord3D *event_pos = event->Get_Current_Pos();

        if (event_pos != nullptr) {
            listener_pos.Sub(event_pos);

            if (listener_pos.Length() >= event->Get_Event_Info()->Max_Range()) {
                return false;
            }

            int player_index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();

            if ((event->Get_Event_Info()->Get_Visibility() & VISIBILITY_SHROUDED) != 0) {
                if (g_thePartitionManager->Get_Shroud_Status_For_Player(player_index, event_pos)) {
                    return false;
                }
            }
        }
    }

    if (Violates_Voice(event)) {
        return Is_Interrupting(event);
    }

    if (g_theAudio->Does_Violate_Limit(event)) {
        return false;
    }

    if (Is_Interrupting(event)) {
        return true;
    }

    if (event->Is_Positional_Audio()) {
        if (m_3dSamplesPlaying < m_3dSampleSlotCount) {
            return true;
        }
    } else if (m_2dSamplesPlaying < m_2dSampleSlotCount) {
        return true;
    }

    if (g_theAudio->Is_Playing_Lower_Priority(event)) {
        return true;
    }

    if (Is_Interrupting(event)) {
        return g_theAudio->Is_Playing_Already(event);
    }

    return false;
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

    if (event->Get_Object_ID() != INVALID_OBJECT_ID) {
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
