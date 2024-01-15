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

class DX8VertexBufferClass;
class DX8IndexBufferClass;

class W3DBufferManager
{
public:
    enum VBM_FVF_TYPES
    {
        VBM_FVF_XYZ,
        VBM_FVF_XYZD,

        VBM_FVF_XYZUV1,
        VBM_FVF_XYZDUV1,

        VBM_FVF_XYZUV2,
        VBM_FVF_XYZDUV2,

        VBM_FVF_XYZN,
        VBM_FVF_XYZND,

        VBM_FVF_XYZNUV1,
        VBM_FVF_XYZNDUV1,

        VBM_FVF_XYZNUV2,
        VBM_FVF_XYZNDUV2,

        VBM_FVF_XYZRHW,
        VBM_FVF_XYZRHWD,

        VBM_FVF_XYZRHWUV1,
        VBM_FVF_XYZRHWDUV1,

        VBM_FVF_XYZRHWUV2,
        VBM_FVF_XYZRHWDUV2,

        MAX_FVF
    };

    struct W3DVertexBufferSlot;

    struct W3DRenderTask
    {
        W3DRenderTask *m_nextTask;
    };

    struct W3DVertexBuffer
    {
        VBM_FVF_TYPES m_format;
        W3DVertexBufferSlot *m_usedSlots;
        int m_startFreeIndex;
        int m_size;
        W3DVertexBuffer *m_nextVB;
        DX8VertexBufferClass *m_DX8VertexBuffer;
        W3DRenderTask *m_renderTaskList;
    };

    struct W3DVertexBufferSlot
    {
        int m_size;
        int m_start;
        W3DVertexBuffer *m_VB;
        W3DVertexBufferSlot *m_prevSameSize;
        W3DVertexBufferSlot *m_nextSameSize;
        W3DVertexBufferSlot *m_prevSameVB;
        W3DVertexBufferSlot *m_nextSameVB;
    };

    struct W3DIndexBufferSlot;

    struct W3DIndexBuffer
    {
        W3DIndexBufferSlot *m_usedSlots;
        int m_startFreeIndex;
        int m_size;
        W3DIndexBuffer *m_nextIB;
        DX8IndexBufferClass *m_DX8IndexBuffer;
    };

    struct W3DIndexBufferSlot
    {
        int m_size;
        int m_start;
        W3DIndexBuffer *m_IB;
        W3DIndexBufferSlot *m_prevSameSize;
        W3DIndexBufferSlot *m_nextSameSize;
        W3DIndexBufferSlot *m_prevSameIB;
        W3DIndexBufferSlot *m_nextSameIB;
    };

    enum
    {
        MAX_NUMBER_SLOTS = 4096,

        // Don't change these two
        // Changing these breaks shadows so something elsewhere relies on them
        MAX_VB_SIZES = 128,
        MAX_IB_SIZES = 128,

        MAX_VERTEX_BUFFERS_CREATED = 32,
        MAX_INDEX_BUFFERS_CREATED = 32,
    };

    W3DBufferManager();
    ~W3DBufferManager();

    void Free_All_Slots();
    void Free_All_Buffers();

    void Release_Resources();
    bool ReAcquire_Resources();

    W3DVertexBufferSlot *Get_Slot(VBM_FVF_TYPES fvf_type, int size);
    void Release_Slot(W3DVertexBufferSlot *vb_slot);
    W3DVertexBufferSlot *Allocate_Slot_Storage(VBM_FVF_TYPES fvf_type, int size);

    W3DIndexBufferSlot *Get_Slot(int size);
    void Release_Slot(W3DIndexBufferSlot *ib_slot);
    W3DIndexBufferSlot *Allocate_Slot_Storage(int size);

    W3DVertexBuffer *Get_Next_Vertex_Buffer(W3DVertexBuffer *vb, VBM_FVF_TYPES fvf_type);
    W3DIndexBuffer *Get_Next_Index_Buffer(W3DIndexBuffer *ib);

    static int Get_DX8_Format(VBM_FVF_TYPES fvf_type);

#ifdef GAME_DLL
    W3DBufferManager *Hook_Ctor() { return new (this) W3DBufferManager; }
    void Hook_Dtor() { W3DBufferManager::~W3DBufferManager(); }

    W3DVertexBufferSlot *Hook_VB_Get_Slot(VBM_FVF_TYPES fvf_type, int size) { return Get_Slot(fvf_type, size); }
    void Hook_VB_Release_Slot(W3DVertexBufferSlot *vb_slot) { Release_Slot(vb_slot); }
    W3DVertexBufferSlot *Hook_VB_Allocate_Slot_Storage(VBM_FVF_TYPES fvf_type, int size)
    {
        return Allocate_Slot_Storage(fvf_type, size);
    }

    W3DIndexBufferSlot *Hook_IB_Get_Slot(int size) { return Get_Slot(size); }
    void Hook_IB_Release_Slot(W3DIndexBufferSlot *ib_slot) { Release_Slot(ib_slot); }
    W3DIndexBufferSlot *Hook_IB_Allocate_Slot_Storage(int size) { return Allocate_Slot_Storage(size); }
#endif

private:
    W3DVertexBufferSlot *m_W3DVertexBufferSlots[MAX_FVF][MAX_VB_SIZES];
    W3DVertexBuffer *m_W3DVertexBuffers[MAX_FVF];

    W3DVertexBufferSlot m_W3DVertexBufferEmptySlots[MAX_NUMBER_SLOTS];
    int m_numEmptyVertexSlotsAllocated;
    W3DVertexBuffer m_W3DEmptyVertexBuffers[MAX_VERTEX_BUFFERS_CREATED];
    int m_numEmptyVertexBuffersAllocated;

    W3DIndexBufferSlot *m_W3DIndexBufferSlots[MAX_IB_SIZES];
    W3DIndexBuffer *m_W3DIndexBuffers;

    W3DIndexBufferSlot m_W3DIndexBufferEmptySlots[MAX_NUMBER_SLOTS];
    int m_numEmptyIndexSlotsAllocated;
    W3DIndexBuffer m_W3DEmptyIndexBuffers[MAX_INDEX_BUFFERS_CREATED];
    int m_numEmptyIndexBuffersAllocated;
};

#ifdef GAME_DLL
extern W3DBufferManager *&g_theW3DBufferManager;
#else
extern W3DBufferManager *g_theW3DBufferManager;
#endif
