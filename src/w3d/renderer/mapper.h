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

    TextureMapperClass(unsigned int stage = 0);
    TextureMapperClass(const TextureMapperClass &that) : m_stage(that.m_stage) {}
    ~TextureMapperClass() override {}

    virtual int Mapper_ID() const { return MAPPER_ID_UNKNOWN; }
    virtual TextureMapperClass *Clone() const = 0;
    virtual bool Is_Time_Variant() { return false; }
    virtual void Apply(int uv_array_index) = 0;
    virtual void Reset() {}
    virtual bool Needs_Normals() { return false; }
    virtual void Calculate_Texture_Matrix(Matrix4 &matrix) = 0;

    void Set_Stage(int stage) { m_stage = stage; }
    int Get_Stage() const { return m_stage; }

protected:
    unsigned int m_stage;
};

class ScaleTextureMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(ScaleTextureMapperClass)
public:
    ScaleTextureMapperClass(const Vector2 &scale, unsigned int stage);
    ScaleTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    ScaleTextureMapperClass(const ScaleTextureMapperClass &that);
    ~ScaleTextureMapperClass() override {}

    int Mapper_ID() const override { return MAPPER_ID_SCALE; }

    TextureMapperClass *Clone() const override;

    void Apply(int32_t uv_array_index) override;

    void Calculate_Texture_Matrix(Matrix4 &matrix) override;

protected:
    Vector2 m_scale;
};

class LinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(LinearOffsetTextureMapperClass)
public:
    LinearOffsetTextureMapperClass(const Vector2 &offset_per_sec,
        const Vector2 &initial_offset,
        bool clamp_fix,
        const Vector2 &scale,
        unsigned int stage);
    LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass &that);

    ~LinearOffsetTextureMapperClass() override {}

    int Mapper_ID() const override { return MAPPER_ID_LINEAR_OFFSET; }

    TextureMapperClass *Clone() const override { return new LinearOffsetTextureMapperClass(*this); }

    bool Is_Time_Variant() override { return true; }

    void Reset() override;

    void Calculate_Texture_Matrix(Matrix4 &matrix) override;

    void Set_Current_UV_Offset(const Vector2 &current) { m_currentUVOffset = current; }
    Vector2 Get_Current_UV_Offset() { return m_currentUVOffset; }

    void Set_UV_Offset_Delta(const Vector2 &per_second)
    {
        m_uvOffsetDeltaPerMS = per_second;
        m_uvOffsetDeltaPerMS *= -0.001f;
    }

    void Set_Sync_Time(unsigned int time) { m_lastUsedSyncTime = time; }
    unsigned int Get_Sync_Time() { return m_lastUsedSyncTime; }

protected:
    Vector2 m_currentUVOffset;
    Vector2 m_uvOffsetDeltaPerMS;
    unsigned int m_lastUsedSyncTime;
    Vector2 m_initialUVOffset;
    bool m_clampFix;
};

class GridTextureMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(GridTextureMapperClass)
public:
    GridTextureMapperClass(
        float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int frame_offset, unsigned int stage);
    GridTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    GridTextureMapperClass(const GridTextureMapperClass &src);
    ~GridTextureMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_GRID; }
    TextureMapperClass *Clone() const override { return new GridTextureMapperClass(*this); }
    bool Is_Time_Variant() override { return true; }
    void Apply(int uv_array_index) override;
    void Reset() override;
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

    void Set_Frame(unsigned int frame) { m_currentFrame = frame; }
    void Set_Frame_Per_Second(float fps);

protected:
    void Initialize(float fps, unsigned int gridwidth_log2);
    void Update_Temporal_State();
    void Calculate_UV_Offset(float *u_offset, float *v_offset);

    int m_sign;
    unsigned int m_msPerFrame;
    float m_ooGridWidth;
    unsigned int m_gridWidthLog2;
    unsigned int m_lastFrame;
    unsigned int m_frameOffset;

    unsigned int m_remainder;
    unsigned int m_currentFrame;
    unsigned int m_lastUsedSyncTime;
};

class RotateTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(RotateTextureMapperClass)
public:
    RotateTextureMapperClass(float rad_per_sec, const Vector2 &center, const Vector2 &scale, unsigned int stage);
    RotateTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    RotateTextureMapperClass(const RotateTextureMapperClass &src);
    ~RotateTextureMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_ROTATE; }
    TextureMapperClass *Clone() const override { return new RotateTextureMapperClass(*this); }
    bool Is_Time_Variant() override { return true; }
    void Reset() override
    {
        m_currentAngle = 0.0f;
        m_lastUsedSyncTime = W3D::Get_Sync_Time();
    }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

private:
    float m_currentAngle;
    float m_radiansPerMilliSec;
    Vector2 m_center;
    unsigned int m_lastUsedSyncTime;
};

class SineLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(SineLinearOffsetTextureMapperClass)
public:
    SineLinearOffsetTextureMapperClass(const Vector3 &uafp, const Vector3 &vafp, const Vector2 &scale, unsigned int stage);
    SineLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    SineLinearOffsetTextureMapperClass(const SineLinearOffsetTextureMapperClass &src);
    ~SineLinearOffsetTextureMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_SINE_LINEAR_OFFSET; }
    TextureMapperClass *Clone() const override { return new SineLinearOffsetTextureMapperClass(*this); }
    bool Is_Time_Variant() override { return true; }
    void Reset() override
    {
        m_currentAngle = 0.0f;
        m_lastUsedSyncTime = W3D::Get_Sync_Time();
    }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

private:
    Vector3 m_uafp;
    Vector3 m_vafp;
    float m_currentAngle;
    unsigned int m_lastUsedSyncTime;
};

class StepLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(StepLinearOffsetTextureMapperClass)
public:
    StepLinearOffsetTextureMapperClass(
        const Vector2 &step, float steps_per_sec, bool clamp_fix, const Vector2 &scale, unsigned int stage);
    StepLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    StepLinearOffsetTextureMapperClass(const StepLinearOffsetTextureMapperClass &src);
    ~StepLinearOffsetTextureMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_STEP_LINEAR_OFFSET; }
    TextureMapperClass *Clone() const override { return new StepLinearOffsetTextureMapperClass(*this); }
    bool Is_Time_Variant() override { return true; }
    void Reset() override;
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

private:
    Vector2 m_step;
    float m_stepsPerMilliSec;
    Vector2 m_currentStep;
    float m_remainder;
    unsigned int m_lastUsedSyncTime;
    bool m_clampFix;
};

class ZigZagLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(ZigZagLinearOffsetTextureMapperClass)
public:
    ZigZagLinearOffsetTextureMapperClass(const Vector2 &speed, float period, const Vector2 &scale, unsigned int stage);
    ZigZagLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    ZigZagLinearOffsetTextureMapperClass(const ZigZagLinearOffsetTextureMapperClass &src);
    ~ZigZagLinearOffsetTextureMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_ZIGZAG_LINEAR_OFFSET; }
    TextureMapperClass *Clone() const override { return new ZigZagLinearOffsetTextureMapperClass(*this); }
    bool Is_Time_Variant() override { return true; }
    void Reset() override;
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

private:
    Vector2 m_speed;
    float m_period;
    float m_halfPeriod;
    float m_remainder;
    unsigned int m_lastUsedSyncTime;
};

class ClassicEnvironmentMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(ClassicEnvironmentMapperClass)
public:
    ClassicEnvironmentMapperClass(unsigned int stage) : TextureMapperClass(stage) {}
    ClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage){};
    ClassicEnvironmentMapperClass(const ClassicEnvironmentMapperClass &src) : TextureMapperClass(src) {}
    ~ClassicEnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_CLASSIC_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new ClassicEnvironmentMapperClass(*this); }
    bool Needs_Normals() override { return true; }
    void Apply(int uv_array_index) override;
    void Calculate_Texture_Matrix(Matrix4 &mat) override;
};

class EnvironmentMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(EnvironmentMapperClass)
public:
    EnvironmentMapperClass(unsigned int stage) : TextureMapperClass(stage) {}
    EnvironmentMapperClass(const EnvironmentMapperClass &src) : TextureMapperClass(src) {}
    ~EnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new EnvironmentMapperClass(*this); }
    bool Needs_Normals() override { return true; }
    void Apply(int uv_array_index) override;
    void Calculate_Texture_Matrix(Matrix4 &mat) override;
};

