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
#include "w3dtypes.h"
#include "dx8vertexbuffer.h"
#include "dx8fvf.h"
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "w3d.h"
FVFInfoClass _DynamicFVFInfo(DX8_FVF_XYZNDUV2, 0);
#define dynamic_fvf_type DX8_FVF_XYZNDUV2
unsigned int _VertexBufferCount;
unsigned int _VertexBufferTotalVertices;
unsigned int _VertexBufferTotalSize;
bool _DynamicSortingVertexArrayInUse;
SortingVertexBufferClass *_DynamicSortingVertexArray;
unsigned short _DynamicSortingVertexArraySize;
unsigned short _DynamicSortingVertexArrayOffset;
bool _DynamicDX8VertexBufferInUse;
DX8VertexBufferClass *_DynamicDX8VertexBuffer;
unsigned short _DynamicDX8VertexBufferSize = 5000;
unsigned short _DynamicDX8VertexBufferOffset;

VertexBufferClass::VertexBufferClass(
    unsigned int type_, unsigned int FVF, unsigned short vertex_count_, unsigned int vertex_size) :
    type(type_), VertexCount(vertex_count_), engine_refs(0)
{
    captainslog_assert(VertexCount);
    captainslog_assert(type == BUFFER_TYPE_DX8 || type == BUFFER_TYPE_SORTING);
    captainslog_assert((FVF!=0 && vertex_size==0) || (FVF==0 && vertex_size!=0));
    fvf_info = new FVFInfoClass(FVF, vertex_size);
    _VertexBufferCount++;
    _VertexBufferTotalVertices += VertexCount;
    _VertexBufferTotalSize += VertexCount * fvf_info->Get_FVF_Size();
}

VertexBufferClass::~VertexBufferClass()
{
    _VertexBufferCount--;
    _VertexBufferTotalVertices -= VertexCount;
    _VertexBufferTotalSize -= VertexCount * fvf_info->Get_FVF_Size();
    if (fvf_info) {
        delete fvf_info;
    }
}

unsigned int VertexBufferClass::Get_Total_Buffer_Count()
{
    return _VertexBufferCount;
}

unsigned int VertexBufferClass::Get_Total_Allocated_Indices()
{
    return _VertexBufferTotalVertices;
}

unsigned int VertexBufferClass::Get_Total_Allocated_Memory()
{
    return _VertexBufferTotalSize;
}

void VertexBufferClass::Add_Engine_Ref()
{
    engine_refs++;
}

void VertexBufferClass::Release_Engine_Ref()
{
    engine_refs--;
    captainslog_assert(engine_refs >= 0);
}

VertexBufferClass::WriteLockClass::WriteLockClass(VertexBufferClass *VertexBuffer, int Flags) :
    VertexBufferLockClass(VertexBuffer)
{
    captainslog_assert(VertexBuffer);
    captainslog_assert(!VertexBuffer->Engine_Refs());
    VertexBuffer->Add_Ref();
    switch (VertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(VertexBuffer)
                ->Get_DX8_Vertex_Buffer()
                ->Lock(0, 0, (BYTE **)&Vertices, Flags);
#endif
            break;
        }
        case BUFFER_TYPE_SORTING: {
            Vertices = static_cast<SortingVertexBufferClass *>(VertexBuffer)->Get_Sorting_Vertex_Buffer();
            break;
        }
        default:
            captainslog_assert(0);
            break;
    }
}

