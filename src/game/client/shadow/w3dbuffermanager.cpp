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
#include <algorithm>

// TODO need general cleanup advice
// TODO what is going on with count

static const int FVFTypeIndexList[W3DBufferManager::MAX_FVF] = {
    DX8_FVF_XYZ,
    DX8_FVF_XYZD,

    DX8_FVF_XYZUV1,
    DX8_FVF_XYZDUV1,

    DX8_FVF_XYZUV2,
    DX8_FVF_XYZDUV2,

    DX8_FVF_XYZN,
    DX8_FVF_XYZND,

    DX8_FVF_XYZNUV1,
    DX8_FVF_XYZNDUV1,

    DX8_FVF_XYZNUV2,
    DX8_FVF_XYZNDUV2,

    DX8_FVF_XYZRHW,
    DX8_FVF_XYZRHWD,

    DX8_FVF_XYZRHWUV1,
    DX8_FVF_XYZRHWDUV1,

    DX8_FVF_XYZRHWUV2,
    DX8_FVF_XYZRHWDUV2,
};

int W3DBufferManager::Get_DX8_Format(VBM_FVF_TYPES fvf_type)
{
    return FVFTypeIndexList[fvf_type];
}

W3DBufferManager::W3DBufferManager()
{
    m_numEmptyVertexSlotsAllocated = 0;
    m_numEmptyVertexBuffersAllocated = 0;
    m_W3DIndexBuffers = nullptr;
    m_numEmptyIndexSlotsAllocated = 0;
    m_numEmptyIndexBuffersAllocated = 0;

    for (auto &vertexBuffer : m_W3DVertexBuffers) {
        vertexBuffer = nullptr;
    }

    for (auto &vertexBufferSlot : m_W3DVertexBufferSlots) {
        for (int j = 0; j < MAX_VB_SIZES; ++j) {
            vertexBufferSlot[j] = nullptr;
        }
    }

    // bugfix from BFME2, clear the buffers
    for (auto &vertexBufferSlot : m_W3DVertexBufferEmptySlots) {
        vertexBufferSlot.m_size = 0;
        vertexBufferSlot.m_start = 0;
        vertexBufferSlot.m_VB = nullptr;
        vertexBufferSlot.m_prevSameSize = nullptr;
        vertexBufferSlot.m_nextSameSize = nullptr;
        vertexBufferSlot.m_prevSameVB = nullptr;
        vertexBufferSlot.m_nextSameVB = nullptr;
    }

    for (auto &vertexBuffer : m_W3DEmptyVertexBuffers) {
        vertexBuffer.m_format = VBM_FVF_XYZ;
        vertexBuffer.m_usedSlots = nullptr;
        vertexBuffer.m_startFreeIndex = 0;
        vertexBuffer.m_size = 0;
        vertexBuffer.m_nextVB = nullptr;
        vertexBuffer.m_DX8VertexBuffer = nullptr;
        vertexBuffer.m_renderTaskList = nullptr;
    }

    for (auto &indexBufferSlot : m_W3DIndexBufferSlots) {
        indexBufferSlot = nullptr;
    }

    // bugfix from BFME2, clear the buffers
    for (auto &indexBufferSlot : m_W3DIndexBufferEmptySlots) {
        indexBufferSlot.m_size = 0;
        indexBufferSlot.m_start = 0;
        indexBufferSlot.m_IB = nullptr;
        indexBufferSlot.m_prevSameSize = nullptr;
        indexBufferSlot.m_nextSameSize = nullptr;
        indexBufferSlot.m_prevSameIB = nullptr;
        indexBufferSlot.m_nextSameIB = nullptr;
    }

    for (auto &indexBuffer : m_W3DEmptyIndexBuffers) {
        indexBuffer.m_usedSlots = nullptr;
        indexBuffer.m_startFreeIndex = 0;
        indexBuffer.m_size = 0;
        indexBuffer.m_nextIB = nullptr;
        indexBuffer.m_DX8IndexBuffer = nullptr;
    }
}

W3DBufferManager::~W3DBufferManager()
{
    Free_All_Slots();
    Free_All_Buffers();
}

