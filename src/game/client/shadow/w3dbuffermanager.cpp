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

#include "w3dbuffermanager.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "refcount.h"
#include <d3d8types.h>

// TODO need help with RefCount, not sure whats the right thing to use here
// TODO need advice on how to do asserts, what should be a asset, what should be a fatal assert
// TODO need general cleanup advice
// TODO help on weird access in get and release slot

static int FVFTypeIndexList[W3DBufferManager::MAX_FVF] = {
    D3DFVF_XYZ | D3DFVF_TEX0, // XYZ
    D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0, // XYZD

    D3DFVF_XYZ | D3DFVF_TEX1, // XYZUV
    D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, // XYZDUV

    D3DFVF_XYZ | D3DFVF_TEX2, // XYZUV2
    D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2, // XYZDUV2

    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX0, // XYZN
    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX0, // XYZND

    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, // XYZNUV
    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1, // XYZNDUV

    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2, // XYZNUV2
    D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2, // XYZNDUV2

    D3DFVF_XYZRHW | D3DFVF_TEX0, // XYZRHW
    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX0, // XYZRHWD

    D3DFVF_XYZRHW | D3DFVF_TEX1, // XYZRHWUV
    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, // XYZRHWDUV

    D3DFVF_XYZRHW | D3DFVF_TEX2, // XYZRHWUV2
    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2 // XYZRHWDUV2
};

int W3DBufferManager::Get_DX8_Format(VBM_FVF_TYPES fvf_type)
{
    return FVFTypeIndexList[fvf_type];
}

W3DBufferManager::W3DBufferManager()
{
    m_numEmptySlotsAllocated = 0;
    m_numEmptyVertexBuffersAllocated = 0;
    m_W3DIndexBuffers = nullptr;
    m_numEmptyIndexSlotsAllocated = 0;
    m_numEmptyIndexBuffersAllocated = 0;

    for (int i = 0; i < MAX_FVF; ++i) {
        m_W3DVertexBuffers[i] = nullptr;
    }

    for (int j = 0; j < MAX_FVF; ++j) {
        for (int k = 0; k < MAX_VB_SIZES; ++k) {
            m_W3DVertexBufferSlots[j][k] = nullptr;
        }
    }

    // bugfix from BFME2, clear the buffers
    for (int l = 0; l < MAX_NUMBER_SLOTS; ++l) {
        m_W3DVertexBufferEmptySlots[l].m_size = 0;
        m_W3DVertexBufferEmptySlots[l].m_start = 0;
        m_W3DVertexBufferEmptySlots[l].m_VB = nullptr;
        m_W3DVertexBufferEmptySlots[l].m_prevSameSize = nullptr;
        m_W3DVertexBufferEmptySlots[l].m_nextSameSize = nullptr;
        m_W3DVertexBufferEmptySlots[l].m_prevSameVB = nullptr;
        m_W3DVertexBufferEmptySlots[l].m_nextSameVB = nullptr;
    }

    for (int m = 0; m < MAX_VERTEX_BUFFERS_CREATED; ++m) {
        m_W3DEmptyVertexBuffers[m].m_format = VBM_FVF_XYZ;
        m_W3DEmptyVertexBuffers[m].m_usedSlots = nullptr;
        m_W3DEmptyVertexBuffers[m].m_startFreeIndex = 0;
        m_W3DEmptyVertexBuffers[m].m_size = 0;
        m_W3DEmptyVertexBuffers[m].m_nextVB = nullptr;
        m_W3DEmptyVertexBuffers[m].m_DX8VertexBuffer = nullptr;
        m_W3DEmptyVertexBuffers[m].m_renderTaskList = nullptr;
    }

    for (int l = 0; l < MAX_IB_SIZES; ++l) {
        m_W3DIndexBufferSlots[l] = nullptr;
    }

    // bugfix from BFME2, clear the buffers
    for (int ii = 0; ii < MAX_NUMBER_SLOTS; ++ii) {
        m_W3DIndexBufferEmptySlots[ii].m_size = 0;
        m_W3DIndexBufferEmptySlots[ii].m_start = 0;
        m_W3DIndexBufferEmptySlots[ii].m_IB = nullptr;
        m_W3DIndexBufferEmptySlots[ii].m_prevSameSize = nullptr;
        m_W3DIndexBufferEmptySlots[ii].m_nextSameSize = nullptr;
        m_W3DIndexBufferEmptySlots[ii].m_prevSameIB = nullptr;
        m_W3DIndexBufferEmptySlots[ii].m_nextSameIB = nullptr;
    }

    for (int jj = 0; jj < MAX_INDEX_BUFFERS_CREATED; ++jj) {
        m_W3DEmptyIndexBuffers[jj].m_usedSlots = nullptr;
        m_W3DEmptyIndexBuffers[jj].m_startFreeIndex = 0;
        m_W3DEmptyIndexBuffers[jj].m_size = 0;
        m_W3DEmptyIndexBuffers[jj].m_nextIB = nullptr;
        m_W3DEmptyIndexBuffers[jj].m_DX8IndexBuffer = nullptr;
    }
}

