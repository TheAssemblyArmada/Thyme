/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Implements manager for drawing snow/rain effects.
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
#include "snow.h"

class MinMaxAABoxClass;
class RenderInfoClass;
class TextureClass;
class IndexBufferClass;

struct IDirect3DVertexBuffer8;

class W3DSnowManager : public SnowManager
{
public:
    enum
    {
        MAX_PARTICLE_COUNT = 2048,
        VERTEX_COUNT = 4096,
    };

    W3DSnowManager();
    virtual ~W3DSnowManager();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void Update_INI_Settings() override;

    void Release_Resources();
    bool Re_Acquire_Resources();

    void Render(RenderInfoClass &rinfo);

private:
    void Render_Sub_Box(RenderInfoClass &rinfo, int x1, int y1, int x2, int y2);
    void Render_As_Quads(RenderInfoClass &rinfo, int x1, int y1, int x2, int y2);

    inline void Render_Sub_Box_Internal(
        RenderInfoClass &rinfo, MinMaxAABoxClass &minmax_aabox, int x1, int y1, int x2, int y2);

private:
    IndexBufferClass *m_indexBuffer;
    TextureClass *m_texture;
#ifdef BUILD_WITH_D3D8
    IDirect3DVertexBuffer8 *m_vertexBuffer;
#endif
    int m_vertexBufferOffset;
    int m_maxParticleCount;
    int m_numVertices;
    int m_unk5;
    float m_unk6;
    float m_unk7;
    int m_currentParticleCount;
    float m_unk9;
};