void W3DBufferManager::Free_All_Slots()
{
    for (auto &vertexBufferSlot : m_W3DVertexBufferSlots) {

        for (int j = 0; j < MAX_VB_SIZES; ++j) {
            W3DVertexBufferSlot *vb_slot = vertexBufferSlot[j];
            while (vb_slot != nullptr) {
                if (vb_slot->m_prevSameVB != nullptr) {
                    vb_slot->m_prevSameVB->m_nextSameVB = vb_slot->m_nextSameVB;
                } else {
                    vb_slot->m_VB->m_usedSlots = nullptr;
                }

                if (vb_slot->m_nextSameVB != nullptr) {
                    vb_slot->m_nextSameVB->m_prevSameVB = vb_slot->m_prevSameVB;
                }

                vb_slot = vb_slot->m_nextSameSize;
                --m_numEmptyVertexSlotsAllocated;
            }

            vertexBufferSlot[j] = nullptr;
        }
    }
    for (auto &indexBufferSlot : m_W3DIndexBufferSlots) {
        W3DIndexBufferSlot *ib_slot = indexBufferSlot;
        while (ib_slot != nullptr) {
            if (ib_slot->m_prevSameIB != nullptr) {
                ib_slot->m_prevSameIB->m_nextSameIB = ib_slot->m_nextSameIB;
            } else {
                ib_slot->m_IB->m_usedSlots = nullptr;
            }

            if (ib_slot->m_nextSameIB != nullptr) {
                ib_slot->m_nextSameIB->m_prevSameIB = ib_slot->m_prevSameIB;
            }

            ib_slot = ib_slot->m_nextSameSize;
            --m_numEmptyIndexSlotsAllocated;
        }

        indexBufferSlot = nullptr;
    }

    captainslog_dbgassert(m_numEmptyVertexSlotsAllocated == 0, "Failed to free all empty vertex buffer slots");
    captainslog_dbgassert(m_numEmptyIndexSlotsAllocated == 0, "Failed to free all empty index buffer slots");
}

void W3DBufferManager::Free_All_Buffers()
{
    Free_All_Slots();

    for (auto &m_W3DVertexBuffer : m_W3DVertexBuffers) {
        for (W3DVertexBuffer *vb = m_W3DVertexBuffer; vb != nullptr; vb = vb->m_nextVB) {
            captainslog_dbgassert(vb->m_usedSlots == nullptr, "Freeing Non-Empty Vertex Buffer");

            Ref_Ptr_Release(vb->m_DX8VertexBuffer);

            --m_numEmptyVertexBuffersAllocated;
        }

        m_W3DVertexBuffer = nullptr;
    }

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib != nullptr; ib = ib->m_nextIB) {
        captainslog_dbgassert(ib->m_usedSlots == nullptr, "Freeing Non-Empty Index Buffer");

        Ref_Ptr_Release(ib->m_DX8IndexBuffer);

        --m_numEmptyIndexBuffersAllocated;
    }

    m_W3DIndexBuffers = nullptr;

    captainslog_dbgassert(m_numEmptyVertexBuffersAllocated == 0, "Failed to free all empty vertex buffers");
    captainslog_dbgassert(m_numEmptyIndexBuffersAllocated == 0, "Failed to free all empty index buffers");
}

void W3DBufferManager::Release_Resources()
{
    for (int i = 0; i < MAX_FVF; ++i) {
        for (W3DVertexBuffer *vb = m_W3DVertexBuffers[i]; vb != nullptr; vb = vb->m_nextVB) {
            Ref_Ptr_Release(vb->m_DX8VertexBuffer);
        }
    }

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib != nullptr; ib = ib->m_nextIB) {
        Ref_Ptr_Release(ib->m_DX8IndexBuffer);
    }
}

bool W3DBufferManager::ReAcquire_Resources()
{
    for (int i = 0; i < MAX_FVF; ++i) {
        for (W3DVertexBuffer *vb = m_W3DVertexBuffers[i]; vb != nullptr; vb = vb->m_nextVB) {

            captainslog_dbgassert(vb->m_DX8VertexBuffer == nullptr, "ReAcquire of existing vertex buffer");

            vb->m_DX8VertexBuffer =
                new DX8VertexBufferClass(Get_DX8_Format(vb->m_format), vb->m_size, DX8VertexBufferClass::USAGE_DEFAULT, 0);

            captainslog_dbgassert(vb->m_DX8VertexBuffer, "Failed ReAcquire of vertex buffer");

            if (vb->m_DX8VertexBuffer == nullptr) {
                return false;
            }
        }
    }

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib != nullptr; ib = ib->m_nextIB) {

        captainslog_dbgassert(ib->m_DX8IndexBuffer == nullptr, "ReAcquire of existing index buffer");

        ib->m_DX8IndexBuffer = new DX8IndexBufferClass(ib->m_size, DX8IndexBufferClass::USAGE_DEFAULT);

        captainslog_dbgassert(ib->m_DX8IndexBuffer, "Failed ReAcquire of index buffer");

        if (ib->m_DX8IndexBuffer == nullptr) {
            return false;
        }
    }

    return true;
}

W3DBufferManager::W3DVertexBufferSlot *W3DBufferManager::Get_Slot(VBM_FVF_TYPES fvf_type, int size)
{
    size = (size + 31) & ~31;
    int size_index = (size / 32) - 1;

    captainslog_dbgassert(size_index < MAX_IB_SIZES && size, "Allocating too large vertex buffer slot");

    W3DVertexBufferSlot *vb_slot = m_W3DVertexBufferSlots[fvf_type][size_index];

    if (vb_slot != nullptr) {
        m_W3DVertexBufferSlots[fvf_type][size_index] = vb_slot->m_nextSameSize;

        if (vb_slot->m_nextSameSize != nullptr) {
            vb_slot->m_nextSameSize->m_prevSameSize = nullptr;
        }

        return vb_slot;
    }

    return Allocate_Slot_Storage(fvf_type, size);
}

void W3DBufferManager::Release_Slot(W3DVertexBufferSlot *vb_slot)
{
    int size_index = (vb_slot->m_size / 32) - 1;

    vb_slot->m_nextSameSize = m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][size_index];

    if (m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][size_index] != nullptr) {
        m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][size_index]->m_prevSameSize = vb_slot;
    }

    m_W3DVertexBufferSlots[vb_slot->m_VB->m_format][size_index] = vb_slot;
}

W3DBufferManager::W3DVertexBufferSlot *W3DBufferManager::Allocate_Slot_Storage(VBM_FVF_TYPES fvf_type, int size)
{
    captainslog_dbgassert(m_numEmptyVertexSlotsAllocated < MAX_NUMBER_SLOTS, "Nore more VB Slots");

    for (W3DVertexBuffer *vb = m_W3DVertexBuffers[fvf_type]; vb != nullptr; vb = vb->m_nextVB) {

        if (vb->m_size - vb->m_startFreeIndex >= size && m_numEmptyVertexSlotsAllocated < MAX_NUMBER_SLOTS) {
            W3DVertexBufferSlot *slot = &m_W3DVertexBufferEmptySlots[m_numEmptyVertexSlotsAllocated];

            slot->m_size = size;
            slot->m_start = vb->m_startFreeIndex;
            slot->m_VB = vb;
            slot->m_nextSameVB = vb->m_usedSlots;
            slot->m_prevSameVB = nullptr;

            if (vb->m_usedSlots != nullptr) {
                vb->m_usedSlots->m_prevSameVB = slot;
            }

            slot->m_nextSameSize = nullptr;
            slot->m_prevSameSize = nullptr;
            vb->m_usedSlots = slot;
            vb->m_startFreeIndex += size;
            ++m_numEmptyVertexSlotsAllocated;

            return slot;
        }
    }

    captainslog_dbgassert(
        m_numEmptyVertexBuffersAllocated < MAX_VERTEX_BUFFERS_CREATED, "Reached Max Static VB Shadow Geometry");

    if (m_numEmptyVertexBuffersAllocated < MAX_VERTEX_BUFFERS_CREATED) {
        W3DVertexBuffer *cur = m_W3DVertexBuffers[fvf_type];

        m_W3DVertexBuffers[fvf_type] = &m_W3DEmptyVertexBuffers[m_numEmptyVertexBuffersAllocated];
        m_W3DVertexBuffers[fvf_type]->m_nextVB = cur;
        ++m_numEmptyVertexBuffersAllocated;

        W3DVertexBuffer *buf = m_W3DVertexBuffers[fvf_type];

        // TODO investigate
        // We allocate count for vertex buffer but size and start index is set by the original size
        // As it stands this doesn't seem to make any sense, i'd understand a 4 byte alignement on the buffer but this...
        int vb_size = std::max(8192, size);

        buf->m_DX8VertexBuffer =
            new DX8VertexBufferClass(Get_DX8_Format(fvf_type), vb_size, DX8VertexBufferClass::USAGE_DEFAULT, 0);
        buf->m_format = fvf_type;
        buf->m_startFreeIndex = size;
        buf->m_size = vb_size;
        // BUGFIX Original didn't clear this
        buf->m_renderTaskList = nullptr;

        W3DVertexBufferSlot *slot = &m_W3DVertexBufferEmptySlots[m_numEmptyVertexSlotsAllocated++];
        buf->m_usedSlots = slot;

        slot->m_size = size;
        slot->m_start = 0;
        slot->m_VB = buf;
        slot->m_nextSameVB = nullptr;
        slot->m_prevSameVB = nullptr;
        slot->m_nextSameSize = nullptr;
        slot->m_prevSameSize = nullptr;

        return slot;
    }

    return nullptr;
}