W3DBufferManager::~W3DBufferManager()
{
    Free_All_Slots();
    Free_All_Buffers();
}

void W3DBufferManager::Free_All_Slots()
{
    for (int i = 0; i < MAX_FVF; ++i) {

        for (int j = 0; j < MAX_VB_SIZES; ++j) {
            W3DVertexBufferSlot *vb_slot = m_W3DVertexBufferSlots[i][j];

            while (vb_slot) {

                if (vb_slot->m_prevSameVB) {
                    vb_slot->m_prevSameVB->m_nextSameVB = vb_slot->m_nextSameVB;
                } else {
                    vb_slot->m_VB->m_usedSlots = nullptr;
                }

                if (vb_slot->m_nextSameVB) {
                    vb_slot->m_nextSameVB->m_prevSameVB = vb_slot->m_prevSameVB;
                }

                vb_slot = vb_slot->m_nextSameSize;
                --m_numEmptySlotsAllocated;
            }

            m_W3DVertexBufferSlots[i][j] = nullptr;
        }
    }
    for (int k = 0; k < MAX_IB_SIZES; ++k) {

        W3DIndexBufferSlot *ib_slot = m_W3DIndexBufferSlots[k];

        while (ib_slot) {

            if (ib_slot->m_prevSameIB) {
                ib_slot->m_prevSameIB->m_nextSameIB = ib_slot->m_nextSameIB;
            } else {
                ib_slot->m_IB->m_usedSlots = nullptr;
            }

            if (ib_slot->m_nextSameIB) {
                ib_slot->m_nextSameIB->m_prevSameIB = ib_slot->m_prevSameIB;
            }

            ib_slot = ib_slot->m_nextSameSize;
            --m_numEmptyIndexSlotsAllocated;
        }

        m_W3DIndexBufferSlots[k] = nullptr;
    }

    // bfme2 m_numEmptySlotsAllocated==0
    if (m_numEmptySlotsAllocated) {
        if (!byte_E1B29C) {
            TheCurrentAllowCrashPtr = (int *)&byte_E1B29C;
            DebugCrash("Failed to free all empty vertex buffer slots");
            TheCurrentAllowCrashPtr = 0;
        }
    }

    // bfme2 m_numEmptyIndexSlotsAllocated==0
    if (m_numEmptyIndexSlotsAllocated) {
        if (!byte_E1B29D) {
            TheCurrentAllowCrashPtr = (int *)&byte_E1B29D;
            DebugCrash("Failed to free all empty index buffer slots");
            TheCurrentAllowCrashPtr = 0;
        }
    }
}