class EdgeMapperClass : public TextureMapperClass
{
    IMPLEMENT_W3D_POOL(EdgeMapperClass)
public:
    EdgeMapperClass(unsigned int stage);
    EdgeMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    EdgeMapperClass(const EdgeMapperClass &src);
    ~EdgeMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_EDGE; }
    TextureMapperClass *Clone() const override { return new EdgeMapperClass(*this); }
    bool Is_Time_Variant() override { return true; }
    void Apply(int uv_array_index) override;
    void Reset() override;
    bool Needs_Normals() override { return true; }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

protected:
    unsigned int m_lastUsedSyncTime;
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
    WSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    WSEnvMapperClass(unsigned int stage, AxisType axis) : TextureMapperClass(stage), m_axis(axis) {}
    WSEnvMapperClass(const WSEnvMapperClass &src) : TextureMapperClass(src), m_axis(src.m_axis) {}
    ~WSEnvMapperClass() override {}
    bool Needs_Normals() override { return true; }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

protected:
    AxisType m_axis;
};

class WSClassicEnvironmentMapperClass : public WSEnvMapperClass
{
    IMPLEMENT_W3D_POOL(WSClassicEnvironmentMapperClass)
public:
    WSClassicEnvironmentMapperClass(unsigned int stage, AxisType axis) : WSEnvMapperClass(stage, axis) {}
    WSClassicEnvironmentMapperClass(const WSEnvMapperClass &src) : WSEnvMapperClass(src) {}
    WSClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
        WSEnvMapperClass(ini, section, stage){};
    ~WSClassicEnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_WS_CLASSIC_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new WSClassicEnvironmentMapperClass(*this); }
    void Apply(int uv_array_index) override;
};

class WSEnvironmentMapperClass : public WSEnvMapperClass
{
    IMPLEMENT_W3D_POOL(WSEnvironmentMapperClass)
public:
    WSEnvironmentMapperClass(unsigned int stage, AxisType axis) : WSEnvMapperClass(stage, axis) {}
    WSEnvironmentMapperClass(const WSEnvMapperClass &src) : WSEnvMapperClass(src) {}
    WSEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
        WSEnvMapperClass(ini, section, stage){};
    ~WSEnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_WS_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new WSEnvironmentMapperClass(*this); }
    void Apply(int uv_array_index) override;
};

class GridClassicEnvironmentMapperClass : public GridTextureMapperClass
{
    IMPLEMENT_W3D_POOL(GridClassicEnvironmentMapperClass)
public:
    GridClassicEnvironmentMapperClass(
        float fps, unsigned int gridwidth, unsigned int last_frame, unsigned int offset, unsigned int stage) :
        GridTextureMapperClass(fps, gridwidth, last_frame, offset, stage)
    {
    }
    GridClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
        GridTextureMapperClass(ini, section, stage)
    {
    }
    GridClassicEnvironmentMapperClass(const GridTextureMapperClass &src) : GridTextureMapperClass(src) {}
    ~GridClassicEnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_GRID_CLASSIC_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new GridClassicEnvironmentMapperClass(*this); }
    void Apply(int uv_array_index) override;
    bool Needs_Normals() override { return true; }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;
};

class GridEnvironmentMapperClass : public GridTextureMapperClass
{
    IMPLEMENT_W3D_POOL(GridEnvironmentMapperClass)
public:
    GridEnvironmentMapperClass(
        float fps, unsigned int gridwidth, unsigned int last_frame, unsigned int offset, unsigned int stage) :
        GridTextureMapperClass(fps, gridwidth, last_frame, offset, stage)
    {
    }
    GridEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
        GridTextureMapperClass(ini, section, stage)
    {
    }
    GridEnvironmentMapperClass(const GridTextureMapperClass &src) : GridTextureMapperClass(src) {}
    ~GridEnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_GRID_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new GridEnvironmentMapperClass(*this); }
    void Apply(int uv_array_index) override;
    bool Needs_Normals() override { return true; }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;
};