VertexBufferClass::WriteLockClass::~WriteLockClass()
{
    switch (VertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(VertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock();
#endif
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            captainslog_assert(0);
            break;
    }
    VertexBuffer->Release_Ref();
}

VertexBufferClass::AppendLockClass::AppendLockClass(
    VertexBufferClass *VertexBuffer, unsigned int start_index, unsigned int index_range) :
    VertexBufferLockClass(VertexBuffer)
{
    captainslog_assert(VertexBuffer);
    captainslog_assert(!VertexBuffer->Engine_Refs());
    captainslog_assert(start_index + index_range <= VertexBuffer->Get_Vertex_Count());
    VertexBuffer->Add_Ref();
    switch (VertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(VertexBuffer)
                ->Get_DX8_Vertex_Buffer()
                ->Lock(VertexBuffer->FVF_Info().Get_FVF_Size() * start_index,
                    VertexBuffer->FVF_Info().Get_FVF_Size() * index_range,
                    (BYTE **)&Vertices,
                    0);
#endif
            break;
        }
        case BUFFER_TYPE_SORTING: {
            Vertices = static_cast<SortingVertexBufferClass *>(VertexBuffer)->Get_Sorting_Vertex_Buffer() + start_index;
            break;
        }
        default:
            captainslog_assert(0);
            break;
    }
}

VertexBufferClass::AppendLockClass::~AppendLockClass()
{
    switch (VertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8VertexBufferClass *>(VertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock();
#endif
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            captainslog_assert(0);
            break;
    }
    VertexBuffer->Release_Ref();
}

SortingVertexBufferClass::SortingVertexBufferClass(unsigned short VertexCount) :
    VertexBufferClass(BUFFER_TYPE_SORTING, DX8_FVF_XYZNDUV2, VertexCount, 0)
{
    VertexBuffer = new VertexFormatXYZNDUV2[VertexCount];
}

SortingVertexBufferClass::~SortingVertexBufferClass()
{
    delete[] VertexBuffer;
}

DX8VertexBufferClass::DX8VertexBufferClass(
    unsigned int FVF, unsigned short vertex_count_, UsageType usage, unsigned int flags) :
    VertexBufferClass(BUFFER_TYPE_DX8, FVF, vertex_count_, flags), VertexBuffer(nullptr)
{
    Create_Vertex_Buffer(usage);
}

DX8VertexBufferClass::~DX8VertexBufferClass()
{
#ifdef BUILD_WITH_D3D8
    VertexBuffer->Release();
#endif
}

void DX8VertexBufferClass::Create_Vertex_Buffer(UsageType usage)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(!VertexBuffer);
    int d3dusage = ((usage & USAGE_NPATCHES) >= 1 ? D3DUSAGE_NPATCHES : 0)
        | ((usage & USAGE_DYNAMIC) < 1 ? D3DUSAGE_WRITEONLY : D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY)
        | ((usage & USAGE_SOFTWAREPROCESSING) >= 1 ? D3DUSAGE_SOFTWAREPROCESSING : 0);
    if (!DX8Wrapper::Get_Caps()->Use_TnL()) {
        d3dusage |= D3DUSAGE_SOFTWAREPROCESSING;
    }
    HRESULT res;
    DX8CALL_HRES(CreateVertexBuffer(fvf_info->Get_FVF_Size() * VertexCount,
                    d3dusage,
                    fvf_info->Get_FVF(),
                    (D3DPOOL)(unsigned __int8)(usage & 1 ^ 1),
                    &VertexBuffer),
        res);
    if (res < 0) {
        captainslog_warn("Vertex buffer creation failed, trying to release assets...\n");
        TextureBaseClass::Invalidate_Old_Unused_Textures(5000);
        W3D::_Invalidate_Mesh_Cache();
        DX8CALL(ResourceManagerDiscardBytes(0));
        DX8CALL_HRES(CreateVertexBuffer(fvf_info->Get_FVF_Size() * VertexCount,
                         d3dusage,
                         fvf_info->Get_FVF(),
                         (D3DPOOL)(unsigned __int8)(usage & 1 ^ 1),
                         &VertexBuffer),
            res);
        captainslog_warn("...Vertex buffer creation succesful\n");
    }
#endif
}

