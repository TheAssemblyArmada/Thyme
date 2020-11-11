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
#include "w3d.h"

/**
 * 0x00864ED0
 */
TextureMapperClass::TextureMapperClass(uint32_t stage) : m_stage(stage)
{
    // TODO 2 is MeshMatDescClass::MAX_TEX_STAGES
    if (m_stage >= 2) {
        m_stage = 2 - 1;
    }
}

ScaleTextureMapperClass::ScaleTextureMapperClass(const Vector2 &scale, uint32_t stage) :
    TextureMapperClass(stage), m_scale(scale)
{
}

ScaleTextureMapperClass::ScaleTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage) :
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
    const Vector2 &offset_per_sec, const Vector2 &initial_offset, bool clamp_fix, const Vector2 &scale, uint32_t stage) :
    ScaleTextureMapperClass(scale, stage),
    m_lastUsedSyncTime(W3D::Get_Sync_Time()),
    m_initialUVOffset(initial_offset),
    m_clampFix(clamp_fix)
{
    m_currentUVOffset = m_initialUVOffset;
    m_uvOffsetDeltaPerMS = offset_per_sec * -0.001f;
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, uint32_t stage) :

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