void W3DBufferManager::Free_All_Buffers()
{
    Free_All_Slots();

    for (int i = 0; i < MAX_FVF; ++i) {

        for (W3DVertexBuffer *vb = m_W3DVertexBuffers[i]; vb; vb = vb->m_nextVB) {

            // bfme2 vb->m_usedSlots == NULL
            if (vb->m_usedSlots) {
                if (!byte_E1B29E) {
                    TheCurrentAllowCrashPtr = (int *)&byte_E1B29E;
                    DebugCrash("Freeing Non-Empty Vertex Buffer");
                    TheCurrentAllowCrashPtr = 0;
                }
            }

            if (vb->m_DX8VertexBuffer) {
                RefCountClass::Release_Ref(&vb->m_DX8VertexBuffer->vertbuf.ref);
            }

            vb->m_DX8VertexBuffer = nullptr;
            --m_numEmptyVertexBuffersAllocated;
        }

        m_W3DVertexBuffers[i] = nullptr;
    }

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib; ib = ib->m_nextIB) {

        // bfme2 ib->m_usedSlots == NULL
        if (ib->m_usedSlots) {
            if (!byte_E1B29F) {
                TheCurrentAllowCrashPtr = (int *)&byte_E1B29F;
                DebugCrash("Freeing Non-Empty Index Buffer");
                TheCurrentAllowCrashPtr = 0;
            }
        }

        if (ib->m_DX8IndexBuffer) {
            RefCountClass::Release_Ref(&ib->m_DX8IndexBuffer->ib.ref);
        }

        ib->m_DX8IndexBuffer = nullptr;
        --m_numEmptyIndexBuffersAllocated;
    }

    m_W3DIndexBuffers = nullptr;

    // bfme2 m_numEmptyVertexBuffersAllocated==0
    if (m_numEmptyVertexBuffersAllocated) {
        if (!byte_E1B2A0) {
            TheCurrentAllowCrashPtr = (int *)&byte_E1B2A0;
            DebugCrash("Failed to free all empty vertex buffers");
            TheCurrentAllowCrashPtr = 0;
        }
    }

    // bfme2 m_numEmptyIndexBuffersAllocated==0
    if (m_numEmptyIndexBuffersAllocated) {
        if (!byte_E1B2A1) {
            TheCurrentAllowCrashPtr = (int *)&byte_E1B2A1;
            DebugCrash("Failed to free all empty index buffers");
            TheCurrentAllowCrashPtr = 0;
        }
    }
}

void W3DBufferManager::Release_Resources()
{
    for (int i = 0; i < MAX_FVF; ++i) {

        for (W3DVertexBuffer *vb = m_W3DVertexBuffers[i]; vb; vb = vb->m_nextVB) {

            if (vb->m_DX8VertexBuffer) {
                RefCountClass::Release_Ref(&vb->m_DX8VertexBuffer->vertbuf.ref);
            }

            vb->m_DX8VertexBuffer = nullptr;
        }
    }

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib; ib = ib->m_nextIB) {

        if (ib->m_DX8IndexBuffer) {
            RefCountClass::Release_Ref(&ib->m_DX8IndexBuffer->ib.ref);
        }

        ib->m_DX8IndexBuffer = nullptr;
    }
}

bool W3DBufferManager::ReAcquire_Resources()
{
    for (int i = 0; i < MAX_FVF; ++i) {

        for (W3DVertexBuffer *vb = m_W3DVertexBuffers[i]; vb; vb = vb->m_nextVB) {

            // bfme2 vb->m_DX8VertexBuffer == NULL
            if (vb->m_DX8VertexBuffer) {

                if (!byte_E1B2A2) {
                    TheCurrentAllowCrashPtr = (int *)&byte_E1B2A2;
                    DebugCrash("ReAcquire of existing vertex buffer");
                    TheCurrentAllowCrashPtr = 0;
                }
            }

            vb->m_DX8VertexBuffer = new DX8VertexBufferClass(
                Get_DX8_Format(vb->m_format), LOWORD(vb->m_size), DX8VertexBufferClass::USAGE_DEFAULT, 0);

            // bfme2 vb->m_DX8VertexBuffer
            if (!vb->m_DX8VertexBuffer && !byte_E1B2A3) {
                TheCurrentAllowCrashPtr = (int *)&byte_E1B2A3;
                DebugCrash("Failed ReAcquire of vertex buffer");
                TheCurrentAllowCrashPtr = 0;
            }

            if (!vb->m_DX8VertexBuffer) {
                return false;
            }
        }
    }

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib; ib = ib->m_nextIB) {

        // bfme2 ib->m_DX8IndexBuffer == NULL
        if (ib->m_DX8IndexBuffer) {

            if (!byte_E1B2A4) {
                TheCurrentAllowCrashPtr = (int *)&byte_E1B2A4;
                DebugCrash("ReAcquire of existing index buffer");
                TheCurrentAllowCrashPtr = 0;
            }
        }

        ib->m_DX8IndexBuffer = new DX8IndexBufferClass(ib->m_size, DX8IndexBufferClass::USAGE_DEFAULT);

        // bfme2 ib->m_DX8IndexBuffer
        if (!ib->m_DX8IndexBuffer && !byte_E1B2A5) {
            TheCurrentAllowCrashPtr = (int *)&byte_E1B2A5;
            DebugCrash("Failed ReAcquire of index buffer");
            TheCurrentAllowCrashPtr = 0;
        }

        if (!ib->m_DX8IndexBuffer) {
            return false;
        }
    }

    return true;
}

