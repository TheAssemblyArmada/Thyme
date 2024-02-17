/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Particle Emitter Loader
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
#include "part_emt.h"
#include "proto.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "w3d_util.h"
#include "w3derr.h"

class ChunkLoadClass;
class ChunkSaveClass;
class Vector3Randomizer;

class ParticleEmitterDefClass
{
public:
    ParticleEmitterDefClass();
    ParticleEmitterDefClass(const ParticleEmitterDefClass &src);

    virtual ~ParticleEmitterDefClass();
    const ParticleEmitterDefClass &operator=(const ParticleEmitterDefClass &src);
    virtual W3DErrorType Load_W3D(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Save_W3D(ChunkSaveClass &chunk_save);

    const char *Get_Name() const { return m_pName; }
    virtual void Set_Name(const char *pname);
    unsigned int Get_Version() const { return m_version; }
    int Get_Render_Mode() const { return m_infoV2.RenderMode; }
    int Get_Frame_Mode() const { return m_infoV2.FrameMode; }
    const char *Get_Texture_Filename() const { return m_info.TextureFilename; }
    float Get_Lifetime() const { return m_info.Lifetime; }
    float Get_Emission_Rate() const { return m_info.EmissionRate; }
    float Get_Max_Emissions() const { return m_info.MaxEmissions; }
    float Get_Fade_Time() const { return m_info.FadeTime; }
    float Get_Gravity() const { return m_info.Gravity; }
    float Get_Elasticity() const { return m_info.Elasticity; }
    Vector3 Get_Velocity() const { return Vector3(m_info.Velocity.x, m_info.Velocity.y, m_info.Velocity.z); }
    float Get_Future_Start_Time() const { return m_extraInfo.FutureStartTime; }

    Vector3 Get_Acceleration() const { return Vector3(m_info.Acceleration.x, m_info.Acceleration.y, m_info.Acceleration.z); }

    unsigned int Get_Burst_Size() const { return m_infoV2.BurstSize; }
    float Get_Outward_Vel() const { return m_infoV2.OutwardVel; }
    float Get_Vel_Inherit() const { return m_infoV2.VelInherit; }

    virtual void Set_Render_Mode(int mode) { m_infoV2.RenderMode = mode; }
    virtual void Set_Frame_Mode(int mode) { m_infoV2.FrameMode = mode; }
    virtual void Set_Texture_Filename(const char *pname);
    virtual void Set_Lifetime(float value) { m_info.Lifetime = value; }
    virtual void Set_Emission_Rate(float value) { m_info.EmissionRate = value; }
    virtual void Set_Max_Emissions(float value) { m_info.MaxEmissions = value; }
    virtual void Set_Fade_Time(float value) { m_info.FadeTime = value; }
    virtual void Set_Gravity(float value) { m_info.Gravity = value; }
    virtual void Set_Elasticity(float value) { m_info.Elasticity = value; }

    virtual void Set_Velocity(const Vector3 &value)
    {
        m_info.Velocity.x = value.X;
        m_info.Velocity.y = value.Y;
        m_info.Velocity.z = value.Z;
    }

    virtual void Set_Acceleration(const Vector3 &value)
    {
        m_info.Acceleration.x = value.X;
        m_info.Acceleration.y = value.Y;
        m_info.Acceleration.z = value.Z;
    }

    virtual void Set_Burst_Size(unsigned int count) { m_infoV2.BurstSize = count; }
    virtual void Set_Outward_Vel(float value) { m_infoV2.OutwardVel = value; }
    virtual void Set_Vel_Inherit(float value) { m_infoV2.VelInherit = value; }
    virtual void Set_Future_Start_Time(float value) { m_extraInfo.FutureStartTime = value; }

    void Get_Shader(ShaderClass &shader) const { W3dUtilityClass::Convert_Shader(m_infoV2.Shader, &shader); }
    virtual void Set_Shader(const ShaderClass &shader) { W3dUtilityClass::Convert_Shader(shader, &m_infoV2.Shader); }
    Vector3Randomizer *Get_Creation_Volume() const { return m_pCreationVolume->Clone(); }
    Vector3Randomizer *Get_Velocity_Random() const { return m_pVelocityRandomizer->Clone(); }

    virtual void Set_Creation_Volume(Vector3Randomizer *randomizer);
    virtual void Set_Velocity_Random(Vector3Randomizer *randomizer);
    virtual void Get_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes) const;
    virtual void Get_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes) const;
    virtual void Get_Size_Keyframes(ParticlePropertyStruct<float> &keyframes) const;
    virtual void Get_Rotation_Keyframes(ParticlePropertyStruct<float> &rotationframes) const;
    virtual void Get_Frame_Keyframes(ParticlePropertyStruct<float> &frameframes) const;
    virtual void Get_Blur_Time_Keyframes(ParticlePropertyStruct<float> &blurtimeframes) const;
    virtual float Get_Initial_Orientation_Random() const { return m_initialOrientationRandom; }

    virtual void Set_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes);
    virtual void Set_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes);
    virtual void Set_Size_Keyframes(ParticlePropertyStruct<float> &keyframes);
    virtual void Set_Rotation_Keyframes(ParticlePropertyStruct<float> &keyframes, float orient_rnd);
    virtual void Set_Frame_Keyframes(ParticlePropertyStruct<float> &keyframes);
    virtual void Set_Blur_Time_Keyframes(ParticlePropertyStruct<float> &keyframes);

    const char *Get_User_String() const { return m_pUserString; }
    int Get_User_Type() const { return m_iUserType; }
    virtual void Set_User_String(const char *pstring);
    virtual void Set_User_Type(int type) { m_iUserType = type; }
    const W3dEmitterLinePropertiesStruct *Get_Line_Properties() const { return &m_lineProperties; }

    int Get_Line_Texture_Mapping_Mode() const
    {
        return (m_lineProperties.Flags & W3D_ELINE_TEXTURE_MAP_MODE_MASK) >> W3D_ELINE_TEXTURE_MAP_MODE_OFFSET;
    }

    int Is_Merge_Intersections() const { return m_lineProperties.Flags & W3D_ELINE_MERGE_INTERSECTIONS; }
    int Is_Freeze_Random() const { return m_lineProperties.Flags & W3D_ELINE_FREEZE_RANDOM; }
    int Is_Sorting_Disabled() const { return m_lineProperties.Flags & W3D_ELINE_DISABLE_SORTING; }
    int Are_End_Caps_Enabled() const { return m_lineProperties.Flags & W3D_ELINE_END_CAPS; }
    int Get_Subdivision_Level() const { return m_lineProperties.SubdivisionLevel; }
    float Get_Noise_Amplitude() const { return m_lineProperties.NoiseAmplitude; }
    float Get_Merge_Abort_Factor() const { return m_lineProperties.MergeAbortFactor; }
    float Get_Texture_Tile_Factor() const { return m_lineProperties.TextureTileFactor; }
    Vector2 Get_UV_Offset_Rate() const { return Vector2(m_lineProperties.UPerSec, m_lineProperties.VPerSec); }

    virtual void Set_Line_Texture_Mapping_Mode(int mode);

    virtual void Set_Merge_Intersections(int onoff)
    {
        if (onoff) {
            m_lineProperties.Flags |= W3D_ELINE_MERGE_INTERSECTIONS;
        } else {
            m_lineProperties.Flags &= ~W3D_ELINE_MERGE_INTERSECTIONS;
        };
    }

    virtual void Set_Freeze_Random(int onoff)
    {
        if (onoff) {
            m_lineProperties.Flags |= W3D_ELINE_FREEZE_RANDOM;
        } else {
            m_lineProperties.Flags &= ~W3D_ELINE_FREEZE_RANDOM;
        };
    }

    virtual void Set_Disable_Sorting(int onoff)
    {
        if (onoff) {
            m_lineProperties.Flags |= W3D_ELINE_DISABLE_SORTING;
        } else {
            m_lineProperties.Flags &= ~W3D_ELINE_DISABLE_SORTING;
        };
    }

    virtual void Set_End_Caps(int onoff)
    {
        if (onoff) {
            m_lineProperties.Flags |= W3D_ELINE_END_CAPS;
        } else {
            m_lineProperties.Flags &= ~W3D_ELINE_END_CAPS;
        };
    }

    virtual void Set_Subdivision_Level(int lvl) { m_lineProperties.SubdivisionLevel = lvl; }
    virtual void Set_Noise_Amplitude(float k) { m_lineProperties.NoiseAmplitude = k; }
    virtual void Set_Merge_Abort_Factor(float k) { m_lineProperties.MergeAbortFactor = k; }
    virtual void Set_Texture_Tile_Factor(float k) { m_lineProperties.TextureTileFactor = k; }

    virtual void Set_UV_Offset_Rate(const Vector2 &rate)
    {
        m_lineProperties.UPerSec = rate.X;
        m_lineProperties.VPerSec = rate.Y;
    }

