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
#pragma once
#include "always.h"
#include <new>

class FrameMetrics
{
public:
    FrameMetrics();
    virtual ~FrameMetrics();

    void Init();
    void Reset();

    void Do_Per_Frame_Metrics(uint32_t frame);

    void Process_Latency_Response(uint32_t frame);

    void Add_Cushion(int32_t cushion);

    int32_t Get_Average_FPS() const { return (int32_t)m_averageFps; }
    float Get_Average_Latency() const { return m_averageLatency; }
    int32_t Get_Minimum_Cushion() const { return m_minimumCushion; }

#ifdef GAME_DLL
    FrameMetrics *Hook_Ctor() { return new (this) FrameMetrics; }
    void Hook_Dtor() { FrameMetrics::~FrameMetrics(); }

#endif

private:
    float *m_fpsList;
    int32_t m_lastFpsTime;
    int32_t m_fpsListIndex;
    float m_averageFps;
    float *m_latencyList;
    int32_t *m_pendingLatencies;
    float m_averageLatency;
    uint32_t m_cushionIndex;
    int32_t m_minimumCushion;
};
