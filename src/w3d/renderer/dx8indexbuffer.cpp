/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Index Buffer classes
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtypes.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "dx8caps.h"
#include "w3d.h"

unsigned int _IndexBufferCount;
unsigned int _IndexBufferTotalIndices;
unsigned int _IndexBufferTotalSize;
bool _DynamicSortingIndexArrayInUse;
SortingIndexBufferClass *_DynamicSortingIndexArray;
unsigned short _DynamicSortingIndexArraySize;
unsigned short _DynamicSortingIndexArrayOffset;
bool _DynamicDX8IndexBufferInUse;
DX8IndexBufferClass *_DynamicDX8IndexBuffer;
unsigned short _DynamicDX8IndexBufferSize = 5000;
unsigned short _DynamicDX8IndexBufferOffset;

IndexBufferClass::IndexBufferClass(unsigned int type_, unsigned short index_count_) :
    index_count(index_count_),
    engine_refs(0), type(type_)
{
    captainslog_assert(type == BUFFER_TYPE_DX8 || type == BUFFER_TYPE_SORTING);
    captainslog_assert(index_count);
    _IndexBufferCount++;
    _IndexBufferTotalIndices += index_count;
    _IndexBufferTotalSize += 2 * index_count;
}

IndexBufferClass::~IndexBufferClass()
{
    _IndexBufferCount--;
    _IndexBufferTotalIndices -= index_count;
    _IndexBufferTotalSize -= 2 * index_count;
}

void IndexBufferClass::Add_Engine_Ref()
{
    engine_refs++;
}

void IndexBufferClass::Release_Engine_Ref()
{
    engine_refs--;
    captainslog_assert(engine_refs >= 0);
}

unsigned int IndexBufferClass::Get_Total_Buffer_Count()
{
    return _IndexBufferCount;
}

unsigned int IndexBufferClass::Get_Total_Allocated_Indices()
{
    return _IndexBufferTotalIndices;
}

unsigned int IndexBufferClass::Get_Total_Allocated_Memory()
{
    return _IndexBufferTotalSize;
}

IndexBufferClass::WriteLockClass::WriteLockClass(IndexBufferClass *index_buffer_, unsigned int flags) :
    index_buffer(index_buffer_)
{
    captainslog_assert(index_buffer);
    captainslog_assert(!index_buffer->Engine_Refs());
    index_buffer->Add_Ref();
    switch (index_buffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8IndexBufferClass *>(index_buffer)
                ->Get_DX8_Index_Buffer()->Lock(0, 0, (BYTE **)&indices, flags);
#endif
            break;
        }
        case BUFFER_TYPE_SORTING: {
            indices = static_cast<SortingIndexBufferClass *>(index_buffer)->Get_Sorting_Index_Buffer();
            break;
        }
        default:
            captainslog_assert(0);
            break;
    }
}

IndexBufferClass::WriteLockClass::~WriteLockClass()
{
    switch (index_buffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8IndexBufferClass *>(index_buffer)->Get_DX8_Index_Buffer()->Unlock();
#endif
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            captainslog_assert(0);
            break;
    }
    index_buffer->Release_Ref();
}

IndexBufferClass::AppendLockClass::AppendLockClass(
    IndexBufferClass *index_buffer_, unsigned int start_index, unsigned int index_range) :
    index_buffer(index_buffer_)
{
    captainslog_assert(start_index + index_range <= index_buffer->Get_Index_Count());
    captainslog_assert(index_buffer);
    captainslog_assert(!index_buffer->Engine_Refs());

    index_buffer->Add_Ref();
    switch (index_buffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8IndexBufferClass *>(index_buffer)
                ->Get_DX8_Index_Buffer()
                ->Lock(2 * start_index, 2 * index_range, (BYTE **)&indices, 0);
#endif
            break;
        }
        case BUFFER_TYPE_SORTING: {
            indices = static_cast<SortingIndexBufferClass *>(index_buffer)->Get_Sorting_Index_Buffer() + start_index;
            break;
        }
        default:
            captainslog_assert(0);
            break;
    }
}

