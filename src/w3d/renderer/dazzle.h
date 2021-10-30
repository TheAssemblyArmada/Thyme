/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief dazzle
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
#include "proto.h"
#include "w3d_file.h"

class DazzleLoaderClass : public PrototypeLoaderClass
{
public:
    virtual int Chunk_Type() override { return W3D_CHUNK_DAZZLE; }
    virtual PrototypeClass *Load_W3D(ChunkLoadClass &cload) override;
};
