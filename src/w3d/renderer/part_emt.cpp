/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Particle Emitter
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "part_emt.h"
#include "assetmgr.h"
#include "part_ldr.h"
#include "scene.h"
#include "texture.h"
#include "w3d.h"
#include "w3derr.h"
#include <gcd_lcm.h>
#include <limits.h>

Vector3 s_inheritedWorldSpaceEmitterVel;
bool ParticleEmitterClass::s_debugDisable = false;
bool ParticleEmitterClass::s_defaultRemoveOnComplete = true;

ParticleEmitterClass::ParticleEmitterClass(float emit_rate,
    unsigned int burst_size,
    Vector3Randomizer *pos_rnd,
    Vector3 base_vel,
    Vector3Randomizer *vel_rnd,
    float out_vel,
    float vel_inherit_factor,
    ParticlePropertyStruct<Vector3> &color,
    ParticlePropertyStruct<float> &opacity,
    ParticlePropertyStruct<float> &size,
    ParticlePropertyStruct<float> &rotation,
    float orient_rnd,
    ParticlePropertyStruct<float> &frames,
    ParticlePropertyStruct<float> &blur_times,
    Vector3 accel,
    float max_age,
    float future_start_time,
    TextureClass *tex,
    ShaderClass shader,
    int max_particles,
    int max_buffer_size,
    bool pingpong,
    int render_mode,
    int frame_mode,
    const W3dEmitterLinePropertiesStruct *line_props) :
    m_oneTimeBurstSize(1),
    m_oneTimeBurst(false),
    m_posRand(pos_rnd),
    m_baseVel(base_vel * 0.001f),
    m_velRand(vel_rnd),
    m_outwardVel(out_vel * 0.001f),
    m_velInheritFactor(vel_inherit_factor),
    m_emitRemain(0),
    m_prevQ(true),
    m_prevOrig(0.0f, 0.0f, 0.0f),
    m_active(false),
    m_firstTime(true),
    m_particlesLeft(max_particles),
    m_maxParticles(max_particles),
    m_isComplete(false),
    m_nameString(nullptr),
    m_userString(nullptr),
    m_removeOnComplete(s_defaultRemoveOnComplete),
    m_isInScene(false),
    m_groupID(0),
    m_buffer(nullptr),
    m_unk(false)
{
    m_emitRate = emit_rate > 0.0f ? (unsigned int)(1000.0f / emit_rate) : 1000;
    m_burstSize = burst_size != 0 ? burst_size : 1;
    max_age = max_age > 0.0f ? max_age : 1.0f;
    m_velRand->Scale(0.001f);
    int max_num = m_burstSize * emit_rate * (max_age + 1);

    if (max_particles > 0) {
        max_num = std::min(max_num, max_particles);
    }

    if (max_buffer_size > 0) {
        max_num = std::min(max_num, max_buffer_size);
    }

    max_num = std::max(max_num, 2);

    m_buffer = new ParticleBufferClass(this,
        max_num,
        color,
        opacity,
        size,
        rotation,
        orient_rnd,
        frames,
        blur_times,
        accel / 1000000.0f,
        max_age,
        future_start_time,
        tex,
        shader,
        pingpong,
        render_mode,
        frame_mode,
        line_props);

    m_bufferSceneNeeded = true;
    m_nameString = strdup("ParticleEmitter");
}

ParticleEmitterClass::ParticleEmitterClass(const ParticleEmitterClass &src) : RenderObjClass(src), m_isInScene(false)
{
    m_emitRate = src.m_emitRate;
    m_burstSize = src.m_burstSize;
    m_oneTimeBurstSize = src.m_oneTimeBurstSize;
    m_oneTimeBurst = src.m_oneTimeBurst;

    if (src.m_posRand != nullptr) {
        m_posRand = src.m_posRand->Clone();
    } else {
        m_posRand = nullptr;
    }

    m_baseVel = src.m_baseVel;

    if (src.m_velRand != nullptr) {
        m_velRand = src.m_velRand->Clone();
    } else {
        m_velRand = nullptr;
    }

    m_outwardVel = src.m_outwardVel;
    m_velInheritFactor = src.m_velInheritFactor;
    m_emitRemain = src.m_emitRemain;
    m_prevQ = src.m_prevQ;
    m_prevOrig = src.m_prevOrig;
    m_maxParticles = src.m_maxParticles;
    m_particlesLeft = src.m_particlesLeft;

    m_buffer = static_cast<ParticleBufferClass *>(src.m_buffer->Clone());
    m_buffer->Set_Emitter(this);
    m_bufferSceneNeeded = true;

    m_active = true;
    m_firstTime = true;
    m_isComplete = false;

    m_nameString = strdup(src.m_nameString);
    m_userString = strdup(src.m_userString);
    m_removeOnComplete = src.m_removeOnComplete;
    m_isInScene = false;
    m_groupID = 0;
    m_unk = src.m_unk;
}

ParticleEmitterClass &ParticleEmitterClass::operator=(const ParticleEmitterClass &that)
{
    RenderObjClass::operator=(that);
    return *this;
}

ParticleEmitterClass::~ParticleEmitterClass()
{
    m_buffer->Emitter_Is_Dead();
    m_buffer->Release_Ref();

    if (m_posRand != nullptr) {
        delete m_posRand;
        m_posRand = nullptr;
    }

    if (m_velRand != nullptr) {
        delete m_velRand;
        m_velRand = nullptr;
    }

    if (m_nameString != nullptr) {
        free(m_nameString);
        m_nameString = nullptr;
    }

    if (m_userString != nullptr) {
        free(m_userString);
        m_userString = nullptr;
    }
}

ParticleEmitterClass *ParticleEmitterClass::Create_From_Definition(const ParticleEmitterDefClass &definition)
{
    ParticleEmitterClass *emitter = nullptr;
    const char *texture_filename = definition.Get_Texture_Filename();
    TextureClass *texture = nullptr;

    if (texture_filename != nullptr && texture_filename[0] != 0) {
        texture = W3DAssetManager::Get_Instance()->Get_Texture(texture_filename, MIP_LEVELS_ALL, WW3D_FORMAT_UNKNOWN);
    }

    ShaderClass shader;
    definition.Get_Shader(shader);

    if (W3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
        shader.Enable_Fog("ParticleEmitterClass");
    }

    ParticlePropertyStruct<Vector3> color_keys;
    ParticlePropertyStruct<float> opacity_keys;
    ParticlePropertyStruct<float> size_keys;
    ParticlePropertyStruct<float> rotation_keys;
    ParticlePropertyStruct<float> frame_keys;
    ParticlePropertyStruct<float> blur_time_keys;

    definition.Get_Color_Keyframes(color_keys);
    definition.Get_Opacity_Keyframes(opacity_keys);
    definition.Get_Size_Keyframes(size_keys);
    definition.Get_Rotation_Keyframes(rotation_keys);
    definition.Get_Frame_Keyframes(frame_keys);
    definition.Get_Blur_Time_Keyframes(blur_time_keys);

    emitter = new ParticleEmitterClass(definition.Get_Emission_Rate(),
        definition.Get_Burst_Size(),
        definition.Get_Creation_Volume(),
        definition.Get_Velocity(),
        definition.Get_Velocity_Random(),
        definition.Get_Outward_Vel(),
        definition.Get_Vel_Inherit(),
        color_keys,
        opacity_keys,
        size_keys,
        rotation_keys,
        definition.Get_Initial_Orientation_Random(),
        frame_keys,
        blur_time_keys,
        definition.Get_Acceleration(),
        definition.Get_Lifetime(),
        definition.Get_Future_Start_Time(),
        texture,
        shader,
        definition.Get_Max_Emissions(),
        0,
        false,
        definition.Get_Render_Mode(),
        definition.Get_Frame_Mode(),
        definition.Get_Line_Properties());

    if (color_keys.KeyTimes != nullptr) {
        delete[] color_keys.KeyTimes;
    }

    if (color_keys.Values != nullptr) {
        delete[] color_keys.Values;
    }

    if (opacity_keys.KeyTimes != nullptr) {
        delete[] opacity_keys.KeyTimes;
    }

    if (opacity_keys.Values != nullptr) {
        delete[] opacity_keys.Values;
    }

    if (size_keys.KeyTimes != nullptr) {
        delete[] size_keys.KeyTimes;
    }

    if (size_keys.Values != nullptr) {
        delete[] size_keys.Values;
    }

    if (rotation_keys.KeyTimes != nullptr) {
        delete[] rotation_keys.KeyTimes;
    }

    if (rotation_keys.Values != nullptr) {
        delete[] rotation_keys.Values;
    }

    if (frame_keys.KeyTimes != nullptr) {
        delete[] frame_keys.KeyTimes;
    }

    if (frame_keys.Values != nullptr) {
        delete[] frame_keys.Values;
    }

    if (blur_time_keys.KeyTimes != nullptr) {
        delete[] blur_time_keys.KeyTimes;
    }

    if (blur_time_keys.Values != nullptr) {
        delete[] blur_time_keys.Values;
    }

    emitter->Set_Name(definition.Get_Name());
    Ref_Ptr_Release(texture);
    return emitter;
}

