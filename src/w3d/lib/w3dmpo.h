/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Alternative MemoryPoolObject style class for the W3D classes.
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
#include "gamememory.h"
#include "mempoolobj.h"
#include <captainslog.h>

// I think this stands for W3DMemoryPoolObject
class W3DMPO
{
public:
    // This enforcer is a checker to make sure a class has been set up correctly with the implement pool macro.
    virtual int glueEnforcer() = 0;
    virtual ~W3DMPO() {}
};

#define IMPLEMENT_NAMED_W3D_POOL(classname, poolname) \
private: \
    static MemoryPool *Get_Class_Pool() \
    { \
        static MemoryPool *const The##classname##Pool = \
            g_memoryPoolFactory->Create_Memory_Pool(#poolname, sizeof(classname), -1, -1); \
        captainslog_dbgassert(The##classname##Pool->Get_Alloc_Size() == sizeof(classname), \
            "Pool %s is wrong size for class (need %d, currently %d)", \
            #classname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        return The##classname##Pool; \
    } \
\
public: \
    void *operator new(size_t size) { return Allocate_From_Pool(Get_Class_Pool(), sizeof(classname)); } \
    void *operator new(size_t size, void *where) { return where; } \
    void operator delete(void *p, void *where) {} \
    void operator delete(void *ptr) { return Free_From_Pool(Get_Class_Pool(), ptr); } \
    virtual int glueEnforcer() override { return 4; }; \
\
private:

#define IMPLEMENT_W3D_POOL(classname) IMPLEMENT_NAMED_W3D_POOL(classname, classname)
