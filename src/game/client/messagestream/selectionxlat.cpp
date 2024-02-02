/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Selection Xlat
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "selectionxlat.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
SelectionTranslator *g_theSelectionTranslator;
#endif

SelectionTranslator::SelectionTranslator() :
    m_leftMouseButtonIsDown(false),
    m_dragSelecting(false),
    m_lastGroupSelTime(0),
    m_lastGroupSelGroup(-1),
    m_displayedMaxWarning(false),
    m_wheelPos(0)
#ifdef GAME_DEBUG_STRUCTS
    ,
    m_handOfGodMode(false)
#endif
{
    m_selectAnchor.x = 0;
    m_selectAnchor.y = 0;
    m_deselectAnchor.x = 0;
    m_deselectAnchor.y = 0;
    m_viewPos.Zero();
    g_theSelectionTranslator = this;
}

SelectionTranslator::~SelectionTranslator() {}

GameMessageDisposition SelectionTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, SelectionTranslator, const GameMessage *>(
        PICK_ADDRESS(0x00614E50, 0x00A1D2D5), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}

bool Can_Select_Drawable(const Drawable *drawable, bool drag_selecting)
{
#ifdef GAME_DLL
    return Call_Function<bool, const Drawable *, bool>(PICK_ADDRESS(0x00614AD0, 0x00A1CCBC), drawable, drag_selecting);
#else
    return false;
#endif
}