RenderObjClass *ParticleEmitterClass::Clone() const
{
    return new ParticleEmitterClass(*this);
}

void ParticleEmitterClass::Restart()
{
    Start();
}

void ParticleEmitterClass::Notify_Added(SceneClass *scene)
{
    RenderObjClass::Notify_Added(scene);
    scene->Register(this, SceneClass::ON_FRAME_UPDATE);

    if (!m_firstTime) {
        m_active = true;
    }

    m_isInScene = true;
}

void ParticleEmitterClass::Notify_Removed(SceneClass *scene)
{
    scene->Unregister(this, SceneClass::ON_FRAME_UPDATE);
    RenderObjClass::Notify_Removed(scene);
    m_active = false;
    m_isInScene = false;
}

void ParticleEmitterClass::Scale(float scale)
{
    if (m_posRand != nullptr) {
        m_posRand->Scale(scale);
    }

    m_baseVel *= scale;

    if (m_velRand != nullptr) {
        m_velRand->Scale(scale);
    }

    m_outwardVel *= scale;
    m_buffer->Scale(scale);
}

void ParticleEmitterClass::On_Frame_Update()
{
    if (m_active && !m_isComplete) {
        if (m_firstTime) {
            if (m_bufferSceneNeeded) {
                if (Is_In_Scene()) {
                    m_buffer->Add(m_scene);
                    m_bufferSceneNeeded = false;
                } else {
                    return;
                }
            }

            m_bufferSceneNeeded = false;
            m_prevQ = Build_Quaternion(Get_Transform());
            m_prevOrig = Get_Transform().Get_Translation();
            m_firstTime = false;
        }
    }

    if (Is_Complete()) {
        if (Is_In_Scene() && Is_Remove_On_Complete_Enabled()) {
            m_scene->Register(this, SceneClass::RELEASE);
        }
    }
}

void ParticleEmitterClass::Reset()
{
    m_active = true;
    m_prevQ = Build_Quaternion(Get_Transform());
    m_prevOrig = Get_Transform().Get_Translation();
    m_particlesLeft = m_maxParticles;
    m_emitRemain = 0;
    m_isComplete = false;
}

void ParticleEmitterClass::Start()
{
    m_active = true;
    m_prevQ = Build_Quaternion(Get_Transform());
    m_prevOrig = Get_Transform().Get_Translation();

    if (m_isComplete) {
        m_particlesLeft = m_maxParticles;
        m_isComplete = false;
    }

    m_groupID++;
    m_buffer->Set_Current_Group_ID(m_groupID);
}

void ParticleEmitterClass::Stop()
{
    m_active = false;
}

bool ParticleEmitterClass::Is_Stopped()
{
    return !m_active;
}

