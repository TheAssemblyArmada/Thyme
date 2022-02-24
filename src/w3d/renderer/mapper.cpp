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
#include "mapper.h"
#include "dx8wrapper.h"
#include "gamemath.h"
#include "iniclass.h"
#include "matrix4.h"
#include "meshmatdesc.h"
#include "random.h"
#include "w3d.h"
#include <cmath>

Random4Class rand4;

TextureMapperClass::TextureMapperClass(unsigned int stage) : m_stage(stage)
{
    if (m_stage >= MeshMatDescClass::MAX_TEX_STAGES) {
        m_stage = MeshMatDescClass::MAX_TEX_STAGES - 1;
    }
}

ScaleTextureMapperClass::ScaleTextureMapperClass(const Vector2 &scale, unsigned int stage) :
    TextureMapperClass(stage), m_scale(scale)
{
}

ScaleTextureMapperClass::ScaleTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
    TextureMapperClass(stage)
{
    m_scale.U = ini.Get_Float(section, "UScale", 1.0f);
    m_scale.V = ini.Get_Float(section, "VScale", 1.0f);
}

ScaleTextureMapperClass::ScaleTextureMapperClass(const ScaleTextureMapperClass &that) :
    TextureMapperClass(that), m_scale(that.m_scale)
{
}

TextureMapperClass *ScaleTextureMapperClass::Clone() const
{
    return new ScaleTextureMapperClass(*this);
}

void ScaleTextureMapperClass::Apply(int32_t uv_array_index)
{
    Matrix4 m;

    Calculate_Texture_Matrix(m);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), m);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, uv_array_index);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void ScaleTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &m)
{
    m.Make_Identity();
    m[0].X = m_scale.U;
    m[1].Y = m_scale.V;
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(
    const Vector2 &offset_per_sec, const Vector2 &initial_offset, bool clamp_fix, const Vector2 &scale, unsigned int stage) :
    ScaleTextureMapperClass(scale, stage),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_initialUVOffset(initial_offset),
    m_clampFix(clamp_fix)
{
    m_currentUVOffset = m_initialUVOffset;
    m_uvOffsetDeltaPerMS = offset_per_sec * -0.001f;
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(
    const INIClass &ini, const char *section, unsigned int stage) :

    ScaleTextureMapperClass(ini, section, stage), m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    float u_offset_per_sec = ini.Get_Float(section, "UPerSec", 0.0f);
    float v_offset_per_sec = ini.Get_Float(section, "VPerSec", 0.0f);
    m_uvOffsetDeltaPerMS = Vector2(u_offset_per_sec, v_offset_per_sec) * -0.001f;

    float initial_u_offset = ini.Get_Float(section, "UOffset", 0.0f);
    float initial_v_offset = ini.Get_Float(section, "VOffset", 0.0f);
    m_initialUVOffset = Vector2(initial_u_offset, initial_v_offset);
    m_currentUVOffset = m_initialUVOffset;

    m_clampFix = ini.Get_Bool(section, "ClampFix", false);
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass &that) :
    ScaleTextureMapperClass(that),
    m_uvOffsetDeltaPerMS(that.m_uvOffsetDeltaPerMS),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_initialUVOffset(that.m_initialUVOffset),
    m_clampFix(that.m_clampFix)
{
    m_currentUVOffset = m_initialUVOffset;
}

void LinearOffsetTextureMapperClass::Reset()
{
    Set_Current_UV_Offset(Vector2(0.0f, 0.0f));
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
}

void LinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &matrix)
{
    float delta = (float)W3D::Get_Sync_Time() - m_lastUsedSyncTime;
    float offset_u = m_currentUVOffset.X + m_uvOffsetDeltaPerMS.X * delta;
    float offset_v = m_currentUVOffset.Y + m_uvOffsetDeltaPerMS.Y * delta;

    if (m_clampFix) {
        offset_u = std::clamp(offset_u, -m_scale.X, m_scale.X);
        offset_v = std::clamp(offset_v, -m_scale.Y, m_scale.Y);
    } else {
        offset_u -= GameMath::Floor(offset_u);
        offset_v -= GameMath::Floor(offset_v);
    }

    matrix.Make_Identity();
    matrix[0].Z = offset_u;
    matrix[0].X = m_scale.X;
    matrix[1].Z = offset_v;
    matrix[1].Y = m_scale.Y;

    m_currentUVOffset.X = offset_u;
    m_currentUVOffset.Y = offset_v;
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
}

