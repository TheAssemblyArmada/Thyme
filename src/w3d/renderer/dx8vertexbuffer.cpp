/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief Vertex Buffer classes
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "dx8vertexbuffer.h"
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "w3d.h"

FVFInfoClass g_dynamicFVFInfo(DX8_FVF_XYZNDUV2, 0);
unsigned int g_vertexBufferCount;
unsigned int g_vertexBufferTotalVertices;
unsigned int g_vertexBufferTotalSize;
bool g_dynamicSortingVertexArrayInUse;
SortingVertexBufferClass *g_dynamicSortingVertexArray;
unsigned short g_dynamicSortingVertexArraySize;
unsigned short g_dynamicSortingVertexArrayOffset;
bool g_dynamicDX8VertexBufferInUse;
DX8VertexBufferClass *g_dynamicDX8VertexBuffer;
unsigned short g_dynamicDX8VertexBufferSize = 5000;
unsigned short g_dynamicDX8VertexBufferOffset;

VertexBufferClass::VertexBufferClass(
    unsigned int type_, unsigned int fvf, unsigned short vertex_count_, unsigned int vertex_size) :
    m_type(type_), m_vertexCount(vertex_count_), m_engineRefs(0)
{
    captainslog_assert(m_vertexCount);
    captainslog_assert(m_type == BUFFER_TYPE_DX8 || m_type == BUFFER_TYPE_SORTING);
    captainslog_assert((fvf != 0 && vertex_size == 0) || (fvf == 0 && vertex_size != 0));
    m_fvfInfo = new FVFInfoClass(fvf, vertex_size);
    g_vertexBufferCount++;
    g_vertexBufferTotalVertices += m_vertexCount;
    g_vertexBufferTotalSize += m_vertexCount * m_fvfInfo->Get_FVF_Size();
}

VertexBufferClass::~VertexBufferClass()
{
    g_vertexBufferCount--;
    g_vertexBufferTotalVertices -= m_vertexCount;
    g_vertexBufferTotalSize -= m_vertexCount * m_fvfInfo->Get_FVF_Size();

    if (m_fvfInfo) {
        delete m_fvfInfo;
    }
}

unsigned int VertexBufferClass::Get_Total_Buffer_Count()
{
    return g_vertexBufferCount;
}

unsigned int VertexBufferClass::Get_Total_Allocated_Indices()
{
    return g_vertexBufferTotalVertices;
}

unsigned int VertexBufferClass::Get_Total_Allocated_Memory()
{
    return g_vertexBufferTotalSize;
}

void VertexBufferClass::Add_Engine_Ref() const
{
    m_engineRefs++;
}

void VertexBufferClass::Release_Engine_Ref()
{
    m_engineRefs--;
    captainslog_assert(m_engineRefs >= 0);
}

VertexBufferClass::WriteLockClass::WriteLockClass(VertexBufferClass *vertex_buffer, int flags) :
    VertexBufferLockClass(vertex_buffer)
{
    captainslog_assert(vertex_buffer);
    captainslog_assert(!vertex_buffer->Engine_Refs());
    vertex_buffer->Add_Ref();

    switch (vertex_buffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(vertex_buffer)
                ->Get_DX8_Vertex_Buffer()
                ->Lock(0, 0, reinterpret_cast<BYTE **>(&m_vertices), flags);
#endif
            break;
        }
        case BUFFER_TYPE_SORTING: {
            m_vertices = static_cast<SortingVertexBufferClass *>(vertex_buffer)->Get_Sorting_Vertex_Buffer();
            break;
        }
        default:
            captainslog_assert(0);
            break;
    }
}

