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
#include "part_ldr.h"
#include "assetmgr.h"
#include "chunkio.h"
#include "part_emt.h"
#include "texture.h"
#include "w3derr.h"

const char *s_emitterTypeNames[EMITTER_TYPEID_COUNT] = { "Default" };

#define BYTE_TO_FLOAT(byte) (((float)byte) / 255.0f)

#define FLOAT_TO_BYTE(fval) ((unsigned char)(max(fval, 0.0f) * 255.0f))

#define RGBA_TO_VECTOR3(rgba) Vector3(BYTE_TO_FLOAT(rgba.R), BYTE_TO_FLOAT(rgba.G), BYTE_TO_FLOAT(rgba.B))

#define VECTOR3_TO_RGBA(vector3, rgba) \
    rgba.R = FLOAT_TO_BYTE(vector3.X); \
    rgba.G = FLOAT_TO_BYTE(vector3.Y); \
    rgba.B = FLOAT_TO_BYTE(vector3.Z);

ParticleEmitterDefClass::ParticleEmitterDefClass() :
    m_pName(nullptr),
    m_pUserString(nullptr),
    m_iUserType(EMITTER_TYPEID_DEFAULT),
    m_version(0L),
    m_shader(0x8441B),
    m_initialOrientationRandom(0),
    m_pCreationVolume(nullptr),
    m_pVelocityRandomizer(nullptr)
{
    memset(&m_info, 0, sizeof(m_info));
    memset(&m_infoV2, 0, sizeof(m_infoV2));
    memset(&m_extraInfo, 0, sizeof(m_extraInfo));

    memset(&m_colorKeyframes, 0, sizeof(m_colorKeyframes));
    memset(&m_opacityKeyframes, 0, sizeof(m_opacityKeyframes));
    memset(&m_sizeKeyframes, 0, sizeof(m_sizeKeyframes));
    memset(&m_rotationKeyframes, 0, sizeof(m_rotationKeyframes));
    memset(&m_frameKeyframes, 0, sizeof(m_frameKeyframes));
    memset(&m_blurTimeKeyframes, 0, sizeof(m_blurTimeKeyframes));
    memset(&m_lineProperties, 0, sizeof(m_lineProperties));
}

ParticleEmitterDefClass::ParticleEmitterDefClass(const ParticleEmitterDefClass &src) :
    m_pName(nullptr),
    m_pUserString(nullptr),
    m_iUserType(EMITTER_TYPEID_DEFAULT),
    m_version(0L),
    m_shader(0x8441B),
    m_initialOrientationRandom(src.m_initialOrientationRandom),
    m_pCreationVolume(nullptr),
    m_pVelocityRandomizer(nullptr)
{
    memset(&m_info, 0, sizeof(m_info));
    memset(&m_infoV2, 0, sizeof(m_infoV2));
    memset(&m_extraInfo, 0, sizeof(m_extraInfo));

    memset(&m_colorKeyframes, 0, sizeof(m_colorKeyframes));
    memset(&m_opacityKeyframes, 0, sizeof(m_opacityKeyframes));
    memset(&m_sizeKeyframes, 0, sizeof(m_sizeKeyframes));
    memset(&m_rotationKeyframes, 0, sizeof(m_rotationKeyframes));
    memset(&m_frameKeyframes, 0, sizeof(m_frameKeyframes));
    memset(&m_blurTimeKeyframes, 0, sizeof(m_blurTimeKeyframes));
    memset(&m_lineProperties, 0, sizeof(m_lineProperties));

    (*this) = src;
}

ParticleEmitterDefClass::~ParticleEmitterDefClass()
{
    if (m_pName != nullptr) {
        free(m_pName);
        m_pName = nullptr;
    }

    if (m_pUserString != nullptr) {
        free(m_pUserString);
        m_pUserString = nullptr;
    }

    Free_Props();

    if (m_pCreationVolume != nullptr) {
        delete m_pCreationVolume;
        m_pCreationVolume = nullptr;
    }

    if (m_pVelocityRandomizer != nullptr) {
        delete m_pVelocityRandomizer;
        m_pVelocityRandomizer = nullptr;
    }
}

const ParticleEmitterDefClass &ParticleEmitterDefClass::operator=(const ParticleEmitterDefClass &src)
{
    Set_Name(src.Get_Name());
    Set_User_String(src.Get_User_String());
    Set_User_Type(src.Get_User_Type());
    m_version = src.m_version;

    memcpy(&m_info, &src.m_info, sizeof(m_info));
    memcpy(&m_infoV2, &src.m_infoV2, sizeof(m_infoV2));
    memcpy(&m_extraInfo, &src.m_extraInfo, sizeof(m_extraInfo));
    memcpy(&m_lineProperties, &src.m_lineProperties, sizeof(m_lineProperties));
    Free_Props();

    Copy_Emitter_Property_Struct(m_colorKeyframes, src.m_colorKeyframes);
    Copy_Emitter_Property_Struct(m_opacityKeyframes, src.m_opacityKeyframes);
    Copy_Emitter_Property_Struct(m_sizeKeyframes, src.m_sizeKeyframes);
    Copy_Emitter_Property_Struct(m_rotationKeyframes, src.m_rotationKeyframes);
    Copy_Emitter_Property_Struct(m_frameKeyframes, src.m_frameKeyframes);
    Copy_Emitter_Property_Struct(m_blurTimeKeyframes, src.m_blurTimeKeyframes);
    m_initialOrientationRandom = src.m_initialOrientationRandom;

    if (m_pCreationVolume != nullptr) {
        delete m_pCreationVolume;
        m_pCreationVolume = nullptr;
    }

    if (m_pVelocityRandomizer != nullptr) {
        delete m_pVelocityRandomizer;
        m_pVelocityRandomizer = nullptr;
    }

    m_pCreationVolume = Create_Randomizer(m_infoV2.CreationVolume);
    m_pVelocityRandomizer = Create_Randomizer(m_infoV2.VelRandom);
    return (*this);
}