GridTextureMapperClass::GridTextureMapperClass(
    float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int frame_offset, unsigned int stage) :
    TextureMapperClass(stage), m_lastFrame(last_frame), m_frameOffset(frame_offset)
{
    Initialize(fps, gridwidth_log2);
}

GridTextureMapperClass::GridTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
    TextureMapperClass(stage)
{
    float fps = ini.Get_Float(section, "FPS", 1.0f);
    unsigned int gridwidth_log2 = ini.Get_Int(section, "Log2Width", 1);
    m_lastFrame = ini.Get_Int(section, "Last", 0);
    m_frameOffset = ini.Get_Int(section, "Offset", 0);
    Initialize(fps, gridwidth_log2);
}

GridTextureMapperClass::GridTextureMapperClass(const GridTextureMapperClass &src) :
    TextureMapperClass(src),
    m_sign(src.m_sign),
    m_msPerFrame(src.m_msPerFrame),
    m_ooGridWidth(src.m_ooGridWidth),
    m_gridWidthLog2(src.m_gridWidthLog2),
    m_lastFrame(src.m_lastFrame),
    m_frameOffset(src.m_frameOffset)
{
    GridTextureMapperClass::Reset();
}

void GridTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    Update_Temporal_State();

    float u_offset, v_offset;
    Calculate_UV_Offset(&u_offset, &v_offset);

    mat.Make_Identity();
    mat[0].Z = u_offset;
    mat[1].Z = v_offset;
}

void GridTextureMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | uv_array_index);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void GridTextureMapperClass::Reset()
{
    m_remainder = 0;
    m_currentFrame = m_sign == -1 ? (m_lastFrame - m_frameOffset - 1) : m_frameOffset;
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
}

void GridTextureMapperClass::Set_Frame_Per_Second(float fps)
{
    Initialize(fps, m_gridWidthLog2);
}

void GridTextureMapperClass::Initialize(float fps, unsigned int gridwidth_log2)
{
    unsigned int grid_width = (1 << gridwidth_log2);

    if (m_lastFrame == 0) {
        m_lastFrame = (grid_width * grid_width);
    }

    unsigned int offset = m_frameOffset % m_lastFrame;
    m_frameOffset = offset;
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
    m_gridWidthLog2 = gridwidth_log2;
    m_ooGridWidth = 1.0f / (float)(grid_width);

    if (fps == 0.0f) {
        m_currentFrame = offset;
        m_sign = 0;
        m_msPerFrame = 1;
    } else if (fps < 0.0f) {
        m_sign = -1;
        m_msPerFrame = (unsigned int)(1000.0f / GameMath::Fabs(fps));
        m_currentFrame = m_lastFrame - offset - 1;
    } else {
        m_sign = 1;
        m_msPerFrame = (unsigned int)(1000.0f / GameMath::Fabs(fps));
        m_currentFrame = offset;
    }

    m_remainder = 0;
}

void GridTextureMapperClass::Update_Temporal_State()
{
    unsigned int now = W3D::Get_Sync_Time();
    unsigned int delta = now - m_lastUsedSyncTime;
    m_remainder += delta;
    m_lastUsedSyncTime = now;

    int new_frame = (int)m_currentFrame + ((int)(m_remainder / m_msPerFrame) * m_sign);
    new_frame = new_frame % m_lastFrame;

    if (new_frame < 0) {
        m_currentFrame = m_lastFrame + new_frame;
    } else {
        m_currentFrame = (unsigned int)new_frame;
    }

    m_remainder = m_remainder % m_msPerFrame;
}

void GridTextureMapperClass::Calculate_UV_Offset(float *u_offset, float *v_offset)
{
    unsigned int row_mask = ~(0xFFFFFFFF << m_gridWidthLog2);
    unsigned int col_mask = row_mask << m_gridWidthLog2;
    unsigned int x = m_currentFrame & row_mask;
    unsigned int y = (m_currentFrame & col_mask) >> m_gridWidthLog2;
    *u_offset = x * m_ooGridWidth;
    *v_offset = y * m_ooGridWidth;
}

RotateTextureMapperClass::RotateTextureMapperClass(
    float rad_per_sec, const Vector2 &center, const Vector2 &scale, unsigned int stage) :
    ScaleTextureMapperClass(scale, stage),
    m_currentAngle(0.0f),
    m_radiansPerMilliSec(rad_per_sec / 1000.0f),
    m_center(center),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
}

RotateTextureMapperClass::RotateTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
    ScaleTextureMapperClass(ini, section, stage), m_currentAngle(0.0f), m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    m_radiansPerMilliSec = 2 * GAMEMATH_PI * ini.Get_Float(section, "Speed", 0.1f) / 1000.0f;
    m_center.U = ini.Get_Float(section, "UCenter", 0.0f);
    m_center.V = ini.Get_Float(section, "VCenter", 0.0f);
}

RotateTextureMapperClass::RotateTextureMapperClass(const RotateTextureMapperClass &src) :
    ScaleTextureMapperClass(src),
    m_currentAngle(0.0f),
    m_radiansPerMilliSec(src.m_radiansPerMilliSec),
    m_center(src.m_center),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
}

void RotateTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    unsigned int now = W3D::Get_Sync_Time();
    unsigned int delta = now - m_lastUsedSyncTime;
    m_lastUsedSyncTime = now;

    m_currentAngle += m_radiansPerMilliSec * delta;
    m_currentAngle = fmod(m_currentAngle, 2 * GAMEMATH_PI);

    if (m_currentAngle < 0.0f) {
        m_currentAngle += 2 * GAMEMATH_PI;
    }

    float c = GameMath::Cos(m_currentAngle);
    float s = GameMath::Sin(m_currentAngle);

    mat.Make_Identity();
    mat[0].Set(m_scale.X * c, -m_scale.X * s, -m_scale.X * (c * m_center.U - s * m_center.V - m_center.U), 0.0f);
    mat[1].Set(m_scale.Y * s, m_scale.Y * c, -m_scale.Y * (s * m_center.U + c * m_center.V - m_center.V), 0.0f);
}

SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(
    const Vector3 &uafp, const Vector3 &vafp, const Vector2 &scale, unsigned int stage) :
    ScaleTextureMapperClass(scale, stage),
    m_uafp(uafp),
    m_vafp(vafp),
    m_currentAngle(0.0f),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
}

SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(
    const INIClass &ini, const char *section, unsigned int stage) :
    ScaleTextureMapperClass(ini, section, stage), m_currentAngle(0.0f), m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    m_uafp.X = ini.Get_Float(section, "UAmp", 1.0f);
    m_uafp.Y = ini.Get_Float(section, "UFreq", 1.0f);
    m_uafp.Z = ini.Get_Float(section, "UPhase", 0.0f);

    m_vafp.X = ini.Get_Float(section, "VAmp", 1.0f);
    m_vafp.Y = ini.Get_Float(section, "VFreq", 1.0f);
    m_vafp.Z = ini.Get_Float(section, "VPhase", 0.0f);
}

SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(const SineLinearOffsetTextureMapperClass &src) :
    ScaleTextureMapperClass(src),
    m_uafp(src.m_uafp),
    m_vafp(src.m_vafp),
    m_currentAngle(0.0f),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
}

void SineLinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    unsigned int now = W3D::Get_Sync_Time();
    unsigned int delta = now - m_lastUsedSyncTime;
    m_lastUsedSyncTime = now;

    const float ms_to_radians = 2 * GAMEMATH_PI / 1000.0f;

    m_currentAngle += delta * ms_to_radians;

    float offset_u = m_uafp.X * GameMath::Sin(m_uafp.Y * m_currentAngle + m_uafp.Z * GAMEMATH_PI);
    float offset_v = m_vafp.X * GameMath::Sin(m_vafp.Y * m_currentAngle + m_vafp.Z * GAMEMATH_PI);

    mat.Make_Identity();
    mat[0].Z = offset_u;
    mat[0].X = m_scale.X;
    mat[1].Z = offset_v;
    mat[1].Y = m_scale.Y;
}

StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(
    const Vector2 &step, float steps_per_sec, bool clamp_fix, const Vector2 &scale, unsigned int stage) :
    ScaleTextureMapperClass(scale, stage),
    m_step(step),
    m_stepsPerMilliSec(steps_per_sec / 1000.0f),
    m_currentStep(0.0f, 0.0f),
    m_remainder(0),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_clampFix(clamp_fix)
{
}

StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(
    const INIClass &ini, const char *section, unsigned int stage) :
    ScaleTextureMapperClass(ini, section, stage),
    m_currentStep(0.0f, 0.0f),
    m_remainder(0),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    m_step.U = ini.Get_Float(section, "UStep", 0.0f);
    m_step.V = ini.Get_Float(section, "VStep", 0.0f);
    m_stepsPerMilliSec = ini.Get_Float(section, "SPS", 0.0f) / 1000.0f;
    m_clampFix = ini.Get_Bool(section, "ClampFix", false);
}

StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(const StepLinearOffsetTextureMapperClass &src) :
    ScaleTextureMapperClass(src),
    m_step(src.m_step),
    m_stepsPerMilliSec(src.m_stepsPerMilliSec),
    m_currentStep(0.0f, 0.0f),
    m_remainder(0),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_clampFix(src.m_clampFix)
{
}

void StepLinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    unsigned int now = W3D::Get_Sync_Time();
    unsigned int delta = now - m_lastUsedSyncTime;
    m_lastUsedSyncTime = now;

    m_remainder += delta;
    int num_steps = (int)(m_stepsPerMilliSec * m_remainder);

    if (num_steps != 0) {
        m_currentStep += m_step * (float)num_steps;
        m_remainder -= num_steps / (float)m_stepsPerMilliSec;
    }

    if (m_clampFix) {
        m_currentStep.U = std::clamp(m_currentStep.U, -m_scale.X, m_scale.X);
        m_currentStep.V = std::clamp(m_currentStep.V, -m_scale.Y, m_scale.Y);
    } else {
        m_currentStep.U -= GameMath::Floor(m_currentStep.U);
        m_currentStep.V -= GameMath::Floor(m_currentStep.V);
    }

    mat.Make_Identity();
    mat[0].Z = m_currentStep.U;
    mat[0].X = m_scale.X;
    mat[1].Z = m_currentStep.V;
    mat[1].Y = m_scale.Y;
}

void StepLinearOffsetTextureMapperClass::Reset()
{
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
    m_currentStep.Set(0.0f, 0.0f);
    m_remainder = 0;
}

ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(
    const Vector2 &speed, float period, const Vector2 &scale, unsigned int stage) :
    ScaleTextureMapperClass(scale, stage),
    m_speed(speed / 1000.0f),
    m_period(period * 1000.0f),
    m_remainder(0),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    if (m_period < 0.0f) {
        m_period = -m_period;
    }

    m_halfPeriod = 0.5f * m_period;
}

ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(
    const INIClass &ini, const char *section, unsigned int stage) :
    ScaleTextureMapperClass(ini, section, stage), m_remainder(0), m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    m_speed.U = ini.Get_Float(section, "UPerSec", 0.0f) / 1000.0f;
    m_speed.V = ini.Get_Float(section, "VPerSec", 0.0f) / 1000.0f;
    m_period = ini.Get_Float(section, "Period", 0.0f) * 1000.0f;

    if (m_period < 0.0f) {
        m_period = -m_period;
    }

    m_halfPeriod = 0.5f * m_period;
}

ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(const ZigZagLinearOffsetTextureMapperClass &src) :
    ScaleTextureMapperClass(src),
    m_speed(src.m_speed),
    m_period(src.m_period),
    m_halfPeriod(src.m_halfPeriod),
    m_remainder(0),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
}

void ZigZagLinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    unsigned int now = W3D::Get_Sync_Time();
    unsigned int delta = now - m_lastUsedSyncTime;
    m_lastUsedSyncTime = now;
    m_remainder += delta;

    float offset_u = 0.0f;
    float offset_v = 0.0f;

    if (m_period > 0.0f) {
        int num_periods = (int)(m_remainder / m_period);
        m_remainder -= num_periods * m_period;

        float time;

        if (m_remainder > m_halfPeriod) {
            time = m_period - m_remainder;
        } else {
            time = m_remainder;
        }

        offset_u = m_speed.U * time;
        offset_v = m_speed.V * time;
    }

    mat.Make_Identity();
    mat[0].Z = offset_u;
    mat[0].X = m_scale.X;
    mat[1].Z = offset_v;
    mat[1].Y = m_scale.Y;
}

