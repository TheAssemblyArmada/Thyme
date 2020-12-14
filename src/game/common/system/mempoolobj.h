/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for object intended to be allocated from specialised memory pools.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "mempool.h"
#include "mempoolfact.h"
#include <captainslog.h>
#include <new>

enum ErrorCode
{
    CODE_01 = 0xDEAD0001,
    CODE_02 = 0xDEAD0002,
    CODE_03 = 0xDEAD0003,
    CODE_05 = 0xDEAD0005,
    CODE_06 = 0xDEAD0006,
    CODE_07 = 0xDEAD0007,
};

//
// Base class for any object requiring efficient memory handling ?
//
class MemoryPoolObject
{
public:
    void Delete_Instance();

protected:
    virtual ~MemoryPoolObject() {}
    virtual MemoryPool *Get_Object_Pool() = 0;

    void *operator new(size_t);
    void operator delete(void *) { captainslog_debug("This should be impossible"); }

    // Class implementing Get_Object_Pool needs to provide these
    // use macros below to generated them.
};

// Use within a class declaration on a none virtual MemoryPoolObject
// based class to implement required functions. "classname" must match
// the name of the class in which it is used, "poolname" should match a
// gamememoryinit.cpp entry.
#define IMPLEMENT_NAMED_POOL(classname, poolname) \
private: \
    static MemoryPool *Get_Class_Pool() \
    { \
        captainslog_dbgassert(g_memoryPoolFactory, "TheMemoryPoolFactory is nullptr\n"); \
        static MemoryPool *The##classname##Pool = \
            g_memoryPoolFactory->Create_Memory_Pool(#poolname, sizeof(classname), -1, -1); \
        captainslog_dbgassert( \
            The##classname##Pool, "Pool \"%s\" not found (did you set it up in initMemoryPools?)\n", #poolname); \
        captainslog_dbgassert(The##classname##Pool->Get_Alloc_Size() >= sizeof(classname), \
            "Pool \"%s\" is too small for this class (currently %d, need %d)\n", \
            #poolname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        captainslog_dbgassert(The##classname##Pool->Get_Alloc_Size() <= sizeof(classname), \
            "Pool \"%s\" is too large for this class (currently %d, need %d)\n", \
            #poolname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        return The##classname##Pool; \
    } \
    virtual MemoryPool *Get_Object_Pool() override { return Get_Class_Pool(); } \
\
public: \
    enum classname##MagicEnum{ classname##_GLUE_NOT_IMPLEMENTED }; \
\
    void *operator new(size_t size); \
    void *operator new(size_t size, classname##MagicEnum) \
    { \
        captainslog_dbgassert(size == sizeof(classname), \
            "The wrong operator new is being called; ensure all objects in the hierarchy have MemoryPoolGlue set up " \
            "correctly"); \
        return Get_Class_Pool()->Allocate_Block(); \
    } \
    void operator delete(void *ptr) \
    { \
        captainslog_debug("Please call deleteInstance instead of delete."); \
        Get_Class_Pool()->Free_Block(ptr); \
    } \
    void operator delete(void *ptr, classname##MagicEnum) { Get_Class_Pool()->Free_Block(ptr); }

#define IMPLEMENT_POOL(classname) IMPLEMENT_NAMED_POOL(classname, classname);

#define IMPLEMENT_ABSTRACT_POOL(classname) \
private: \
    virtual MemoryPool *Get_Object_Pool() override { throw CODE_01; } \
\
    enum classname##MagicEnum{ classname##_GLUE_NOT_IMPLEMENTED }; \
\
protected: \
    void *operator new(size_t size); \
    void *operator new(size_t size, classname##MagicEnum); \
    void operator delete(void *ptr) { captainslog_debug("this should be impossible to call (abstract base class)"); } \
    void operator delete(void *ptr, classname##MagicEnum);

#define NEW_POOL_OBJ(classname, ...) new (classname::classname##_GLUE_NOT_IMPLEMENTED) classname(__VA_ARGS__)

/**
 * @brief Delete an instance of a MemoryPoolObject, ensuring correct pool is used via virtual call.
 */
inline void MemoryPoolObject::Delete_Instance()
{
    if (this) {
        MemoryPool *pool = Get_Object_Pool();
        this->~MemoryPoolObject();
        pool->Free_Block(this);
    }
}

//
// Class to hold the memory pool for a runtime of a function and if still held destroy it when the function is done.
//
class MemoryPoolObjectHolder
{
public:
    MemoryPoolObjectHolder(MemoryPoolObject *obj = nullptr) : m_mpo(obj) {}

    ~MemoryPoolObjectHolder() { m_mpo->Delete_Instance(); }

    void Hold(MemoryPoolObject *obj)
    {
        if (m_mpo != nullptr) {
            captainslog_debug("Already holding!");
        }
        m_mpo = obj;
    }

    void Release() { m_mpo = nullptr; }

private:
    MemoryPoolObject *m_mpo;
};
