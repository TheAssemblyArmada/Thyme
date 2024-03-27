/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Prototype Loader Interface
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

class ChunkLoadClass;
class PrototypeClass;
class RenderObjClass;

class PrototypeLoaderClass
{
public:
    virtual int32_t Chunk_Type() = 0;
    virtual PrototypeClass *Load_W3D(ChunkLoadClass &chunk) = 0;
};

class PrototypeClass
{
public:
    // #BUGFIX Initialize all members
    PrototypeClass() : m_nextHash(nullptr) {}

    virtual const char *Get_Name() const = 0;
    virtual int32_t Get_Class_ID() const = 0;
    virtual RenderObjClass *Create() = 0;
    virtual void Delete_Self() = 0;
    virtual ~PrototypeClass(){};

    PrototypeClass *Get_Next_Hash() const { return m_nextHash; }
    void Set_Next_Hash(PrototypeClass *next) { m_nextHash = next; }

private:
    PrototypeClass *m_nextHash;
};
