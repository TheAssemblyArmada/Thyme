/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Meta Map
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "metaevent.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
MetaMap *g_theMetaMap;
#endif

MetaMap::~MetaMap()
{
    while (m_metaMaps != nullptr) {
        MetaMapRec *map = m_metaMaps->m_next;
        m_metaMaps->Delete_Instance();
        m_metaMaps = map;
    }
}

MetaEventTranslator::MetaEventTranslator() : m_lastKeyDown(0), m_lastModState(0)
{
    for (int i = 0; i < 3; i++) {
        m_lastButtonPos[i].x = 0;
        m_lastButtonPos[i].y = 0;
        m_lastButtonState[i] = false;
    }
}

GameMessageDisposition MetaEventTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, MetaEventTranslator, const GameMessage *>(
        PICK_ADDRESS(0x00497DA0, 0x009C0D65), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}
