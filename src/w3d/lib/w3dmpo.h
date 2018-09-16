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

// I think this stands for W3DMemoryPoolObject
class W3DMPO
{
public:
    // I think the glueEnforcer comes from this original is a pure virtual but its always a return 4 in classes that derive
    // from it so ill just set it here
    virtual int const glueEnforcer() { return 4; };
    virtual ~W3DMPO() {}
};

#define IMPLEMENT_W3D_POOL(classname) \
private: \
    static MemoryPool *Get_Class_Pool() \
    { \
        static MemoryPool *const The##classname##Pool = \
            g_memoryPoolFactory->Create_Memory_Pool(#classname, sizeof(classname), -1, -1); \
        ASSERT_PRINT(The##classname##Pool->Get_Alloc_Size() == sizeof(classname), \
            "Pool %s is wrong size for class (need %d, currently %d)", \
            #classname, \
            sizeof(classname), \
            The##classname##Pool->Get_Alloc_Size()); \
        return The##classname##Pool; \
    } \
\
public: \
    void *operator new(size_t size) { return Allocate_From_Pool(Get_Class_Pool(), sizeof(classname)); } \
    void *operator new(size_t size, void *dst) { return dst; } \
    void operator delete(void *p, void *q) {} \
    void operator delete(void *ptr) { return Free_From_Pool(Get_Class_Pool(), ptr); } \
\
private:
