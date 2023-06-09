/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Stats Collector
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "statscollector.h"
#include "gamelogic.h"

#ifndef GAME_DLL
StatsCollector *g_theStatsCollector;
#endif

StatsCollector::StatsCollector() :
    m_buildCommands(0),
    m_moveCommands(0),
    m_attackCommands(0),
    m_scrollMapCommands(0),
    m_AIUnits(0),
    m_playerUnits(0),
    m_scrollBeginTime(0),
    m_scrollTime(0),
    m_isScrolling(false),
    m_timeCount(0),
    m_lastUpdate(0)
{
    m_startFrame = g_theGameLogic->Get_Frame();
}

void StatsCollector::Update()
{
#ifdef GAME_DLL
    Call_Method<void, StatsCollector>(PICK_ADDRESS(0x00608020, 0x009C33A5), this);
#endif
}

void StatsCollector::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, StatsCollector>(PICK_ADDRESS(0x00607E50, 0x009C3132), this);
#endif
}
