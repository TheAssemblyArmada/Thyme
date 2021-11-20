/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Water Tracks system
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
#include "shader.h"

class DX8VertexBufferClass;
class DX8IndexBufferClass;
class RenderInfoClass;
class Vector2;
class VertexMaterialClass;
class WaterTracksObj;

enum WaveType
{
    WAVE_TYPE_POND = 0,
    WAVE_TYPE_OCEAN,
    WAVE_TYPE_CLOSE_OCEAN,
    WAVE_TYPE_CLOSE_OCEAN_DOUBLE,
    WAVE_TYPE_RADIAL,
};

class WaterTracksRenderSystem
{
public:
    WaterTracksRenderSystem();
    ~WaterTracksRenderSystem();
    void Release_Resources();
    void Re_Acquire_Resources();
    void Flush(RenderInfoClass &rinfo);
    void Update();
    void Init();
    void Shutdown();
    WaterTracksObj *Bind_Track(WaveType type);
    void Un_Bind_Track(WaterTracksObj *mod);
    void Save_Tracks();
    void Load_Tracks();
    void Release_Track(WaterTracksObj *mod);
    void Reset();
    WaterTracksObj *Find_Track(Vector2 &start, Vector2 &end, WaveType type);

private:
    DX8VertexBufferClass *m_vertexBuffer;
    DX8IndexBufferClass *m_indexBuffer;
    VertexMaterialClass *m_vertexMaterialClass;
    ShaderClass m_shaderClass;
    WaterTracksObj *m_usedModules;
    WaterTracksObj *m_freeModules;
    int m_stripSizeX;
    int m_stripSizeY;
    int m_batchStart;
    float m_level;
};

#ifdef GAME_DLL
extern WaterTracksRenderSystem *&g_theWaterTracksRenderSystem;
#else
extern WaterTracksRenderSystem *g_theWaterTracksRenderSystem;
#endif