W3DBufferManager::W3DVertexBufferSlot *W3DBufferManager::Get_Slot(VBM_FVF_TYPES fvf_type, int size)
{
    signed int a3a = (size + 31) & 0xFFFFFFE0;

    int v5 = (a3a >> 5) - 1;

    // bfme2 sizeIndex < MAX_VB_SIZES && size
    if ((v5 >= MAX_VB_SIZES || !a3a) && !byte_E1B2A6) {
        TheCurrentAllowCrashPtr = (int *)&byte_E1B2A6;
        DebugCrash("Allocating too large vertex buffer slot");
        TheCurrentAllowCrashPtr = 0;
    }

    W3DVertexBufferSlot *vb_slot = m_W3DVertexBufferSlots[fvf_type][v5];

    if (vb_slot) {
        m_W3DVertexBufferSlots[fvf_type][v5] = vb_slot->m_nextSameSize;

        if (vb_slot->m_nextSameSize) {
            vb_slot->m_nextSameSize->m_prevSameSize = nullptr;
        }
        return vb_slot;
    }

    return Allocate_Slot_Storage(fvf_type, a3a);
}

void W3DBufferManager::Release_Slot(W3DVertexBufferSlot *vb_slot)
{
    int v2 = (vb_slot->m_size >> 5) - 1;
    vb_slot->m_nextSameSize = m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][v2];

    if (m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][v2]) {
        m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][v2]->m_prevSameSize = vb_slot;
    }

    m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][v2] = vb_slot;
}

W3DBufferManager::W3DVertexBufferSlot *W3DBufferManager::Allocate_Slot_Storage(VBM_FVF_TYPES fvf_type, int size)
{
    W3DVertexBufferSlot *v4; // ST2C_4
    int v6; // [esp+4h] [ebp-28h]
    W3DVertexBuffer *vba; // [esp+18h] [ebp-14h]
    W3DVertexBuffer *vbb; // [esp+18h] [ebp-14h]
    W3DVertexBufferSlot *v12; // [esp+1Ch] [ebp-10h]

    // bfme2 m_numEmptySlotsAllocated < MAX_NUMBER_SLOTS
    if (m_numEmptySlotsAllocated >= MAX_NUMBER_SLOTS && !byte_E1B2A7) {
        TheCurrentAllowCrashPtr = (int *)&byte_E1B2A7;
        DebugCrash("Nore more VB Slots");
        TheCurrentAllowCrashPtr = 0;
    }

    for (W3DVertexBuffer *vb = m_W3DVertexBuffers[fvf_type]; vb; vb = vb->m_nextVB) {

        if (vb->m_size - vb->m_startFreeIndex >= size && m_numEmptySlotsAllocated < MAX_NUMBER_SLOTS) {
            v12 = &m_W3DVertexBufferEmptySlots[m_numEmptySlotsAllocated];
            v12->m_size = size;
            v12->m_start = vb->m_startFreeIndex;
            v12->m_VB = vb;
            v12->m_nextSameVB = vb->m_usedSlots;
            v12->m_prevSameVB = nullptr;
            if (vb->m_usedSlots) {
                vb->m_usedSlots->m_prevSameVB = v12;
            }
            v12->m_nextSameSize = nullptr;
            v12->m_prevSameSize = nullptr;
            vb->m_usedSlots = v12;
            vb->m_startFreeIndex += size;
            ++m_numEmptySlotsAllocated;
            return v12;
        }
    }

    vba = m_W3DVertexBuffers[fvf_type];

    // bfme2 m_numEmptyVertexBuffersAllocated < MAX_VERTEX_BUFFERS_CREATED
    if (m_numEmptyVertexBuffersAllocated >= MAX_VERTEX_BUFFERS_CREATED && !byte_E1B2A8) {
        TheCurrentAllowCrashPtr = (int *)&byte_E1B2A8;
        DebugCrash("Reached Max Static VB Shadow Geometry");
        TheCurrentAllowCrashPtr = 0;
    }

    if (m_numEmptyVertexBuffersAllocated < MAX_VERTEX_BUFFERS_CREATED) {
        m_W3DVertexBuffers[fvf_type] = &m_W3DEmptyVertexBuffers[m_numEmptyVertexBuffersAllocated];
        m_W3DVertexBuffers[fvf_type]->m_nextVB = vba;
        ++m_numEmptyVertexBuffersAllocated;
        vbb = m_W3DVertexBuffers[fvf_type];

        if (size < 8192) {
            v6 = 8192;
        } else {
            v6 = size;
        }

        vbb->m_DX8VertexBuffer =
            new DX8VertexBufferClass(Get_DX8_Format(fvf_type), (unsigned __int16)v6, DX8VertexBufferClass::USAGE_DEFAULT, 0);
        vbb->m_format = fvf_type;
        vbb->m_startFreeIndex = size;
        vbb->m_size = v6;
        v4 = &m_W3DVertexBufferEmptySlots[m_numEmptySlotsAllocated++];
        vbb->m_usedSlots = v4;
        v4->m_size = size;
        v4->m_start = 0;
        v4->m_VB = vbb;
        v4->m_nextSameVB = nullptr;
        v4->m_prevSameVB = nullptr;
        v4->m_nextSameSize = nullptr;
        v4->m_prevSameSize = nullptr;
        return v4;
    }

    return nullptr;
}

W3DBufferManager::W3DIndexBufferSlot *W3DBufferManager::Get_Slot(int size)
{
    signed int a2a = (size + 31) & 0xFFFFFFE0;
    int v5 = (a2a >> 5) - 1;

    // bfme2 sizeIndex < MAX_IB_SIZES && size
    if ((v5 >= MAX_IB_SIZES || !a2a) && !byte_E1B2A9) {
        TheCurrentAllowCrashPtr = (int *)&byte_E1B2A9;
        DebugCrash("Allocating too large index buffer slot");
        TheCurrentAllowCrashPtr = 0;
    }

    W3DIndexBufferSlot *ib_slot = m_W3DIndexBufferSlots[v5];

    if (ib_slot) {
        m_W3DIndexBufferSlots[v5] = ib_slot->m_nextSameSize;
        if (ib_slot->m_nextSameSize) {
            ib_slot->m_nextSameSize->m_prevSameSize = nullptr;
        }

        return ib_slot;
    }

    return Allocate_Slot_Storage(a2a);
}

void W3DBufferManager::Release_Slot(W3DIndexBufferSlot *ib_slot)
{
    int v2; // [esp+4h] [ebp-4h]

    v2 = (ib_slot->m_size >> 5) - 1;
    ib_slot->m_nextSameSize = m_W3DIndexBufferSlots[v2];

    if (m_W3DIndexBufferSlots[v2]) {
        m_W3DIndexBufferSlots[v2]->m_prevSameSize = ib_slot;
    }

    m_W3DIndexBufferSlots[v2] = ib_slot;
}

