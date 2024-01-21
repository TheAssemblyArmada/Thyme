/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Look At Translator
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "lookatxlat.h"
#include "gamelogic.h"

#ifndef GAME_DLL
LookAtTranslator *g_theLookAtTranslator;
#endif

bool LookAtTranslator::Has_Mouse_Moved_Recently()
{
    if (m_lastMouseMoveFrame > g_theGameLogic->Get_Frame()) {
        m_lastMouseMoveFrame = 0;
    }

    return m_lastMouseMoveFrame + 30 >= g_theGameLogic->Get_Frame();
}