DynamicVBAccessClass::DynamicVBAccessClass(unsigned int t, unsigned int fvf, unsigned short vertex_count_) :
    Type(t), FVFInfo(_DynamicFVFInfo), VertexCount(vertex_count_), VertexBuffer(nullptr)
{
    captainslog_assert(fvf == dynamic_fvf_type);
    captainslog_assert(
        Type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8 || Type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING);
    if (Type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        Allocate_DX8_Dynamic_Buffer();
    } else {
        Allocate_Sorting_Dynamic_Buffer();
    }
}

DynamicVBAccessClass::~DynamicVBAccessClass()
{
    if (Type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        _DynamicDX8VertexBufferInUse = false;
        _DynamicDX8VertexBufferOffset += VertexCount;
    } else {
        _DynamicSortingVertexArrayInUse = 0;
        _DynamicSortingVertexArrayOffset += VertexCount;
    }
    Ref_Ptr_Release(VertexBuffer);
}

void DynamicVBAccessClass::_Deinit()
{
    if (_DynamicDX8VertexBuffer) {
        captainslog_assert((_DynamicDX8VertexBuffer == NULL) || (_DynamicDX8VertexBuffer->Num_Refs() == 1));
        _DynamicDX8VertexBuffer->Release_Ref();
    }
    _DynamicDX8VertexBuffer = nullptr;
    _DynamicDX8VertexBufferInUse = false;
    _DynamicDX8VertexBufferSize = 5000;
    _DynamicDX8VertexBufferOffset = 0;
    if (_DynamicSortingVertexArray) {
        captainslog_assert((_DynamicSortingVertexArray == NULL) || (_DynamicSortingVertexArray->Num_Refs() == 1));
        _DynamicSortingVertexArray->Release_Ref();
    }
    _DynamicSortingVertexArray = nullptr;
    captainslog_assert(!_DynamicSortingVertexArrayInUse);
    _DynamicSortingVertexArrayInUse = false;
    _DynamicSortingVertexArraySize = 0;
    _DynamicSortingVertexArrayOffset = 0;
}

void DynamicVBAccessClass::Allocate_DX8_Dynamic_Buffer()
{
    captainslog_assert(!_DynamicDX8VertexBufferInUse);
    _DynamicDX8VertexBufferInUse = true;
    if (VertexCount > _DynamicDX8VertexBufferSize) {
        Ref_Ptr_Release(_DynamicDX8VertexBuffer);
        _DynamicDX8VertexBufferSize = 5000;
        if (VertexCount > 4999) {
            _DynamicDX8VertexBufferSize = VertexCount;
        }
    }
    if (!_DynamicDX8VertexBuffer) {
        DX8VertexBufferClass::UsageType usage = DX8VertexBufferClass::USAGE_DYNAMIC;
        if (DX8Wrapper::Get_Caps()->Supports_NPatches()) {
            usage = (DX8VertexBufferClass::UsageType)(DX8VertexBufferClass::USAGE_DYNAMIC | DX8VertexBufferClass::USAGE_NPATCHES);
        }
        _DynamicDX8VertexBuffer = new DX8VertexBufferClass(DX8_FVF_XYZNDUV2, _DynamicDX8VertexBufferSize, usage, 0);
        _DynamicDX8VertexBufferOffset = 0;
    }
    if (_DynamicDX8VertexBufferOffset + VertexCount > _DynamicDX8VertexBufferSize) {
        _DynamicDX8VertexBufferOffset = 0;
    }
    _DynamicDX8VertexBuffer->Add_Ref();
    Ref_Ptr_Release(VertexBuffer);
    VertexBuffer = _DynamicDX8VertexBuffer;
    VertexBufferOffset = _DynamicDX8VertexBufferOffset;
}