VertexBufferClass::WriteLockClass::~WriteLockClass()
{
    switch (m_vertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(m_vertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock();
#endif
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            captainslog_assert(0);
            break;
    }

    m_vertexBuffer->Release_Ref();
}

VertexBufferClass::AppendLockClass::AppendLockClass(
    VertexBufferClass *vertex_buffer, unsigned int start_index, unsigned int index_range) :
    VertexBufferLockClass(vertex_buffer)
{
    captainslog_assert(vertex_buffer);
    captainslog_assert(!vertex_buffer->Engine_Refs());
    captainslog_assert(start_index + index_range <= vertex_buffer->Get_Vertex_Count());
    vertex_buffer->Add_Ref();

    switch (vertex_buffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(vertex_buffer)
                ->Get_DX8_Vertex_Buffer()
                ->Lock(vertex_buffer->FVF_Info().Get_FVF_Size() * start_index,
                    vertex_buffer->FVF_Info().Get_FVF_Size() * index_range,
                    reinterpret_cast<BYTE **>(&m_vertices),
                    0);
#endif
            break;
        }
        case BUFFER_TYPE_SORTING: {
            m_vertices = static_cast<SortingVertexBufferClass *>(vertex_buffer)->Get_Sorting_Vertex_Buffer() + start_index;
            break;
        }
        default:
            captainslog_assert(0);
            break;
    }
}

VertexBufferClass::AppendLockClass::~AppendLockClass()
{
    switch (m_vertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(m_vertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock();
#endif
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            captainslog_assert(0);
            break;
    }

    m_vertexBuffer->Release_Ref();
}

SortingVertexBufferClass::SortingVertexBufferClass(unsigned short m_vertexCount) :
    VertexBufferClass(BUFFER_TYPE_SORTING, DX8_FVF_XYZNDUV2, m_vertexCount, 0)
{
    m_vertexBuffer = new VertexFormatXYZNDUV2[m_vertexCount];
}

SortingVertexBufferClass::~SortingVertexBufferClass()
{
    delete[] m_vertexBuffer;
}

DX8VertexBufferClass::DX8VertexBufferClass(
    unsigned int fvf, unsigned short vertex_count_, UsageType usage, unsigned int flags) :
    VertexBufferClass(BUFFER_TYPE_DX8, fvf, vertex_count_, flags)
#ifdef BUILD_WITH_D3D8
    ,
    m_vertexBuffer(nullptr)
#endif
{
    Create_Vertex_Buffer(usage);
}

DX8VertexBufferClass::~DX8VertexBufferClass()
{
#ifdef BUILD_WITH_D3D8
    m_vertexBuffer->Release();
#endif
}

void DX8VertexBufferClass::Create_Vertex_Buffer(UsageType usage)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(!m_vertexBuffer);
    int d3dusage = ((usage & USAGE_DYNAMIC) < 1 ? D3DUSAGE_WRITEONLY : D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY)
        | ((usage & USAGE_SOFTWAREPROCESSING) >= 1 ? D3DUSAGE_SOFTWAREPROCESSING : 0);

    if (!DX8Wrapper::Get_Current_Caps()->Use_Hardware_TnL()) {
        d3dusage |= D3DUSAGE_SOFTWAREPROCESSING;
    }

    HRESULT res;
    DX8CALL_HRES(CreateVertexBuffer(m_fvfInfo->Get_FVF_Size() * m_vertexCount,
                     d3dusage,
                     m_fvfInfo->Get_FVF(),
                     (D3DPOOL)(unsigned __int8)(usage & 1 ^ 1),
                     &m_vertexBuffer),
        res);

    if (FAILED(res)) {
        captainslog_warn("Vertex buffer creation failed, trying to release assets...");
        TextureBaseClass::Invalidate_Old_Unused_Textures(5000);
        W3D::Invalidate_Mesh_Cache();
        DX8CALL(ResourceManagerDiscardBytes(0));
        DX8CALL_HRES(CreateVertexBuffer(m_fvfInfo->Get_FVF_Size() * m_vertexCount,
                         d3dusage,
                         m_fvfInfo->Get_FVF(),
                         (D3DPOOL)(unsigned __int8)(usage & 1 ^ 1),
                         &m_vertexBuffer),
            res);
        captainslog_warn("...Vertex buffer creation succesful");
    }
#endif
}

