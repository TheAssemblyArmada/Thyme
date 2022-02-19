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
#include "vector3.h"
#include "w3d.h"
#include "w3dmpo.h"

class Matrix4;
class INIClass;

// Base class for all texture mappers.
class TextureMapperClass : public W3DMPO, public RefCountClass
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
        MAPPER_ID_GRID_WS_CLASSIC_ENVIRONMENT,
        MAPPER_ID_GRID_WS_ENVIRONMENT
    };

    TextureMapperClass(uint32_t stage = 0);
    TextureMapperClass(const TextureMapperClass &that) : m_stage(that.m_stage) {}
    virtual ~TextureMapperClass() {}

    virtual int32_t Mapper_ID() const { return MAPPER_ID_UNKNOWN; }
    virtual TextureMapperClass *Clone() const = 0;
    virtual bool Is_Time_Variant() { return false; }
    virtual void Apply(int32_t uv_array_index) = 0;
    virtual void Reset() {}
    virtual bool Needs_Normals() { return false; }
    virtual void Calculate_Texture_Matrix(Matrix4 &matrix) = 0;

    void Set_Stage(int32_t stage) { m_stage = stage; }
    int32_t Get_Stage() const { return m_stage; }

protected:
    uint32_t m_stage;
};

class ScaleTextureMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(ScaleTextureMapperClass)
public:
    ScaleTextureMapperClass(const Vector2 &scale, uint32_t stage);
    ScaleTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    ScaleTextureMapperClass(const ScaleTextureMapperClass &that);
    virtual ~ScaleTextureMapperClass() {}

    virtual int32_t Mapper_ID() const { return MAPPER_ID_SCALE; }

    virtual TextureMapperClass *Clone() const;

    virtual void Apply(int32_t uv_array_index);

    virtual void Calculate_Texture_Matrix(Matrix4 &matrix);

protected:
    Vector2 m_scale;
};

class LinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(LinearOffsetTextureMapperClass)
public:
    LinearOffsetTextureMapperClass(
        const Vector2 &offset_per_sec, const Vector2 &initial_offset, bool clamp_fix, const Vector2 &scale, uint32_t stage);
    LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass &that);

    virtual ~LinearOffsetTextureMapperClass() {}

    virtual int32_t Mapper_ID() const { return MAPPER_ID_LINEAR_OFFSET; }

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

class GridTextureMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(GridTextureMapperClass)
public:
    GridTextureMapperClass(float fps, uint32_t gridwidth_log2, uint32_t last_frame, uint32_t frame_offset, uint32_t stage);
    GridTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    GridTextureMapperClass(const GridTextureMapperClass &src);
    virtual ~GridTextureMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_GRID; }
    virtual TextureMapperClass *Clone() const { return new GridTextureMapperClass(*this); }
    virtual bool Is_Time_Variant() { return true; }
    virtual void Apply(int32_t uv_array_index);
    virtual void Reset();
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

    void Set_Frame(uint32_t frame) { m_currentFrame = frame; }
    void Set_Frame_Per_Second(float fps);

protected:
    void Initialize(float fps, uint32_t gridwidth_log2);
    void Update_Temporal_State();
    void Calculate_UV_Offset(float *u_offset, float *v_offset);

    int32_t m_sign;
    uint32_t m_msPerFrame;
    float m_ooGridWidth;
    uint32_t m_gridWidthLog2;
    uint32_t m_lastFrame;
    uint32_t m_frameOffset;

    uint32_t m_remainder;
    uint32_t m_currentFrame;
    uint32_t m_lastUsedSyncTime;
};

class RotateTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(RotateTextureMapperClass)
public:
    RotateTextureMapperClass(float rad_per_sec, const Vector2 &center, const Vector2 &scale, uint32_t stage);
    RotateTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    RotateTextureMapperClass(const RotateTextureMapperClass &src);
    virtual ~RotateTextureMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_ROTATE; }
    virtual TextureMapperClass *Clone() const { return new RotateTextureMapperClass(*this); }
    virtual bool Is_Time_Variant() { return true; }
    virtual void Reset()
    {
        m_currentAngle = 0.0f;
        m_lastUsedSyncTime = W3D::Get_Sync_Time();
    }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

private:
    float m_currentAngle;
    float m_radiansPerMilliSec;
    Vector2 m_center;
    uint32_t m_lastUsedSyncTime;
};

class SineLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(SineLinearOffsetTextureMapperClass)
public:
    SineLinearOffsetTextureMapperClass(const Vector3 &uafp, const Vector3 &vafp, const Vector2 &scale, uint32_t stage);
    SineLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    SineLinearOffsetTextureMapperClass(const SineLinearOffsetTextureMapperClass &src);
    virtual ~SineLinearOffsetTextureMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_SINE_LINEAR_OFFSET; }
    virtual TextureMapperClass *Clone() const { return new SineLinearOffsetTextureMapperClass(*this); }
    virtual bool Is_Time_Variant() { return true; }
    virtual void Reset()
    {
        m_currentAngle = 0.0f;
        m_lastUsedSyncTime = W3D::Get_Sync_Time();
    }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

private:
    Vector3 m_uafp;
    Vector3 m_vafp;
    float m_currentAngle;
    uint32_t m_lastUsedSyncTime;
};

class StepLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(StepLinearOffsetTextureMapperClass)
public:
    StepLinearOffsetTextureMapperClass(
        const Vector2 &step, float steps_per_sec, bool clamp_fix, const Vector2 &scale, uint32_t stage);
    StepLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    StepLinearOffsetTextureMapperClass(const StepLinearOffsetTextureMapperClass &src);
    virtual ~StepLinearOffsetTextureMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_STEP_LINEAR_OFFSET; }
    virtual TextureMapperClass *Clone() const { return new StepLinearOffsetTextureMapperClass(*this); }
    virtual bool Is_Time_Variant() { return true; }
    virtual void Reset();
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

private:
    Vector2 m_step;
    float m_stepsPerMilliSec;
    Vector2 m_currentStep;
    float m_remainder;
    uint32_t m_lastUsedSyncTime;
    bool m_clampFix;
};

class ZigZagLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(ZigZagLinearOffsetTextureMapperClass)
public:
    ZigZagLinearOffsetTextureMapperClass(const Vector2 &speed, float period, const Vector2 &scale, uint32_t stage);
    ZigZagLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    ZigZagLinearOffsetTextureMapperClass(const ZigZagLinearOffsetTextureMapperClass &src);
    virtual ~ZigZagLinearOffsetTextureMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_ZIGZAG_LINEAR_OFFSET; }
    virtual TextureMapperClass *Clone() const { return new ZigZagLinearOffsetTextureMapperClass(*this); }
    virtual bool Is_Time_Variant() { return true; }
    virtual void Reset();
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

private:
    Vector2 m_speed;
    float m_period;
    float m_halfPeriod;
    float m_remainder;
    uint32_t m_lastUsedSyncTime;
};

class ClassicEnvironmentMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(ClassicEnvironmentMapperClass)
public:
    ClassicEnvironmentMapperClass(uint32_t stage) : TextureMapperClass(stage) {}
    ClassicEnvironmentMapperClass(const INIClass &ini, const char *section, uint32_t stage){};
    ClassicEnvironmentMapperClass(const ClassicEnvironmentMapperClass &src) : TextureMapperClass(src) {}
    virtual ~ClassicEnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_CLASSIC_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new ClassicEnvironmentMapperClass(*this); }
    virtual bool Needs_Normals() { return true; }
    virtual void Apply(int32_t uv_array_index);
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);
};

class EnvironmentMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(EnvironmentMapperClass)
public:
    EnvironmentMapperClass(uint32_t stage) : TextureMapperClass(stage) {}
    EnvironmentMapperClass(const EnvironmentMapperClass &src) : TextureMapperClass(src) {}
    virtual ~EnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new EnvironmentMapperClass(*this); }
    virtual bool Needs_Normals() { return true; }
    virtual void Apply(int32_t uv_array_index);
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);
};

class EdgeMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(EdgeMapperClass)
public:
    EdgeMapperClass(uint32_t stage);
    EdgeMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    EdgeMapperClass(const EdgeMapperClass &src);
    virtual ~EdgeMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_EDGE; }
    virtual TextureMapperClass *Clone() const { return new EdgeMapperClass(*this); }
    virtual bool Is_Time_Variant() { return true; }
    virtual void Apply(int32_t uv_array_index);
    virtual void Reset();
    virtual bool Needs_Normals() { return true; }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

protected:
    uint32_t m_lastUsedSyncTime;
    float m_vSpeed;
    float m_vOffset;
    bool m_useReflect;
};