void ParticleEmitterClass::Set_Position_Randomizer(Vector3Randomizer *rand)
{
    if (m_posRand != nullptr) {
        delete m_posRand;
        m_posRand = nullptr;
    }

    m_posRand = rand;
}

void ParticleEmitterClass::Set_Velocity_Randomizer(Vector3Randomizer *rand)
{
    if (m_velRand != nullptr) {
        delete m_velRand;
        m_velRand = nullptr;
    }

    m_velRand = rand;

    if (m_velRand != nullptr) {
        m_velRand->Scale(0.001f);
    }
}

Vector3Randomizer *ParticleEmitterClass::Get_Creation_Volume() const
{
    Vector3Randomizer *randomizer = nullptr;

    if (m_posRand != nullptr) {
        randomizer = m_posRand->Clone();
    }

    return randomizer;
}

Vector3Randomizer *ParticleEmitterClass::Get_Velocity_Random() const
{
    Vector3Randomizer *randomizer = nullptr;

    if (m_velRand != nullptr) {
        randomizer = m_velRand->Clone();
        randomizer->Scale(1000.0F);
    }

    return randomizer;
}

void ParticleEmitterClass::Set_Base_Velocity(const Vector3 &base_vel)
{
    m_baseVel = base_vel * 0.001f;
}

void ParticleEmitterClass::Set_Outwards_Velocity(float out_vel)
{
    m_outwardVel = out_vel * 0.001f;
}

void ParticleEmitterClass::Set_Velocity_Inheritance_Factor(float inh_factor)
{
    m_velInheritFactor = inh_factor;
}

void ParticleEmitterClass::Emit()
{
    if (m_active && !m_isComplete) {
        Quaternion curr_quat;
        Vector3 curr_orig;
        curr_quat = Build_Quaternion(Get_Transform());
        curr_orig = Get_Transform().Get_Translation();
        Create_New_Particles(curr_quat, curr_orig);
        m_prevQ = curr_quat;
        m_prevOrig = curr_orig;
    } else {
        m_prevQ = Build_Quaternion(Get_Transform());
        m_prevOrig = Get_Transform().Get_Translation();
    }
}

void ParticleEmitterClass::Update_Cached_Bounding_Volumes() const
{
    m_cachedBoundingSphere.Init(Get_Position(), 0.0f);
    m_cachedBoundingBox.m_center = Get_Position();
    m_cachedBoundingBox.m_extent.Set(0.0f, 0.0f, 0.0f);
    Validate_Cached_Bounding_Volumes();
}

void ParticleEmitterClass::Create_New_Particles(const Quaternion &curr_quat, const Vector3 &curr_orig)
{
    Quaternion quat;
    Vector3 orig;
    unsigned int frametime = W3D::Get_Frame_Time();

    if (frametime > 100 * m_emitRate) {
        unsigned int buf_size = m_buffer->Get_Buffer_Size();
        unsigned int gcd = Greatest_Common_Divisor(buf_size, m_burstSize);
        unsigned int bursts = buf_size / gcd;
        unsigned int cycle_time = m_emitRate * bursts;
        if (cycle_time > 1) {
            frametime = frametime % cycle_time;
        } else {
            frametime = 1;
        }
    }

    m_emitRemain += frametime;
    float fl_frametime = (float)frametime;
    float alpha = 1 - ((float)m_emitRemain / fl_frametime);
    float d_alpha = (float)m_emitRate / fl_frametime;
    SlerpInfoStruct slerp_info;
    Slerp_Setup(m_prevQ, curr_quat, &slerp_info);

    if (m_velInheritFactor != 0.0f) {
        s_inheritedWorldSpaceEmitterVel = (curr_orig - m_prevOrig) * (m_velInheritFactor / fl_frametime);
    } else {
        s_inheritedWorldSpaceEmitterVel.Set(0.0f, 0.0f, 0.0f);
    }

    for (; m_emitRemain > m_emitRate;) {
        m_emitRemain -= m_emitRate;
        alpha += d_alpha;
        quat = Cached_Slerp(m_prevQ, curr_quat, alpha, &slerp_info);
        orig = Lerp(m_prevOrig, curr_orig, alpha);
        unsigned int age = W3D::Get_Sync_Time() - m_emitRemain;
        unsigned int burst_size = m_burstSize;

        if (m_oneTimeBurst) {
            burst_size = m_oneTimeBurstSize;
            m_oneTimeBurst = false;
        }

        if (m_particlesLeft > 0) {
            if (burst_size > (unsigned int)m_particlesLeft) {
                burst_size = (unsigned int)m_particlesLeft;
                m_particlesLeft = 0;
            } else {
                m_particlesLeft -= burst_size;
            }

            if (m_particlesLeft <= 0) {
                m_isComplete = true;
            }
        }

        for (unsigned int i = 0; i < burst_size; i++) {
            Initialize_Particle(m_buffer->Add_Uninitialized_New_Particle(), age, quat, orig);
        }

        if (m_isComplete) {
            break;
        }
    }
}

