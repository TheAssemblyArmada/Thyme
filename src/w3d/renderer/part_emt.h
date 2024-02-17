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
#pragma once

#include "always.h"
#include "part_buf.h"
#include "quat.h"
#include "random.h"
#include "rendobj.h"
#include "v3_rnd.h"
#include "w3d_file.h"
#include "w3derr.h"

class ParticleEmitterDefClass;
class ChunkSaveClass;
struct NewParticleStruct;

template<class T> struct ParticlePropertyStruct
{
    T Start;
    T Rand;
    unsigned int NumKeyFrames;
    float *KeyTimes;
    T *Values;
};

template<class T>
__inline void Copy_Emitter_Property_Struct(ParticlePropertyStruct<T> &dest, const ParticlePropertyStruct<T> &src)
{
    dest.Start = src.Start;
    dest.Rand = src.Rand;
    dest.NumKeyFrames = src.NumKeyFrames;
    dest.KeyTimes = nullptr;
    dest.Values = nullptr;

    if (dest.NumKeyFrames > 0) {
        dest.KeyTimes = new float[dest.NumKeyFrames];
        dest.Values = new T[dest.NumKeyFrames];
        memcpy(dest.KeyTimes, src.KeyTimes, sizeof(float) * dest.NumKeyFrames);
        memcpy(dest.Values, src.Values, sizeof(T) * dest.NumKeyFrames);
    }

    return;
}

class ParticleEmitterClass : public RenderObjClass
{
public:
    ParticleEmitterClass(float emit_rate,
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
        ShaderClass shader = ShaderClass::s_presetAdditiveSpriteShader,
        int max_particles = 0,
        int max_buffer_size = -1,
        bool pingpong = false,
        int render_mode = W3D_EMITTER_RENDER_MODE_TRI_PARTICLES,
        int frame_mode = W3D_EMITTER_FRAME_MODE_1x1,
        const W3dEmitterLinePropertiesStruct *line_props = nullptr);

    ParticleEmitterClass(const ParticleEmitterClass &src);
    ParticleEmitterClass &operator=(const ParticleEmitterClass &);
    virtual ~ParticleEmitterClass();
    virtual RenderObjClass *Clone() const;
    static ParticleEmitterClass *Create_From_Definition(const ParticleEmitterDefClass &definition);
    ParticleEmitterDefClass *Build_Definition() const;
    W3DErrorType Save(ChunkSaveClass &chunk_save) const;
    virtual int Class_ID() const { return CLASSID_PARTICLEEMITTER; }
    virtual const char *Get_Name() const { return m_nameString; }
    virtual void Set_Name(const char *pname);

    virtual void Notify_Added(SceneClass *scene);
    virtual void Notify_Removed(SceneClass *scene);
    virtual void Render(RenderInfoClass &rinfo) {}
    virtual void Restart();
    virtual void Scale(float scale);
    virtual void On_Frame_Update();

    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
    {
        sphere.Center.Set(0, 0, 0);
        sphere.Radius = 0;
    }

    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const
    {
        box.m_center.Set(0, 0, 0);
        box.m_extent.Set(0, 0, 0);
    }

    virtual void Set_Hidden(int onoff)
    {
        RenderObjClass::Set_Hidden(onoff);
        Update_On_Visibilty();
    }

    virtual void Set_Visible(int onoff)
    {
        RenderObjClass::Set_Visible(onoff);
        Update_On_Visibilty();
    }

    virtual void Set_Animation_Hidden(int onoff)
    {
        RenderObjClass::Set_Animation_Hidden(onoff);
        Update_On_Visibilty();
    }

    virtual void Set_Force_Visible(int onoff)
    {
        RenderObjClass::Set_Force_Visible(onoff);
        Update_On_Visibilty();
    }

    virtual void Set_LOD_Bias(float bias)
    {
        if (m_buffer) {
            m_buffer->Set_LOD_Bias(bias);
        }
    }

    void Reset();
    void Start();
    void Stop();
    bool Is_Stopped();
    void Set_Position_Randomizer(Vector3Randomizer *rand);
    void Set_Velocity_Randomizer(Vector3Randomizer *rand);
    void Set_Base_Velocity(const Vector3 &base_vel);
    void Set_Outwards_Velocity(float out_vel);
    void Set_Velocity_Inheritance_Factor(float inh_factor);

