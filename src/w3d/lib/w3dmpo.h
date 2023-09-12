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
#include <captainslog.h>
#include <new>

void *Create_W3D_Mem_Pool(char const *name, int size);
void *Allocate_From_W3D_Mem_Pool(void *pool, int size);
void Free_From_W3D_Mem_Pool(void *pool, void *data);

// I think this stands for W3DMemoryPoolObject
class W3DMPO
{
public:
    // This enforcer is a checker to make sure a class has been set up correctly with the implement pool macro.
    virtual int glueEnforcer() = 0;
    virtual ~W3DMPO() {}
};

#ifdef __SANITIZE_ADDRESS__
#define IMPLEMENT_NAMED_W3D_POOL(classname, poolname) \
public: \
    virtual int glueEnforcer() { return 4; }; \
\
private:
#else
#define IMPLEMENT_NAMED_W3D_POOL(classname, poolname) \
private: \
    static void *Get_Class_Pool() \
    { \
        static void *const The##classname##Pool = Create_W3D_Mem_Pool(#poolname, sizeof(classname)); \
        return The##classname##Pool; \
    } \
\
public: \
    void *operator new(size_t size) { return Allocate_From_W3D_Mem_Pool(Get_Class_Pool(), sizeof(classname)); } \
    void *operator new(size_t size, void *where) { return where; } \
    void operator delete(void *p, void *where) {} \
    void operator delete(void *ptr) { Free_From_W3D_Mem_Pool(Get_Class_Pool(), ptr); } \
    virtual int glueEnforcer() { return 4; }; \
\
private:
#endif

#define IMPLEMENT_W3D_POOL(classname) IMPLEMENT_NAMED_W3D_POOL(classname, classname)