W3DBufferManager::W3DIndexBufferSlot *W3DBufferManager::Allocate_Slot_Storage(int size)
{
    W3DIndexBufferSlot *v3; // ST24_4
    int v5; // [esp+4h] [ebp-28h]
    W3DIndexBufferSlot *v8; // [esp+18h] [ebp-14h]
    W3DIndexBuffer *iba; // [esp+1Ch] [ebp-10h]
    W3DIndexBuffer *ibb; // [esp+1Ch] [ebp-10h]

    // bfme2 m_numEmptyIndexSlotsAllocated < MAX_NUMBER_SLOTS
    if (m_numEmptyIndexSlotsAllocated >= 0x1000 && !byte_E1B2AA) {
        TheCurrentAllowCrashPtr = (int *)&byte_E1B2AA;
        DebugCrash("Nore more IB Slots");
        TheCurrentAllowCrashPtr = 0;
    }

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib; ib = ib->m_nextIB) {

        if (ib->m_size - ib->m_startFreeIndex >= size && m_numEmptyIndexSlotsAllocated < MAX_NUMBER_SLOTS) {
            v8 = &m_W3DIndexBufferEmptySlots[m_numEmptyIndexSlotsAllocated];
            v8->m_size = size;
            v8->m_start = ib->m_startFreeIndex;
            v8->m_IB = ib;
            v8->m_nextSameIB = ib->m_usedSlots;
            v8->m_prevSameIB = nullptr;
            if (ib->m_usedSlots) {
                ib->m_usedSlots->m_prevSameIB = v8;
            }
            v8->m_nextSameSize = nullptr;
            v8->m_prevSameSize = nullptr;
            ib->m_usedSlots = v8;
            ib->m_startFreeIndex += size;
            ++m_numEmptyIndexSlotsAllocated;
            return v8;
        }
    }

    iba = m_W3DIndexBuffers;

    // bfme2 m_numEmptyIndexBuffersAllocated < MAX_INDEX_BUFFERS_CREATED
    if (m_numEmptyIndexBuffersAllocated >= MAX_INDEX_BUFFERS_CREATED && !byte_E1B2AB) {
        TheCurrentAllowCrashPtr = (int *)&byte_E1B2AB;
        DebugCrash("Reached Max Static IB Shadow Geometry");
        TheCurrentAllowCrashPtr = 0;
    }

    if (m_numEmptyIndexBuffersAllocated < MAX_INDEX_BUFFERS_CREATED) {
        m_W3DIndexBuffers = &m_W3DEmptyIndexBuffers[m_numEmptyIndexBuffersAllocated];
        m_W3DIndexBuffers->m_nextIB = iba;
        ++m_numEmptyIndexBuffersAllocated;
        ibb = m_W3DIndexBuffers;

        if (size < 32768) {
            v5 = 32768;
        } else {
            v5 = size;
        }

        ibb->m_DX8IndexBuffer = new DX8IndexBufferClass(v5, DX8IndexBufferClass::USAGE_DEFAULT);
        ibb->m_startFreeIndex = size;
        ibb->m_size = v5;
        v3 = &m_W3DIndexBufferEmptySlots[m_numEmptyIndexSlotsAllocated++];
        ibb->m_usedSlots = v3;
        v3->m_size = size;
        v3->m_start = 0;
        v3->m_IB = ibb;
        v3->m_nextSameIB = nullptr;
        v3->m_prevSameIB = nullptr;
        v3->m_nextSameSize = nullptr;
        v3->m_prevSameSize = nullptr;
        return v3;
    }

    return nullptr;
}

W3DBufferManager::W3DVertexBuffer *W3DBufferManager::Get_Next_Vertex_Buffer(W3DVertexBuffer *vb, VBM_FVF_TYPES fvf_type)
{
    if (vb == nullptr) {
        return m_W3DVertexBuffers[fvf_type];
    }

    return vb->m_nextVB;
}

W3DBufferManager::W3DIndexBuffer *W3DBufferManager::Get_Next_Index_Buffer(W3DIndexBuffer *ib)
{
    if (ib == nullptr) {
        return m_W3DIndexBuffers;
    }

    return ib->m_nextIB;
}