class WSEnvMapperClass : public TextureMapperClass
{
public:
    enum AxisType
    {
        AXIS_X = 0,
        AXIS_Y = 1,
        AXIS_Z = 2,
    };
    WSEnvMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    WSEnvMapperClass(uint32_t stage, AxisType axis) : TextureMapperClass(stage), m_axis(axis) {}
    WSEnvMapperClass(const WSEnvMapperClass &src) : TextureMapperClass(src), m_axis(src.m_axis) {}
    virtual ~WSEnvMapperClass() {}
    virtual bool Needs_Normals() { return true; }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

protected:
    AxisType m_axis;
};

class WSClassicEnvironmentMapperClass : public WSEnvMapperClass
{
    IMPLEMENT_W3D_POOL(WSClassicEnvironmentMapperClass)
public:
    WSClassicEnvironmentMapperClass(uint32_t stage, AxisType axis) : WSEnvMapperClass(stage, axis) {}
    WSClassicEnvironmentMapperClass(const WSEnvMapperClass &src) : WSEnvMapperClass(src) {}
    WSClassicEnvironmentMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
        WSEnvMapperClass(ini, section, stage){};
    virtual ~WSClassicEnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_WS_CLASSIC_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new WSClassicEnvironmentMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);
};

class WSEnvironmentMapperClass : public WSEnvMapperClass
{
    IMPLEMENT_W3D_POOL(WSEnvironmentMapperClass)
public:
    WSEnvironmentMapperClass(uint32_t stage, AxisType axis) : WSEnvMapperClass(stage, axis) {}
    WSEnvironmentMapperClass(const WSEnvMapperClass &src) : WSEnvMapperClass(src) {}
    WSEnvironmentMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
        WSEnvMapperClass(ini, section, stage){};
    virtual ~WSEnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_WS_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new WSEnvironmentMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);
};

class GridClassicEnvironmentMapperClass : public GridTextureMapperClass
{
    IMPLEMENT_W3D_POOL(GridClassicEnvironmentMapperClass)
public:
    GridClassicEnvironmentMapperClass(float fps, uint32_t gridwidth, uint32_t last_frame, uint32_t offset, uint32_t stage) :
        GridTextureMapperClass(fps, gridwidth, last_frame, offset, stage)
    {
    }
    GridClassicEnvironmentMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
        GridTextureMapperClass(ini, section, stage)
    {
    }
    GridClassicEnvironmentMapperClass(const GridTextureMapperClass &src) : GridTextureMapperClass(src) {}
    virtual ~GridClassicEnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_GRID_CLASSIC_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new GridClassicEnvironmentMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);
    virtual bool Needs_Normals() { return true; }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);
};

class GridEnvironmentMapperClass : public GridTextureMapperClass
{
    IMPLEMENT_W3D_POOL(GridEnvironmentMapperClass)
public:
    GridEnvironmentMapperClass(float fps, uint32_t gridwidth, uint32_t last_frame, uint32_t offset, uint32_t stage) :
        GridTextureMapperClass(fps, gridwidth, last_frame, offset, stage)
    {
    }
    GridEnvironmentMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
        GridTextureMapperClass(ini, section, stage)
    {
    }
    GridEnvironmentMapperClass(const GridTextureMapperClass &src) : GridTextureMapperClass(src) {}
    virtual ~GridEnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_GRID_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new GridEnvironmentMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);
    virtual bool Needs_Normals() { return true; }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);
};

class ScreenMapperClass : public LinearOffsetTextureMapperClass
{
    IMPLEMENT_W3D_POOL(ScreenMapperClass)
public:
    ScreenMapperClass(
        const Vector2 &offset_per_sec, const Vector2 &initial_offset, bool clamp_fix, const Vector2 &scale, uint32_t stage) :
        LinearOffsetTextureMapperClass(offset_per_sec, initial_offset, clamp_fix, scale, stage)
    {
    }
    ScreenMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
        LinearOffsetTextureMapperClass(ini, section, stage)
    {
    }
    ScreenMapperClass(const LinearOffsetTextureMapperClass &src) : LinearOffsetTextureMapperClass(src) {}
    virtual ~ScreenMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_SCREEN; }
    virtual TextureMapperClass *Clone() const { return new ScreenMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);
    virtual bool Is_Time_Variant() { return true; }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);
};

class RandomTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(RandomTextureMapperClass)
public:
    RandomTextureMapperClass(float fps, const Vector2 &scale, uint32_t stage);
    RandomTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    RandomTextureMapperClass(const RandomTextureMapperClass &src);
    virtual ~RandomTextureMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_RANDOM; }
    virtual TextureMapperClass *Clone() const { return new RandomTextureMapperClass(*this); }
    virtual void Reset();
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

protected:
    void Randomize();
    float m_fpms;
    float m_remainder;
    float m_currentAngle;
    Vector2 m_center;
    Vector2 m_speed;
    uint32_t m_lastUsedSyncTime;
};

class BumpEnvTextureMapperClass : public LinearOffsetTextureMapperClass
{
    IMPLEMENT_W3D_POOL(BumpEnvTextureMapperClass)
public:
    BumpEnvTextureMapperClass(float rad_per_sec,
        float scale_factor,
        const Vector2 &offset_per_sec,
        const Vector2 &initial_offset,
        bool clamp_fix,
        const Vector2 &scale,
        uint32_t stage);
    BumpEnvTextureMapperClass(INIClass &ini, const char *section, uint32_t stage);
    BumpEnvTextureMapperClass(const BumpEnvTextureMapperClass &src);
    virtual ~BumpEnvTextureMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_BUMPENV; }
    virtual TextureMapperClass *Clone() const { return new BumpEnvTextureMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);

protected:
    uint32_t m_lastUsedSyncTime;
    float m_currentAngle;
    float m_radiansPerSecond;
    float m_scaleFactor;
};

class GridWSEnvMapperClass : public GridTextureMapperClass
{
public:
    enum AxisType
    {
        AXIS_X = 0,
        AXIS_Y = 1,
        AXIS_Z = 2,
    };
    GridWSEnvMapperClass(const INIClass &ini, const char *section, uint32_t stage);
    GridWSEnvMapperClass(
        float fps, uint32_t gridwidth_log2, uint32_t last_frame, uint32_t offset, AxisType axis, uint32_t stage) :
        GridTextureMapperClass(fps, gridwidth_log2, last_frame, offset, stage), m_axis(AXIS_Z)
    {
    }
    GridWSEnvMapperClass(const GridWSEnvMapperClass &src) : GridTextureMapperClass(src), m_axis(src.m_axis) {}
    virtual ~GridWSEnvMapperClass() {}
    virtual bool Needs_Normals() { return true; }
    virtual void Calculate_Texture_Matrix(Matrix4 &mat);

protected:
    AxisType m_axis;
};

class GridWSClassicEnvironmentMapperClass : public GridWSEnvMapperClass
{
public:
    GridWSClassicEnvironmentMapperClass(
        float fps, uint32_t gridwidth_log2, uint32_t last_frame, uint32_t offset, AxisType axis, uint32_t stage) :
        GridWSEnvMapperClass(fps, gridwidth_log2, last_frame, offset, axis, stage)
    {
    }
    GridWSClassicEnvironmentMapperClass(const GridWSEnvMapperClass &src) : GridWSEnvMapperClass(src) {}
    GridWSClassicEnvironmentMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
        GridWSEnvMapperClass(ini, section, stage){};
    virtual ~GridWSClassicEnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_GRID_WS_CLASSIC_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new GridWSClassicEnvironmentMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);
};

class GridWSEnvironmentMapperClass : public GridWSEnvMapperClass
{
public:
    GridWSEnvironmentMapperClass(
        float fps, uint32_t gridwidth_log2, uint32_t last_frame, uint32_t offset, AxisType axis, uint32_t stage) :
        GridWSEnvMapperClass(fps, gridwidth_log2, last_frame, offset, axis, stage)
    {
    }
    GridWSEnvironmentMapperClass(const GridWSEnvMapperClass &src) : GridWSEnvMapperClass(src) {}
    GridWSEnvironmentMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
        GridWSEnvMapperClass(ini, section, stage){};
    virtual ~GridWSEnvironmentMapperClass() {}
    virtual int32_t Mapper_ID() const { return MAPPER_ID_GRID_WS_ENVIRONMENT; }
    virtual TextureMapperClass *Clone() const { return new GridWSEnvironmentMapperClass(*this); }
    virtual void Apply(int32_t uv_array_index);
};