void ParticleEmitterDefClass::Free_Props()
{
    m_colorKeyframes.NumKeyFrames = 0;
    m_opacityKeyframes.NumKeyFrames = 0;
    m_sizeKeyframes.NumKeyFrames = 0;
    m_rotationKeyframes.NumKeyFrames = 0;
    m_frameKeyframes.NumKeyFrames = 0;
    m_blurTimeKeyframes.NumKeyFrames = 0;

    if (m_colorKeyframes.KeyTimes != nullptr) {
        delete[] m_colorKeyframes.KeyTimes;
        m_colorKeyframes.KeyTimes = nullptr;
    }

    if (m_colorKeyframes.Values != nullptr) {
        delete[] m_colorKeyframes.Values;
        m_colorKeyframes.Values = nullptr;
    }

    if (m_opacityKeyframes.KeyTimes != nullptr) {
        delete[] m_opacityKeyframes.KeyTimes;
        m_opacityKeyframes.KeyTimes = nullptr;
    }

    if (m_opacityKeyframes.Values != nullptr) {
        delete[] m_opacityKeyframes.Values;
        m_opacityKeyframes.Values = nullptr;
    }

    if (m_sizeKeyframes.KeyTimes != nullptr) {
        delete[] m_sizeKeyframes.KeyTimes;
        m_sizeKeyframes.KeyTimes = nullptr;
    }

    if (m_sizeKeyframes.Values != nullptr) {
        delete[] m_sizeKeyframes.Values;
        m_sizeKeyframes.Values = nullptr;
    }

    if (m_rotationKeyframes.KeyTimes != nullptr) {
        delete[] m_rotationKeyframes.KeyTimes;
        m_rotationKeyframes.KeyTimes = nullptr;
    }

    if (m_rotationKeyframes.Values != nullptr) {
        delete[] m_rotationKeyframes.Values;
        m_rotationKeyframes.Values = nullptr;
    }

    if (m_frameKeyframes.KeyTimes != nullptr) {
        delete[] m_frameKeyframes.KeyTimes;
        m_frameKeyframes.KeyTimes = nullptr;
    }

    if (m_frameKeyframes.Values != nullptr) {
        delete[] m_frameKeyframes.Values;
        m_frameKeyframes.Values = nullptr;
    }

    if (m_blurTimeKeyframes.KeyTimes != nullptr) {
        delete[] m_blurTimeKeyframes.KeyTimes;
        m_blurTimeKeyframes.KeyTimes = nullptr;
    }

    if (m_blurTimeKeyframes.Values != nullptr) {
        delete[] m_blurTimeKeyframes.Values;
        m_blurTimeKeyframes.Values = nullptr;
    }
}

void ParticleEmitterDefClass::Set_Velocity_Random(Vector3Randomizer *randomizer)
{
    if (m_pVelocityRandomizer != nullptr) {
        delete m_pVelocityRandomizer;
        m_pVelocityRandomizer = nullptr;
    }

    m_pVelocityRandomizer = randomizer;

    if (m_pVelocityRandomizer != nullptr) {
        Initialize_Randomizer_Struct(*m_pVelocityRandomizer, m_infoV2.VelRandom);
    }
}

void ParticleEmitterDefClass::Set_Creation_Volume(Vector3Randomizer *randomizer)
{
    if (m_pCreationVolume != nullptr) {
        delete m_pCreationVolume;
        m_pCreationVolume = nullptr;
    }

    m_pCreationVolume = randomizer;

    if (m_pCreationVolume != nullptr) {
        Initialize_Randomizer_Struct(*m_pCreationVolume, m_infoV2.CreationVolume);
    }
}

void ParticleEmitterDefClass::Set_User_String(const char *pstring)
{
    if (m_pUserString != nullptr) {
        free(m_pUserString);
        m_pUserString = nullptr;
    }

    m_pUserString = strdup(pstring);
}

void ParticleEmitterDefClass::Set_Name(const char *pname)
{
    if (m_pName != nullptr) {
        free(m_pName);
        m_pName = nullptr;
    }

    m_pName = strdup(pname);
}

void ParticleEmitterDefClass::Set_Texture_Filename(const char *pname)
{
    strcpy(m_info.TextureFilename, pname);
    Normalize_Filename();
}

void ParticleEmitterDefClass::Normalize_Filename()
{
#ifdef PLATFORM_WINDOWS
    char path[MAX_PATH];
    strcpy(path, m_info.TextureFilename);
    const char *filename = strrchr(path, '\\');

    if (filename != nullptr) {
        filename++;
        strcpy(m_info.TextureFilename, filename);
    }
#endif
}

W3DErrorType ParticleEmitterDefClass::Load_W3D(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_LOAD_FAILED;
    Initialize_To_Ver2();

    if ((Read_Header(chunk_load) == W3D_ERROR_OK) && (Read_User_Data(chunk_load) == W3D_ERROR_OK)
        && (Read_Info(chunk_load) == W3D_ERROR_OK)) {
        if (m_version > 0x00010000) {
            if ((Read_InfoV2(chunk_load) == W3D_ERROR_OK) && (Read_Props(chunk_load) == W3D_ERROR_OK)) {
                ret_val = W3D_ERROR_OK;
            }
        } else {
            Convert_To_Ver2();
            ret_val = W3D_ERROR_OK;
        }
    }

    while (chunk_load.Open_Chunk() && ret_val == W3D_ERROR_OK) {
        switch (chunk_load.Cur_Chunk_ID()) {
            case W3D_CHUNK_EMITTER_LINE_PROPERTIES:
                ret_val = Read_Line_Properties(chunk_load);
                break;
            case W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES:
                ret_val = Read_Rotation_Keyframes(chunk_load);
                break;
            case W3D_CHUNK_EMITTER_FRAME_KEYFRAMES:
                ret_val = Read_Frame_Keyframes(chunk_load);
                break;
            case W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES:
                ret_val = Read_Blur_Time_Keyframes(chunk_load);
                break;
            case W3D_CHUNK_EMITTER_EXTRA_INFO:
                ret_val = Read_Extra_Info(chunk_load);
                break;
            default:
                captainslog_debug("Unhandled Chunk! File: %s Line: %d", __FILE__, __LINE__);
                break;
        }

        chunk_load.Close_Chunk();
    }

    return ret_val;
}