IndexBufferClass::AppendLockClass::~AppendLockClass()
{
    switch (index_buffer->Type()) {
        case BUFFER_TYPE_DX8: {
#ifdef BUILD_WITH_D3D8
            static_cast<DX8IndexBufferClass *>(index_buffer)->Get_DX8_Index_Buffer()->Unlock();
#endif
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            captainslog_assert(0);
            break;
    }
    index_buffer->Release_Ref();
}

DX8IndexBufferClass::DX8IndexBufferClass(unsigned short index_count_, UsageType usage) :
    IndexBufferClass(BUFFER_TYPE_DX8, index_count)
{
#ifdef BUILD_WITH_D3D8
    captainslog_assert(index_count);
    int d3dusage = ((usage & USAGE_NPATCHES) >= 1 ? D3DUSAGE_NPATCHES : 0)
        | ((usage & USAGE_DYNAMIC) < 1 ? D3DUSAGE_WRITEONLY : D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY)
        | ((usage & USAGE_SOFTWAREPROCESSING) >= 1 ? D3DUSAGE_SOFTWAREPROCESSING : 0);
    if (!DX8Wrapper::Get_Caps()->Use_TnL()) {
        d3dusage |= D3DUSAGE_SOFTWAREPROCESSING;
    }
    HRESULT res;
    DX8CALL_HRES(CreateIndexBuffer(
                     2 * index_count, d3dusage, D3DFMT_INDEX16, (D3DPOOL)(unsigned __int8)(usage & 1 ^ 1), &index_buffer),
        res);
    if (res < 0) {
        captainslog_warn("Index buffer creation failed, trying to release assets...\n");
        TextureBaseClass::Invalidate_Old_Unused_Textures(5000);
        W3D::_Invalidate_Mesh_Cache();
        DX8CALL(ResourceManagerDiscardBytes(0));
        DX8CALL_HRES(
            CreateIndexBuffer(
                2 * index_count, d3dusage, D3DFMT_INDEX16, (D3DPOOL)(unsigned __int8)(usage & 1 ^ 1), &index_buffer),
            res);
        captainslog_warn("...Index buffer creation succesful\n");
    }
#endif
}

DX8IndexBufferClass::~DX8IndexBufferClass()
{
#ifdef BUILD_WITH_D3D8
    index_buffer->Release();
#endif
}

SortingIndexBufferClass::SortingIndexBufferClass(unsigned short index_count_) :
    IndexBufferClass(BUFFER_TYPE_SORTING, index_count_)
{
    captainslog_assert(index_count);
    index_buffer = new unsigned short[index_count];
}

SortingIndexBufferClass::~SortingIndexBufferClass()
{
    delete[] index_buffer;
}

DynamicIBAccessClass::DynamicIBAccessClass(unsigned short type_, unsigned short index_count_) :
    Type(type_),
    IndexCount(index_count_), IndexBuffer(nullptr)
{
    captainslog_assert(Type == BUFFER_TYPE_DYNAMIC_DX8 || Type == BUFFER_TYPE_DYNAMIC_SORTING);
    if (Type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        Allocate_DX8_Dynamic_Buffer();
    } else {
        Allocate_Sorting_Dynamic_Buffer();
    }
}

DynamicIBAccessClass::~DynamicIBAccessClass()
{
    Ref_Ptr_Release(IndexBuffer);
    if (Type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
        _DynamicDX8IndexBufferInUse = false;
        _DynamicDX8IndexBufferOffset += IndexCount;
    } else {
        _DynamicSortingIndexArrayInUse = false;
        _DynamicSortingIndexArrayOffset += IndexCount;
    }
}

void DynamicIBAccessClass::Allocate_Sorting_Dynamic_Buffer()
{
    captainslog_assert(!_DynamicSortingIndexArrayInUse);
    _DynamicSortingIndexArrayInUse = true;
    int new_index_count = _DynamicSortingIndexArrayOffset + IndexCount;
    captainslog_assert(new_index_count < 65536);
    if (new_index_count > _DynamicSortingIndexArraySize) {
        Ref_Ptr_Release(_DynamicSortingIndexArray);
        _DynamicSortingIndexArraySize = 5000;
        if (new_index_count > 4999) {
            _DynamicSortingIndexArraySize = new_index_count;
        }
    }
    if (!_DynamicSortingIndexArray) {
        _DynamicSortingIndexArray = new SortingIndexBufferClass(_DynamicSortingIndexArraySize);
        _DynamicSortingIndexArrayOffset = 0;
    }
    _DynamicSortingIndexArray->Add_Ref();
    Ref_Ptr_Release(IndexBuffer);
    IndexBuffer = _DynamicSortingIndexArray;
    IndexBufferOffset = _DynamicSortingIndexArrayOffset;
}

void DynamicIBAccessClass::Allocate_DX8_Dynamic_Buffer()
{
    captainslog_assert(!_DynamicDX8IndexBufferInUse);
    _DynamicDX8IndexBufferInUse = true;
    if (IndexCount > _DynamicDX8IndexBufferSize) {
        Ref_Ptr_Release(_DynamicDX8IndexBuffer);
        _DynamicDX8IndexBufferSize = 5000;
        if (IndexCount > 4999) {
            _DynamicDX8IndexBufferSize = IndexCount;
        }
    }
    if (!_DynamicDX8IndexBuffer) {
        DX8IndexBufferClass::UsageType usage = DX8IndexBufferClass::USAGE_DYNAMIC;
        if (DX8Wrapper::Get_Caps()->Supports_NPatches()) {
            usage = (DX8IndexBufferClass::UsageType)(DX8IndexBufferClass::USAGE_DYNAMIC | DX8IndexBufferClass::USAGE_NPATCHES);
        }
        _DynamicDX8IndexBuffer = new DX8IndexBufferClass(_DynamicDX8IndexBufferSize, usage);
        _DynamicDX8IndexBufferOffset = 0;
    }
    if (_DynamicDX8IndexBufferOffset + IndexCount > _DynamicDX8IndexBufferSize) {
        _DynamicDX8IndexBufferOffset = 0;
    }
    _DynamicDX8IndexBuffer->Add_Ref();
    Ref_Ptr_Release(IndexBuffer);
    IndexBuffer = _DynamicDX8IndexBuffer;
    IndexBufferOffset = _DynamicDX8IndexBufferOffset;
}

DynamicIBAccessClass::WriteLockClass::WriteLockClass(DynamicIBAccessClass *ib_access_) : DynamicIBAccess(ib_access_)
{
    DynamicIBAccess->IndexBuffer->Add_Ref();
    if (DynamicIBAccess->Type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
#ifdef BUILD_WITH_D3D8
        captainslog_assert(DynamicIBAccess);
        DX8IndexBufferClass *buffer = static_cast<DX8IndexBufferClass *>(DynamicIBAccess->IndexBuffer);
        buffer->Get_DX8_Index_Buffer()->Lock(
            2 * DynamicIBAccess->IndexBufferOffset,
            2 * DynamicIBAccess->IndexCount,
            (BYTE **)&Indices,
            DynamicIBAccess->IndexBufferOffset != 0 ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);
#endif
    } else if (DynamicIBAccess->Type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING) {
        SortingIndexBufferClass *buffer = static_cast<SortingIndexBufferClass *>(DynamicIBAccess->IndexBuffer);
        Indices = buffer->Get_Sorting_Index_Buffer() + DynamicIBAccess->IndexBufferOffset;
    } else {
        captainslog_assert(0);
    }
}

DynamicIBAccessClass::WriteLockClass::~WriteLockClass()
{
    if (DynamicIBAccess->Type == IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8) {
#ifdef BUILD_WITH_D3D8
        DX8IndexBufferClass *buffer = static_cast<DX8IndexBufferClass *>(DynamicIBAccess->IndexBuffer);
        buffer->Get_DX8_Index_Buffer()->Unlock();
#endif
    } else if (DynamicIBAccess->Type != IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING) {
        captainslog_assert(0);
    }
    DynamicIBAccess->IndexBuffer->Release_Ref();
}

void DynamicIBAccessClass::_Deinit()
{
    if (_DynamicDX8IndexBuffer) {
        captainslog_assert((_DynamicDX8IndexBuffer == NULL) || (_DynamicDX8IndexBuffer->Num_Refs() == 1));
        _DynamicDX8IndexBuffer->Release_Ref();
    }
    _DynamicDX8IndexBuffer = nullptr;
    _DynamicDX8IndexBufferInUse = false;
    _DynamicDX8IndexBufferSize = 5000;
    _DynamicDX8IndexBufferOffset = 0;
    if (_DynamicSortingIndexArray) {
        captainslog_assert((_DynamicSortingIndexArray == NULL) || (_DynamicSortingIndexArray->Num_Refs() == 1));
        _DynamicSortingIndexArray->Release_Ref();
    }
    _DynamicSortingIndexArray = nullptr;
    _DynamicSortingIndexArrayInUse = false;
    _DynamicSortingIndexArraySize = 0;
    _DynamicSortingIndexArrayOffset = 0;
}

void DynamicIBAccessClass::_Reset(bool frame_changed)
{
    _DynamicSortingIndexArrayOffset = 0;
    if (frame_changed) {
        _DynamicDX8IndexBufferOffset = 0;
    }
}

unsigned short DynamicIBAccessClass::Get_Default_Index_Count()
{
    return _DynamicDX8IndexBufferSize;
}

unsigned short DynamicIBAccessClass::Get_Next_Index()
{
    return _IndexBufferTotalIndices + 1;
}

//unimplemented, not used
//void IndexBufferClass::Copy(unsigned short *indices, unsigned int first_index, unsigned int count)
//void IndexBufferClass::Copy(unsigned int *indices, unsigned int first_index, unsigned int count)