DynamicVBAccessClass::DynamicVBAccessClass(unsigned int t, unsigned int fvf, unsigned short vertex_count_) :
    m_fvfInfo(g_dynamicFVFInfo), m_type(t), m_vertexCount(vertex_count_), m_vertexBuffer(nullptr)
{
    captainslog_assert(fvf == dynamic_fvf_type);
    captainslog_assert(
        m_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8 || m_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING);

    if (m_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        Allocate_DX8_Dynamic_Buffer();
    } else {
        Allocate_Sorting_Dynamic_Buffer();
    }
}

DynamicVBAccessClass::~DynamicVBAccessClass()
{
    if (m_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        g_dynamicDX8VertexBufferInUse = false;
        g_dynamicDX8VertexBufferOffset += m_vertexCount;
    } else {
        g_dynamicSortingVertexArrayInUse = 0;
        g_dynamicSortingVertexArrayOffset += m_vertexCount;
    }

    Ref_Ptr_Release(m_vertexBuffer);
}

void DynamicVBAccessClass::Deinit()
{
    if (g_dynamicDX8VertexBuffer) {
        captainslog_assert((g_dynamicDX8VertexBuffer == NULL) || (g_dynamicDX8VertexBuffer->Num_Refs() == 1));
        g_dynamicDX8VertexBuffer->Release_Ref();
    }

    g_dynamicDX8VertexBuffer = nullptr;
    g_dynamicDX8VertexBufferInUse = false;
    g_dynamicDX8VertexBufferSize = 5000;
    g_dynamicDX8VertexBufferOffset = 0;

    if (g_dynamicSortingVertexArray) {
        captainslog_assert((g_dynamicSortingVertexArray == NULL) || (g_dynamicSortingVertexArray->Num_Refs() == 1));
        g_dynamicSortingVertexArray->Release_Ref();
    }

    g_dynamicSortingVertexArray = nullptr;
    captainslog_assert(!g_dynamicSortingVertexArrayInUse);
    g_dynamicSortingVertexArrayInUse = false;
    g_dynamicSortingVertexArraySize = 0;
    g_dynamicSortingVertexArrayOffset = 0;
}

void DynamicVBAccessClass::Allocate_DX8_Dynamic_Buffer()
{
    captainslog_assert(!g_dynamicDX8VertexBufferInUse);
    g_dynamicDX8VertexBufferInUse = true;

    if (m_vertexCount > g_dynamicDX8VertexBufferSize) {
        Ref_Ptr_Release(g_dynamicDX8VertexBuffer);
        g_dynamicDX8VertexBufferSize = 5000;
        if (m_vertexCount > 4999) {
            g_dynamicDX8VertexBufferSize = m_vertexCount;
        }
    }

    if (!g_dynamicDX8VertexBuffer) {
        DX8VertexBufferClass::UsageType usage = DX8VertexBufferClass::USAGE_DYNAMIC;
        g_dynamicDX8VertexBuffer = new DX8VertexBufferClass(DX8_FVF_XYZNDUV2, g_dynamicDX8VertexBufferSize, usage, 0);
        g_dynamicDX8VertexBufferOffset = 0;
    }

    if (g_dynamicDX8VertexBufferOffset + m_vertexCount > g_dynamicDX8VertexBufferSize) {
        g_dynamicDX8VertexBufferOffset = 0;
    }

    g_dynamicDX8VertexBuffer->Add_Ref();
    Ref_Ptr_Release(m_vertexBuffer);
    m_vertexBuffer = g_dynamicDX8VertexBuffer;
    m_vertexBufferOffset = g_dynamicDX8VertexBufferOffset;
}

