/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "framemetrics.h"
#include "display.h"
#include "globaldata.h"
#include "networkutil.h"
#include "rtsutils.h"

FrameMetrics::FrameMetrics() :
    m_fpsList(nullptr),
    m_lastFpsTime(0),
    m_fpsListIndex(0),
    m_averageFps(0.0f),
    m_pendingLatencies(nullptr),
    m_averageLatency(0.0f),
    m_cushionIndex(0),
    m_minimumCushion(0)
{
    m_pendingLatencies = new int[MAX_FRAMES_AHEAD];

    for (int i = 0; i < MAX_FRAMES_AHEAD; ++i) {
        m_pendingLatencies[i] = 0;
    }

    m_fpsList = new float[g_theWriteableGlobalData->m_networkFPSHistoryLength];

    m_latencyList = new float[g_theWriteableGlobalData->m_networkLatencyHistoryLength];
}

FrameMetrics::~FrameMetrics()
{
    delete[] m_fpsList;
    delete[] m_latencyList;
    delete[] m_pendingLatencies;
}

void FrameMetrics::Init()
{
    m_averageFps = 30.0f;
    m_averageLatency = 0.2f;

    m_minimumCushion = -1;

    // TODO these arrays likely should be a scaleable container
    // if the lengths change at any point this will crash

    for (int i = 0; i < g_theWriteableGlobalData->m_networkFPSHistoryLength; ++i) {
        m_fpsList[i] = 30.0f;
    }

    m_fpsListIndex = 0;

    for (int i = 0; i < g_theWriteableGlobalData->m_networkLatencyHistoryLength; ++i) {
        m_latencyList[i] = 0.2f;
    }

    m_cushionIndex = 0;
}

void FrameMetrics::Reset()
{
    Init();

    // BUGFIX these weren't reset
    // TODO make sure this doesn't have adverse conciquences
    m_lastFpsTime = 0;

    for (int i = 0; i < MAX_FRAMES_AHEAD; ++i) {
        m_pendingLatencies[i] = 0;
    }
}

void FrameMetrics::Do_Per_Frame_Metrics(unsigned int frame)
{
    int time = (int)rts::Get_Time();

    if (time - m_lastFpsTime >= 1000) {
        m_averageFps -= m_fpsList[m_fpsListIndex] / (float)g_theWriteableGlobalData->m_networkFPSHistoryLength;

        m_fpsList[m_fpsListIndex] = g_theDisplay->Get_Average_FPS();

        m_averageFps += m_fpsList[m_fpsListIndex++] / (float)g_theWriteableGlobalData->m_networkFPSHistoryLength;

        m_fpsListIndex %= (unsigned int)g_theWriteableGlobalData->m_networkFPSHistoryLength;

        m_lastFpsTime = time;
    }

    m_pendingLatencies[frame % MAX_FRAMES_AHEAD] = time;
}

void FrameMetrics::Process_Latency_Response(unsigned int frame)
{
    int time_diff = (int)rts::Get_Time() - m_pendingLatencies[frame % MAX_FRAMES_AHEAD];
    unsigned int index = frame % g_theWriteableGlobalData->m_networkLatencyHistoryLength;

    m_averageLatency -= m_latencyList[index] / (float)(unsigned int)g_theWriteableGlobalData->m_networkLatencyHistoryLength;

    m_latencyList[index] = (float)time_diff / 1000.0f;

    m_averageLatency += m_latencyList[index] / (float)(unsigned int)g_theWriteableGlobalData->m_networkLatencyHistoryLength;
}

void FrameMetrics::Add_Cushion(int cushion)
{
    ++m_cushionIndex;

    m_cushionIndex %= g_theWriteableGlobalData->m_networkCushionHistoryLength;
    if (m_cushionIndex == 0) {
        m_minimumCushion = -1;
    }
    if (cushion < m_minimumCushion || m_minimumCushion == -1) {
        m_minimumCushion = cushion;
    }
}