    void Set_Acceleration(const Vector3 &acceleration)
    {
        if (m_buffer != nullptr) {
            m_buffer->Set_Acceleration(acceleration / 1000000.0f);
        }
    }

    void Reset_Colors(ParticlePropertyStruct<Vector3> &new_props)
    {
        if (m_buffer) {
            m_buffer->Reset_Colors(new_props);
        }
    }

    void Reset_Opacity(ParticlePropertyStruct<float> &new_props)
    {
        if (m_buffer) {
            m_buffer->Reset_Opacity(new_props);
        }
    }

    void Reset_Size(ParticlePropertyStruct<float> &new_props)
    {
        if (m_buffer) {
            m_buffer->Reset_Size(new_props);
        }
    }

    void Reset_Rotations(ParticlePropertyStruct<float> &new_props, float orient_rnd)
    {
        if (m_buffer) {
            m_buffer->Reset_Rotations(new_props, orient_rnd);
        }
    }

    void Reset_Frames(ParticlePropertyStruct<float> &new_props)
    {
        if (m_buffer) {
            m_buffer->Reset_Frames(new_props);
        }
    }

    void Reset_Blur_Times(ParticlePropertyStruct<float> &new_props)
    {
        if (m_buffer) {
            m_buffer->Reset_Blur_Times(new_props);
        }
    }

    void Set_Emission_Rate(float rate) { m_emitRate = rate > 0.0f ? (unsigned int)(1000.0f / rate) : 1000U; }
    void Set_Burst_Size(int size) { m_burstSize = size != 0 ? size : 1; }

    void Set_One_Time_Burst(int size)
    {
        m_oneTimeBurstSize = size != 0 ? size : 1;
        m_oneTimeBurst = true;
    }

    virtual void Emit();
    ParticleBufferClass *Peek_Buffer() { return m_buffer; }
    void Buffer_Scene_Not_Needed() { m_bufferSceneNeeded = false; }

    void Remove_Buffer_From_Scene()
    {
        m_buffer->Remove();
        m_firstTime = true;
        m_bufferSceneNeeded = true;
    }

    virtual bool Is_Complete() { return m_isComplete; }
    bool Is_Remove_On_Complete_Enabled() { return m_removeOnComplete; }
    void Enable_Remove_On_Complete(bool onoff) { m_removeOnComplete = onoff; }
    static bool Default_Remove_On_Complete() { return s_defaultRemoveOnComplete; }
    static void Set_Default_Remove_On_Complete(bool onoff) { s_defaultRemoveOnComplete = onoff; }
    virtual int Get_User_Type() const { return EMITTER_TYPEID_DEFAULT; }
    virtual const char *Get_User_String() const { return nullptr; }
    int Get_Render_Mode() const { return m_buffer->Get_Render_Mode(); }
    int Get_Frame_Mode() const { return m_buffer->Get_Frame_Mode(); }
    float Get_Particle_Size() const { return m_buffer->Get_Particle_Size(); }
    Vector3 Get_Acceleration() const { return m_buffer->Get_Acceleration(); }
    float Get_Lifetime() const { return m_buffer->Get_Lifetime(); }
    float Get_Future_Start_Time() const { return m_buffer->Get_Future_Start_Time(); }
    Vector3 Get_End_Color() const { return m_buffer->Get_End_Color(); }
    float Get_End_Opacity() const { return m_buffer->Get_End_Opacity(); }
    TextureClass *Get_Texture() const { return m_buffer->Get_Texture(); }
    void Set_Texture(TextureClass *tex) { m_buffer->Set_Texture(tex); }
    float Get_Fade_Time() const { return m_buffer->Get_Fade_Time(); }
    Vector3 Get_Start_Color() const { return m_buffer->Get_Start_Color(); }
    float Get_Start_Opacity() const { return m_buffer->Get_Start_Opacity(); }
    float Get_Position_Random() const { return m_posRand ? m_posRand->Get_Maximum_Extent() : 0.0f; }
    float Get_Emission_Rate() const { return 1000.0f / float(m_emitRate); }
    int Get_Burst_Size() const { return m_burstSize; }
    int Get_Max_Particles() const { return m_maxParticles; }
    Vector3 Get_Start_Velocity() const { return m_baseVel * 1000.0F; }
    Vector3Randomizer *Get_Creation_Volume() const;
    Vector3Randomizer *Get_Velocity_Random() const;
    float Get_Outwards_Vel() const { return m_outwardVel * 1000.0F; }
    float Get_Velocity_Inherit() const { return m_velInheritFactor; }
    ShaderClass Get_Shader() const { return m_buffer->Get_Shader(); }
    void Get_Color_Key_Frames(ParticlePropertyStruct<Vector3> &colors) const { m_buffer->Get_Color_Key_Frames(colors); }