void DynamicVBAccessClass::Allocate_Sorting_Dynamic_Buffer()
{
    captainslog_assert(!g_dynamicSortingVertexArrayInUse);
    g_dynamicSortingVertexArrayInUse = true;
    int new_vertex_count = g_dynamicSortingVertexArrayOffset + m_vertexCount;
    captainslog_assert(new_vertex_count < 65536);

    if (new_vertex_count > g_dynamicSortingVertexArraySize) {
        Ref_Ptr_Release(g_dynamicSortingVertexArray);
        g_dynamicSortingVertexArraySize = 5000;
        if (new_vertex_count > 4999) {
            g_dynamicSortingVertexArraySize = new_vertex_count;
        }
    }

    if (!g_dynamicSortingVertexArray) {
        g_dynamicSortingVertexArray = new SortingVertexBufferClass(g_dynamicSortingVertexArraySize);
        g_dynamicSortingVertexArrayOffset = 0;
    }

    g_dynamicSortingVertexArray->Add_Ref();
    Ref_Ptr_Release(m_vertexBuffer);
    m_vertexBuffer = g_dynamicSortingVertexArray;
    m_vertexBufferOffset = g_dynamicSortingVertexArrayOffset;
}

DynamicVBAccessClass::WriteLockClass::WriteLockClass(DynamicVBAccessClass *dynamic_vb_access_) :
    m_dynamicVBAccess(dynamic_vb_access_)
{
    if (m_dynamicVBAccess->m_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        captainslog_assert(g_dynamicDX8VertexBuffer);
        m_vertices = nullptr;
#ifdef BUILD_WITH_D3D8
        DX8VertexBufferClass *buffer = static_cast<DX8VertexBufferClass *>(m_dynamicVBAccess->m_vertexBuffer);
        buffer->Get_DX8_Vertex_Buffer()->Lock(
            g_dynamicDX8VertexBuffer->FVF_Info().Get_FVF_Size() * m_dynamicVBAccess->m_vertexBufferOffset,
            buffer->FVF_Info().Get_FVF_Size() * m_dynamicVBAccess->m_vertexCount,
            reinterpret_cast<BYTE **>(&m_vertices),
            (m_dynamicVBAccess->m_vertexBufferOffset != 0 ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD) | D3DLOCK_NOSYSLOCK);
#endif
    } else if (m_dynamicVBAccess->m_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING) {
        SortingVertexBufferClass *buffer = static_cast<SortingVertexBufferClass *>(m_dynamicVBAccess->m_vertexBuffer);
        m_vertices = buffer->Get_Sorting_Vertex_Buffer() + m_dynamicVBAccess->m_vertexBufferOffset;
    } else {
        captainslog_assert(0);
        m_vertices = nullptr;
    }
}

DynamicVBAccessClass::WriteLockClass::~WriteLockClass()
{
    if (m_dynamicVBAccess->m_type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
#ifdef BUILD_WITH_D3D8
        DX8VertexBufferClass *buffer = static_cast<DX8VertexBufferClass *>(m_dynamicVBAccess->m_vertexBuffer);
        buffer->Get_DX8_Vertex_Buffer()->Unlock();
#endif
    } else if (m_dynamicVBAccess->m_type != VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING) {
        captainslog_assert(0);
    }
}

void DynamicVBAccessClass::Reset(bool frame_changed)
{
    g_dynamicSortingVertexArrayOffset = 0;

    if (frame_changed) {
        g_dynamicDX8VertexBufferOffset = 0;
    }
}

unsigned short DynamicVBAccessClass::Get_Default_Vertex_Count()
{
    return g_dynamicDX8VertexBufferSize;
}

// unimplemented, not used
// DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector3 *normals, Vector2 *tex_coords, unsigned short
// VertexCount, UsageType usage) DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector3 *normals, Vector4
// *diffuse, Vector2 *tex_coords, unsigned short VertexCount, UsageType usage)
// DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector4 *diffuse, Vector2 *tex_coords, unsigned short
// VertexCount, UsageType usage) DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector2 *tex_coords, unsigned
// short VertexCount, UsageType usage) void DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, Vector2 *uv, unsigned int
// first_vertex, unsigned int count) void DX8VertexBufferClass::Copy(Vector3 *loc, unsigned int first_vertex, unsigned int
// count) void DX8VertexBufferClass::Copy(Vector3 *loc, Vector2 *uv, unsigned int first_vertex, unsigned int count) void
// DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, unsigned int first_vertex, unsigned int count) void
// DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex, unsigned
// int count) void DX8VertexBufferClass::Copy(Vector3 *loc, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex,
// unsigned int count)
