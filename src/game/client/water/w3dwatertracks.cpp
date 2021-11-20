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
#include "w3dwatertracks.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#ifndef GAME_DLL
WaterTracksRenderSystem *g_theWaterTracksRenderSystem;
#endif

WaterTracksRenderSystem::WaterTracksRenderSystem() :
    m_usedModules(nullptr),
    m_freeModules(nullptr),
    m_indexBuffer(nullptr),
    m_vertexMaterialClass(nullptr),
    m_vertexBuffer(nullptr),
    m_stripSizeX(2),
    m_stripSizeY(2),
    m_batchStart(0)
{
    g_theWaterTracksRenderSystem = this;
}

WaterTracksRenderSystem::~WaterTracksRenderSystem()
{
    Shutdown();
    m_vertexMaterialClass = nullptr;
}

void WaterTracksRenderSystem::Re_Acquire_Resources()
{
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexBuffer);
    int size = (2 * m_stripSizeX + 2) * (m_stripSizeY - 1) - 2;
    m_indexBuffer = new DX8IndexBufferClass(size, DX8IndexBufferClass::USAGE_DEFAULT);

    {
        IndexBufferClass::WriteLockClass lock(m_indexBuffer, 0);
        unsigned short *indices = lock.Get_Index_Array();
        int i1 = 0;
        int i2 = 0;
        int i3 = 0;

        while (i1 < size) {
            while (i3 < (i2 + 1) * m_stripSizeX) {
                indices[i1] = m_stripSizeX + i3;
                indices[i1 + 1] = i3++;
                i1 += 2;
            }

            if (i1 < size) {
                indices[i1] = i3 - 1;
                indices[i1 + 1] = m_stripSizeX + i3;
                i1 += 2;
            }

            i2++;
        }
    }

    m_vertexBuffer = new DX8VertexBufferClass(
        DX8_FVF_XYZDUV1, 1000 * m_stripSizeY * m_stripSizeX, DX8VertexBufferClass::USAGE_DYNAMIC, 0);
    m_batchStart = 0;
}

void WaterTracksRenderSystem::Release_Resources()
{
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexBuffer);
}

void WaterTracksRenderSystem::Init()
{
#ifdef GAME_DLL
    Call_Method<void, WaterTracksRenderSystem>(PICK_ADDRESS(0x007A1800, 0x0064D0CD), this);
#endif
}

void WaterTracksRenderSystem::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, WaterTracksRenderSystem>(PICK_ADDRESS(0x007A18A0, 0x0064D221), this);
#endif
}

void WaterTracksRenderSystem::Shutdown()
{
#ifdef GAME_DLL
    Call_Method<void, WaterTracksRenderSystem>(PICK_ADDRESS(0x007A1930, 0x0064D264), this);
#endif
}

void WaterTracksRenderSystem::Flush(RenderInfoClass &rinfo)
{
#ifdef GAME_DLL
    Call_Method<void, WaterTracksRenderSystem, RenderInfoClass &>(PICK_ADDRESS(0x007A1A40, 0x0064D40B), this, rinfo);
#endif
}

void WaterTracksRenderSystem::Load_Tracks()
{
#ifdef GAME_DLL
    Call_Method<void, WaterTracksRenderSystem>(PICK_ADDRESS(0x007A1FD0, 0x0064D907), this);
#endif
}