void ParticleEmitterDefClass::Initialize_To_Ver2()
{
    memset(&m_info, 0, sizeof(m_info));
    memset(&m_infoV2, 0, sizeof(m_infoV2));
    memset(&m_extraInfo, 0, sizeof(m_extraInfo));
    m_infoV2.BurstSize = 1;
    m_infoV2.OutwardVel = 0;
    m_infoV2.VelInherit = 0;
    W3dUtilityClass::Convert_Shader(ShaderClass::s_presetAdditiveSpriteShader, &m_infoV2.Shader);

    m_infoV2.CreationVolume.ClassID = Vector3Randomizer::CLASSID_SOLIDBOX;
    m_infoV2.CreationVolume.Value1 = 0;
    m_infoV2.CreationVolume.Value2 = 0;
    m_infoV2.CreationVolume.Value3 = 0;

    m_infoV2.VelRandom.ClassID = Vector3Randomizer::CLASSID_SOLIDBOX;
    m_infoV2.VelRandom.Value1 = 0;
    m_infoV2.VelRandom.Value2 = 0;
    m_infoV2.VelRandom.Value3 = 0;

    Free_Props();
}

void ParticleEmitterDefClass::Convert_To_Ver2()
{
    if (m_version < 0x00020000) {
        m_infoV2.BurstSize = 1;
        m_infoV2.OutwardVel = 0;
        m_infoV2.VelInherit = 0;
        ShaderClass shader = ShaderClass::s_presetAdditiveSpriteShader;
        TextureClass *ptexture = W3DAssetManager::Get_Instance()->Get_Texture(m_info.TextureFilename);

        if (ptexture != nullptr) {
            if (Has_Alpha(ptexture->Get_Texture_Format())) {
                shader = ShaderClass::s_presetAlphaSpriteShader;
            }

            ptexture->Release_Ref();
        }

        W3dUtilityClass::Convert_Shader(shader, &m_infoV2.Shader);
        m_infoV2.CreationVolume.ClassID = Vector3Randomizer::CLASSID_SOLIDBOX;
        m_infoV2.CreationVolume.Value1 = m_info.PositionRandom / 1000.0f;
        m_infoV2.CreationVolume.Value2 = m_info.PositionRandom / 1000.0f;
        m_infoV2.CreationVolume.Value3 = m_info.PositionRandom / 1000.0f;

        m_infoV2.VelRandom.ClassID = Vector3Randomizer::CLASSID_SOLIDBOX;
        m_infoV2.VelRandom.Value1 = m_info.VelocityRandom;
        m_infoV2.VelRandom.Value2 = m_info.VelocityRandom;
        m_infoV2.VelRandom.Value3 = m_info.VelocityRandom;

        if (m_pCreationVolume != nullptr) {
            delete m_pCreationVolume;
            m_pCreationVolume = nullptr;
        }

        if (m_pVelocityRandomizer != nullptr) {
            delete m_pVelocityRandomizer;
            m_pVelocityRandomizer = nullptr;
        }

        m_pCreationVolume = Create_Randomizer(m_infoV2.CreationVolume);
        m_pVelocityRandomizer = Create_Randomizer(m_infoV2.VelRandom);
        Free_Props();

        m_colorKeyframes.Start = RGBA_TO_VECTOR3(m_info.StartColor);
        m_colorKeyframes.Rand = Vector3(0, 0, 0);
        m_colorKeyframes.NumKeyFrames = 1;
        m_colorKeyframes.KeyTimes = new float(m_info.FadeTime);
        m_colorKeyframes.Values = new Vector3(RGBA_TO_VECTOR3(m_info.EndColor));

        m_opacityKeyframes.Start = (m_info.StartColor.A) / 255;
        m_opacityKeyframes.Rand = 0;
        m_opacityKeyframes.NumKeyFrames = 1;
        m_opacityKeyframes.KeyTimes = new float(m_info.FadeTime);
        m_opacityKeyframes.Values = new float(m_info.EndColor.A / 255);

        m_sizeKeyframes.Start = m_info.StartSize;
        m_sizeKeyframes.Rand = 0;
        m_sizeKeyframes.NumKeyFrames = 0;
    }
}

