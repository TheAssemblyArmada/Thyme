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
#pragma once
#include "always.h"
#include "refcount.h"
#include "w3dmpo.h"
#include "w3dtypes.h"

class IndexBufferClass : public W3DMPO, public RefCountClass
{
    ALLOW_HOOKING
public:
    enum
    {
        BUFFER_TYPE_DX8,
        BUFFER_TYPE_SORTING,
        BUFFER_TYPE_DYNAMIC_DX8,
        BUFFER_TYPE_DYNAMIC_SORTING,
        BUFFER_TYPE_INVALID,
    };
    class WriteLockClass
    {
        ALLOW_HOOKING
    public:
        WriteLockClass(IndexBufferClass *index_buffer_, unsigned int flags);
        ~WriteLockClass();
        unsigned short *Get_Index_Array() { return m_indices; }

    private:
#ifdef GAME_DLL
        WriteLockClass *Hook_Ctor(IndexBufferClass *index_buffer_, int Flags)
        {
            return new (this) WriteLockClass(index_buffer_, Flags);
        }
        void Hook_Dtor() { WriteLockClass::~WriteLockClass(); }
#endif
        IndexBufferClass *m_indexBuffer;
        unsigned short *m_indices;
    };
    class AppendLockClass
    {
        ALLOW_HOOKING
    public:
        AppendLockClass(IndexBufferClass *index_buffer_, unsigned int start_index, unsigned int index_range);
        ~AppendLockClass();
        unsigned short *Get_Index_Array() { return m_indices; }

    private:
#ifdef GAME_DLL
        AppendLockClass *Hook_Ctor(IndexBufferClass *index_buffer_, unsigned int start_index, unsigned int index_range)
        {
            return new (this) AppendLockClass(index_buffer_, start_index, index_range);
        }
        void Hook_Dtor() { AppendLockClass::~AppendLockClass(); }
#endif
        IndexBufferClass *m_indexBuffer;
        unsigned short *m_indices;
    };

public:
    IndexBufferClass(unsigned int type_, unsigned short index_count_);
    ~IndexBufferClass();
    void Add_Engine_Ref() const;
    void Release_Engine_Ref();
    void Copy(unsigned short *indices, unsigned int first_index, unsigned int count);
    void Copy(unsigned int *indices, unsigned int first_index, unsigned int count);
    unsigned short Get_Index_Count() { return m_indexCount; }
    unsigned int Type() const { return m_type; }
    unsigned int Engine_Refs() const { return m_engineRefs; }
    static unsigned int Get_Total_Buffer_Count();
    static unsigned int Get_Total_Allocated_Indices();
    static unsigned int Get_Total_Allocated_Memory();

protected:
    mutable int m_engineRefs;
    unsigned short m_indexCount;
    unsigned int m_type;
};

class DX8IndexBufferClass : public IndexBufferClass
{
    IMPLEMENT_W3D_POOL(DX8IndexBufferClass)
    ALLOW_HOOKING
public:
    enum UsageType
    {
        USAGE_DEFAULT,
        USAGE_DYNAMIC,
        USAGE_SOFTWAREPROCESSING
    };
    DX8IndexBufferClass(unsigned short index_count_, UsageType usage);
    ~DX8IndexBufferClass();
#ifdef BUILD_WITH_D3D8
    IDirect3DIndexBuffer8 *Get_DX8_Index_Buffer() { return m_indexBuffer; }
#endif

private:
#ifdef GAME_DLL
    DX8IndexBufferClass *Hook_Ctor(unsigned short index_count_, UsageType usage)
    {
        return new (this) DX8IndexBufferClass(index_count_, usage);
    }
#endif
#ifdef BUILD_WITH_D3D8
    IDirect3DIndexBuffer8 *m_indexBuffer;
#endif
};

class SortingIndexBufferClass : public IndexBufferClass
{
    IMPLEMENT_W3D_POOL(SortingIndexBufferClass)
    ALLOW_HOOKING
public:
    SortingIndexBufferClass(unsigned short index_count_);
    ~SortingIndexBufferClass();
    unsigned short *Get_Sorting_Index_Buffer() { return m_indexBuffer; }

private:
#ifdef GAME_DLL
    SortingIndexBufferClass *Hook_Ctor(unsigned short index_count_)
    {
        return new (this) SortingIndexBufferClass(index_count_);
    }
#endif
    unsigned short *m_indexBuffer;
};

class DynamicIBAccessClass : public W3DMPO
{
    IMPLEMENT_W3D_POOL(DynamicIBAccessClass)
    ALLOW_HOOKING
public:
    class WriteLockClass
    {
        ALLOW_HOOKING
    public:
        WriteLockClass(DynamicIBAccessClass *ib_access_);
        ~WriteLockClass();
        unsigned short *Get_Index_Array() { return m_indices; }

    private:
#ifdef GAME_DLL
        WriteLockClass *Hook_Ctor(DynamicIBAccessClass *ib_access_) { return new (this) WriteLockClass(ib_access_); }
        void Hook_Dtor() { WriteLockClass::~WriteLockClass(); }
#endif
        DynamicIBAccessClass *m_dynamicIBAccess;
        unsigned short *m_indices;
    };

public:
    DynamicIBAccessClass(unsigned short type_, unsigned short index_count_);
    ~DynamicIBAccessClass();
    void Allocate_Sorting_Dynamic_Buffer();
    void Allocate_DX8_Dynamic_Buffer();
    static void Deinit();
    static void Reset(bool frame_changed);
    static unsigned short Get_Default_Index_Count();
    static unsigned short Get_Next_Index();
    unsigned int Get_Type() const { return m_type; }
    unsigned short Get_Index_Offset() const { return m_indexBufferOffset; }
    IndexBufferClass *Get_Index_Buffer() const { return m_indexBuffer; }

private:
#ifdef GAME_DLL
    DynamicIBAccessClass *Hook_Ctor(unsigned short type_, unsigned short index_count_)
    {
        return new (this) DynamicIBAccessClass(type_, index_count_);
    }
    void Hook_Dtor() { DynamicIBAccessClass::~DynamicIBAccessClass(); }
#endif
    unsigned int m_type;
    unsigned short m_indexCount;
    unsigned short m_indexBufferOffset;
    IndexBufferClass *m_indexBuffer;
};
