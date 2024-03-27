/**
/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Particle Buffer
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
#include "linegrp.h"
#include "pointgr.h"
#include "rendobj.h"
#include "seglinerenderer.h"
#include <algorithm>

class ParticleEmitterClass;
template<class T> struct ParticlePropertyStruct;
struct NewParticleStruct
{
    Vector3 m_position;
    Vector3 m_velocity;
    unsigned int m_timeStamp;
    unsigned char m_groupID;
    bool operator!=(const NewParticleStruct &p) { return (p.m_timeStamp != m_timeStamp) || (p.m_position != m_position); }
    bool operator==(const NewParticleStruct &p) { return (p.m_timeStamp == m_timeStamp) && (p.m_position == m_position); }
};
class ParticleBufferClass : public RenderObjClass
{
public:
    ParticleBufferClass(ParticleEmitterClass *emitter,
        unsigned int buffer_size,
        ParticlePropertyStruct<Vector3> &color,
        ParticlePropertyStruct<float> &opacity,
        ParticlePropertyStruct<float> &size,
        ParticlePropertyStruct<float> &rotation,
        float orient_rnd,
        ParticlePropertyStruct<float> &frame,
        ParticlePropertyStruct<float> &blurtime,
        Vector3 accel,
        float max_age,
        float future_start_time,
        TextureClass *tex,
        ShaderClass shader,
        bool pingpong,
        int render_mode,
        int frame_mode,
        const W3dEmitterLinePropertiesStruct *line_props);

    ParticleBufferClass(const ParticleBufferClass &src);
    ParticleBufferClass &operator=(const ParticleBufferClass &);
    virtual ~ParticleBufferClass();
    virtual RenderObjClass *Clone() const;
    virtual int Class_ID() const { return CLASSID_PARTICLEBUFFER; }

    virtual int Get_Num_Polys() const;
    int Get_Particle_Count() const;
    virtual void Render(RenderInfoClass &rinfo);
    virtual void Scale(float scale);
    virtual void On_Frame_Update();

    virtual void Notify_Added(SceneClass *scene);
    virtual void Notify_Removed(SceneClass *scene);

    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const;

    virtual void Prepare_LOD(CameraClass &camera);
    virtual void Increment_LOD();
    virtual void Decrement_LOD();
    virtual float Get_Cost() const;
    virtual float Get_Value() const;
    virtual float Get_Post_Increment_Value() const;
    virtual void Set_LOD_Level(int lod);
    virtual int Get_LOD_Level() const;
    virtual int Get_LOD_Count() const;
    virtual void Set_LOD_Bias(float bias) { m_lodBias = std::max(bias, 0.0f); }
    virtual int Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const;
    void Reset_Colors(ParticlePropertyStruct<Vector3> &new_props);
    void Reset_Opacity(ParticlePropertyStruct<float> &new_props);
    void Reset_Size(ParticlePropertyStruct<float> &new_props);
    void Reset_Rotations(ParticlePropertyStruct<float> &new_rotations, float orient_rnd);
    void Reset_Frames(ParticlePropertyStruct<float> &new_frames);
    void Reset_Blur_Times(ParticlePropertyStruct<float> &new_blur_times);
    void Emitter_Is_Dead();
    void Set_Emitter(ParticleEmitterClass *emitter);
    virtual bool Is_Complete() { return m_isEmitterDead && !m_nonNewNum && !m_newNum; }
    NewParticleStruct *Add_Uninitialized_New_Particle();

    void Set_Acceleration(const Vector3 &acceleration)
    {
        m_accel = acceleration;
        m_hasAccel = ((m_accel.X != 0) || (m_accel.Y != 0) || (m_accel.Z != 0));
    }

    void Set_Current_Group_ID(unsigned char id) { m_currentGroupID = id; }

    int Get_Render_Mode() const { return m_renderMode; }
    int Get_Frame_Mode() const { return m_frameMode; }
    float Get_Particle_Size() const { return m_sizeKeyFrameValues[0]; }
    Vector3 Get_Acceleration() const { return m_accel * 1000000.0F; }
    float Get_Lifetime() const { return (float(m_maxAge)) / 1000.0F; }
    float Get_Future_Start_Time() const { return (float(m_futureStartTime)) / 1000.0F; }
    Vector3 Get_Start_Color() const { return m_colorKeyFrameValues[0]; }
    float Get_Start_Opacity() const { return m_alphaKeyFrameValues[0]; }

    Vector3 Get_End_Color() const
    {
        return (m_numColorKeyFrames > 1) ? m_colorKeyFrameValues[m_numColorKeyFrames - 1] : m_colorKeyFrameValues[0];
    }

    float Get_End_Opacity() const
    {
        return (m_numAlphaKeyFrames > 1) ? m_alphaKeyFrameValues[m_numAlphaKeyFrames - 1] : m_alphaKeyFrameValues[0];
    }

    TextureClass *Get_Texture() const;
    void Set_Texture(TextureClass *tex);
    float Get_Fade_Time() const { return (m_numColorKeyFrames > 1) ? (((float)m_colorKeyFrameTimes[1]) / 1000.0f) : 0.0f; }
    ShaderClass Get_Shader() const;
    int Get_Line_Texture_Mapping_Mode() const;
    int Is_Merge_Intersections() const;
    int Is_Freeze_Random() const;
    int Is_Sorting_Disabled() const;
    int Are_End_Caps_Enabled() const;
    int Get_Subdivision_Level() const;
    float Get_Noise_Amplitude() const;
    float Get_Merge_Abort_Factor() const;
    float Get_Texture_Tile_Factor() const;
    Vector2 Get_UV_Offset_Rate() const;
    unsigned int Get_Buffer_Size() const { return m_maxNum; }
    void Get_Color_Key_Frames(ParticlePropertyStruct<Vector3> &colors) const;
    void Get_Opacity_Key_Frames(ParticlePropertyStruct<float> &opacities) const;
    void Get_Size_Key_Frames(ParticlePropertyStruct<float> &sizes) const;
    void Get_Rotation_Key_Frames(ParticlePropertyStruct<float> &rotations) const;
    void Get_Frame_Key_Frames(ParticlePropertyStruct<float> &frames) const;
    void Get_Blur_Time_Key_Frames(ParticlePropertyStruct<float> &blurtimes) const;
    float Get_Initial_Orientation_Random() const { return m_initialOrientationRandom; }
    static unsigned int Get_Total_Active_Count() { return s_totalActiveCount; }
    static void Set_LOD_Max_Screen_Size(int lod_level, float max_screen_size);
    static float Get_LOD_Max_Screen_Size(int lod_level);

protected:
    virtual void Update_Cached_Bounding_Volumes() const;
    void Render_Particles(RenderInfoClass &rinfo);
    void Render_Line(RenderInfoClass &rinfo);
    void Render_Line_Group(RenderInfoClass &rinfo);
    void Update_Kinematic_Particle_State();
    void Update_Visual_Particle_State();
    void Update_Bounding_Box();
    void Generate_APT(ShareBufferClass<unsigned int> **apt, unsigned int &active_point_count);
    void Combine_Color_And_Alpha();
    void Get_New_Particles();
    void Kill_Old_Particles();
    void Update_Non_New_Particles(unsigned int elapsed);

    NewParticleStruct *m_newParticleQueue;
    unsigned int m_newParticleQueueStart;
    unsigned int m_newParticleQueueEnd;
    int m_newParticleQueueCount;
    int m_renderMode;
    int m_frameMode;
    Vector3 m_accel;
    bool m_hasAccel;
    unsigned int m_maxAge;
    unsigned int m_futureStartTime;
    unsigned int m_lastUpdateTime;
    bool m_isEmitterDead;
    float m_maxSize;
    unsigned int m_maxNum;
    unsigned int m_start;
    unsigned int m_end;
    unsigned int m_newEnd;
    int m_nonNewNum;
    int m_newNum;
    AABoxClass m_boundingBox;
    bool m_boundingBoxDirty;
    unsigned int m_numColorKeyFrames;
    unsigned int *m_colorKeyFrameTimes;
    Vector3 *m_colorKeyFrameValues;
    Vector3 *m_colorKeyFrameDeltas;
    unsigned int m_numAlphaKeyFrames;
    unsigned int *m_alphaKeyFrameTimes;
    float *m_alphaKeyFrameValues;
    float *m_alphaKeyFrameDeltas;
    unsigned int m_numSizeKeyFrames;
    unsigned int *m_sizeKeyFrameTimes;
    float *m_sizeKeyFrameValues;
    float *m_sizeKeyFrameDeltas;
    unsigned int m_numRotationKeyFrames;
    unsigned int *m_rotationKeyFrameTimes;
    float *m_rotationKeyFrameValues;
    float *m_halfRotationKeyFrameDeltas;
    float *m_orientationKeyFrameValues;
    unsigned int m_numFrameKeyFrames;
    unsigned int *m_frameKeyFrameTimes;
    float *m_frameKeyFrameValues;
    float *m_frameKeyFrameDeltas;
    unsigned int m_numBlurTimeKeyFrames;
    unsigned int *m_blurTimeKeyFrameTimes;
    float *m_blurTimeKeyFrameValues;
    float *m_blurTimeKeyFrameDeltas;

    Vector4 m_defaultTailDiffuse;
    unsigned int m_numRandomColorEntriesMinus1;
    Vector3 *m_randomColorEntries;
    unsigned int m_numRandomAlphaEntriesMinus1;
    float *m_randomAlphaEntries;
    unsigned int m_numRandomSizeEntriesMinus1;
    float *m_randomSizeEntries;
    unsigned int m_numRandomRotationEntriesMinus1;
    float *m_randomRotationEntries;
    unsigned int m_numRandomOrientationEntriesMinus1;
    float *m_randomOrientationEntries;
    unsigned int m_numRandomFrameEntriesMinus1;
    float *m_randomFrameEntries;
    unsigned int m_numRandomBlurTimeEntriesMinus1;
    float *m_randomBlurTimeEntries;

    Vector3 m_colorRandom;
    float m_opacityRandom;
    float m_sizeRandom;
    float m_rotationRandom;
    float m_frameRandom;
    float m_blurTimeRandom;
    float m_initialOrientationRandom;
    PointGroupClass *m_pointGroup;
    SegLineRendererClass *m_lineRenderer;
    LineGroupClass *m_lineGroup;
    ShareBufferClass<Vector3> *m_position[2];
    ShareBufferClass<Vector4> *m_diffuse;
    ShareBufferClass<Vector3> *m_color;
    ShareBufferClass<float> *m_alpha;
    ShareBufferClass<float> *m_size;
    ShareBufferClass<uint8_t> *m_frame;
    ShareBufferClass<float> *m_uCoord;
    ShareBufferClass<Vector3> *m_tailPosition;
    ShareBufferClass<Vector4> *m_tailDiffuse;
    ShareBufferClass<uint8_t> *m_orientation;
    ShareBufferClass<unsigned int> *m_APT;
    ShareBufferClass<uint8_t> *m_groupID;
    bool m_pingPongPosition;
    Vector3 *m_velocity;
    unsigned int *m_timeStamp;
    ParticleEmitterClass *m_emitter;
    unsigned int m_decimationThreshold;
    static const unsigned int s_permutationArray[16];
    unsigned int m_lodCount;
    float m_cost[17];
    float m_value[18];
    float m_lodBias;
    float m_projectedArea;
    uint8_t m_currentGroupID;

    static unsigned int s_totalActiveCount;
    static float s_LODMaxScreenSizes[17];

    enum TailDiffuseTypeEnum
    {
        BLACK,
        WHITE,
        SAME_AS_HEAD,
        SAME_AS_HEAD_ALPHA_ZERO
    };
    TailDiffuseTypeEnum Determine_Tail_Diffuse();
};
