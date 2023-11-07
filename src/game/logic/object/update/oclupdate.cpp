/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief OCL Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "oclupdate.h"
#include "gamelogic.h"

float OCLUpdate::Get_Countdown_Percent() const
{
    return 1.0f
        - (float)(m_creationTimerEndFrame - g_theGameLogic->Get_Frame())
        / (float)(m_creationTimerEndFrame - m_creationTimerStartFrame);
}

unsigned int OCLUpdate::Get_Remaining_Frames() const
{
    return m_creationTimerEndFrame - g_theGameLogic->Get_Frame();
}