    void Get_Opacity_Key_Frames(ParticlePropertyStruct<float> &opacities) const
    {
        m_buffer->Get_Opacity_Key_Frames(opacities);
    }

    void Get_Size_Key_Frames(ParticlePropertyStruct<float> &sizes) const { m_buffer->Get_Size_Key_Frames(sizes); }

    void Get_Rotation_Key_Frames(ParticlePropertyStruct<float> &rotations) const
    {
        m_buffer->Get_Rotation_Key_Frames(rotations);
    }

    void Get_Frame_Key_Frames(ParticlePropertyStruct<float> &frames) const { m_buffer->Get_Frame_Key_Frames(frames); }

    void Get_Blur_Time_Key_Frames(ParticlePropertyStruct<float> &blurtimes) const
    {
        m_buffer->Get_Blur_Time_Key_Frames(blurtimes);
    }

    float Get_Initial_Orientation_Random() const { return m_buffer->Get_Initial_Orientation_Random(); }
    int Get_Line_Texture_Mapping_Mode() const { return m_buffer->Get_Line_Texture_Mapping_Mode(); }
    int Is_Merge_Intersections() const { return m_buffer->Is_Merge_Intersections(); }
    int Is_Freeze_Random() const { return m_buffer->Is_Freeze_Random(); }
    int Is_Sorting_Disabled() const { return m_buffer->Is_Sorting_Disabled(); }
    int Are_End_Caps_Enabled() const { return m_buffer->Are_End_Caps_Enabled(); }
    int Get_Subdivision_Level() const { return m_buffer->Get_Subdivision_Level(); }
    float Get_Noise_Amplitude() const { return m_buffer->Get_Noise_Amplitude(); }
    float Get_Merge_Abort_Factor() const { return m_buffer->Get_Merge_Abort_Factor(); }
    float Get_Texture_Tile_Factor() const { return m_buffer->Get_Texture_Tile_Factor(); }
    Vector2 Get_UV_Offset_Rate() const { return m_buffer->Get_UV_Offset_Rate(); }

protected:
    virtual void Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list, bool textures_only = false);
    virtual void Update_On_Visibilty();

private:
    virtual void Update_Cached_Bounding_Volumes() const;
    void Create_New_Particles(const Quaternion &curr_quat, const Vector3 &curr_orig);
    void Initialize_Particle(NewParticleStruct *newpart, unsigned int age, const Quaternion &quat, const Vector3 &orig);

    unsigned int m_emitRate;
    unsigned int m_burstSize;
    unsigned int m_oneTimeBurstSize;
    bool m_oneTimeBurst;
    Vector3Randomizer *m_posRand;
    Vector3 m_baseVel;
    Vector3Randomizer *m_velRand;
    float m_outwardVel;
    float m_velInheritFactor;
    unsigned int m_emitRemain;
    Quaternion m_prevQ;
    Vector3 m_prevOrig;
    bool m_active;
    bool m_firstTime;
    bool m_bufferSceneNeeded;
    int m_particlesLeft;
    int m_maxParticles;
    bool m_isComplete;
    char *m_nameString;
    char *m_userString;
    bool m_removeOnComplete;
    bool m_isInScene;
    unsigned char m_groupID;
    ParticleBufferClass *m_buffer;
    bool m_unk;
    static bool s_defaultRemoveOnComplete;
    static bool s_debugDisable;
};
