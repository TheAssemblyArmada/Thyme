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
#include "aabox.h"
#include "shader.h"
#include "sphere.h"

class DX8VertexBufferClass;
class DX8IndexBufferClass;
class RenderInfoClass;
class Vector2;
class VertexMaterialClass;
class WaterTracksObj;
class TextureClass;

enum WaveType
{
    WAVE_TYPE_POND = 0,
    WAVE_TYPE_OCEAN,
    WAVE_TYPE_CLOSE_OCEAN,
    WAVE_TYPE_CLOSE_OCEAN_DOUBLE,
    WAVE_TYPE_RADIAL,
    WAVE_TYPE_UNK,
};
DEFINE_ENUMERATION_OPERATORS(WaveType);

struct WaveInfo
{
    float final_width;
    float final_height;
    float distance_from_shore;
    float initial_velocity;
    int32_t time_to_fade;
    float initial_width_fraction;
    float initial_height_fraction;
    int32_t time_to_compress;
    int32_t time_offset_second_wave;
    const char *texture_name;
    const char *wave_name;
};

class WaterTracksObj
{
public:
    WaterTracksObj();
    ~WaterTracksObj();

    virtual void Render();
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const;

    int32_t Free_Water_Tracks_Resources();
    void Init(float width, Vector2 const &start, Vector2 const &end, const char *texture);
    void Init(float width, float length, Vector2 const &start, Vector2 const &end, const char *texture, int32_t time_offset);
    int32_t Render(DX8VertexBufferClass *vertex_buffer, int32_t batch_start);
    int32_t Update(int32_t msElapsed);

private:
    TextureClass *m_stageZeroTexture;
    SphereClass m_boundingSphere;
    AABoxClass m_boundingBox;
    int32_t m_type;
    int32_t m_x;
    int32_t m_y;
    bool m_bound;
    Vector2 m_startPos;
    Vector2 m_waveDir;
    Vector2 m_perpDir;
    Vector2 m_initialStart;
    Vector2 m_initialEnd;
    int32_t m_timeOffsetSecondWave;
    int32_t m_fadeMs;
    int32_t m_totalMs;
    int32_t m_elapsedMs;
    float m_widthFraction;
    float m_heightFraction;
    float m_length;
    int32_t m_unk;
    float m_width;
    float m_velocity;
    float m_distanceFromShore;
    float m_timeUntilBreak;
    float m_velocityUnk;
    float m_timingUnk2;
    float m_timingUnk3;
    float m_scaleUnk;
    float m_timeToCompress;
    float m_flipUV; // is used as a bool and appears to be defined wrong
    WaterTracksObj *m_nextSystem;
    WaterTracksObj *m_prevSystem;
    friend class WaterTracksRenderSystem;
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
    int32_t m_stripSizeX;
    int32_t m_stripSizeY;
    int32_t m_batchStart;
    float m_level;
    friend class WaterTracksObj;
};

#ifdef GAME_DLL
extern WaterTracksRenderSystem *&g_theWaterTracksRenderSystem;
#else
extern WaterTracksRenderSystem *g_theWaterTracksRenderSystem;
#endif