protected:
    virtual W3DErrorType Read_Header(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_User_Data(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_Info(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_InfoV2(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_Props(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_Line_Properties(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_Rotation_Keyframes(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_Frame_Keyframes(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_Blur_Time_Keyframes(ChunkLoadClass &chunk_load);
    virtual W3DErrorType Read_Extra_Info(ChunkLoadClass &chunk_load);

    virtual bool Read_Color_Keyframe(ChunkLoadClass &chunk_load, float *key_time, Vector3 *value);
    virtual bool Read_Opacity_Keyframe(ChunkLoadClass &chunk_load, float *key_time, float *value);
    virtual bool Read_Size_Keyframe(ChunkLoadClass &chunk_load, float *key_time, float *value);

    virtual W3DErrorType Save_Header(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_User_Data(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Info(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_InfoV2(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Props(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Line_Properties(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Frame_Keyframes(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Rotation_Keyframes(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Blur_Time_Keyframes(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Extra_Info(ChunkSaveClass &chunk_save);

    virtual W3DErrorType Save_Color_Keyframes(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Opacity_Keyframes(ChunkSaveClass &chunk_save);
    virtual W3DErrorType Save_Size_Keyframes(ChunkSaveClass &chunk_save);

    virtual Vector3Randomizer *Create_Randomizer(W3dVolumeRandomizerStruct &info);
    virtual void Initialize_Randomizer_Struct(const Vector3Randomizer &randomizer, W3dVolumeRandomizerStruct &info);
    virtual void Initialize_To_Ver2();
    virtual void Convert_To_Ver2();
    virtual void Normalize_Filename();
    virtual void Free_Props();

protected:
    char *m_pName;
    char *m_pUserString;
    int m_iUserType;
    unsigned int m_version;
    ShaderClass m_shader;
    W3dEmitterInfoStruct m_info;
    W3dEmitterInfoStructV2 m_infoV2;
    W3dEmitterExtraInfoStruct m_extraInfo;
    W3dEmitterLinePropertiesStruct m_lineProperties;
    ParticlePropertyStruct<Vector3> m_colorKeyframes;
    ParticlePropertyStruct<float> m_opacityKeyframes;
    ParticlePropertyStruct<float> m_sizeKeyframes;
    ParticlePropertyStruct<float> m_rotationKeyframes;
    float m_initialOrientationRandom;
    ParticlePropertyStruct<float> m_frameKeyframes;
    ParticlePropertyStruct<float> m_blurTimeKeyframes;
    Vector3Randomizer *m_pCreationVolume;
    Vector3Randomizer *m_pVelocityRandomizer;
};

inline void ParticleEmitterDefClass::Set_Line_Texture_Mapping_Mode(int mode)
{
    m_lineProperties.Flags &= ~W3D_ELINE_TEXTURE_MAP_MODE_MASK;
    m_lineProperties.Flags |= ((mode << W3D_ELINE_TEXTURE_MAP_MODE_OFFSET) & W3D_ELINE_TEXTURE_MAP_MODE_MASK);
}

class ParticleEmitterPrototypeClass : public W3DMPO, public PrototypeClass
{
    IMPLEMENT_W3D_POOL(ParticleEmitterPrototypeClass);

public:
    ParticleEmitterPrototypeClass(ParticleEmitterDefClass *pdef) { m_pDefinition = pdef; }
    virtual ~ParticleEmitterPrototypeClass() override { delete m_pDefinition; }
    virtual const char *Get_Name() const override { return m_pDefinition->Get_Name(); }
    virtual int Get_Class_ID() const override { return RenderObjClass::CLASSID_PARTICLEEMITTER; }
    virtual RenderObjClass *Create() override;
    virtual void Delete_Self() override { delete this; };
    virtual ParticleEmitterDefClass *Get_Definition() const { return m_pDefinition; }

protected:
    ParticleEmitterDefClass *m_pDefinition;
};

class ParticleEmitterLoaderClass : public PrototypeLoaderClass
{
public:
    virtual int Chunk_Type() { return W3D_CHUNK_EMITTER; }
    virtual PrototypeClass *Load_W3D(ChunkLoadClass &chunk_load);
};