void ZigZagLinearOffsetTextureMapperClass::Reset()
{
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
    m_remainder = 0;
}

void ClassicEnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    mat = Matrix4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

void ClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void EnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    mat = Matrix4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

void EnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

EdgeMapperClass::EdgeMapperClass(unsigned int stage) :
    TextureMapperClass(stage), m_lastUsedSyncTime(W3D::Get_Sync_Time()), m_vSpeed(0.0f), m_vOffset(0.0f), m_useReflect(false)
{
}

EdgeMapperClass::EdgeMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
    TextureMapperClass(stage), m_lastUsedSyncTime(W3D::Get_Sync_Time()), m_vSpeed(0.0f), m_vOffset(0.0f), m_useReflect(false)
{
    m_vSpeed = ini.Get_Float(section, "VPerSec", 0.0f);
    m_vOffset = ini.Get_Float(section, "VStart", 0.0f);
    m_useReflect = ini.Get_Bool(section, "UseReflect", false);
}

EdgeMapperClass::EdgeMapperClass(const EdgeMapperClass &src) :
    TextureMapperClass(src.m_stage),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_vSpeed(src.m_vSpeed),
    m_vOffset(src.m_vOffset),
    m_useReflect(src.m_useReflect)
{
}

void EdgeMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    unsigned int now = W3D::Get_Sync_Time();

    float delta = (now - m_lastUsedSyncTime) * 0.001f;
    m_lastUsedSyncTime = now;

    m_vOffset += delta * m_vSpeed;
    m_vOffset -= GameMath::Floor(m_vOffset);

    mat = Matrix4(0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, m_vOffset, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

void EdgeMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    if (m_useReflect) {
        DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    } else {
        DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
    }
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void EdgeMapperClass::Reset()
{
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
    m_vOffset = 0.0f;
}

WSEnvMapperClass::WSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
    TextureMapperClass(stage), m_axis(AXIS_Z)
{
    char axis[2];
    ini.Get_String(section, "Axis", "Z", axis, 2);

    switch (axis[0]) {
        case 'X':
        case 'x':
            m_axis = AXIS_X;
            break;
        case 'Y':
        case 'y':
            m_axis = AXIS_Y;
            break;
        case 'Z':
        case 'z':
            m_axis = AXIS_Z;
            break;
        default:
            m_axis = AXIS_Z;
            break;
    }
}

void WSEnvMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    if (m_axis == AXIS_X) {
        mat.Init(0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    } else if (m_axis == AXIS_Y) {
        mat.Init(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    } else if (m_axis == AXIS_Z) {
        mat.Init(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    Matrix4 view;
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_Transform(D3DTS_VIEW, view);
    mat = mat
        * Matrix4(view[0].X,
            view[1].X,
            view[2].X,
            0.0f,
            view[0].Y,
            view[1].Y,
            view[2].Y,
            0.0f,
            view[0].Z,
            view[1].Z,
            view[2].Z,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f);
#endif
}

void WSClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void WSEnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void GridClassicEnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    Update_Temporal_State();
    float u_offset, v_offset;
    Calculate_UV_Offset(&u_offset, &v_offset);

    float del = 0.5f * m_ooGridWidth;
    mat = Matrix4(
        del, 0.0f, 0.0f, u_offset + del, 0.0f, del, 0.0f, v_offset + del, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

void GridClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void GridEnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    Update_Temporal_State();
    float u_offset, v_offset;
    Calculate_UV_Offset(&u_offset, &v_offset);

    float del = 0.5f * m_ooGridWidth;
    mat = Matrix4(
        del, 0.0f, 0.0f, u_offset + del, 0.0f, del, 0.0f, v_offset + del, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

void GridEnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void ScreenMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    unsigned int delta = W3D::Get_Sync_Time() - m_lastUsedSyncTime;
    float del = (float)delta;
    float offset_u = m_currentUVOffset.X + m_uvOffsetDeltaPerMS.X * del;
    float offset_v = m_currentUVOffset.Y + m_uvOffsetDeltaPerMS.Y * del;

    if (m_clampFix) {
        offset_u = std::clamp(offset_u, -m_scale.X, m_scale.X);
        offset_v = std::clamp(offset_v, -m_scale.Y, m_scale.Y);
    } else {
        offset_u = offset_u - GameMath::Floor(offset_u);
        offset_v = offset_v - GameMath::Floor(offset_v);
    }

#ifdef BUILD_WITH_D3D8
    Matrix4 matx(true);
    DX8Wrapper::Get_Transform(D3DTS_PROJECTION, matx);
    matx[0] *= m_scale.X;
    matx[1] *= m_scale.Y;
    Vector4 last(matx[3]);
    last *= offset_u;
    matx[0] += last;
    last = matx[3];
    last *= offset_v;
    matx[1] += last;
    mat = matx;
#endif

    m_currentUVOffset.X = offset_u;
    m_currentUVOffset.Y = offset_v;
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
}

void ScreenMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED | D3DTTFF_COUNT3);
#endif
}

RandomTextureMapperClass::RandomTextureMapperClass(float fps, const Vector2 &scale, unsigned int stage) :
    ScaleTextureMapperClass(scale, stage),
    m_fpms(fps / 1000.0f),
    m_remainder(0),
    m_speed(0.0f, 0.0f),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    Randomize();
}

RandomTextureMapperClass::RandomTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
    ScaleTextureMapperClass(ini, section, stage), m_remainder(0), m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    m_fpms = ini.Get_Float(section, "FPS", 0.0f) / 1000.0f;
    m_speed.U = ini.Get_Float(section, "UPerSec", 0.0f) / 1000.0f;
    m_speed.V = ini.Get_Float(section, "VPerSec", 0.0f) / 1000.0f;
    Randomize();
}

RandomTextureMapperClass::RandomTextureMapperClass(const RandomTextureMapperClass &src) :
    ScaleTextureMapperClass(src),
    m_fpms(src.m_fpms),
    m_remainder(0),
    m_speed(src.m_speed),
    m_lastUsedSyncTime(W3D::Get_Sync_Time())
{
    Randomize();
}

void RandomTextureMapperClass::Randomize()
{
    m_currentAngle = 2 * GAMEMATH_PI * rand4.Get_Float();
    m_center.U = rand4.Get_Float();
    m_center.V = rand4.Get_Float();
}

void RandomTextureMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{

    unsigned int now = W3D::Get_Sync_Time();
    unsigned int delta = now - m_lastUsedSyncTime;
    m_lastUsedSyncTime = now;
    m_remainder += delta;

    if (m_fpms != 0.0f) {
        int num_frames = (int)(m_remainder * m_fpms);

        if (num_frames != 0) {
            Randomize();
            m_remainder -= num_frames / m_fpms;
        }
    }

    float sin = GameMath::Sin(m_currentAngle);
    float cos = GameMath::Cos(m_currentAngle);

    mat.Make_Identity();
    mat[0][0] = sin * m_scale.X;
    mat[0][1] = -cos * m_scale.Y;
    mat[1][0] = cos * m_scale.X;
    mat[1][1] = sin * m_scale.Y;
    float uoff = m_center.U + m_remainder * m_speed.U;
    float voff = m_center.V + m_remainder * m_speed.V;
    uoff = fmod(uoff, 1.0f);
    voff = fmod(voff, 1.0f);
    mat[0].Z = uoff;
    mat[1].Z = voff;
}

void RandomTextureMapperClass::Reset()
{
    m_lastUsedSyncTime = W3D::Get_Sync_Time();
    m_remainder = 0;
}

BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(float rad_per_sec,
    float scale_factor,
    const Vector2 &offset_per_sec,
    const Vector2 &initial_offset,
    bool clamp_fix,
    const Vector2 &scale,
    unsigned int stage) :
    LinearOffsetTextureMapperClass(offset_per_sec, initial_offset, clamp_fix, scale, stage),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_currentAngle(0.0f),
    m_radiansPerSecond(rad_per_sec),
    m_scaleFactor(scale_factor)
{
}

BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(INIClass &ini, const char *section, unsigned int stage) :
    LinearOffsetTextureMapperClass(ini, section, stage), m_lastUsedSyncTime(W3D::Get_Sync_Time()), m_currentAngle(0.0f)
{
    m_radiansPerSecond = 2 * GAMEMATH_PI * ini.Get_Float(section, "BumpRotation", 0.0f);
    m_scaleFactor = ini.Get_Float(section, "BumpScale", 1.0f);
}

BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(const BumpEnvTextureMapperClass &src) :
    LinearOffsetTextureMapperClass(src),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_currentAngle(0.0f),
    m_radiansPerSecond(src.m_radiansPerSecond),
    m_scaleFactor(src.m_scaleFactor)
{
}

inline unsigned long F2DW(float f)
{
    return *reinterpret_cast<unsigned long *>(&f);
}

void BumpEnvTextureMapperClass::Apply(int uv_array_index)
{
    LinearOffsetTextureMapperClass::Apply(uv_array_index);

    unsigned int now = W3D::Get_Sync_Time();
    unsigned int delta = now - m_lastUsedSyncTime;
    m_lastUsedSyncTime = now;

    m_currentAngle += m_radiansPerSecond * delta * 0.001f;
    m_currentAngle = fmod(m_currentAngle, 2 * GAMEMATH_PI);

    float c = m_scaleFactor * GameMath::Fast_Cos(m_currentAngle);
    float s = m_scaleFactor * GameMath::Fast_Sin(m_currentAngle);

#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_BUMPENVMAT00, F2DW(c));
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_BUMPENVMAT01, F2DW(-s));
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_BUMPENVMAT10, F2DW(s));
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_BUMPENVMAT11, F2DW(c));
#endif
}