W3DBufferManager::W3DIndexBufferSlot *W3DBufferManager::Get_Slot(int size)
{
    size = (size + 31) & ~31;
    int size_index = (size / 32) - 1;

    captainslog_dbgassert(size_index < MAX_IB_SIZES && size, "Allocating too large index buffer slot");

    W3DIndexBufferSlot *ib_slot = m_W3DIndexBufferSlots[size_index];

    if (ib_slot != nullptr) {
        m_W3DIndexBufferSlots[size_index] = ib_slot->m_nextSameSize;
        if (ib_slot->m_nextSameSize != nullptr) {
            ib_slot->m_nextSameSize->m_prevSameSize = nullptr;
        }

        return ib_slot;
    }

    return Allocate_Slot_Storage(size);
}

void W3DBufferManager::Release_Slot(W3DIndexBufferSlot *ib_slot)
{
    int size_index = (ib_slot->m_size / 32) - 1;

    ib_slot->m_nextSameSize = m_W3DIndexBufferSlots[size_index];

    if (m_W3DIndexBufferSlots[size_index] != nullptr) {
        m_W3DIndexBufferSlots[size_index]->m_prevSameSize = ib_slot;
    }

    m_W3DIndexBufferSlots[size_index] = ib_slot;
}

W3DBufferManager::W3DIndexBufferSlot *W3DBufferManager::Allocate_Slot_Storage(int size)
{
    captainslog_dbgassert(m_numEmptyIndexSlotsAllocated < MAX_NUMBER_SLOTS, "Nore more IB Slots");

    for (W3DIndexBuffer *ib = m_W3DIndexBuffers; ib != nullptr; ib = ib->m_nextIB) {

        if (ib->m_size - ib->m_startFreeIndex >= size && m_numEmptyIndexSlotsAllocated < MAX_NUMBER_SLOTS) {
            W3DIndexBufferSlot *slot = &m_W3DIndexBufferEmptySlots[m_numEmptyIndexSlotsAllocated];

            slot->m_size = size;
            slot->m_start = ib->m_startFreeIndex;
            slot->m_IB = ib;
            slot->m_nextSameIB = ib->m_usedSlots;
            slot->m_prevSameIB = nullptr;

            if (ib->m_usedSlots != nullptr) {
                ib->m_usedSlots->m_prevSameIB = slot;
            }

            slot->m_nextSameSize = nullptr;
            slot->m_prevSameSize = nullptr;
            ib->m_usedSlots = slot;
            ib->m_startFreeIndex += size;
            ++m_numEmptyIndexSlotsAllocated;

            return slot;
        }
    }

    captainslog_dbgassert(
        m_numEmptyIndexBuffersAllocated < MAX_INDEX_BUFFERS_CREATED, "Reached Max Static IB Shadow Geometry");

    if (m_numEmptyIndexBuffersAllocated < MAX_INDEX_BUFFERS_CREATED) {
        W3DIndexBuffer *cur = m_W3DIndexBuffers;

        m_W3DIndexBuffers = &m_W3DEmptyIndexBuffers[m_numEmptyIndexBuffersAllocated];
        m_W3DIndexBuffers->m_nextIB = cur;
        ++m_numEmptyIndexBuffersAllocated;

        W3DIndexBuffer *buf = m_W3DIndexBuffers;

        // TODO investigate
        // We allocate count for vertex buffer but size and start index is set by the original size
        // As it stands this doesn't seem to make any sense, i'd understand a 4 byte alignement on the buffer but this...
        int ib_size = std::max(32768, size);

        buf->m_DX8IndexBuffer = new DX8IndexBufferClass(ib_size, DX8IndexBufferClass::USAGE_DEFAULT);
        buf->m_startFreeIndex = size;
        buf->m_size = ib_size;

        W3DIndexBufferSlot *slot = &m_W3DIndexBufferEmptySlots[m_numEmptyIndexSlotsAllocated++];
        buf->m_usedSlots = slot;

        slot->m_size = size;
        slot->m_start = 0;
        slot->m_IB = buf;
        slot->m_nextSameIB = nullptr;
        slot->m_prevSameIB = nullptr;
        slot->m_nextSameSize = nullptr;
        slot->m_prevSameSize = nullptr;

        return slot;
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