void ParticleEmitterClass::Initialize_Particle(
    NewParticleStruct *newpart, unsigned int timestamp, const Quaternion &quat, const Vector3 &orig)
{
    newpart->m_timeStamp = timestamp;
    Vector3 rand_pos;

    if (m_posRand != nullptr) {
        m_posRand->Get_Vector(rand_pos);
    } else {
        rand_pos.Set(0.0f, 0.0f, 0.0f);
    }

    newpart->m_position = quat.Rotate_Vector(rand_pos) + orig;
    Vector3 rand_vel;

    if (m_velRand != nullptr) {
        m_velRand->Get_Vector(rand_vel);
    } else {
        rand_vel.Set(0.0f, 0.0f, 0.0f);
    }

    if (m_outwardVel) {
        Vector3 outwards;
        float pos_l2 = rand_pos.Length2();

        if (pos_l2) {
            outwards = rand_pos * (m_outwardVel * GameMath::Inv_Sqrt(pos_l2));
        } else {
            outwards.X = m_outwardVel;
            outwards.Y = 0.0f;
            outwards.Z = 0.0f;
        }

        rand_vel += outwards;
    }

    rand_vel += m_baseVel;
    newpart->m_velocity = s_inheritedWorldSpaceEmitterVel + quat.Rotate_Vector(rand_vel);
    newpart->m_groupID = m_groupID;
}