class ScreenMapperClass : public LinearOffsetTextureMapperClass
{
    IMPLEMENT_W3D_POOL(ScreenMapperClass)
public:
    ScreenMapperClass(const Vector2 &offset_per_sec,
        const Vector2 &initial_offset,
        bool clamp_fix,
        const Vector2 &scale,
        unsigned int stage) :
        LinearOffsetTextureMapperClass(offset_per_sec, initial_offset, clamp_fix, scale, stage)
    {
    }
    ScreenMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
        LinearOffsetTextureMapperClass(ini, section, stage)
    {
    }
    ScreenMapperClass(const LinearOffsetTextureMapperClass &src) : LinearOffsetTextureMapperClass(src) {}
    ~ScreenMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_SCREEN; }
    TextureMapperClass *Clone() const override { return new ScreenMapperClass(*this); }
    void Apply(int uv_array_index) override;
    bool Is_Time_Variant() override { return true; }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;
};

class RandomTextureMapperClass : public ScaleTextureMapperClass
{
    IMPLEMENT_W3D_POOL(RandomTextureMapperClass)
public:
    RandomTextureMapperClass(float fps, const Vector2 &scale, unsigned int stage);
    RandomTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    RandomTextureMapperClass(const RandomTextureMapperClass &src);
    ~RandomTextureMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_RANDOM; }
    TextureMapperClass *Clone() const override { return new RandomTextureMapperClass(*this); }
    void Reset() override;
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

protected:
    void Randomize();
    float m_fpms;
    float m_remainder;
    float m_currentAngle;
    Vector2 m_center;
    Vector2 m_speed;
    unsigned int m_lastUsedSyncTime;
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
        unsigned int stage);
    BumpEnvTextureMapperClass(INIClass &ini, const char *section, unsigned int stage);
    BumpEnvTextureMapperClass(const BumpEnvTextureMapperClass &src);
    ~BumpEnvTextureMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_BUMPENV; }
    TextureMapperClass *Clone() const override { return new BumpEnvTextureMapperClass(*this); }
    void Apply(int uv_array_index) override;

protected:
    unsigned int m_lastUsedSyncTime;
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
    GridWSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage);
    GridWSEnvMapperClass(float fps,
        unsigned int gridwidth_log2,
        unsigned int last_frame,
        unsigned int offset,
        AxisType axis,
        unsigned int stage) :
        GridTextureMapperClass(fps, gridwidth_log2, last_frame, offset, stage), m_axis(AXIS_Z)
    {
    }
    GridWSEnvMapperClass(const GridWSEnvMapperClass &src) : GridTextureMapperClass(src), m_axis(src.m_axis) {}
    ~GridWSEnvMapperClass() override {}
    bool Needs_Normals() override { return true; }
    void Calculate_Texture_Matrix(Matrix4 &mat) override;

protected:
    AxisType m_axis;
};

class GridWSClassicEnvironmentMapperClass : public GridWSEnvMapperClass
{
public:
    GridWSClassicEnvironmentMapperClass(float fps,
        unsigned int gridwidth_log2,
        unsigned int last_frame,
        unsigned int offset,
        AxisType axis,
        unsigned int stage) :
        GridWSEnvMapperClass(fps, gridwidth_log2, last_frame, offset, axis, stage)
    {
    }
    GridWSClassicEnvironmentMapperClass(const GridWSEnvMapperClass &src) : GridWSEnvMapperClass(src) {}
    GridWSClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
        GridWSEnvMapperClass(ini, section, stage){};
    ~GridWSClassicEnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_GRID_WS_CLASSIC_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new GridWSClassicEnvironmentMapperClass(*this); }
    void Apply(int uv_array_index) override;
};

class GridWSEnvironmentMapperClass : public GridWSEnvMapperClass
{
public:
    GridWSEnvironmentMapperClass(float fps,
        unsigned int gridwidth_log2,
        unsigned int last_frame,
        unsigned int offset,
        AxisType axis,
        unsigned int stage) :
        GridWSEnvMapperClass(fps, gridwidth_log2, last_frame, offset, axis, stage)
    {
    }
    GridWSEnvironmentMapperClass(const GridWSEnvMapperClass &src) : GridWSEnvMapperClass(src) {}
    GridWSEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
        GridWSEnvMapperClass(ini, section, stage){};
    ~GridWSEnvironmentMapperClass() override {}
    int Mapper_ID() const override { return MAPPER_ID_GRID_WS_ENVIRONMENT; }
    TextureMapperClass *Clone() const override { return new GridWSEnvironmentMapperClass(*this); }
    void Apply(int uv_array_index) override;
};