W3DErrorType ParticleEmitterDefClass::Read_Header(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_LOAD_FAILED;
    if (chunk_load.Open_Chunk() && (chunk_load.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_HEADER)) {
        W3dEmitterHeaderStruct header = { 0 };

        if (chunk_load.Read(&header, sizeof(header)) == sizeof(header)) {
            m_pName = strdup(header.Name);
            m_version = header.Version;
            ret_val = W3D_ERROR_OK;
        }

        chunk_load.Close_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Read_Extra_Info(ChunkLoadClass &chunk_load)
{
    memset(&m_extraInfo, 0, sizeof(m_extraInfo));

    if (chunk_load.Read(&m_extraInfo, sizeof(m_extraInfo)) == sizeof(m_extraInfo)) {
        return W3D_ERROR_OK;
    }

    return W3D_ERROR_LOAD_FAILED;
}

W3DErrorType ParticleEmitterDefClass::Read_User_Data(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_LOAD_FAILED;
    if (chunk_load.Open_Chunk() && (chunk_load.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_USER_DATA)) {
        W3dEmitterUserInfoStruct user_info = { 0 };

        if (chunk_load.Read(&user_info, sizeof(user_info)) == sizeof(user_info)) {
            ret_val = W3D_ERROR_OK;
            m_iUserType = user_info.Type;

            if (user_info.SizeofStringParam > 0) {
                m_pUserString = static_cast<char *>(malloc(sizeof(char) * (user_info.SizeofStringParam + 1)));
                m_pUserString[0] = 0;

                if (chunk_load.Read(m_pUserString, user_info.SizeofStringParam) != user_info.SizeofStringParam) {
                    ret_val = W3D_ERROR_LOAD_FAILED;
                }
            }
        }

        chunk_load.Close_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Read_Info(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_LOAD_FAILED;

    if (chunk_load.Open_Chunk() && (chunk_load.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_INFO)) {
        memset(&m_info, 0, sizeof(m_info));

        if (chunk_load.Read(&m_info, sizeof(m_info)) == sizeof(m_info)) {
            ret_val = W3D_ERROR_OK;
        }

        chunk_load.Close_Chunk();
    }

    return ret_val;
}

Vector3Randomizer *ParticleEmitterDefClass::Create_Randomizer(W3dVolumeRandomizerStruct &info)
{
    Vector3Randomizer *randomizer = nullptr;

    switch (info.ClassID) {
        case Vector3Randomizer::CLASSID_SOLIDBOX:
            randomizer = new Vector3SolidBoxRandomizer(Vector3(info.Value1, info.Value2, info.Value3));
            break;
        case Vector3Randomizer::CLASSID_SOLIDSPHERE:
            randomizer = new Vector3SolidSphereRandomizer(info.Value1);
            break;
        case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
            randomizer = new Vector3HollowSphereRandomizer(info.Value1);
            break;
        case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
            randomizer = new Vector3SolidCylinderRandomizer(info.Value1, info.Value2);
            break;
    }

    return randomizer;
}

void ParticleEmitterDefClass::Initialize_Randomizer_Struct(
    const Vector3Randomizer &randomizer, W3dVolumeRandomizerStruct &info)
{
    info.ClassID = randomizer.Class_ID();

    switch (randomizer.Class_ID()) {
        case Vector3Randomizer::CLASSID_SOLIDBOX: {
            Vector3 extents = ((Vector3SolidBoxRandomizer &)randomizer).Get_Extents();
            info.Value1 = extents.X;
            info.Value2 = extents.Y;
            info.Value3 = extents.Z;
        } break;
        case Vector3Randomizer::CLASSID_SOLIDSPHERE:
            info.Value1 = ((Vector3SolidSphereRandomizer &)randomizer).Get_Radius();
            break;
        case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
            info.Value1 = ((Vector3HollowSphereRandomizer &)randomizer).Get_Radius();
            break;
        case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
            info.Value1 = ((Vector3SolidCylinderRandomizer &)randomizer).Get_Height();
            info.Value2 = ((Vector3SolidCylinderRandomizer &)randomizer).Get_Radius();
            break;
    }
}

W3DErrorType ParticleEmitterDefClass::Read_InfoV2(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_LOAD_FAILED;

    if (chunk_load.Open_Chunk() && (chunk_load.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_INFOV2)) {
        memset(&m_infoV2, 0, sizeof(m_infoV2));

        if (chunk_load.Read(&m_infoV2, sizeof(m_infoV2)) == sizeof(m_infoV2)) {
            if (m_pCreationVolume != nullptr) {
                delete m_pCreationVolume;
                m_pCreationVolume = nullptr;
            }

            if (m_pVelocityRandomizer != nullptr) {
                delete m_pVelocityRandomizer;
                m_pVelocityRandomizer = nullptr;
            }

            m_pCreationVolume = Create_Randomizer(m_infoV2.CreationVolume);
            m_pVelocityRandomizer = Create_Randomizer(m_infoV2.VelRandom);
            ret_val = W3D_ERROR_OK;
        }

        chunk_load.Close_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Read_Props(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_LOAD_FAILED;
    Free_Props();

    if (chunk_load.Open_Chunk() && (chunk_load.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_PROPS)) {
        W3dEmitterPropertyStruct info = { 0 };

        if (chunk_load.Read(&info, sizeof(info)) == sizeof(info)) {
            unsigned int index = 0;
            m_colorKeyframes.NumKeyFrames = info.ColorKeyframes - 1;
            m_opacityKeyframes.NumKeyFrames = info.OpacityKeyframes - 1;
            m_sizeKeyframes.NumKeyFrames = info.SizeKeyframes - 1;
            m_colorKeyframes.Rand = RGBA_TO_VECTOR3(info.ColorRandom);
            m_opacityKeyframes.Rand = info.OpacityRandom;
            m_sizeKeyframes.Rand = info.SizeRandom;

            if (m_colorKeyframes.NumKeyFrames > 0) {
                m_colorKeyframes.KeyTimes = new float[m_colorKeyframes.NumKeyFrames];
                m_colorKeyframes.Values = new Vector3[m_colorKeyframes.NumKeyFrames];
            }

            if (m_opacityKeyframes.NumKeyFrames > 0) {
                m_opacityKeyframes.KeyTimes = new float[m_opacityKeyframes.NumKeyFrames];
                m_opacityKeyframes.Values = new float[m_opacityKeyframes.NumKeyFrames];
            }

            if (m_sizeKeyframes.NumKeyFrames > 0) {
                m_sizeKeyframes.KeyTimes = new float[m_sizeKeyframes.NumKeyFrames];
                m_sizeKeyframes.Values = new float[m_sizeKeyframes.NumKeyFrames];
            }

            Read_Color_Keyframe(chunk_load, nullptr, &m_colorKeyframes.Start);

            for (index = 0; index < m_colorKeyframes.NumKeyFrames; index++) {
                Read_Color_Keyframe(chunk_load, &m_colorKeyframes.KeyTimes[index], &m_colorKeyframes.Values[index]);
            }

            int last_keyframe = (m_colorKeyframes.NumKeyFrames - 1);

            if (last_keyframe > 0 && m_colorKeyframes.Values[last_keyframe].X == 0
                && m_colorKeyframes.Values[last_keyframe].Y == 0 && m_colorKeyframes.Values[last_keyframe].Z == 0
                && (m_colorKeyframes.Rand.X > 0 || m_colorKeyframes.Rand.Y > 0 || m_colorKeyframes.Rand.Z > 0)) {
                m_colorKeyframes.Values[last_keyframe].X = -m_colorKeyframes.Rand.X;
                m_colorKeyframes.Values[last_keyframe].Y = -m_colorKeyframes.Rand.Y;
                m_colorKeyframes.Values[last_keyframe].Z = -m_colorKeyframes.Rand.Z;
            }

            Read_Opacity_Keyframe(chunk_load, nullptr, &m_opacityKeyframes.Start);

            for (index = 0; index < m_opacityKeyframes.NumKeyFrames; index++) {
                Read_Opacity_Keyframe(chunk_load, &m_opacityKeyframes.KeyTimes[index], &m_opacityKeyframes.Values[index]);
            }

            Read_Size_Keyframe(chunk_load, nullptr, &m_sizeKeyframes.Start);

            for (index = 0; index < m_sizeKeyframes.NumKeyFrames; index++) {
                Read_Size_Keyframe(chunk_load, &m_sizeKeyframes.KeyTimes[index], &m_sizeKeyframes.Values[index]);
            }

            ret_val = W3D_ERROR_OK;
        }

        chunk_load.Close_Chunk();
    }

    return ret_val;
}

bool ParticleEmitterDefClass::Read_Color_Keyframe(ChunkLoadClass &chunk_load, float *key_time, Vector3 *value)
{
    bool retval = false;
    W3dEmitterColorKeyframeStruct key_frame = { 0 };

    if (chunk_load.Read(&key_frame, sizeof(key_frame)) == sizeof(key_frame)) {
        if (key_time != nullptr) {
            (*key_time) = key_frame.Time;
        }

        if (value != nullptr) {
            (*value) = RGBA_TO_VECTOR3(key_frame.Color);
        }

        retval = true;
    }

    return retval;
}

bool ParticleEmitterDefClass::Read_Opacity_Keyframe(ChunkLoadClass &chunk_load, float *key_time, float *value)
{
    bool retval = false;
    W3dEmitterOpacityKeyframeStruct key_frame = { 0 };
    if (chunk_load.Read(&key_frame, sizeof(key_frame)) == sizeof(key_frame)) {
        if (key_time != nullptr) {
            (*key_time) = key_frame.Time;
        }

        if (value != nullptr) {
            (*value) = key_frame.Opacity;
        }
        retval = true;
    }

    return retval;
}

bool ParticleEmitterDefClass::Read_Size_Keyframe(ChunkLoadClass &chunk_load, float *key_time, float *value)
{
    bool retval = false;
    W3dEmitterSizeKeyframeStruct key_frame = { 0 };

    if (chunk_load.Read(&key_frame, sizeof(key_frame)) == sizeof(key_frame)) {
        if (key_time != nullptr) {
            (*key_time) = key_frame.Time;
        }

        if (value != nullptr) {
            (*value) = key_frame.Size;
        }

        retval = true;
    }

    return retval;
}

W3DErrorType ParticleEmitterDefClass::Read_Line_Properties(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_LOAD_FAILED;

    if (chunk_load.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_INFO) {
        if (chunk_load.Read(&m_lineProperties, sizeof(m_lineProperties)) == sizeof(m_lineProperties)) {
            ret_val = W3D_ERROR_OK;
        }
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Read_Rotation_Keyframes(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_OK;
    W3dEmitterRotationHeaderStruct header;

    if (chunk_load.Read(&header, sizeof(header)) != sizeof(header)) {
        ret_val = W3D_ERROR_LOAD_FAILED;
    }

    m_rotationKeyframes.NumKeyFrames = header.KeyframeCount;
    m_rotationKeyframes.Rand = header.Random;
    m_initialOrientationRandom = header.OrientationRandom;
    W3dEmitterRotationKeyframeStruct key;

    if (chunk_load.Read(&key, sizeof(key)) == sizeof(key)) {
        m_rotationKeyframes.Start = key.Rotation;
    }

    if (m_rotationKeyframes.NumKeyFrames > 0) {
        m_rotationKeyframes.KeyTimes = new float[m_rotationKeyframes.NumKeyFrames];
        m_rotationKeyframes.Values = new float[m_rotationKeyframes.NumKeyFrames];
    }

    for (unsigned int i = 0; (i < header.KeyframeCount) && (ret_val == W3D_ERROR_OK); i++) {
        W3dEmitterRotationKeyframeStruct key2;

        if (chunk_load.Read(&key2, sizeof(key2)) == sizeof(key2)) {
            m_rotationKeyframes.KeyTimes[i] = key2.Time;
            m_rotationKeyframes.Values[i] = key2.Rotation;
        } else {
            m_rotationKeyframes.KeyTimes[i] = 0.0f;
            m_rotationKeyframes.Values[i] = 0.0f;
            ret_val = W3D_ERROR_LOAD_FAILED;
        }
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Read_Frame_Keyframes(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_OK;
    W3dEmitterFrameHeaderStruct header;

    if (chunk_load.Read(&header, sizeof(header)) != sizeof(header)) {
        ret_val = W3D_ERROR_LOAD_FAILED;
    }

    W3dEmitterFrameKeyframeStruct key;

    if (chunk_load.Read(&key, sizeof(key)) == sizeof(key)) {
        m_frameKeyframes.Start = key.Frame;
    }

    m_frameKeyframes.NumKeyFrames = header.KeyframeCount;
    m_frameKeyframes.Rand = header.Random;

    if (m_frameKeyframes.NumKeyFrames > 0) {
        m_frameKeyframes.KeyTimes = new float[m_frameKeyframes.NumKeyFrames];
        m_frameKeyframes.Values = new float[m_frameKeyframes.NumKeyFrames];
    }

    for (unsigned int i = 0; (i < header.KeyframeCount) && (ret_val == W3D_ERROR_OK); i++) {
        W3dEmitterFrameKeyframeStruct key2;

        if (chunk_load.Read(&key2, sizeof(key2)) != sizeof(key2)) {
            ret_val = W3D_ERROR_LOAD_FAILED;
        }

        m_frameKeyframes.KeyTimes[i] = key2.Time;
        m_frameKeyframes.Values[i] = key2.Frame;
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Read_Blur_Time_Keyframes(ChunkLoadClass &chunk_load)
{
    W3DErrorType ret_val = W3D_ERROR_OK;
    W3dEmitterBlurTimeHeaderStruct header;

    if (chunk_load.Read(&header, sizeof(header)) != sizeof(header)) {
        ret_val = W3D_ERROR_LOAD_FAILED;
    }

    W3dEmitterBlurTimeKeyframeStruct key;

    if (chunk_load.Read(&key, sizeof(key)) == sizeof(key)) {
        m_blurTimeKeyframes.Start = key.BlurTime;
    }

    m_blurTimeKeyframes.NumKeyFrames = header.KeyframeCount;
    m_blurTimeKeyframes.Rand = header.Random;

    if (m_blurTimeKeyframes.NumKeyFrames > 0) {
        m_blurTimeKeyframes.KeyTimes = new float[m_blurTimeKeyframes.NumKeyFrames];
        m_blurTimeKeyframes.Values = new float[m_blurTimeKeyframes.NumKeyFrames];
    }

    for (unsigned int i = 0; (i < header.KeyframeCount) && (ret_val == W3D_ERROR_OK); i++) {
        W3dEmitterBlurTimeKeyframeStruct key2;

        if (chunk_load.Read(&key2, sizeof(key2)) != sizeof(key2)) {
            ret_val = W3D_ERROR_LOAD_FAILED;
        }

        m_blurTimeKeyframes.KeyTimes[i] = key2.Time;
        m_blurTimeKeyframes.Values[i] = key2.BlurTime;
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_W3D(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER)) {
        if ((Save_Header(chunk_save) == W3D_ERROR_OK) && (Save_User_Data(chunk_save) == W3D_ERROR_OK)
            && (Save_Info(chunk_save) == W3D_ERROR_OK) && (Save_InfoV2(chunk_save) == W3D_ERROR_OK)
            && (Save_Props(chunk_save) == W3D_ERROR_OK) && (Save_Rotation_Keyframes(chunk_save) == W3D_ERROR_OK)
            && (Save_Frame_Keyframes(chunk_save) == W3D_ERROR_OK) && (Save_Blur_Time_Keyframes(chunk_save) == W3D_ERROR_OK)
            && (Save_Extra_Info(chunk_save) == W3D_ERROR_OK)) {
            ret_val = W3D_ERROR_OK;
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Header(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_HEADER)) {
        W3dEmitterHeaderStruct header = { 0 };
        header.Version = W3D_CURRENT_EMITTER_VERSION;
        strncpy(header.Name, m_pName, sizeof(header.Name));
        header.Name[sizeof(header.Name) - 1] = 0;

        if (chunk_save.Write(&header, sizeof(header)) == sizeof(header)) {
            ret_val = W3D_ERROR_OK;
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_User_Data(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_USER_DATA)) {
        unsigned int string_len = m_pUserString ? (strlen(m_pUserString) + 1) : 0;
        W3dEmitterUserInfoStruct user_info = { 0 };
        user_info.Type = m_iUserType;
        user_info.SizeofStringParam = string_len;

        if (chunk_save.Write(&user_info, sizeof(user_info)) == sizeof(user_info)) {
            ret_val = W3D_ERROR_OK;

            if (m_pUserString != nullptr) {
                if (chunk_save.Write(m_pUserString, string_len) != string_len) {
                    ret_val = W3D_ERROR_SAVE_FAILED;
                }
            }
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Info(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_INFO)) {
        if (chunk_save.Write(&m_info, sizeof(m_info)) == sizeof(m_info)) {
            ret_val = W3D_ERROR_OK;
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Extra_Info(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_INFO)) {
        W3dEmitterExtraInfoStruct info;
        memset(&info, 0, sizeof(info));
        info.FutureStartTime = m_extraInfo.FutureStartTime;

        if (chunk_save.Write(&info, sizeof(info)) == sizeof(info)) {
            ret_val = W3D_ERROR_OK;
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_InfoV2(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_INFOV2)) {
        if (chunk_save.Write(&m_infoV2, sizeof(m_infoV2)) == sizeof(m_infoV2)) {
            ret_val = W3D_ERROR_OK;
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Props(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_PROPS)) {
        W3dEmitterPropertyStruct info = { 0 };
        info.ColorKeyframes = m_colorKeyframes.NumKeyFrames + 1;
        info.OpacityKeyframes = m_opacityKeyframes.NumKeyFrames + 1;
        info.SizeKeyframes = m_sizeKeyframes.NumKeyFrames + 1;
        info.OpacityRandom = m_opacityKeyframes.Rand;
        info.SizeRandom = m_sizeKeyframes.Rand;
        VECTOR3_TO_RGBA(m_colorKeyframes.Rand, info.ColorRandom);

        if (chunk_save.Write(&info, sizeof(info)) == sizeof(info)) {
            if ((Save_Color_Keyframes(chunk_save) == W3D_ERROR_OK) && (Save_Opacity_Keyframes(chunk_save) == W3D_ERROR_OK)
                && (Save_Size_Keyframes(chunk_save) == W3D_ERROR_OK)) {
                ret_val = W3D_ERROR_OK;
            }
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}
W3DErrorType ParticleEmitterDefClass::Save_Color_Keyframes(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;
    W3dEmitterColorKeyframeStruct info = { 0 };
    info.Time = 0;
    VECTOR3_TO_RGBA(m_colorKeyframes.Start, info.Color);

    if (chunk_save.Write(&info, sizeof(info)) == sizeof(info)) {
        int count = m_colorKeyframes.NumKeyFrames;
        bool success = true;

        for (int index = 0; (index < count) && success; index++) {
            info.Time = m_colorKeyframes.KeyTimes[index];
            VECTOR3_TO_RGBA(m_colorKeyframes.Values[index], info.Color);
            success = (chunk_save.Write(&info, sizeof(info)) == sizeof(info));
        }

        ret_val = success ? W3D_ERROR_OK : W3D_ERROR_SAVE_FAILED;
    }

    return ret_val;
}
W3DErrorType ParticleEmitterDefClass::Save_Opacity_Keyframes(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;
    W3dEmitterOpacityKeyframeStruct info = { 0 };
    info.Time = 0;
    info.Opacity = m_opacityKeyframes.Start;

    if (chunk_save.Write(&info, sizeof(info)) == sizeof(info)) {
        int count = m_opacityKeyframes.NumKeyFrames;
        bool success = true;

        for (int index = 0; (index < count) && success; index++) {
            info.Time = m_opacityKeyframes.KeyTimes[index];
            info.Opacity = m_opacityKeyframes.Values[index];
            success = (chunk_save.Write(&info, sizeof(info)) == sizeof(info));
        }

        ret_val = success ? W3D_ERROR_OK : W3D_ERROR_SAVE_FAILED;
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Size_Keyframes(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;
    W3dEmitterSizeKeyframeStruct info = { 0 };
    info.Time = 0;
    info.Size = m_sizeKeyframes.Start;

    if (chunk_save.Write(&info, sizeof(info)) == sizeof(info)) {
        int count = m_sizeKeyframes.NumKeyFrames;
        bool success = true;

        for (int index = 0; (index < count) && success; index++) {
            info.Time = m_sizeKeyframes.KeyTimes[index];
            info.Size = m_sizeKeyframes.Values[index];
            success = (chunk_save.Write(&info, sizeof(info)) == sizeof(info));
        }

        ret_val = success ? W3D_ERROR_OK : W3D_ERROR_SAVE_FAILED;
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Line_Properties(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_LINE_PROPERTIES)) {
        if (chunk_save.Write(&m_lineProperties, sizeof(m_lineProperties)) == sizeof(m_lineProperties)) {
            ret_val = W3D_ERROR_OK;
        }

        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Rotation_Keyframes(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES)) {
        W3dEmitterRotationHeaderStruct header;
        header.KeyframeCount = m_rotationKeyframes.NumKeyFrames;
        header.Random = m_rotationKeyframes.Rand;
        header.OrientationRandom = m_initialOrientationRandom;
        chunk_save.Write(&header, sizeof(W3dEmitterRotationHeaderStruct));

        bool success = true;
        W3dEmitterRotationKeyframeStruct key;
        key.Time = 0;
        key.Rotation = m_rotationKeyframes.Start;

        chunk_save.Write(&key, sizeof(key));

        for (unsigned int index = 0; (index < header.KeyframeCount) && success; index++) {
            key.Time = m_rotationKeyframes.KeyTimes[index];
            key.Rotation = m_rotationKeyframes.Values[index];
            success = (chunk_save.Write(&key, sizeof(key)) == sizeof(key));
        }

        ret_val = success ? W3D_ERROR_OK : W3D_ERROR_SAVE_FAILED;
        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Frame_Keyframes(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_FRAME_KEYFRAMES)) {
        W3dEmitterFrameHeaderStruct header;
        header.KeyframeCount = m_frameKeyframes.NumKeyFrames;
        header.Random = m_frameKeyframes.Rand;
        chunk_save.Write(&header, sizeof(W3dEmitterFrameHeaderStruct));

        bool success = true;
        W3dEmitterFrameKeyframeStruct key;
        key.Time = 0;
        key.Frame = m_frameKeyframes.Start;
        chunk_save.Write(&key, sizeof(key));

        for (unsigned int index = 0; (index < header.KeyframeCount) && success; index++) {
            key.Time = m_frameKeyframes.KeyTimes[index];
            key.Frame = m_frameKeyframes.Values[index];
            success = (chunk_save.Write(&key, sizeof(key)) == sizeof(key));
        }

        ret_val = success ? W3D_ERROR_OK : W3D_ERROR_SAVE_FAILED;
        chunk_save.End_Chunk();
    }

    return ret_val;
}

W3DErrorType ParticleEmitterDefClass::Save_Blur_Time_Keyframes(ChunkSaveClass &chunk_save)
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;

    if (chunk_save.Begin_Chunk(W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES)) {
        W3dEmitterBlurTimeHeaderStruct header;
        header.KeyframeCount = m_blurTimeKeyframes.NumKeyFrames;
        header.Random = m_blurTimeKeyframes.Rand;
        chunk_save.Write(&header, sizeof(W3dEmitterBlurTimeHeaderStruct));

        bool success = true;
        W3dEmitterBlurTimeKeyframeStruct key;
        key.Time = 0;
        key.BlurTime = m_blurTimeKeyframes.Start;
        chunk_save.Write(&key, sizeof(key));

        for (unsigned int index = 0; (index < header.KeyframeCount) && success; index++) {
            key.Time = m_blurTimeKeyframes.KeyTimes[index];
            key.BlurTime = m_blurTimeKeyframes.Values[index];
            success = (chunk_save.Write(&key, sizeof(key)) == sizeof(key));
        }

        ret_val = success ? W3D_ERROR_OK : W3D_ERROR_SAVE_FAILED;
        chunk_save.End_Chunk();
    }

    return ret_val;
}

void ParticleEmitterDefClass::Set_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes)
{
    if (m_colorKeyframes.KeyTimes != nullptr) {
        delete[] m_colorKeyframes.KeyTimes;
        m_colorKeyframes.KeyTimes = nullptr;
    }

    if (m_colorKeyframes.Values != nullptr) {
        delete[] m_colorKeyframes.Values;
        m_colorKeyframes.Values = nullptr;
    }

    Copy_Emitter_Property_Struct(m_colorKeyframes, keyframes);
}

void ParticleEmitterDefClass::Set_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    if (m_opacityKeyframes.KeyTimes != nullptr) {
        delete[] m_opacityKeyframes.KeyTimes;
        m_opacityKeyframes.KeyTimes = nullptr;
    }

    if (m_opacityKeyframes.Values != nullptr) {
        delete[] m_opacityKeyframes.Values;
        m_opacityKeyframes.Values = nullptr;
    }

    Copy_Emitter_Property_Struct(m_opacityKeyframes, keyframes);
}

void ParticleEmitterDefClass::Set_Size_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    if (m_sizeKeyframes.KeyTimes != nullptr) {
        delete[] m_sizeKeyframes.KeyTimes;
        m_sizeKeyframes.KeyTimes = nullptr;
    }

    if (m_sizeKeyframes.Values != nullptr) {
        delete[] m_sizeKeyframes.Values;
        m_sizeKeyframes.Values = nullptr;
    }

    Copy_Emitter_Property_Struct(m_sizeKeyframes, keyframes);
}

void ParticleEmitterDefClass::Set_Rotation_Keyframes(ParticlePropertyStruct<float> &keyframes, float orient_rnd)
{
    if (m_rotationKeyframes.KeyTimes != nullptr) {
        delete[] m_rotationKeyframes.KeyTimes;
        m_rotationKeyframes.KeyTimes = nullptr;
    }

    if (m_rotationKeyframes.Values != nullptr) {
        delete[] m_rotationKeyframes.Values;
        m_rotationKeyframes.Values = nullptr;
    }

    Copy_Emitter_Property_Struct(m_rotationKeyframes, keyframes);
    m_initialOrientationRandom = orient_rnd;
}

void ParticleEmitterDefClass::Set_Frame_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    if (m_frameKeyframes.KeyTimes != nullptr) {
        delete[] m_frameKeyframes.KeyTimes;
        m_frameKeyframes.KeyTimes = nullptr;
    }

    if (m_frameKeyframes.Values != nullptr) {
        delete[] m_frameKeyframes.Values;
        m_frameKeyframes.Values = nullptr;
    }

    Copy_Emitter_Property_Struct(m_frameKeyframes, keyframes);
}

void ParticleEmitterDefClass::Set_Blur_Time_Keyframes(ParticlePropertyStruct<float> &keyframes)
{
    if (m_blurTimeKeyframes.KeyTimes != nullptr) {
        delete[] m_blurTimeKeyframes.KeyTimes;
        m_blurTimeKeyframes.KeyTimes = nullptr;
    }

    if (m_blurTimeKeyframes.Values != nullptr) {
        delete[] m_blurTimeKeyframes.Values;
        m_blurTimeKeyframes.Values = nullptr;
    }

    Copy_Emitter_Property_Struct(m_blurTimeKeyframes, keyframes);
}

void ParticleEmitterDefClass::Get_Color_Keyframes(ParticlePropertyStruct<Vector3> &keyframes) const
{
    Copy_Emitter_Property_Struct(keyframes, m_colorKeyframes);
}

void ParticleEmitterDefClass::Get_Opacity_Keyframes(ParticlePropertyStruct<float> &keyframes) const
{
    Copy_Emitter_Property_Struct(keyframes, m_opacityKeyframes);
}

void ParticleEmitterDefClass::Get_Size_Keyframes(ParticlePropertyStruct<float> &keyframes) const
{
    Copy_Emitter_Property_Struct(keyframes, m_sizeKeyframes);
}

void ParticleEmitterDefClass::Get_Rotation_Keyframes(ParticlePropertyStruct<float> &keyframes) const
{
    Copy_Emitter_Property_Struct(keyframes, m_rotationKeyframes);
}

void ParticleEmitterDefClass::Get_Frame_Keyframes(ParticlePropertyStruct<float> &keyframes) const
{
    Copy_Emitter_Property_Struct(keyframes, m_frameKeyframes);
}

void ParticleEmitterDefClass::Get_Blur_Time_Keyframes(ParticlePropertyStruct<float> &blurtimeframes) const
{
    Copy_Emitter_Property_Struct(blurtimeframes, m_blurTimeKeyframes);
}

RenderObjClass *ParticleEmitterPrototypeClass::Create()
{
    return ParticleEmitterClass::Create_From_Definition(*m_pDefinition);
}

PrototypeClass *ParticleEmitterLoaderClass::Load_W3D(ChunkLoadClass &chunk_load)
{
    ParticleEmitterPrototypeClass *prototype = nullptr;
    ParticleEmitterDefClass *definition = new ParticleEmitterDefClass;

    if (definition != nullptr) {
        if (definition->Load_W3D(chunk_load) != W3D_ERROR_OK) {
            delete definition;
            definition = nullptr;
        } else {
            prototype = new ParticleEmitterPrototypeClass(definition);
        }
    }

    return prototype;
}
