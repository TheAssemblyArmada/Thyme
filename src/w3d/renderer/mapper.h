/**
 * @file
 *
 * @author tomsons26
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
#include "vector2.h"
#include "w3dmpo.h"

class Matrix4;
class INIClass;

// Base class for all texture mappers.
class TextureMapperClass : public W3DMPO, public RefCountClass
{
    IMPLEMENT_W3D_POOL(TextureMapperClass)
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
        MAPPER_ID_GRID_WS_CLASSIC_ENVIRONMENT,
        MAPPER_ID_GRID_WS_ENVIRONMENT
    };

    TextureMapperClass(uint32_t stage = 0);
    TextureMapperClass(const TextureMapperClass &that) : m_stage(that.m_stage) {}
    virtual ~TextureMapperClass() {}

    virtual int Mapper_ID() const { return MAPPER_ID_UNKNOWN; }
    virtual TextureMapperClass *Clone() const = 0;
    virtual bool Is_Time_Variant() { return false; }
    virtual void Apply(int uv_array_index) = 0;
    virtual void Reset() {}
    virtual bool Needs_Normals() { return false; }
    virtual void Calculate_Texture_Matrix(Matrix4 &matrix) = 0;

    void Set_Stage(int stage) { m_stage = stage; }
    int Get_Stage(void) const { return m_stage; }

protected:
    uint32_t m_stage;
};

// Scales UV coordinates
class ScaleTextureMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(ScaleTextureMapperClass)
public:
    ScaleTextureMapperClass(const Vector2 &scale, uint32_t stage);
    ScaleTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    ScaleTextureMapperClass(const ScaleTextureMapperClass &that);
    virtual ~ScaleTextureMapperClass() {}

    virtual int Mapper_ID() const { return MAPPER_ID_SCALE; }

    virtual TextureMapperClass *Clone() const;

    virtual void Apply(int32_t uv_array_index);

    virtual void Calculate_Texture_Matrix(Matrix4 &matrix);

protected:
    Vector2 m_scale;
};

// Modifies the UV coodinates by a linear offset
class LinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(LinearOffsetTextureMapperClass)
public:
    LinearOffsetTextureMapperClass(
        const Vector2 &offset_per_sec, const Vector2 &initial_offset, bool clamp_fix, const Vector2 &scale, uint32_t stage);
    LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass &that);

    virtual ~LinearOffsetTextureMapperClass() {}

    virtual int Mapper_ID() const { return MAPPER_ID_LINEAR_OFFSET; }

    virtual TextureMapperClass *Clone() const { return new LinearOffsetTextureMapperClass(*this); }

    virtual bool Is_Time_Variant() { return true; }

    virtual void Reset();

    virtual void Calculate_Texture_Matrix(Matrix4 &matrix);

    void Set_Current_UV_Offset(const Vector2 &current) { m_currentUVOffset = current; }
    Vector2 Get_Current_UV_Offset() { return m_currentUVOffset; }

    void Set_UV_Offset_Delta(const Vector2 &per_second)
    {
        m_uvOffsetDeltaPerMS = per_second;
        m_uvOffsetDeltaPerMS *= -0.001f;
    }

    void Set_Sync_Time(uint32_t time) { m_lastUsedSyncTime = time; }
    uint32_t Get_Sync_Time() { return m_lastUsedSyncTime; }

protected:
    Vector2 m_currentUVOffset;
    Vector2 m_uvOffsetDeltaPerMS;
    uint32_t m_lastUsedSyncTime;
    Vector2 m_initialUVOffset;
    bool m_clampFix;
};
