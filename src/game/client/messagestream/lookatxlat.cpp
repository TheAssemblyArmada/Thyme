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

LookAtTranslator::LookAtTranslator() :
    m_isScrolling(false),
    m_isRotating(false),
    m_isPitching(false),
    m_isChangingFOV(false),
    m_timestamp(0),
    m_lastPlaneID(0),
    m_scrollType(0),
    m_lastMouseMoveFrame(0)
{
    m_anchor.y = 0;
    m_anchor.x = 0;
    m_currentPos.y = 0;
    m_currentPos.x = 0;
    m_originalAnchor.y = 0;
    m_originalAnchor.x = 0;
    captainslog_dbgassert(g_theLookAtTranslator == nullptr, "Already have a LookAtTranslator - why do you need two?");
    g_theLookAtTranslator = this;
}

LookAtTranslator::~LookAtTranslator()
{
    if (g_theLookAtTranslator == this) {
        g_theLookAtTranslator = nullptr;
    }
}

const ICoord2D *LookAtTranslator::Get_RMB_Scroll_Anchor()
{
    if (m_isScrolling && m_scrollType == 1) {
        return &m_anchor;
    } else {
        return nullptr;
    }
}

GameMessageDisposition LookAtTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, LookAtTranslator, const GameMessage *>(
        PICK_ADDRESS(0x00613E90, 0x00A1F94A), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}

bool LookAtTranslator::Has_Mouse_Moved_Recently()
{
    if (m_lastMouseMoveFrame > g_theGameLogic->Get_Frame()) {
        m_lastMouseMoveFrame = 0;
    }

    return m_lastMouseMoveFrame + 30 >= g_theGameLogic->Get_Frame();
}