void DynamicVBAccessClass::Allocate_Sorting_Dynamic_Buffer()
{
    captainslog_assert(!_DynamicSortingVertexArrayInUse);
    _DynamicSortingVertexArrayInUse = true;
    int new_vertex_count = _DynamicSortingVertexArrayOffset + VertexCount;
    captainslog_assert(new_vertex_count < 65536);
    if (new_vertex_count > _DynamicSortingVertexArraySize) {
        Ref_Ptr_Release(_DynamicSortingVertexArray);
        _DynamicSortingVertexArraySize = 5000;
        if (new_vertex_count > 4999) {
            _DynamicSortingVertexArraySize = new_vertex_count;
        }
    }
    if (!_DynamicSortingVertexArray) {
        _DynamicSortingVertexArray = new SortingVertexBufferClass(_DynamicSortingVertexArraySize);
        _DynamicSortingVertexArrayOffset = 0;
    }
    _DynamicSortingVertexArray->Add_Ref();
    Ref_Ptr_Release(VertexBuffer);
    VertexBuffer = _DynamicSortingVertexArray;
    VertexBufferOffset = _DynamicSortingVertexArrayOffset;
}

DynamicVBAccessClass::WriteLockClass::WriteLockClass(DynamicVBAccessClass *dynamic_vb_access_) :
    DynamicVBAccess(dynamic_vb_access_)
{
    if (DynamicVBAccess->Type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        captainslog_assert(_DynamicDX8VertexBuffer);
#ifdef BUILD_WITH_D3D8
        DX8VertexBufferClass *buffer = static_cast<DX8VertexBufferClass *>(DynamicVBAccess->VertexBuffer);
        buffer->Get_DX8_Vertex_Buffer()->Lock(
            _DynamicDX8VertexBuffer->FVF_Info().Get_FVF_Size() * DynamicVBAccess->VertexBufferOffset,
            buffer->FVF_Info().Get_FVF_Size() * DynamicVBAccess->VertexCount,
            (BYTE **)&Vertices,
            (DynamicVBAccess->VertexBufferOffset != 0 ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD) | D3DLOCK_NOSYSLOCK);
#endif
    } else if (DynamicVBAccess->Type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING) {
        SortingVertexBufferClass *buffer = static_cast<SortingVertexBufferClass *>(DynamicVBAccess->VertexBuffer);
        Vertices = buffer->Get_Sorting_Vertex_Buffer() + DynamicVBAccess->VertexBufferOffset;
    } else {
        captainslog_assert(0);
    }
}

DynamicVBAccessClass::WriteLockClass::~WriteLockClass()
{
    if (DynamicVBAccess->Type == VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
#ifdef BUILD_WITH_D3D8
        DX8VertexBufferClass *buffer = static_cast<DX8VertexBufferClass *>(DynamicVBAccess->VertexBuffer);
        buffer->Get_DX8_Vertex_Buffer()->Unlock();
#endif
    } else if (DynamicVBAccess->Type != VertexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING) {
        captainslog_assert(0);
    }
}

void DynamicVBAccessClass::_Reset(bool frame_changed)
{
    _DynamicSortingVertexArrayOffset = 0;
    if (frame_changed) {
        _DynamicDX8VertexBufferOffset = 0;
    }
}

unsigned short DynamicVBAccessClass::Get_Default_Vertex_Count()
{
    return _DynamicDX8VertexBufferSize;
}

//unimplemented, not used
//DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector3 *normals, Vector2 *tex_coords, unsigned short VertexCount, UsageType usage)
//DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector3 *normals, Vector4 *diffuse, Vector2 *tex_coords, unsigned short VertexCount, UsageType usage)
//DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector4 *diffuse, Vector2 *tex_coords, unsigned short VertexCount, UsageType usage)
//DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector2 *tex_coords, unsigned short VertexCount, UsageType usage)
//void DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, Vector2 *uv, unsigned int first_vertex, unsigned int count)
//void DX8VertexBufferClass::Copy(Vector3 *loc, unsigned int first_vertex, unsigned int count)
//void DX8VertexBufferClass::Copy(Vector3 *loc, Vector2 *uv, unsigned int first_vertex, unsigned int count)
//void DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, unsigned int first_vertex, unsigned int count)
//void DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex, unsigned int count)
//void DX8VertexBufferClass::Copy(Vector3 *loc, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex, unsigned int count)