ParticleEmitterDefClass *ParticleEmitterClass::Build_Definition() const
{
    ParticleEmitterDefClass *definition = new ParticleEmitterDefClass;
    captainslog_assert(definition != nullptr);

    if (definition != nullptr) {
        TextureClass *texture = Get_Texture();

        if (texture != nullptr) {
            definition->Set_Texture_Filename(texture->Get_Name());
            Ref_Ptr_Release(texture);
        }

        definition->Set_Render_Mode(Get_Render_Mode());
        definition->Set_Frame_Mode(Get_Frame_Mode());
        definition->Set_Name(Get_Name());
        definition->Set_Lifetime(Get_Lifetime());
        definition->Set_Future_Start_Time(Get_Future_Start_Time());
        definition->Set_Emission_Rate(Get_Emission_Rate());
        definition->Set_Max_Emissions(Get_Max_Particles());
        definition->Set_Fade_Time(Get_Fade_Time());
        definition->Set_Gravity(0);
        definition->Set_Elasticity(0);
        definition->Set_Velocity(Get_Start_Velocity());
        definition->Set_Acceleration(Get_Acceleration());
        definition->Set_Burst_Size(Get_Burst_Size());
        definition->Set_Outward_Vel(Get_Outwards_Vel());
        definition->Set_Vel_Inherit(Get_Velocity_Inherit());
        definition->Set_Shader(Get_Shader());

        Vector3Randomizer *randomizer = Get_Creation_Volume();
        definition->Set_Creation_Volume(randomizer);
        randomizer = Get_Velocity_Random();
        definition->Set_Velocity_Random(randomizer);
        ParticlePropertyStruct<Vector3> colors;
        Get_Color_Key_Frames(colors);
        definition->Set_Color_Keyframes(colors);

        if (colors.KeyTimes != nullptr) {
            delete[] colors.KeyTimes;
        }

        if (colors.Values != nullptr) {
            delete[] colors.Values;
        }

        ParticlePropertyStruct<float> opacities;
        Get_Opacity_Key_Frames(opacities);
        definition->Set_Opacity_Keyframes(opacities);

        if (opacities.KeyTimes != nullptr) {
            delete[] opacities.KeyTimes;
        }

        if (opacities.Values != nullptr) {
            delete[] opacities.Values;
        }

        ParticlePropertyStruct<float> sizes;
        Get_Size_Key_Frames(sizes);
        definition->Set_Size_Keyframes(sizes);

        if (sizes.KeyTimes != nullptr) {
            delete[] sizes.KeyTimes;
        }

        if (sizes.Values != nullptr) {
            delete[] sizes.Values;
        }

        ParticlePropertyStruct<float> rotations;
        Get_Rotation_Key_Frames(rotations);
        definition->Set_Rotation_Keyframes(rotations, Get_Initial_Orientation_Random());

        if (rotations.KeyTimes != nullptr) {
            delete[] rotations.KeyTimes;
        }

        if (rotations.Values != nullptr) {
            delete[] rotations.Values;
        }

        ParticlePropertyStruct<float> frames;
        Get_Frame_Key_Frames(frames);
        definition->Set_Frame_Keyframes(frames);

        if (frames.KeyTimes != nullptr) {
            delete[] frames.KeyTimes;
        }

        if (frames.Values != nullptr) {
            delete[] frames.Values;
        }

        ParticlePropertyStruct<float> blur_times;
        Get_Blur_Time_Key_Frames(blur_times);
        definition->Set_Blur_Time_Keyframes(blur_times);

        if (blur_times.KeyTimes != nullptr) {
            delete[] blur_times.KeyTimes;
        }

        if (blur_times.Values != nullptr) {
            delete[] blur_times.Values;
        }

        definition->Set_Line_Texture_Mapping_Mode(Get_Line_Texture_Mapping_Mode());
        definition->Set_Merge_Intersections(Is_Merge_Intersections());
        definition->Set_Freeze_Random(Is_Freeze_Random());
        definition->Set_Disable_Sorting(Is_Sorting_Disabled());
        definition->Set_End_Caps(Are_End_Caps_Enabled());
        definition->Set_Subdivision_Level(Get_Subdivision_Level());
        definition->Set_Noise_Amplitude(Get_Noise_Amplitude());
        definition->Set_Merge_Abort_Factor(Get_Merge_Abort_Factor());
        definition->Set_Texture_Tile_Factor(Get_Texture_Tile_Factor());
        definition->Set_UV_Offset_Rate(Get_UV_Offset_Rate());
    }

    return definition;
}

W3DErrorType ParticleEmitterClass::Save(ChunkSaveClass &chunk_save) const
{
    W3DErrorType ret_val = W3D_ERROR_SAVE_FAILED;
    ParticleEmitterDefClass *definition = Build_Definition();

    if (definition != nullptr) {
        ret_val = definition->Save_W3D(chunk_save);
    }

    return ret_val;
}

void ParticleEmitterClass::Set_Name(const char *pname)
{
    if (m_nameString != nullptr) {
        free(m_nameString);
        m_nameString = nullptr;
    }

    m_nameString = strdup(pname);
}

void ParticleEmitterClass::Update_On_Visibilty()
{
    if (Is_Not_Hidden_At_All() && !m_unk && Is_Stopped() && m_isInScene) {
        Start();
    } else if ((!Is_Not_Hidden_At_All() || m_unk) && (!Is_Stopped())) {
        Stop();
    }
}

void ParticleEmitterClass::Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list, bool textures_only)
{
    TextureClass *texture = Get_Texture();

    if (texture != nullptr) {
        file_list.Add(texture->Get_Full_Path());
        Ref_Ptr_Release(texture);
    }

    RenderObjClass::Add_Dependencies_To_List(file_list, textures_only);
}
