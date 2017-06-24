////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MEMPOOLOBJ.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Custom memory manager designed to limit OS calls to allocate
//                 heap memory.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef MEMPOOLOBJ_H
#define MEMPOOLOBJ_H

#include "gamedebug.h"
#include "mempool.h"
#include "mempoolfact.h"

//
// Base class for any object requiring efficient memory handling ?
//
class MemoryPoolObject
{
public:
    virtual ~MemoryPoolObject() {}
    virtual MemoryPool *Get_Object_Pool() = 0;

    // Class implementing Get_Object_Pool needs to provide these
    // use macros below to generated them.
};

// Use within a class declaration on a none virtual MemoryPoolObject
// based class to implement required functions. "classname" must match
// the name of the class in which it is used.
#define IMPLEMENT_POOL(classname) \
    private: \
        static MemoryPool *Get_Class_Pool() \
        { \
            static MemoryPool *The##classname##Pool; \
            static bool PoolInit; \
            if ( !PoolInit ) { \
                PoolInit = true; \
                The##classname##Pool = g_memoryPoolFactory->Create_Memory_Pool(#classname, sizeof(classname), -1, -1); \
            } \
            ASSERT_PRINT(The##classname##Pool->Get_Alloc_Size() == sizeof(classname), "Pool %s is wrong size for class (need %d, currently %d)", #classname, sizeof(classname), The##classname##Pool->Get_Alloc_Size()); \
            return The##classname##Pool; \
        } \
    public: \
        virtual MemoryPool *Get_Object_Pool() override\
        { \
            return Get_Class_Pool(); \
        } \
        void *operator new(size_t size) \
        { \
            return Get_Class_Pool()->Allocate_Block(); \
        } \
        void operator delete(void *ptr) \
        { \
            return Get_Class_Pool()->Free_Block(ptr); \
        }

// Use within a class declaration on a none virtual MemoryPoolObject
// based class to implement required functions. "classname" must match
// the name of the class in which it is used, "poolname" should match a 
// gamememoryinit.cpp entry.
#define IMPLEMENT_NAMED_POOL(classname, poolname) \
    private: \
        static MemoryPool *Get_Class_Pool() \
        { \
            static MemoryPool *The##classname##Pool; \
            static bool PoolInit; \
            if ( !PoolInit ) { \
                PoolInit = true; \
                The##classname##Pool = g_memoryPoolFactory->Create_Memory_Pool(#poolname, sizeof(classname), -1, -1); \
            } \
            ASSERT_PRINT(The##classname##Pool->Get_Alloc_Size() == sizeof(classname), "Pool %s is wrong size for class (need %d, currently %d)", #classname, sizeof(classname), The##classname##Pool->Get_Alloc_Size()); \
            return The##classname##Pool; \
        } \
    public: \
        virtual MemoryPool *Get_Object_Pool() \
        { \
            return Get_Class_Pool(); \
        } \
        void *operator new(size_t size) \
        { \
            return Get_Class_Pool()->Allocate_Block(); \
        } \
        void operator delete(void *ptr) \
        { \
            return Get_Class_Pool()->Free_Block(ptr); \
        }

// Delete a MemoryPoolObject instance.
inline void Delete_Instance(MemoryPoolObject *ptr)
{
    if ( ptr != nullptr ) {
        MemoryPool *pool = ptr->Get_Object_Pool();
        ptr->~MemoryPoolObject();
        pool->Free_Block(ptr);
    }
}

//
// Not much to go on for these, looks heavily inlined.
//
class MemoryPoolObjectHolder
{
public:
    MemoryPoolObjectHolder(MemoryPoolObject *obj) : Obj(obj) {}

    ~MemoryPoolObjectHolder()
    {
        if ( Obj != nullptr ) {
            MemoryPool *mp = Obj->Get_Object_Pool();
            Obj->~MemoryPoolObject();
            mp->Free_Block(Obj);
        }
    }

    void Hold(MemoryPoolObject *obj) { Obj = obj; }
    void Release() { Obj = nullptr; }

private:
    MemoryPoolObject *Obj;
};

#endif