GridWSEnvMapperClass::GridWSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
    GridTextureMapperClass(ini, section, stage), m_axis(AXIS_Z)
{
    char axis[2];
    ini.Get_String(section, "Axis", "Z", axis, 2);

    switch (axis[0]) {
        case 'X':
        case 'x':
            m_axis = AXIS_X;
            break;
        case 'Y':
        case 'y':
            m_axis = AXIS_Y;
            break;
        case 'Z':
        case 'z':
            m_axis = AXIS_Z;
            break;
        default:
            m_axis = AXIS_Z;
            break;
    }
}

void GridWSEnvMapperClass::Calculate_Texture_Matrix(Matrix4 &mat)
{
    Matrix4 view;
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Get_Transform(D3DTS_VIEW, view);
#endif

    Matrix4 m(view[0].X,
        view[1].X,
        view[2].X,
        0.0f,
        view[0].Y,
        view[1].Y,
        view[2].Y,
        0.0f,
        view[0].Z,
        view[1].Z,
        view[2].Z,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f);

    Update_Temporal_State();
    float u_offset, v_offset;
    Calculate_UV_Offset(&u_offset, &v_offset);
    float del = 0.5f * m_ooGridWidth;

    if (m_axis == AXIS_X) {
        mat.Init(0.0f,
            del,
            0.0f,
            u_offset + del,
            0.0f,
            0.0f,
            del,
            v_offset + del,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f);
    } else if (m_axis == AXIS_Y) {
        mat.Init(del,
            0.0f,
            0.0f,
            u_offset + del,
            0.0f,
            0.0f,
            del,
            v_offset + del,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f);
    } else if (m_axis == AXIS_Z) {
        mat.Init(del,
            0.0f,
            0.0f,
            u_offset + del,
            0.0f,
            del,
            0.0f,
            v_offset + del,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f);
    }

    mat = mat
        * Matrix4(view[0].X,
            view[1].X,
            view[2].X,
            0.0f,
            view[0].Y,
            view[1].Y,
            view[2].Y,
            0.0f,
            view[0].Z,
            view[1].Z,
            view[2].Z,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f);
}

void GridWSClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}

void GridWSEnvironmentMapperClass::Apply(int uv_array_index)
{
    Matrix4 mat(true);
    Calculate_Texture_Matrix(mat);
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + m_stage), mat);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    DX8Wrapper::Set_DX8_Texture_Stage_State(m_stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#endif
}
