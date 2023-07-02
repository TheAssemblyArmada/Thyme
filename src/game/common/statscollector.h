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
#pragma once
#include "always.h"
#include "asciistring.h"

class GameMessage;
class StatsCollector
{
public:
    StatsCollector();
    ~StatsCollector() {}
    void Update();
    void Reset();
    void Write_File_End();
    void Collect_Msg_Stats(const GameMessage *msg);

private:
    Utf8String m_statsFileName;
    unsigned int m_buildCommands;
    unsigned int m_moveCommands;
    unsigned int m_attackCommands;
    unsigned int m_scrollMapCommands;
    unsigned int m_AIUnits;
    unsigned int m_playerUnits;
    unsigned int m_scrollBeginTime;
    unsigned int m_scrollTime;
    bool m_isScrolling;
    int m_timeCount;
    int m_lastUpdate;
    unsigned int m_startFrame;
};

#ifdef GAME_DLL
extern StatsCollector *&g_theStatsCollector;
#else
extern StatsCollector *g_theStatsCollector;
#endif
