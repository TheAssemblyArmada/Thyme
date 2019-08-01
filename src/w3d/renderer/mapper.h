/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Texture mapping related classes.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "refcount.h"
#include "w3dmpo.h"

class Matrix4;

class TextureMapperClass : public W3DMPO, RefCountClass
{
public:
    enum
    {
        MAPPER_ID_UNKNOWN,
        MAPPER_ID_LINEAR_OFFSET,
        MAPPER_ID_CLASSIC_ENVIRONMENT,
        MAPPER_ID_ENVIRONMENT,
        MAPPER_ID_SCREEN,
        MAPPER_ID_ANIMATING_1D,
        MAPPER_ID_AXIAL,
        MAPPER_ID_SILHOUETTE,
        MAPPER_ID_SCALE,
        MAPPER_ID_GRID,
        MAPPER_ID_ROTATE,
        MAPPER_ID_SINE_LINEAR_OFFSET,
        MAPPER_ID_STEP_LINEAR_OFFSET,
        MAPPER_ID_ZIGZAG_LINEAR_OFFSET,
        MAPPER_ID_WS_CLASSIC_ENVIRONMENT,
        MAPPER_ID_WS_ENVIRONMENT,
        MAPPER_ID_GRID_CLASSIC_ENVIRONMENT,
        MAPPER_ID_GRID_ENVIRONMENT,
        MAPPER_ID_RANDOM,
        MAPPER_ID_EDGE,
        MAPPER_ID_BUMPENV,
    };

    TextureMapperClass(uint32_t stage = 0);
    TextureMapperClass(const TextureMapperClass &that) : m_stage(that.m_stage) {}

    virtual int Mapper_ID() const { return MAPPER_ID_UNKNOWN; }
    virtual TextureMapperClass *Clone() const = 0;
    virtual bool Is_Time_Variant() { return false; }
    virtual void Apply(int uv_array_index) = 0;
    virtual void Reset() {}
    virtual bool Needs_Normals() { return false; }
    virtual void Calculate_Texture_Matrix(Matrix4 &matrix) = 0;

private:
    uint32_t m_stage;
};
