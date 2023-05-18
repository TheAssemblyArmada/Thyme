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
#include "part_buf.h"
#include "camera.h"
#include "dx8wrapper.h"
#include "part_emt.h"
#include "pot.h"
#include "rinfo.h"
#include "scene.h"
#include "simplevec.h"
#include "sphere.h"
#include "texture.h"
#include "vector3.h"
#include "vp.h"
#include "w3d.h"
#include <limits.h>

const unsigned int ParticleBufferClass::s_permutationArray[16] = { 11, 3, 7, 14, 0, 13, 1, 2, 5, 12, 15, 6, 9, 8, 4, 10 };

const static unsigned int s_maxRandomEntries = 32;
unsigned int ParticleBufferClass::s_totalActiveCount = 0;

float ParticleBufferClass::s_LODMaxScreenSizes[17] = { NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE,
    NO_MAX_SCREEN_SIZE };

static Random4Class s_randGen;
const float s_ooIntMax = 1.0f / (float)INT_MAX;

static const W3dEmitterLinePropertiesStruct s_defaultLineEmitterProps = {
    0, 0, 0.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

ParticleBufferClass::ParticleBufferClass(ParticleEmitterClass *emitter,
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
    const W3dEmitterLinePropertiesStruct *line_props) :
    m_newParticleQueue(nullptr),
    m_newParticleQueueStart(0),
    m_newParticleQueueEnd(0),
    m_newParticleQueueCount(0),
    m_renderMode(render_mode),
    m_frameMode(frame_mode),
    m_maxAge(1000.0f * max_age),
    m_futureStartTime(1000.0f * future_start_time),
    m_lastUpdateTime(W3D::Get_Sync_Time()),
    m_isEmitterDead(false),
    m_maxSize(0.0f),
    m_maxNum(buffer_size),
    m_start(0),
    m_end(0),
    m_newEnd(0),
    m_nonNewNum(0),
    m_newNum(0),
    m_boundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)),
    m_boundingBoxDirty(true),
    m_numColorKeyFrames(0),
    m_colorKeyFrameTimes(nullptr),
    m_colorKeyFrameValues(nullptr),
    m_colorKeyFrameDeltas(nullptr),
    m_numAlphaKeyFrames(0),
    m_alphaKeyFrameTimes(nullptr),
    m_alphaKeyFrameValues(nullptr),
    m_alphaKeyFrameDeltas(nullptr),
    m_numSizeKeyFrames(0),
    m_sizeKeyFrameTimes(nullptr),
    m_sizeKeyFrameValues(nullptr),
    m_sizeKeyFrameDeltas(nullptr),
    m_numRotationKeyFrames(0),
    m_rotationKeyFrameTimes(nullptr),
    m_rotationKeyFrameValues(nullptr),
    m_halfRotationKeyFrameDeltas(nullptr),
    m_orientationKeyFrameValues(nullptr),
    m_numFrameKeyFrames(0),
    m_frameKeyFrameTimes(nullptr),
    m_frameKeyFrameValues(nullptr),
    m_frameKeyFrameDeltas(nullptr),
    m_numBlurTimeKeyFrames(0),
    m_blurTimeKeyFrameTimes(nullptr),
    m_blurTimeKeyFrameValues(nullptr),
    m_blurTimeKeyFrameDeltas(nullptr),
    m_defaultTailDiffuse(0.0f, 0.0f, 0.0f, 0.0f),
    m_numRandomColorEntriesMinus1(0),
    m_randomColorEntries(nullptr),
    m_numRandomAlphaEntriesMinus1(0),
    m_randomAlphaEntries(nullptr),
    m_numRandomSizeEntriesMinus1(0),
    m_randomSizeEntries(nullptr),
    m_numRandomRotationEntriesMinus1(0),
    m_randomRotationEntries(nullptr),
    m_numRandomOrientationEntriesMinus1(0),
    m_randomOrientationEntries(nullptr),
    m_numRandomFrameEntriesMinus1(0),
    m_randomFrameEntries(nullptr),
    m_numRandomBlurTimeEntriesMinus1(0),
    m_randomBlurTimeEntries(nullptr),
    m_colorRandom(0.0f, 0.0f, 0.0f),
    m_opacityRandom(0.0f),
    m_sizeRandom(0.0f),
    m_rotationRandom(0.0f),
    m_frameRandom(0.0f),
    m_initialOrientationRandom(0),
    m_pointGroup(nullptr),
    m_lineRenderer(nullptr),
    m_lineGroup(nullptr),
    m_diffuse(nullptr),
    m_color(nullptr),
    m_alpha(nullptr),
    m_size(nullptr),
    m_frame(nullptr),
    m_uCoord(nullptr),
    m_tailPosition(nullptr),
    m_tailDiffuse(nullptr),
    m_orientation(nullptr),
    m_APT(nullptr),
    m_groupID(nullptr),
    m_pingPongPosition(pingpong),
    m_velocity(nullptr),
    m_timeStamp(nullptr),
    m_emitter(emitter),
    m_decimationThreshold(0),
    m_projectedArea(0.0f),
    m_currentGroupID(0)
{
    m_lodCount = 17;
    m_lodBias = 1.0f;

    m_position[0] = nullptr;
    m_position[1] = nullptr;
    Reset_Colors(color);
    Reset_Opacity(opacity);
    Reset_Size(size);
    Reset_Rotations(rotation, orient_rnd);
    Reset_Frames(frame);
    Reset_Blur_Times(blurtime);
    m_newParticleQueue = new NewParticleStruct[m_maxNum];
    m_accel = accel;
    m_hasAccel = (accel.X != 0.0f) || (accel.Y != 0.0f) || (accel.Z != 0.0f);

    shader.Enable_Fog("ParticleBufferClass");
    switch (m_renderMode) {
        case W3D_EMITTER_RENDER_MODE_TRI_PARTICLES: {
            m_pointGroup = new PointGroupClass();
            m_pointGroup->Set_Flag(PointGroupClass::TRANSFORM, true);
            m_pointGroup->Set_Texture(tex);
            m_pointGroup->Set_Shader(shader);
            m_pointGroup->Set_Frame_Row_Column_Count_Log2(frame_mode);
            m_pointGroup->Set_Point_Mode(PointGroupClass::TRIS);
        } break;
        case W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES: {
            m_pointGroup = new PointGroupClass();
            m_pointGroup->Set_Flag(PointGroupClass::TRANSFORM, true);
            m_pointGroup->Set_Texture(tex);
            m_pointGroup->Set_Shader(shader);
            m_pointGroup->Set_Frame_Row_Column_Count_Log2(frame_mode);
            m_pointGroup->Set_Point_Mode(PointGroupClass::QUADS);
        } break;
        case W3D_EMITTER_RENDER_MODE_LINE: {
            m_lineRenderer = new SegLineRendererClass;
            m_lineRenderer->Init(*line_props);
            m_lineRenderer->Set_Texture(tex);
            m_lineRenderer->Set_Shader(shader);
            m_lineRenderer->Set_Width(Get_Particle_Size());

            if (line_props != nullptr) {
                m_lineRenderer->Init(*line_props);
            } else {
                captainslog_assert(0);
                m_lineRenderer->Init(s_defaultLineEmitterProps);
            }
        } break;
        case W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA: {
            m_lineGroup = new LineGroupClass();
            m_lineGroup->Set_Flag(LineGroupClass::TRANSFORM, true);
            m_lineGroup->Set_Texture(tex);
            m_lineGroup->Set_Shader(shader);
            m_lineGroup->Set_Line_Mode(LineGroupClass::TETRAHEDRON);
            m_tailPosition = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::TailPosition");
            Set_Force_Visible(1);
        } break;
        case W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM: {
            m_lineGroup = new LineGroupClass();
            m_lineGroup->Set_Flag(LineGroupClass::TRANSFORM, true);
            m_lineGroup->Set_Texture(tex);
            m_lineGroup->Set_Shader(shader);
            m_lineGroup->Set_Line_Mode(LineGroupClass::PRISM);
            m_tailPosition = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::TailPosition");
            Set_Force_Visible(1);
        } break;
        default:
            captainslog_assert(0);
            break;
    }

    m_position[0] = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::Position");

    if (m_pingPongPosition) {
        m_position[1] = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::Position");
    }

    m_APT = New_Share_Buffer<unsigned int>(m_maxNum, "ParticleBufferClass::APT");
    m_groupID = New_Share_Buffer<uint8_t>(m_maxNum, "ParticleBufferClass::GroupID");
    m_velocity = new Vector3[m_maxNum];
    m_timeStamp = new unsigned int[m_maxNum];
    int minlod = Calculate_Cost_Value_Arrays(1.0f, m_value, m_cost);

    if (Get_LOD_Level() < minlod) {
        Set_LOD_Level(minlod);
    }

    s_totalActiveCount++;

    if (m_renderMode == W3D_EMITTER_RENDER_MODE_LINE) {
        if (line_props != nullptr) {
            m_lineRenderer = new SegLineRendererClass;
            m_lineRenderer->Init(*line_props);
            m_lineRenderer->Set_Texture(tex);
            m_lineRenderer->Set_Shader(shader);
            m_lineRenderer->Set_Width(Get_Particle_Size());
        } else {
            m_renderMode = W3D_EMITTER_RENDER_MODE_TRI_PARTICLES;
        }
    }
}

ParticleBufferClass::ParticleBufferClass(const ParticleBufferClass &src) :
    RenderObjClass(src),
    m_newParticleQueue(nullptr),
    m_newParticleQueueStart(0),
    m_newParticleQueueEnd(0),
    m_newParticleQueueCount(0),
    m_renderMode(src.m_renderMode),
    m_frameMode(src.m_frameMode),
    m_maxAge(src.m_maxAge),
    m_futureStartTime(src.m_futureStartTime),
    m_lastUpdateTime(W3D::Get_Sync_Time()),
    m_isEmitterDead(false),
    m_maxSize(src.m_maxSize),
    m_maxNum(src.m_maxNum),
    m_start(0),
    m_end(0),
    m_newEnd(0),
    m_nonNewNum(0),
    m_newNum(0),
    m_boundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)),
    m_boundingBoxDirty(true),
    m_numColorKeyFrames(src.m_numColorKeyFrames),
    m_colorKeyFrameTimes(nullptr),
    m_colorKeyFrameValues(nullptr),
    m_colorKeyFrameDeltas(nullptr),
    m_numAlphaKeyFrames(src.m_numAlphaKeyFrames),
    m_alphaKeyFrameTimes(nullptr),
    m_alphaKeyFrameValues(nullptr),
    m_alphaKeyFrameDeltas(nullptr),
    m_numSizeKeyFrames(src.m_numSizeKeyFrames),
    m_sizeKeyFrameTimes(nullptr),
    m_sizeKeyFrameValues(nullptr),
    m_sizeKeyFrameDeltas(nullptr),
    m_numRotationKeyFrames(src.m_numRotationKeyFrames),
    m_rotationKeyFrameTimes(nullptr),
    m_rotationKeyFrameValues(nullptr),
    m_halfRotationKeyFrameDeltas(nullptr),
    m_orientationKeyFrameValues(nullptr),
    m_numFrameKeyFrames(src.m_numFrameKeyFrames),
    m_frameKeyFrameTimes(nullptr),
    m_frameKeyFrameValues(nullptr),
    m_frameKeyFrameDeltas(nullptr),
    m_numBlurTimeKeyFrames(src.m_numBlurTimeKeyFrames),
    m_blurTimeKeyFrameTimes(nullptr),
    m_blurTimeKeyFrameValues(nullptr),
    m_blurTimeKeyFrameDeltas(nullptr),
    m_defaultTailDiffuse(src.m_defaultTailDiffuse),
    m_randomColorEntries(nullptr),
    m_randomAlphaEntries(nullptr),
    m_randomSizeEntries(nullptr),
    m_numRandomRotationEntriesMinus1(0),
    m_randomRotationEntries(nullptr),
    m_numRandomOrientationEntriesMinus1(0),
    m_randomOrientationEntries(nullptr),
    m_numRandomFrameEntriesMinus1(0),
    m_randomFrameEntries(nullptr),
    m_numRandomBlurTimeEntriesMinus1(0),
    m_randomBlurTimeEntries(nullptr),
    m_colorRandom(src.m_colorRandom),
    m_opacityRandom(src.m_opacityRandom),
    m_sizeRandom(src.m_sizeRandom),
    m_rotationRandom(src.m_rotationRandom),
    m_frameRandom(src.m_frameRandom),
    m_initialOrientationRandom(src.m_initialOrientationRandom),
    m_pointGroup(nullptr),
    m_lineRenderer(nullptr),
    m_lineGroup(nullptr),
    m_diffuse(nullptr),
    m_color(nullptr),
    m_alpha(nullptr),
    m_size(nullptr),
    m_frame(nullptr),
    m_uCoord(nullptr),
    m_tailPosition(nullptr),
    m_tailDiffuse(nullptr),
    m_orientation(nullptr),
    m_APT(nullptr),
    m_groupID(nullptr),
    m_pingPongPosition(src.m_pingPongPosition),
    m_velocity(nullptr),
    m_timeStamp(nullptr),
    m_emitter(src.m_emitter),
    m_decimationThreshold(src.m_decimationThreshold),
    m_projectedArea(0.0f)
{
    m_position[0] = nullptr;
    m_position[1] = nullptr;

    m_lodCount = std::min(m_maxNum, (unsigned int)17);
    m_lodBias = src.m_lodBias;

    m_numRandomColorEntriesMinus1 = src.m_numRandomColorEntriesMinus1;

    if (src.m_color != nullptr) {
        m_color = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::Color");
        m_colorKeyFrameTimes = new unsigned int[m_numColorKeyFrames];
        m_colorKeyFrameValues = new Vector3[m_numColorKeyFrames];
        m_colorKeyFrameDeltas = new Vector3[m_numColorKeyFrames];

        for (unsigned int i = 0; i < m_numColorKeyFrames; i++) {
            m_colorKeyFrameTimes[i] = src.m_colorKeyFrameTimes[i];
            m_colorKeyFrameValues[i] = src.m_colorKeyFrameValues[i];
            m_colorKeyFrameDeltas[i] = src.m_colorKeyFrameDeltas[i];
        }

        if (src.m_randomColorEntries != nullptr) {
            m_randomColorEntries = new Vector3[m_numRandomColorEntriesMinus1 + 1];
            for (unsigned int j = 0; j <= m_numRandomColorEntriesMinus1; j++) {
                m_randomColorEntries[j] = src.m_randomColorEntries[j];
            }
        }
    } else {
        m_colorKeyFrameValues = new Vector3[1];
        m_colorKeyFrameValues[0] = src.m_colorKeyFrameValues[0];
    }

    m_numRandomAlphaEntriesMinus1 = src.m_numRandomAlphaEntriesMinus1;

    if (src.m_alpha != nullptr) {
        m_alpha = New_Share_Buffer<float>(m_maxNum, "ParticleBufferClass::Alpha");
        m_alphaKeyFrameTimes = new unsigned int[m_numAlphaKeyFrames];
        m_alphaKeyFrameValues = new float[m_numAlphaKeyFrames];
        m_alphaKeyFrameDeltas = new float[m_numAlphaKeyFrames];

        for (unsigned int i = 0; i < m_numAlphaKeyFrames; i++) {
            m_alphaKeyFrameTimes[i] = src.m_alphaKeyFrameTimes[i];
            m_alphaKeyFrameValues[i] = src.m_alphaKeyFrameValues[i];
            m_alphaKeyFrameDeltas[i] = src.m_alphaKeyFrameDeltas[i];
        }

        if (src.m_randomAlphaEntries != nullptr) {
            m_randomAlphaEntries = new float[m_numRandomAlphaEntriesMinus1 + 1];
            for (unsigned int j = 0; j <= m_numRandomAlphaEntriesMinus1; j++) {
                m_randomAlphaEntries[j] = src.m_randomAlphaEntries[j];
            }
        }
    } else {
        m_alphaKeyFrameValues = new float[1];
        m_alphaKeyFrameValues[0] = src.m_alphaKeyFrameValues[0];
    }

    m_numRandomSizeEntriesMinus1 = src.m_numRandomSizeEntriesMinus1;

    if (src.m_size != nullptr) {
        m_size = New_Share_Buffer<float>(m_maxNum, "ParticleBufferClass::Size");
        m_sizeKeyFrameTimes = new unsigned int[m_numSizeKeyFrames];
        m_sizeKeyFrameValues = new float[m_numSizeKeyFrames];
        m_sizeKeyFrameDeltas = new float[m_numSizeKeyFrames];

        for (unsigned int i = 0; i < m_numSizeKeyFrames; i++) {
            m_sizeKeyFrameTimes[i] = src.m_sizeKeyFrameTimes[i];
            m_sizeKeyFrameValues[i] = src.m_sizeKeyFrameValues[i];
            m_sizeKeyFrameDeltas[i] = src.m_sizeKeyFrameDeltas[i];
        }

        if (src.m_randomSizeEntries != nullptr) {
            m_randomSizeEntries = new float[m_numRandomSizeEntriesMinus1 + 1];
            for (unsigned int j = 0; j <= m_numRandomSizeEntriesMinus1; j++) {
                m_randomSizeEntries[j] = src.m_randomSizeEntries[j];
            }
        }
    } else {
        m_sizeKeyFrameValues = new float[1];
        m_sizeKeyFrameValues[0] = src.m_sizeKeyFrameValues[0];
    }

    m_numRandomRotationEntriesMinus1 = src.m_numRandomRotationEntriesMinus1;
    m_numRandomOrientationEntriesMinus1 = src.m_numRandomOrientationEntriesMinus1;

    if (src.m_orientation != nullptr) {
        m_orientation = New_Share_Buffer<uint8_t>(m_maxNum, "ParticleBufferClass::Orientation");
        m_rotationKeyFrameTimes = new unsigned int[m_numRotationKeyFrames];
        m_rotationKeyFrameValues = new float[m_numRotationKeyFrames];
        m_halfRotationKeyFrameDeltas = new float[m_numRotationKeyFrames];
        m_orientationKeyFrameValues = new float[m_numRotationKeyFrames];

        for (unsigned int i = 0; i < m_numRotationKeyFrames; i++) {
            m_rotationKeyFrameTimes[i] = src.m_rotationKeyFrameTimes[i];
            m_rotationKeyFrameValues[i] = src.m_rotationKeyFrameValues[i];
            m_halfRotationKeyFrameDeltas[i] = src.m_halfRotationKeyFrameDeltas[i];
            m_orientationKeyFrameValues[i] = src.m_orientationKeyFrameValues[i];
        }

        if (src.m_randomRotationEntries != nullptr) {
            m_randomRotationEntries = new float[m_numRandomRotationEntriesMinus1 + 1];
            for (unsigned int j = 0; j <= m_numRandomRotationEntriesMinus1; j++) {
                m_randomRotationEntries[j] = src.m_randomRotationEntries[j];
            }
        }

        if (src.m_randomOrientationEntries != nullptr) {
            m_randomOrientationEntries = new float[m_numRandomOrientationEntriesMinus1 + 1];
            for (unsigned int j = 0; j <= m_numRandomOrientationEntriesMinus1; j++) {
                m_randomOrientationEntries[j] = src.m_randomOrientationEntries[j];
            }
        }
    }

    m_numRandomFrameEntriesMinus1 = src.m_numRandomFrameEntriesMinus1;

    if (src.m_frame != nullptr || src.m_uCoord != nullptr) {
        if (src.m_frame != nullptr) {
            m_frame = New_Share_Buffer<uint8_t>(m_maxNum, "ParticleBufferClass::Frame");
        } else {
            m_uCoord = New_Share_Buffer<float>(m_maxNum, "ParticleBufferClass::UCoord");
        }

        m_frameKeyFrameTimes = new unsigned int[m_numFrameKeyFrames];
        m_frameKeyFrameValues = new float[m_numFrameKeyFrames];
        m_frameKeyFrameDeltas = new float[m_numFrameKeyFrames];

        for (unsigned int i = 0; i < m_numFrameKeyFrames; i++) {
            m_frameKeyFrameTimes[i] = src.m_frameKeyFrameTimes[i];
            m_frameKeyFrameValues[i] = src.m_frameKeyFrameValues[i];
            m_frameKeyFrameDeltas[i] = src.m_frameKeyFrameDeltas[i];
        }

        if (src.m_randomFrameEntries != nullptr) {
            m_randomFrameEntries = new float[m_numRandomFrameEntriesMinus1 + 1];
            for (unsigned int j = 0; j <= m_numRandomFrameEntriesMinus1; j++) {
                m_randomFrameEntries[j] = src.m_randomFrameEntries[j];
            }
        }
    } else {
        m_frameKeyFrameValues = new float[1];
        m_frameKeyFrameValues[0] = src.m_frameKeyFrameValues[0];
    }

    m_numRandomBlurTimeEntriesMinus1 = src.m_numRandomBlurTimeEntriesMinus1;

    if (m_numBlurTimeKeyFrames > 0) {
        m_blurTimeKeyFrameTimes = new unsigned int[m_numBlurTimeKeyFrames];
        m_blurTimeKeyFrameValues = new float[m_numBlurTimeKeyFrames];
        m_blurTimeKeyFrameDeltas = new float[m_numBlurTimeKeyFrames];

        for (unsigned int i = 0; i < m_numBlurTimeKeyFrames; i++) {
            m_blurTimeKeyFrameTimes[i] = src.m_blurTimeKeyFrameTimes[i];
            m_blurTimeKeyFrameValues[i] = src.m_blurTimeKeyFrameValues[i];
            m_blurTimeKeyFrameDeltas[i] = src.m_blurTimeKeyFrameDeltas[i];
        }

        if (src.m_randomBlurTimeEntries != nullptr) {
            m_randomBlurTimeEntries = new float[m_numRandomBlurTimeEntriesMinus1 + 1];
            for (unsigned int j = 0; j <= m_numRandomBlurTimeEntriesMinus1; j++) {
                m_randomBlurTimeEntries[j] = src.m_randomBlurTimeEntries[j];
            }
        }
    } else {
        m_blurTimeKeyFrameValues = new float[1];
        m_blurTimeKeyFrameValues[0] = src.m_blurTimeKeyFrameValues[0];
    }

    m_newParticleQueue = new NewParticleStruct[m_maxNum];
    m_accel = src.m_accel;
    m_hasAccel = src.m_hasAccel;

    switch (m_renderMode) {
        case W3D_EMITTER_RENDER_MODE_TRI_PARTICLES: {
            captainslog_assert(src.m_pointGroup);
            m_pointGroup = new PointGroupClass();
            m_pointGroup->Set_Flag(PointGroupClass::TRANSFORM, true);
            m_pointGroup->Set_Texture(src.m_pointGroup->Peek_Texture());
            m_pointGroup->Set_Shader(src.m_pointGroup->Get_Shader());
            m_pointGroup->Set_Point_Mode(PointGroupClass::TRIS);
            m_pointGroup->Set_Frame_Row_Column_Count_Log2(src.m_pointGroup->Get_Frame_Row_Column_Count_Log2());
        } break;
        case W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES: {
            captainslog_assert(src.m_pointGroup);
            m_pointGroup = new PointGroupClass();
            m_pointGroup->Set_Flag(PointGroupClass::TRANSFORM, true);
            m_pointGroup->Set_Texture(src.m_pointGroup->Peek_Texture());
            m_pointGroup->Set_Shader(src.m_pointGroup->Get_Shader());
            m_pointGroup->Set_Point_Mode(PointGroupClass::QUADS);
            m_pointGroup->Set_Frame_Row_Column_Count_Log2(src.m_pointGroup->Get_Frame_Row_Column_Count_Log2());
        } break;
        case W3D_EMITTER_RENDER_MODE_LINE: {
            captainslog_assert(src.m_lineRenderer);
            m_lineRenderer = new SegLineRendererClass(*src.m_lineRenderer);
        } break;
        case W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA: {
            captainslog_assert(src.m_lineGroup);
            m_lineGroup = new LineGroupClass();
            m_lineGroup->Set_Flag(LineGroupClass::TRANSFORM, true);
            m_lineGroup->Set_Texture(src.m_lineGroup->Peek_Texture());
            m_lineGroup->Set_Shader(src.m_lineGroup->Get_Shader());
            m_lineGroup->Set_Line_Mode(LineGroupClass::TETRAHEDRON);
            m_tailPosition = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::TailPosition");
            Set_Force_Visible(1);
        } break;
        case W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM: {
            captainslog_assert(src.m_lineGroup);
            m_lineGroup = new LineGroupClass();
            m_lineGroup->Set_Flag(LineGroupClass::TRANSFORM, true);
            m_lineGroup->Set_Texture(src.m_lineGroup->Peek_Texture());
            m_lineGroup->Set_Shader(src.m_lineGroup->Get_Shader());
            m_lineGroup->Set_Line_Mode(LineGroupClass::PRISM);
            m_tailPosition = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::TailPosition");
            Set_Force_Visible(1);
        } break;
        default:
            captainslog_assert(0);
            break;
    }

    m_position[0] = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::Position");

    if (m_pingPongPosition) {
        m_position[1] = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::Position");
    }

    m_APT = New_Share_Buffer<unsigned int>(m_maxNum, "ParticleBufferClass::APT");
    m_groupID = New_Share_Buffer<uint8_t>(m_maxNum, "ParticleBufferClass::GroupID");
    m_velocity = new Vector3[m_maxNum];
    m_timeStamp = new unsigned int[m_maxNum];
    int minlod = Calculate_Cost_Value_Arrays(1.0f, m_value, m_cost);

    if (Get_LOD_Level() < minlod) {
        Set_LOD_Level(minlod);
    }

    s_totalActiveCount++;
}

ParticleBufferClass &ParticleBufferClass::operator=(const ParticleBufferClass &that)
{
    RenderObjClass::operator=(that);

    if (this != &that) {
        captainslog_assert(0);
    }

    return *this;
}

ParticleBufferClass::~ParticleBufferClass()
{
    if (m_newParticleQueue != nullptr) {
        delete[] m_newParticleQueue;
    }

    if (m_colorKeyFrameTimes != nullptr) {
        delete[] m_colorKeyFrameTimes;
    }

    if (m_colorKeyFrameValues != nullptr) {
        delete[] m_colorKeyFrameValues;
    }

    if (m_colorKeyFrameDeltas != nullptr) {
        delete[] m_colorKeyFrameDeltas;
    }

    if (m_alphaKeyFrameTimes != nullptr) {
        delete[] m_alphaKeyFrameTimes;
    }

    if (m_alphaKeyFrameValues != nullptr) {
        delete[] m_alphaKeyFrameValues;
    }

    if (m_alphaKeyFrameDeltas != nullptr) {
        delete[] m_alphaKeyFrameDeltas;
    }

    if (m_sizeKeyFrameTimes != nullptr) {
        delete[] m_sizeKeyFrameTimes;
    }

    if (m_sizeKeyFrameValues != nullptr) {
        delete[] m_sizeKeyFrameValues;
    }

    if (m_sizeKeyFrameDeltas != nullptr) {
        delete[] m_sizeKeyFrameDeltas;
    }

    if (m_rotationKeyFrameTimes != nullptr) {
        delete[] m_rotationKeyFrameTimes;
    }

    if (m_rotationKeyFrameValues != nullptr) {
        delete[] m_rotationKeyFrameValues;
    }

    if (m_halfRotationKeyFrameDeltas != nullptr) {
        delete[] m_halfRotationKeyFrameDeltas;
    }

    if (m_orientationKeyFrameValues != nullptr) {
        delete[] m_orientationKeyFrameValues;
    }

    if (m_frameKeyFrameTimes != nullptr) {
        delete[] m_frameKeyFrameTimes;
    }

    if (m_frameKeyFrameValues != nullptr) {
        delete[] m_frameKeyFrameValues;
    }

    if (m_frameKeyFrameDeltas != nullptr) {
        delete[] m_frameKeyFrameDeltas;
    }

    if (m_blurTimeKeyFrameTimes != nullptr) {
        delete[] m_blurTimeKeyFrameTimes;
    }

    if (m_blurTimeKeyFrameValues != nullptr) {
        delete[] m_blurTimeKeyFrameValues;
    }

    if (m_blurTimeKeyFrameDeltas != nullptr) {
        delete[] m_blurTimeKeyFrameDeltas;
    }

    if (m_randomColorEntries != nullptr) {
        delete[] m_randomColorEntries;
    }

    if (m_randomAlphaEntries != nullptr) {
        delete[] m_randomAlphaEntries;
    }

    if (m_randomSizeEntries != nullptr) {
        delete[] m_randomSizeEntries;
    }

    if (m_randomRotationEntries != nullptr) {
        delete[] m_randomRotationEntries;
    }

    if (m_randomOrientationEntries != nullptr) {
        delete[] m_randomOrientationEntries;
    }

    if (m_randomFrameEntries != nullptr) {
        delete[] m_randomFrameEntries;
    }

    if (m_randomBlurTimeEntries != nullptr) {
        delete[] m_randomBlurTimeEntries;
    }

    if (m_pointGroup) {
        delete m_pointGroup;
    }

    if (m_lineRenderer) {
        delete m_lineRenderer;
    }

    if (m_lineGroup) {
        delete m_lineGroup;
    }

    Ref_Ptr_Release(m_position[0]);
    Ref_Ptr_Release(m_position[1]);
    Ref_Ptr_Release(m_diffuse);
    Ref_Ptr_Release(m_tailDiffuse);
    Ref_Ptr_Release(m_color);
    Ref_Ptr_Release(m_alpha);
    Ref_Ptr_Release(m_size);
    Ref_Ptr_Release(m_orientation);
    Ref_Ptr_Release(m_frame);
    Ref_Ptr_Release(m_uCoord);
    Ref_Ptr_Release(m_tailPosition);
    Ref_Ptr_Release(m_APT);
    Ref_Ptr_Release(m_groupID);

    if (m_velocity) {
        delete[] m_velocity;
    }

    if (m_timeStamp) {
        delete[] m_timeStamp;
    }

    if (m_emitter) {
        captainslog_assert(0);
        m_emitter = nullptr;
    }

    s_totalActiveCount--;
}

RenderObjClass *ParticleBufferClass::Clone() const
{
    return new ParticleBufferClass(*this);
}

int ParticleBufferClass::Get_Num_Polys() const
{
    return (int)Get_Cost();
}

int ParticleBufferClass::Get_Particle_Count() const
{
    return m_nonNewNum + m_newNum;
}

void ParticleBufferClass::Render(RenderInfoClass &rinfo)
{
    unsigned int sort_level = SORT_LEVEL_NONE;

    if (!W3D::Is_Sorting_Enabled()) {
        sort_level = Get_Shader().Guess_Sort_Level();
    }

    if (W3D::Are_Static_Sort_Lists_Enabled() && sort_level != SORT_LEVEL_NONE) {
        W3D::Add_To_Static_Sort_List(this, sort_level);
    } else {
        Update_Kinematic_Particle_State();

        if (m_decimationThreshold < m_lodCount - 1) {
            Update_Visual_Particle_State();
        }

        switch (m_renderMode) {
            case W3D_EMITTER_RENDER_MODE_TRI_PARTICLES:
            case W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES:
                Render_Particles(rinfo);
                break;
            case W3D_EMITTER_RENDER_MODE_LINE:
                Render_Line(rinfo);
                break;
            case W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA:
            case W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM:
                Render_Line_Group(rinfo);
                break;
        }
    }
}

void ParticleBufferClass::Generate_APT(ShareBufferClass<unsigned int> **apt, unsigned int &active_point_count)
{
    if (m_nonNewNum < (int)m_maxNum || m_decimationThreshold > 0) {
        unsigned int sub1_start;
        unsigned int sub1_end;
        unsigned int sub2_start;
        unsigned int sub2_end;
        unsigned int i;

        if ((m_start < m_end) || ((m_start == m_end) && m_nonNewNum == 0)) {
            sub1_start = m_start;
            sub1_end = m_end;
            sub2_start = m_end;
            sub2_end = m_end;
        } else {
            sub1_start = 0;
            sub1_end = m_end;
            sub2_start = m_start;
            sub2_end = m_maxNum;
        }

        unsigned int *apt_ptr = m_APT->Get_Array();

        for (i = sub1_start; i < sub1_end; i++) {
            if (s_permutationArray[i & 0xF] >= m_decimationThreshold) {
                apt_ptr[active_point_count++] = i;
            }
        }

        for (i = sub2_start; i < sub2_end; i++) {
            if (s_permutationArray[i & 0xF] >= m_decimationThreshold) {
                apt_ptr[active_point_count++] = i;
            }
        }

        *apt = m_APT;
    } else {
        active_point_count = m_nonNewNum;
    }
}

void ParticleBufferClass::Combine_Color_And_Alpha()
{
    if (m_color != nullptr || m_alpha != nullptr) {
        unsigned cnt = m_maxNum;

        if (m_diffuse == nullptr) {
            m_diffuse = New_Share_Buffer<Vector4>(m_maxNum, "ParticleBufferClass::Diffuse");
        }

        if (m_color != nullptr && m_alpha != nullptr) {
            VectorProcessorClass::Copy(m_diffuse->Get_Array(), m_color->Get_Array(), m_alpha->Get_Array(), cnt);
        } else if (m_color != nullptr) {
            VectorProcessorClass::Copy(m_diffuse->Get_Array(), m_color->Get_Array(), 1.0f, cnt);
        } else {
            VectorProcessorClass::Copy(m_diffuse->Get_Array(), Vector3(1.0f, 1.0f, 1.0f), m_alpha->Get_Array(), cnt);
        }

        VectorProcessorClass::Clamp(m_diffuse->Get_Array(), m_diffuse->Get_Array(), 0.0f, 1.0f, cnt);
    } else if (m_diffuse != nullptr) {
        m_diffuse->Release_Ref();
        m_diffuse = nullptr;
    }
}

void ParticleBufferClass::Render_Particles(RenderInfoClass &rinfo)
{
    ShareBufferClass<unsigned int> *apt = nullptr;
    unsigned int active_point_count = 0;
    Generate_APT(&apt, active_point_count);

    if (m_color == nullptr) {
        m_pointGroup->Set_Point_Color(m_colorKeyFrameValues[0]);
    }

    if (m_alpha == nullptr) {
        m_pointGroup->Set_Point_Alpha(m_alphaKeyFrameValues[0]);
    }

    if (m_size == nullptr) {
        m_pointGroup->Set_Point_Size(m_sizeKeyFrameValues[0]);
    }

    if (m_orientation == nullptr) {
        m_pointGroup->Set_Point_Orientation(0);
    }

    if (m_frame == nullptr) {
        m_pointGroup->Set_Point_Frame(((int)(m_frameKeyFrameValues[0])) & 0xFF);
    }

    int pingpong = 0;

    if (m_pingPongPosition) {
        pingpong = W3D::Get_Frame_Count() & 0x1;
    }

    Combine_Color_And_Alpha();
    m_pointGroup->Set_Arrays(m_position[pingpong], m_diffuse, apt, m_size, m_orientation, m_frame, active_point_count);
    Update_Bounding_Box();
    m_pointGroup->Render(rinfo);
}

void ParticleBufferClass::Render_Line(RenderInfoClass &rinfo)
{
    m_lineRenderer->Set_Freeze_Random(Is_Freeze_Random());
    int pingpong = 0;

    if (m_pingPongPosition) {
        pingpong = W3D::Get_Frame_Count() & 0x1;
    }

    static SimpleDynVecClass<Vector3> tmp_points;
    static SimpleDynVecClass<Vector4> tmp_diffuse;
    static SimpleDynVecClass<unsigned char> tmp_id;

    unsigned char *ids = m_groupID->Get_Array();
    Vector3 *positions = m_position[pingpong]->Get_Array();
    Vector4 *diffuse = nullptr;
    Vector4 color(0.0f, 0.0f, 0.0f, 0.0f);

    Combine_Color_And_Alpha();

    if (m_diffuse != nullptr) {
        diffuse = m_diffuse->Get_Array();
    } else {
        color.X = m_colorKeyFrameValues[0].X;
        color.Y = m_colorKeyFrameValues[0].Y;
        color.Z = m_colorKeyFrameValues[0].Z;
        color.W = m_alphaKeyFrameValues[0];
    }

    unsigned int sub1_end;
    unsigned int sub2_start;

    if ((m_start < m_end) || ((m_start == m_end) && m_nonNewNum == 0)) {
        sub1_end = m_end;
        sub2_start = m_end;
    } else {
        sub1_end = m_maxNum;
        sub2_start = 0;
    }

    tmp_points.Delete_All(false);
    tmp_diffuse.Delete_All(false);
    tmp_id.Delete_All(false);
    unsigned char id = 0;
    Vector4 *col = &color;

    for (unsigned int i = m_start; i < sub1_end; i++) {
        if (s_permutationArray[i & 0xF] >= m_decimationThreshold) {
            tmp_points.Add(positions[i]);

            col = &diffuse[i];

            if (diffuse == nullptr) {
                col = &color;
            }

            tmp_diffuse.Add(*col);

            id = ids[i];
            tmp_id.Add(id);
        }
    }

    for (unsigned int i = sub2_start; i < m_end; i++) {
        if (s_permutationArray[i & 0xF] >= m_decimationThreshold) {
            tmp_points.Add(positions[i]);

            col = &diffuse[i];

            if (diffuse == nullptr) {
                col = &color;
            }

            tmp_diffuse.Add(*col);

            id = ids[i];
            tmp_id.Add(id);
        }
    }

    if (m_emitter != nullptr && !m_emitter->Is_Stopped() && id == m_currentGroupID) {
        tmp_points.Add(m_emitter->Get_Position());
        tmp_diffuse.Add(*col);
        tmp_id.Add(id);
    }

    if (tmp_points.Count() > 0) {
        SphereClass bounding_sphere;
        Get_Obj_Space_Bounding_Sphere(bounding_sphere);
        int index = 0;

        for (int i = 0; i < tmp_points.Count(); index = i) {
            for (unsigned char *j = &tmp_id[i]; i < tmp_points.Count() && tmp_id[index] == *j; j++) {
                i++;
            }

            if (i - index > 1) {
                m_lineRenderer->Render(
                    rinfo, m_transform, i - index, &tmp_points[index], bounding_sphere, &tmp_diffuse[index]);
            }
        }
    }
}

void ParticleBufferClass::Render_Line_Group(RenderInfoClass &rinfo)
{
    ShareBufferClass<unsigned int> *apt = nullptr;
    unsigned int active_point_count = 0;
    Generate_APT(&apt, active_point_count);

    if (m_color == nullptr) {
        m_lineGroup->Set_Line_Color(m_colorKeyFrameValues[0]);
    }

    if (m_alpha == nullptr) {
        m_lineGroup->Set_Line_Alpha(m_alphaKeyFrameValues[0]);
    }

    if (m_size == nullptr) {
        m_lineGroup->Set_Line_Size(m_sizeKeyFrameValues[0]);
    }

    if (m_frame == nullptr) {
        m_lineGroup->Set_Line_UCoord(m_frameKeyFrameValues[0]);
    }

    int pingpong = 0;

    if (m_pingPongPosition) {
        pingpong = W3D::Get_Frame_Count() & 0x1;
    }

    Combine_Color_And_Alpha();
    TailDiffuseTypeEnum tailtype = Determine_Tail_Diffuse();

    switch (tailtype) {
        case BLACK:
            Ref_Ptr_Release(m_tailDiffuse);
            m_defaultTailDiffuse.Set(0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case WHITE:
            Ref_Ptr_Release(m_tailDiffuse);
            m_defaultTailDiffuse.Set(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        case SAME_AS_HEAD_ALPHA_ZERO:
            if (m_diffuse == nullptr) {
                Ref_Ptr_Release(m_tailDiffuse);
                m_defaultTailDiffuse.Set(
                    m_colorKeyFrameValues[0].X, m_colorKeyFrameValues[0].Y, m_colorKeyFrameValues[0].Z, 0);
            } else {
                if (m_tailDiffuse == nullptr) {
                    m_tailDiffuse = New_Share_Buffer<Vector4>(m_maxNum, "ParticleBufferClass::TailDiffuse");
                }

                for (unsigned int i = 0; i < m_maxNum; i++) {
                    Vector4 elt = m_diffuse->Get_Element(i);
                    elt.W = 0;
                    m_tailDiffuse->Set_Element(i, elt);
                }
            }
            break;
        case SAME_AS_HEAD:
            if (m_diffuse == nullptr) {
                Ref_Ptr_Release(m_tailDiffuse);
                m_defaultTailDiffuse.Set(m_colorKeyFrameValues[0].X,
                    m_colorKeyFrameValues[0].Y,
                    m_colorKeyFrameValues[0].Z,
                    m_alphaKeyFrameValues[0]);
            } else {
                if (m_tailDiffuse == nullptr) {
                    m_tailDiffuse = New_Share_Buffer<Vector4>(m_maxNum, "ParticleBufferClass::TailDiffuse");
                }

                VectorProcessorClass::Copy(m_tailDiffuse->Get_Array(), m_diffuse->Get_Array(), m_maxNum);
            }
            break;
        default:
            captainslog_assert(0);
            break;
    }

    if (m_tailDiffuse == nullptr) {
        m_lineGroup->Set_Tail_Diffuse(m_defaultTailDiffuse);
    }

    m_lineGroup->Set_Arrays(
        m_position[pingpong], m_tailPosition, m_diffuse, m_tailDiffuse, apt, m_size, m_uCoord, active_point_count);
    Update_Bounding_Box();
    m_lineGroup->Render(rinfo);
}

void ParticleBufferClass::Scale(float scale)
{
    if (m_numSizeKeyFrames != 0) {
        for (unsigned int i = 0; i < m_numSizeKeyFrames; i++) {
            m_sizeKeyFrameValues[i] *= scale;
            m_sizeKeyFrameDeltas[i] *= scale;
        }
    } else {
        m_sizeKeyFrameValues[0] *= scale;
    }

    if (m_randomSizeEntries) {
        for (unsigned int i = 0; i <= m_numRandomSizeEntriesMinus1; i++) {
            m_randomSizeEntries[i] *= scale;
        }
    }

    if (m_lineRenderer != nullptr) {
        m_lineRenderer->Scale(scale);
    }

    m_maxSize *= scale;
    m_sizeRandom *= scale;
    m_accel *= scale;
}

void ParticleBufferClass::On_Frame_Update()
{
    Invalidate_Cached_Bounding_Volumes();

    if (m_emitter != nullptr) {
        m_emitter->Emit();
    }

    if (Is_Complete()) {
        captainslog_assert(m_scene);
        m_scene->Register(this, SceneClass::RELEASE);
    }
}

void ParticleBufferClass::Notify_Added(SceneClass *scene)
{
    RenderObjClass::Notify_Added(scene);
    scene->Register(this, SceneClass::ON_FRAME_UPDATE);
}

void ParticleBufferClass::Notify_Removed(SceneClass *scene)
{
    scene->Unregister(this, SceneClass::ON_FRAME_UPDATE);
    RenderObjClass::Notify_Removed(scene);
}

void ParticleBufferClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    const_cast<ParticleBufferClass *>(this)->Update_Bounding_Box();
    sphere.Center = m_boundingBox.m_center;
    sphere.Radius = m_boundingBox.m_extent.Length();
}

void ParticleBufferClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    const_cast<ParticleBufferClass *>(this)->Update_Bounding_Box();
    box = m_boundingBox;
}

void ParticleBufferClass::Prepare_LOD(CameraClass &camera)
{
    if (Is_Not_Hidden_At_All() == false) {
        return;
    }

    Vector3 cam = camera.Get_Position();
    ViewportClass viewport = camera.Get_Viewport();
    Vector2 vpr_min, vpr_max;
    camera.Get_View_Plane(vpr_min, vpr_max);
    float width_factor = viewport.Width() / (vpr_max.X - vpr_min.X);
    float height_factor = viewport.Height() / (vpr_max.Y - vpr_min.Y);

    const SphereClass &sphere = Get_Bounding_Sphere();
    float dist = (sphere.Center - cam).Length();
    float bounding_sphere_projected_radius = 0.0f;
    float particle_projected_radius = 0.0f;

    if (dist != 0.0f) {
        float oo_dist = 1.0f / dist;
        bounding_sphere_projected_radius = sphere.Radius * oo_dist;
        particle_projected_radius = m_maxSize * oo_dist;
    }

    float bs_rad_sq = bounding_sphere_projected_radius * bounding_sphere_projected_radius;
    float p_rad_sq = particle_projected_radius * particle_projected_radius * m_maxNum;
    float proj_area = GAMEMATH_PI * std::min(bs_rad_sq, p_rad_sq) * width_factor * height_factor;
    m_projectedArea = 0.9f * m_projectedArea + 0.1f * proj_area;
    int minlod = Calculate_Cost_Value_Arrays(m_projectedArea, m_value, m_cost);

    if (Get_LOD_Level() < minlod) {
        Set_LOD_Level(minlod);
    }
}

void ParticleBufferClass::Increment_LOD()
{
    if (m_decimationThreshold > 0) {
        m_decimationThreshold--;
    }
}

void ParticleBufferClass::Decrement_LOD()
{
    if (m_decimationThreshold < m_lodCount) {
        m_decimationThreshold++;
    }
}

float ParticleBufferClass::Get_Cost() const
{
    return (m_cost[(m_lodCount - 1) - m_decimationThreshold]);
}

float ParticleBufferClass::Get_Value() const
{
    return (m_value[(m_lodCount - 1) - m_decimationThreshold]);
}

float ParticleBufferClass::Get_Post_Increment_Value() const
{
    return (m_value[m_lodCount - m_decimationThreshold]);
}

void ParticleBufferClass::Set_LOD_Level(int lod)
{
    lod = std::clamp(lod, 0, (int)m_lodCount);
    m_decimationThreshold = (m_lodCount - 1) - lod;
}

int ParticleBufferClass::Get_LOD_Level() const
{
    return ((m_lodCount - 1) - m_decimationThreshold);
}

int ParticleBufferClass::Get_LOD_Count() const
{
    return m_lodCount;
}

int ParticleBufferClass::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
    unsigned int lod = 0;
    float cost_factor = 0.0f;

    switch (m_renderMode) {
        case W3D_EMITTER_RENDER_MODE_TRI_PARTICLES:
            cost_factor = (float)m_maxNum * 0.0625f;
            break;
        case W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES:
            cost_factor = (float)m_maxNum * 2.0f * 0.0625f;
            break;
        case W3D_EMITTER_RENDER_MODE_LINE:
            cost_factor = (float)(2 * m_maxNum - 1) * 0.0625f;
            break;
        case W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA:
            cost_factor = (float)m_maxNum * 4.0f * 0.0625f;
            break;
        case W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM:
            cost_factor = (float)m_maxNum * 8.0f * 0.0625f;
            break;
    }

    for (lod = 0; lod < m_lodCount; lod++) {
        costs[lod] = cost_factor * (float)lod;
        costs[lod] = (costs[lod] != 0) ? costs[lod] : 0.000001f;
    }

    for (lod = 0; lod < m_lodCount && s_LODMaxScreenSizes[lod] < screen_area; lod++) {
        values[lod] = AT_MIN_LOD;
    }

    if (lod >= m_lodCount) {
        lod = m_lodCount - 1;
    } else {
        values[lod] = AT_MIN_LOD;
    }

    int minlod = lod;
    lod++;

    for (; lod < m_lodCount; lod++) {
        float polycount = costs[lod];
        float benefit_factor = (polycount > GAMEMATH_EPSILON) ? (1 - (0.5f / (polycount * polycount))) : 0.0f;
        values[lod] = (benefit_factor * screen_area * m_lodBias) / costs[lod];
    }

    values[m_lodCount] = AT_MAX_LOD;
    return minlod;
}

void ParticleBufferClass::Reset_Colors(ParticlePropertyStruct<Vector3> &new_props)
{
    unsigned int i;
    unsigned int ui_previous_key_time = 0;
    unsigned int ui_current_key_time = 0;
    m_colorRandom = new_props.Rand;
    static const float eps_byte = 0.0038f;
    bool color_rand_zero = (GameMath::Fabs(new_props.Rand.X) < eps_byte && GameMath::Fabs(new_props.Rand.Y) < eps_byte
        && GameMath::Fabs(new_props.Rand.Z) < eps_byte);

    if (color_rand_zero && new_props.NumKeyFrames == 0) {
        if (m_color != nullptr) {
            m_color->Release_Ref();
            m_color = nullptr;
        }

        if (m_colorKeyFrameTimes != nullptr) {
            delete[] m_colorKeyFrameTimes;
            m_colorKeyFrameTimes = nullptr;
        }

        if (m_colorKeyFrameDeltas != nullptr) {
            delete[] m_colorKeyFrameDeltas;
            m_colorKeyFrameDeltas = nullptr;
        }

        if (m_colorKeyFrameValues != nullptr) {
            if (m_numColorKeyFrames > 1) {
                delete[] m_colorKeyFrameValues;
                m_colorKeyFrameValues = new Vector3[1];
            }
        } else {
            m_colorKeyFrameValues = new Vector3[1];
        }

        m_numColorKeyFrames = 0;
        m_numRandomColorEntriesMinus1 = 0;
        m_colorKeyFrameValues[0] = new_props.Start;

    } else {
        if (m_color == nullptr) {
            m_color = New_Share_Buffer<Vector3>(m_maxNum, "ParticleBufferClass::Color");
        }

        ui_previous_key_time = 0;
        unsigned int ckey;

        for (ckey = 0; ckey < new_props.NumKeyFrames; ckey++) {
            ui_current_key_time = (unsigned int)(new_props.KeyTimes[ckey] * 1000.0f);
            captainslog_assert(ui_current_key_time > ui_previous_key_time);

            if (ui_current_key_time >= m_maxAge) {
                break;
            }

            ui_previous_key_time = ui_current_key_time;
        }

        bool color_constant_at_end = (ckey == new_props.NumKeyFrames);
        unsigned int new_num_color_key_frames = ckey + 1;

        if (new_num_color_key_frames != m_numColorKeyFrames) {

            if (m_colorKeyFrameTimes != nullptr) {
                delete[] m_colorKeyFrameTimes;
                m_colorKeyFrameTimes = nullptr;
            }

            if (m_colorKeyFrameValues != nullptr) {
                delete[] m_colorKeyFrameValues;
                m_colorKeyFrameValues = nullptr;
            }

            if (m_colorKeyFrameDeltas != nullptr) {
                delete[] m_colorKeyFrameDeltas;
                m_colorKeyFrameDeltas = nullptr;
            }

            m_numColorKeyFrames = new_num_color_key_frames;
            m_colorKeyFrameTimes = new unsigned int[m_numColorKeyFrames];
            m_colorKeyFrameValues = new Vector3[m_numColorKeyFrames];
            m_colorKeyFrameDeltas = new Vector3[m_numColorKeyFrames];
        }

        m_colorKeyFrameTimes[0] = 0;
        m_colorKeyFrameValues[0] = new_props.Start;

        for (i = 1; i < m_numColorKeyFrames; i++) {
            unsigned int im1 = i - 1;
            m_colorKeyFrameTimes[i] = (unsigned int)(new_props.KeyTimes[im1] * 1000.0f);
            m_colorKeyFrameValues[i] = new_props.Values[im1];
        }

        for (i = 0; i < m_numColorKeyFrames - 1; i++) {
            m_colorKeyFrameDeltas[i] = (m_colorKeyFrameValues[i + 1] - m_colorKeyFrameValues[i])
                / (float)(m_colorKeyFrameTimes[i + 1] - m_colorKeyFrameTimes[i]);
        }

        if (color_constant_at_end) {
            m_colorKeyFrameDeltas[i].Set(0.0, 0.0, 0.0);
        } else {
            m_colorKeyFrameDeltas[i] = (new_props.Values[i] - m_colorKeyFrameValues[i])
                / (new_props.KeyTimes[i] * 1000.0f - (float)m_colorKeyFrameTimes[i]);
        }

        if (color_rand_zero) {
            if (m_randomColorEntries) {
                if (m_numRandomColorEntriesMinus1 != 0) {
                    delete[] m_randomColorEntries;
                    m_randomColorEntries = new Vector3[1];
                }
            } else {
                m_randomColorEntries = new Vector3[1];
            }

            m_numRandomColorEntriesMinus1 = 0;
            m_randomColorEntries[0].X = 0.0f;
            m_randomColorEntries[0].Y = 0.0f;
            m_randomColorEntries[0].Z = 0.0f;
        } else {
            unsigned int pot_num = Find_POT(m_maxNum);
            unsigned int default_randomizer_entries = std::min(pot_num, s_maxRandomEntries);

            if (m_randomColorEntries) {
                if (m_numRandomColorEntriesMinus1 != (default_randomizer_entries - 1)) {
                    delete[] m_randomColorEntries;
                    m_randomColorEntries = new Vector3[default_randomizer_entries];
                }
            } else {
                m_randomColorEntries = new Vector3[default_randomizer_entries];
            }

            m_numRandomColorEntriesMinus1 = default_randomizer_entries - 1;
            float rscale = new_props.Rand.X * s_ooIntMax;
            float gscale = new_props.Rand.Y * s_ooIntMax;
            float bscale = new_props.Rand.Z * s_ooIntMax;

            for (unsigned int j = 0; j <= m_numRandomColorEntriesMinus1; j++) {
                m_randomColorEntries[j] = Vector3(s_randGen * rscale, s_randGen * gscale, s_randGen * bscale);
            }
        }
    }
}

void ParticleBufferClass::Reset_Opacity(ParticlePropertyStruct<float> &new_props)
{
    unsigned int i;
    unsigned int ui_previous_key_time = 0;
    unsigned int ui_current_key_time = 0;
    m_opacityRandom = new_props.Rand;
    static const float eps_byte = 0.0038f;
    bool alpha_rand_zero = (GameMath::Fabs(new_props.Rand) < eps_byte);

    if (alpha_rand_zero && new_props.NumKeyFrames == 0) {
        if (m_alpha != nullptr) {
            m_alpha->Release_Ref();
            m_alpha = nullptr;
        }

        if (m_alphaKeyFrameTimes != nullptr) {
            delete[] m_alphaKeyFrameTimes;
            m_alphaKeyFrameTimes = nullptr;
        }

        if (m_alphaKeyFrameDeltas != nullptr) {
            delete[] m_alphaKeyFrameDeltas;
            m_alphaKeyFrameDeltas = nullptr;
        }

        if (m_alphaKeyFrameValues) {
            if (m_numAlphaKeyFrames > 1) {
                delete[] m_alphaKeyFrameValues;
                m_alphaKeyFrameValues = new float[1];
            }
        } else {
            m_alphaKeyFrameValues = new float[1];
        }

        m_numAlphaKeyFrames = 0;
        m_numRandomAlphaEntriesMinus1 = 0;
        m_alphaKeyFrameValues[0] = new_props.Start;

    } else {
        if (m_alpha == nullptr) {
            m_alpha = New_Share_Buffer<float>(m_maxNum, "ParticleBufferClass::Alpha");
        }

        ui_previous_key_time = 0;
        unsigned int akey;

        for (akey = 0; akey < new_props.NumKeyFrames; akey++) {
            ui_current_key_time = (unsigned int)(new_props.KeyTimes[akey] * 1000.0f);
            captainslog_assert(ui_current_key_time > ui_previous_key_time);

            if (ui_current_key_time >= m_maxAge) {
                break;
            }

            ui_previous_key_time = ui_current_key_time;
        }

        bool alpha_constant_at_end = (akey == new_props.NumKeyFrames);
        unsigned int new_num_alpha_key_frames = akey + 1;

        if (new_num_alpha_key_frames != m_numAlphaKeyFrames) {
            if (m_alphaKeyFrameTimes != nullptr) {
                delete[] m_alphaKeyFrameTimes;
                m_alphaKeyFrameTimes = nullptr;
            }

            if (m_alphaKeyFrameValues != nullptr) {
                delete[] m_alphaKeyFrameValues;
                m_alphaKeyFrameValues = nullptr;
            }

            if (m_alphaKeyFrameDeltas != nullptr) {
                delete[] m_alphaKeyFrameDeltas;
                m_alphaKeyFrameDeltas = nullptr;
            }

            m_numAlphaKeyFrames = new_num_alpha_key_frames;
            m_alphaKeyFrameTimes = new unsigned int[m_numAlphaKeyFrames];
            m_alphaKeyFrameValues = new float[m_numAlphaKeyFrames];
            m_alphaKeyFrameDeltas = new float[m_numAlphaKeyFrames];
        }

        m_alphaKeyFrameTimes[0] = 0;
        m_alphaKeyFrameValues[0] = new_props.Start;

        for (i = 1; i < m_numAlphaKeyFrames; i++) {
            unsigned int im1 = i - 1;
            m_alphaKeyFrameTimes[i] = (unsigned int)(new_props.KeyTimes[im1] * 1000.0f);
            m_alphaKeyFrameValues[i] = new_props.Values[im1];
        }

        for (i = 0; i < m_numAlphaKeyFrames - 1; i++) {
            m_alphaKeyFrameDeltas[i] = (m_alphaKeyFrameValues[i + 1] - m_alphaKeyFrameValues[i])
                / (float)(m_alphaKeyFrameTimes[i + 1] - m_alphaKeyFrameTimes[i]);
        }

        if (alpha_constant_at_end) {
            m_alphaKeyFrameDeltas[i] = 0.0f;
        } else {
            m_alphaKeyFrameDeltas[i] = (new_props.Values[i] - m_alphaKeyFrameValues[i])
                / (new_props.KeyTimes[i] * 1000.0f - (float)m_alphaKeyFrameTimes[i]);
        }

        if (alpha_rand_zero) {
            if (m_randomAlphaEntries != nullptr) {
                if (m_numRandomAlphaEntriesMinus1 != 0) {
                    delete[] m_randomAlphaEntries;
                    m_randomAlphaEntries = new float[1];
                }
            } else {
                m_randomAlphaEntries = new float[1];
            }

            m_numRandomAlphaEntriesMinus1 = 0;
            m_randomAlphaEntries[0] = 0.0f;
        } else {
            unsigned int pot_num = Find_POT(m_maxNum);
            unsigned int default_randomizer_entries = std::min(pot_num, s_maxRandomEntries);

            if (m_randomAlphaEntries != nullptr) {
                if (m_numRandomAlphaEntriesMinus1 != (default_randomizer_entries - 1)) {
                    delete[] m_randomAlphaEntries;
                    m_randomAlphaEntries = new float[default_randomizer_entries];
                }
            } else {
                m_randomAlphaEntries = new float[default_randomizer_entries];
            }

            m_numRandomAlphaEntriesMinus1 = default_randomizer_entries - 1;
            float ascale = new_props.Rand * s_ooIntMax;

            for (unsigned int j = 0; j <= m_numRandomAlphaEntriesMinus1; j++) {
                m_randomAlphaEntries[j] = s_randGen * ascale;
            }
        }
    }
}

void ParticleBufferClass::Reset_Size(ParticlePropertyStruct<float> &new_props)
{
    unsigned int i;
    unsigned int ui_previous_key_time = 0;
    unsigned int ui_current_key_time = 0;
    m_sizeRandom = new_props.Rand;
    static const float eps_size = 1.0e-12f;
    bool size_rand_zero = (GameMath::Fabs(new_props.Rand) < eps_size);

    if (size_rand_zero && new_props.NumKeyFrames == 0) {
        if (m_size != nullptr) {
            m_size->Release_Ref();
            m_size = nullptr;
        }

        if (m_sizeKeyFrameTimes != nullptr) {
            delete[] m_sizeKeyFrameTimes;
            m_sizeKeyFrameTimes = nullptr;
        }

        if (m_sizeKeyFrameDeltas != nullptr) {
            delete[] m_sizeKeyFrameDeltas;
            m_sizeKeyFrameDeltas = nullptr;
        }

        if (m_sizeKeyFrameValues != nullptr) {
            if (m_numSizeKeyFrames > 1) {
                delete[] m_sizeKeyFrameValues;
                m_sizeKeyFrameValues = new float[1];
            }
        } else {
            m_sizeKeyFrameValues = new float[1];
        }

        m_numSizeKeyFrames = 0;
        m_numRandomSizeEntriesMinus1 = 0;
        m_sizeKeyFrameValues[0] = new_props.Start;
        m_maxSize = m_sizeKeyFrameValues[0];
    } else {
        if (m_size == nullptr) {
            m_size = New_Share_Buffer<float>(m_maxNum, "ParticleBufferClass::Size");
        }

        ui_previous_key_time = 0;
        unsigned int skey;

        for (skey = 0; skey < new_props.NumKeyFrames; skey++) {
            ui_current_key_time = (unsigned int)(new_props.KeyTimes[skey] * 1000.0f);
            captainslog_assert(ui_current_key_time > ui_previous_key_time);

            if (ui_current_key_time >= m_maxAge) {
                break;
            }

            ui_previous_key_time = ui_current_key_time;
        }

        bool size_constant_at_end = (skey == new_props.NumKeyFrames);
        unsigned int new_num_size_key_frames = skey + 1;

        if (new_num_size_key_frames != m_numSizeKeyFrames) {
            if (m_sizeKeyFrameTimes != nullptr) {
                delete[] m_sizeKeyFrameTimes;
                m_sizeKeyFrameTimes = nullptr;
            }

            if (m_sizeKeyFrameValues != nullptr) {
                delete[] m_sizeKeyFrameValues;
                m_sizeKeyFrameValues = nullptr;
            }

            if (m_sizeKeyFrameDeltas != nullptr) {
                delete[] m_sizeKeyFrameDeltas;
                m_sizeKeyFrameDeltas = nullptr;
            }

            m_numSizeKeyFrames = new_num_size_key_frames;
            m_sizeKeyFrameTimes = new unsigned int[m_numSizeKeyFrames];
            m_sizeKeyFrameValues = new float[m_numSizeKeyFrames];
            m_sizeKeyFrameDeltas = new float[m_numSizeKeyFrames];
        }

        m_sizeKeyFrameTimes[0] = 0;
        m_sizeKeyFrameValues[0] = new_props.Start;

        for (i = 1; i < m_numSizeKeyFrames; i++) {
            unsigned int im1 = i - 1;
            m_sizeKeyFrameTimes[i] = (unsigned int)(new_props.KeyTimes[im1] * 1000.0f);
            m_sizeKeyFrameValues[i] = new_props.Values[im1];
        }

        for (i = 0; i < m_numSizeKeyFrames - 1; i++) {
            m_sizeKeyFrameDeltas[i] = (m_sizeKeyFrameValues[i + 1] - m_sizeKeyFrameValues[i])
                / (float)(m_sizeKeyFrameTimes[i + 1] - m_sizeKeyFrameTimes[i]);
        }

        if (size_constant_at_end) {
            m_sizeKeyFrameDeltas[i] = 0.0f;
        } else {
            m_sizeKeyFrameDeltas[i] = (new_props.Values[i] - m_sizeKeyFrameValues[i])
                / (new_props.KeyTimes[i] * 1000.0f - (float)m_sizeKeyFrameTimes[i]);
        }

        m_maxSize = m_sizeKeyFrameValues[0];

        for (i = 1; i < m_numSizeKeyFrames; i++) {
            m_maxSize = std::max(m_maxSize, m_sizeKeyFrameValues[i]);
        }

        float last_size = m_sizeKeyFrameValues[m_numSizeKeyFrames - 1]
            + m_sizeKeyFrameDeltas[m_numSizeKeyFrames - 1] * (float)(m_maxAge - m_sizeKeyFrameTimes[m_numSizeKeyFrames - 1]);
        m_maxSize = std::max(m_maxSize, last_size);
        m_maxSize += GameMath::Fabs(new_props.Rand);

        if (size_rand_zero) {

            if (m_randomSizeEntries != nullptr) {
                if (m_numRandomSizeEntriesMinus1 != 0) {
                    delete[] m_randomSizeEntries;
                    m_randomSizeEntries = new float[1];
                }
            } else {
                m_randomSizeEntries = new float[1];
            }

            m_numRandomSizeEntriesMinus1 = 0;
            m_randomSizeEntries[0] = 0.0f;
        } else {
            unsigned int pot_num = Find_POT(m_maxNum);
            unsigned int default_randomizer_entries = std::min(pot_num, s_maxRandomEntries);

            if (m_randomSizeEntries != nullptr) {
                if (m_numRandomSizeEntriesMinus1 != (default_randomizer_entries - 1)) {
                    delete[] m_randomSizeEntries;
                    m_randomSizeEntries = new float[default_randomizer_entries];
                }
            } else {
                m_randomSizeEntries = new float[default_randomizer_entries];
            }

            m_numRandomSizeEntriesMinus1 = default_randomizer_entries - 1;

            float sscale = new_props.Rand * s_ooIntMax;
            for (unsigned int j = 0; j <= m_numRandomSizeEntriesMinus1; j++) {
                m_randomSizeEntries[j] = s_randGen * sscale;
            }
        }
    }
}

void ParticleBufferClass::Reset_Rotations(ParticlePropertyStruct<float> &new_props, float orient_rnd)
{
    unsigned int i;
    float oo_intmax = 1.0f / (float)INT_MAX;
    unsigned int ui_previous_key_time = 0;
    unsigned int ui_current_key_time = 0;
    m_rotationRandom = new_props.Rand * 0.001f;
    m_initialOrientationRandom = orient_rnd;
    static const float eps_orientation = 2.77777778e-4f;
    static const float eps_rotation = 2.77777778e-4f;
    bool orientation_rand_zero = GameMath::Fabs(orient_rnd) < eps_orientation;
    bool rotation_rand_zero = GameMath::Fabs(new_props.Rand) < eps_rotation;

    if (orientation_rand_zero && rotation_rand_zero && new_props.NumKeyFrames == 0
        && GameMath::Fabs(new_props.Start) < eps_rotation) {
        Ref_Ptr_Release(m_orientation);

        if (m_rotationKeyFrameTimes != nullptr) {
            delete[] m_rotationKeyFrameTimes;
            m_rotationKeyFrameTimes = nullptr;
        }

        if (m_halfRotationKeyFrameDeltas != nullptr) {
            delete[] m_halfRotationKeyFrameDeltas;
            m_halfRotationKeyFrameDeltas = nullptr;
        }

        if (m_rotationKeyFrameValues != nullptr) {
            delete[] m_rotationKeyFrameValues;
            m_rotationKeyFrameValues = nullptr;
        }

        if (m_orientationKeyFrameValues != nullptr) {
            delete[] m_orientationKeyFrameValues;
            m_orientationKeyFrameValues = nullptr;
        }

        m_numRotationKeyFrames = 0;
        m_numRandomRotationEntriesMinus1 = 0;
        m_numRandomOrientationEntriesMinus1 = 0;

    } else {
        if (m_orientation == nullptr) {
            m_orientation = New_Share_Buffer<uint8_t>(m_maxNum, "ParticleBufferClass::Orientation");
        }

        ui_previous_key_time = 0;
        unsigned int key;

        for (key = 0; key < new_props.NumKeyFrames; key++) {
            ui_current_key_time = (unsigned int)(new_props.KeyTimes[key] * 1000.0f);
            captainslog_assert(ui_current_key_time > ui_previous_key_time);

            if (ui_current_key_time >= m_maxAge) {
                break;
            }

            ui_previous_key_time = ui_current_key_time;
        }

        bool rotation_constant_at_end = (key == new_props.NumKeyFrames);
        unsigned int new_num_key_frames = key + 1;

        if (new_num_key_frames != m_numRotationKeyFrames) {
            if (m_rotationKeyFrameTimes != nullptr) {
                delete[] m_rotationKeyFrameTimes;
                m_rotationKeyFrameTimes = nullptr;
            }

            if (m_rotationKeyFrameValues != nullptr) {
                delete[] m_rotationKeyFrameValues;
                m_rotationKeyFrameValues = nullptr;
            }

            if (m_halfRotationKeyFrameDeltas != nullptr) {
                delete[] m_halfRotationKeyFrameDeltas;
                m_halfRotationKeyFrameDeltas = nullptr;
            }

            if (m_orientationKeyFrameValues != nullptr) {
                delete[] m_orientationKeyFrameValues;
                m_orientationKeyFrameValues = nullptr;
            }

            m_numRotationKeyFrames = new_num_key_frames;
            m_rotationKeyFrameTimes = new unsigned int[m_numRotationKeyFrames];
            m_rotationKeyFrameValues = new float[m_numRotationKeyFrames];
            m_halfRotationKeyFrameDeltas = new float[m_numRotationKeyFrames];
            m_orientationKeyFrameValues = new float[m_numRotationKeyFrames];
        }

        m_rotationKeyFrameTimes[0] = 0;
        m_rotationKeyFrameValues[0] = new_props.Start * 0.001f;

        for (i = 1; i < m_numRotationKeyFrames; i++) {
            unsigned int im1 = i - 1;
            m_rotationKeyFrameTimes[i] = (unsigned int)(new_props.KeyTimes[im1] * 1000.0f);
            m_rotationKeyFrameValues[i] = new_props.Values[im1] * 0.001f;
        }

        for (i = 0; i < m_numRotationKeyFrames - 1; i++) {
            m_halfRotationKeyFrameDeltas[i] = 0.5f
                * ((m_rotationKeyFrameValues[i + 1] - m_rotationKeyFrameValues[i])
                    / (float)(m_rotationKeyFrameTimes[i + 1] - m_rotationKeyFrameTimes[i]));
        }

        if (rotation_constant_at_end) {
            m_halfRotationKeyFrameDeltas[i] = 0.0f;
        } else {
            m_halfRotationKeyFrameDeltas[i] = 0.5f * (new_props.Values[i] * 0.001f - m_rotationKeyFrameValues[i])
                / (new_props.KeyTimes[i] * 1000.0f - (float)m_rotationKeyFrameTimes[i]);
        }

        m_orientationKeyFrameValues[0] = 0.0f;

        for (i = 1; i < m_numRotationKeyFrames; i++) {
            float delta_t = (float)(m_rotationKeyFrameTimes[i] - m_rotationKeyFrameTimes[i - 1]);
            m_orientationKeyFrameValues[i] = m_orientationKeyFrameValues[i - 1]
                + delta_t * (m_rotationKeyFrameValues[i - 1] + m_halfRotationKeyFrameDeltas[i - 1] * delta_t);
        }

        if (rotation_rand_zero) {
            if (m_randomRotationEntries != nullptr) {
                if (m_numRandomRotationEntriesMinus1 != 0) {
                    delete[] m_randomRotationEntries;
                    m_randomRotationEntries = new float[1];
                }
            } else {
                m_randomRotationEntries = new float[1];
            }

            m_numRandomRotationEntriesMinus1 = 0;
            m_randomRotationEntries[0] = 0.0f;
        } else {
            unsigned int pot_num = Find_POT(m_maxNum);
            unsigned int default_randomizer_entries = std::min(pot_num, s_maxRandomEntries);

            if (m_randomRotationEntries != nullptr) {
                if (m_numRandomRotationEntriesMinus1 != (default_randomizer_entries - 1)) {
                    delete[] m_randomRotationEntries;
                    m_randomRotationEntries = new float[default_randomizer_entries];
                }
            } else {
                m_randomRotationEntries = new float[default_randomizer_entries];
            }

            m_numRandomRotationEntriesMinus1 = default_randomizer_entries - 1;
            float scale = new_props.Rand * 0.001f * oo_intmax;

            for (unsigned int j = 0; j <= m_numRandomRotationEntriesMinus1; j++) {
                m_randomRotationEntries[j] = s_randGen * scale;
            }
        }
        if (orientation_rand_zero) {
            if (m_randomOrientationEntries != nullptr) {
                if (m_numRandomOrientationEntriesMinus1 != 0) {
                    delete[] m_randomOrientationEntries;
                    m_randomOrientationEntries = new float[1];
                }
            } else {
                m_randomOrientationEntries = new float[1];
            }

            m_numRandomOrientationEntriesMinus1 = 0;
            m_randomOrientationEntries[0] = 0.0f;
        } else {
            unsigned int pot_num = Find_POT(m_maxNum);
            unsigned int default_randomizer_entries = std::min(pot_num, s_maxRandomEntries);

            if (m_randomOrientationEntries != nullptr) {
                if (m_numRandomOrientationEntriesMinus1 != (default_randomizer_entries - 1)) {
                    delete[] m_randomOrientationEntries;
                    m_randomOrientationEntries = new float[default_randomizer_entries];
                }
            } else {
                m_randomOrientationEntries = new float[default_randomizer_entries];
            }

            m_numRandomOrientationEntriesMinus1 = default_randomizer_entries - 1;
            float scale = orient_rnd * oo_intmax;

            for (unsigned int j = 0; j <= m_numRandomOrientationEntriesMinus1; j++) {
                m_randomOrientationEntries[j] = s_randGen * scale;
            }
        }
    }
}

void ParticleBufferClass::Reset_Frames(ParticlePropertyStruct<float> &new_props)
{
    unsigned int i;
    float oo_intmax = 1.0f / (float)INT_MAX;
    unsigned int ui_previous_key_time = 0;
    unsigned int ui_current_key_time = 0;
    m_frameRandom = new_props.Rand;
    static const float eps_frame = 0.1f;
    bool frame_rand_zero = (GameMath::Fabs(new_props.Rand) < eps_frame);

    if (frame_rand_zero && new_props.NumKeyFrames == 0) {
        Ref_Ptr_Release(m_frame);
        Ref_Ptr_Release(m_uCoord);

        if (m_frameKeyFrameTimes != nullptr) {
            delete[] m_frameKeyFrameTimes;
            m_frameKeyFrameTimes = nullptr;
        }

        if (m_frameKeyFrameDeltas != nullptr) {
            delete[] m_frameKeyFrameDeltas;
            m_frameKeyFrameDeltas = nullptr;
        }

        if (m_frameKeyFrameValues != nullptr) {
            if (m_numFrameKeyFrames > 1) {
                delete[] m_frameKeyFrameValues;
                m_frameKeyFrameValues = new float[1];
            }
        } else {
            m_frameKeyFrameValues = new float[1];
        }

        m_numFrameKeyFrames = 0;
        m_numRandomFrameEntriesMinus1 = 0;
        m_frameKeyFrameValues[0] = new_props.Start;

    } else {
        if ((m_renderMode == W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA)
            || (m_renderMode == W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM)) {
            if (m_uCoord == nullptr) {
                m_uCoord = New_Share_Buffer<float>(m_maxNum, "ParticleBufferClass::UCoord");
            }
        } else {
            if (m_frame == nullptr) {
                m_frame = New_Share_Buffer<uint8_t>(m_maxNum, "ParticleBufferClass::Frame");
            }
        }

        ui_previous_key_time = 0;
        unsigned int key;

        for (key = 0; key < new_props.NumKeyFrames; key++) {
            ui_current_key_time = (unsigned int)(new_props.KeyTimes[key] * 1000.0f);
            captainslog_assert(ui_current_key_time > ui_previous_key_time);

            if (ui_current_key_time >= m_maxAge) {
                break;
            }

            ui_previous_key_time = ui_current_key_time;
        }

        bool frame_constant_at_end = (key == new_props.NumKeyFrames);
        unsigned int new_num_key_frames = key + 1;

        if (new_num_key_frames != m_numFrameKeyFrames) {
            if (m_frameKeyFrameTimes != nullptr) {
                delete[] m_frameKeyFrameTimes;
                m_frameKeyFrameTimes = nullptr;
            }

            if (m_frameKeyFrameValues != nullptr) {
                delete[] m_frameKeyFrameValues;
                m_frameKeyFrameValues = nullptr;
            }

            if (m_frameKeyFrameDeltas != nullptr) {
                delete[] m_frameKeyFrameDeltas;
                m_frameKeyFrameDeltas = nullptr;
            }

            m_numFrameKeyFrames = new_num_key_frames;
            m_frameKeyFrameTimes = new unsigned int[m_numFrameKeyFrames];
            m_frameKeyFrameValues = new float[m_numFrameKeyFrames];
            m_frameKeyFrameDeltas = new float[m_numFrameKeyFrames];
        }
        m_frameKeyFrameTimes[0] = 0;
        m_frameKeyFrameValues[0] = new_props.Start;

        for (i = 1; i < m_numFrameKeyFrames; i++) {
            unsigned int im1 = i - 1;
            m_frameKeyFrameTimes[i] = (unsigned int)(new_props.KeyTimes[im1] * 1000.0f);
            m_frameKeyFrameValues[i] = new_props.Values[im1];
        }

        for (i = 0; i < m_numFrameKeyFrames - 1; i++) {
            m_frameKeyFrameDeltas[i] = (m_frameKeyFrameValues[i + 1] - m_frameKeyFrameValues[i])
                / (float)(m_frameKeyFrameTimes[i + 1] - m_frameKeyFrameTimes[i]);
        }

        if (frame_constant_at_end) {
            m_frameKeyFrameDeltas[i] = 0.0f;
        } else {
            m_frameKeyFrameDeltas[i] = (new_props.Values[i] - m_frameKeyFrameValues[i])
                / (new_props.KeyTimes[i] * 1000.0f - (float)m_frameKeyFrameTimes[i]);
        }

        if (frame_rand_zero) {
            if (m_randomFrameEntries != nullptr) {
                if (m_numRandomFrameEntriesMinus1 != 0) {
                    delete[] m_randomFrameEntries;
                    m_randomFrameEntries = new float[1];
                }
            } else {
                m_randomFrameEntries = new float[1];
            }

            m_numRandomFrameEntriesMinus1 = 0;
            m_randomFrameEntries[0] = 0.0f;
        } else {
            unsigned int pot_num = Find_POT(m_maxNum);
            unsigned int default_randomizer_entries = std::min(pot_num, s_maxRandomEntries);

            if (m_randomFrameEntries != nullptr) {
                if (m_numRandomFrameEntriesMinus1 != (default_randomizer_entries - 1)) {
                    delete[] m_randomFrameEntries;
                    m_randomFrameEntries = new float[default_randomizer_entries];
                }
            } else {
                m_randomFrameEntries = new float[default_randomizer_entries];
            }

            m_numRandomFrameEntriesMinus1 = default_randomizer_entries - 1;
            float scale = new_props.Rand * oo_intmax;

            for (unsigned int j = 0; j <= m_numRandomFrameEntriesMinus1; j++) {
                m_randomFrameEntries[j] = s_randGen * scale;
            }
        }
    }
}

void ParticleBufferClass::Reset_Blur_Times(ParticlePropertyStruct<float> &new_blur_times)
{
    unsigned int i;
    float oo_intmax = 1.0f / (float)INT_MAX;
    unsigned int ui_previous_key_time = 0;
    unsigned int ui_current_key_time = 0;
    m_blurTimeRandom = new_blur_times.Rand;
    static const float eps_blur = 1e-5f;
    bool blurtime_rand_zero = (GameMath::Fabs(new_blur_times.Rand) < eps_blur);

    if (blurtime_rand_zero && new_blur_times.NumKeyFrames == 0) {
        if (m_blurTimeKeyFrameTimes != nullptr) {
            delete[] m_blurTimeKeyFrameTimes;
            m_blurTimeKeyFrameTimes = nullptr;
        }

        if (m_blurTimeKeyFrameDeltas != nullptr) {
            delete[] m_blurTimeKeyFrameDeltas;
            m_blurTimeKeyFrameDeltas = nullptr;
        }

        if (m_blurTimeKeyFrameValues != nullptr) {
            if (m_numBlurTimeKeyFrames > 1) {
                delete[] m_blurTimeKeyFrameValues;
                m_blurTimeKeyFrameValues = new float[1];
            }
        } else {
            m_blurTimeKeyFrameValues = new float[1];
        }

        m_numBlurTimeKeyFrames = 0;
        m_numRandomBlurTimeEntriesMinus1 = 0;
        m_blurTimeKeyFrameValues[0] = new_blur_times.Start;
    } else {
        ui_previous_key_time = 0;
        unsigned int key;

        for (key = 0; key < new_blur_times.NumKeyFrames; key++) {
            ui_current_key_time = (unsigned int)(new_blur_times.KeyTimes[key] * 1000.0f);
            captainslog_assert(ui_current_key_time > ui_previous_key_time);

            if (ui_current_key_time >= m_maxAge) {
                break;
            }

            ui_previous_key_time = ui_current_key_time;
        }
        bool blurtime_constant_at_end = (key == new_blur_times.NumKeyFrames);
        unsigned int new_num_key_frames = key + 1;

        if (new_num_key_frames != m_numBlurTimeKeyFrames) {
            if (m_blurTimeKeyFrameTimes != nullptr) {
                delete[] m_blurTimeKeyFrameTimes;
                m_blurTimeKeyFrameTimes = nullptr;
            }

            if (m_blurTimeKeyFrameValues) {
                delete[] m_blurTimeKeyFrameValues;
                m_blurTimeKeyFrameValues = nullptr;
            }

            if (m_blurTimeKeyFrameDeltas) {
                delete[] m_blurTimeKeyFrameDeltas;
                m_blurTimeKeyFrameDeltas = nullptr;
            }

            m_numBlurTimeKeyFrames = new_num_key_frames;
            m_blurTimeKeyFrameTimes = new unsigned int[m_numBlurTimeKeyFrames];
            m_blurTimeKeyFrameValues = new float[m_numBlurTimeKeyFrames];
            m_blurTimeKeyFrameDeltas = new float[m_numBlurTimeKeyFrames];
        }

        m_blurTimeKeyFrameTimes[0] = 0;
        m_blurTimeKeyFrameValues[0] = new_blur_times.Start;

        for (i = 1; i < m_numBlurTimeKeyFrames; i++) {
            unsigned int im1 = i - 1;
            m_blurTimeKeyFrameTimes[i] = (unsigned int)(new_blur_times.KeyTimes[im1] * 1000.0f);
            m_blurTimeKeyFrameValues[i] = new_blur_times.Values[im1];
        }

        for (i = 0; i < m_numBlurTimeKeyFrames - 1; i++) {
            m_blurTimeKeyFrameDeltas[i] = (m_blurTimeKeyFrameValues[i + 1] - m_blurTimeKeyFrameValues[i])
                / (float)(m_blurTimeKeyFrameTimes[i + 1] - m_blurTimeKeyFrameTimes[i]);
        }

        if (blurtime_constant_at_end) {
            m_blurTimeKeyFrameDeltas[i] = 0.0f;
        } else {
            m_blurTimeKeyFrameDeltas[i] = (new_blur_times.Values[i] - m_blurTimeKeyFrameValues[i])
                / (new_blur_times.KeyTimes[i] * 1000.0f - (float)m_blurTimeKeyFrameTimes[i]);
        }

        if (blurtime_rand_zero) {

            if (m_randomBlurTimeEntries != nullptr) {
                if (m_numRandomBlurTimeEntriesMinus1 != 0) {
                    delete[] m_randomBlurTimeEntries;
                    m_randomBlurTimeEntries = new float[1];
                }
            } else {
                m_randomBlurTimeEntries = new float[1];
            }

            m_numRandomBlurTimeEntriesMinus1 = 0;
            m_randomBlurTimeEntries[0] = 0.0f;
        } else {
            unsigned int pot_num = Find_POT(m_maxNum);
            unsigned int default_randomizer_entries = std::min(pot_num, s_maxRandomEntries);

            if (m_randomBlurTimeEntries != nullptr) {
                if (m_numRandomBlurTimeEntriesMinus1 != (default_randomizer_entries - 1)) {
                    delete[] m_randomBlurTimeEntries;
                    m_randomBlurTimeEntries = new float[default_randomizer_entries];
                }
            } else {
                m_randomBlurTimeEntries = new float[default_randomizer_entries];
            }

            m_numRandomBlurTimeEntriesMinus1 = default_randomizer_entries - 1;
            float scale = new_blur_times.Rand * oo_intmax;

            for (unsigned int j = 0; j <= m_numRandomBlurTimeEntriesMinus1; j++) {
                m_randomBlurTimeEntries[j] = s_randGen * scale;
            }
        }
    }
}
void ParticleBufferClass::Emitter_Is_Dead()
{
    m_isEmitterDead = true;
    m_emitter = nullptr;
}
void ParticleBufferClass::Set_Emitter(ParticleEmitterClass *emitter)
{
    if (m_emitter) {
        m_emitter = nullptr;
    }

    m_emitter = emitter;

    if (m_emitter) {
    }
}

NewParticleStruct *ParticleBufferClass::Add_Uninitialized_New_Particle()
{
    NewParticleStruct *ptr = &(m_newParticleQueue[m_newParticleQueueEnd]);

    if (++m_newParticleQueueEnd == m_maxNum) {
        m_newParticleQueueEnd = 0;
    }

    if (++m_newParticleQueueCount == (signed)(m_maxNum + 1)) {

        if (++m_newParticleQueueStart == m_maxNum) {
            m_newParticleQueueStart = 0;
        }

        m_newParticleQueueCount--;
    }

    return ptr;
}

void ParticleBufferClass::Update_Cached_Bounding_Volumes() const
{
    const_cast<ParticleBufferClass *>(this)->Update_Bounding_Box();
    m_cachedBoundingSphere.Init(m_boundingBox.m_center, m_boundingBox.m_extent.Length());
    m_cachedBoundingBox = m_boundingBox;
    Validate_Cached_Bounding_Volumes();
}

void ParticleBufferClass::Update_Kinematic_Particle_State()
{
    unsigned int elapsed = W3D::Get_Sync_Time() - m_lastUpdateTime;

    if (elapsed == 0) {
        return;
    }

    Get_New_Particles();
    Kill_Old_Particles();

    if (m_nonNewNum > 0) {
        Update_Non_New_Particles(elapsed);
    }

    m_end = m_newEnd;
    m_nonNewNum += m_newNum;
    m_newNum = 0;
    m_lastUpdateTime = W3D::Get_Sync_Time();
    m_boundingBoxDirty = true;
}

void ParticleBufferClass::Update_Visual_Particle_State()
{
    bool is_linegroup =
        ((m_renderMode == W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA) || (m_renderMode == W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM));

    if (m_color == nullptr && m_alpha == nullptr && m_size == nullptr && m_orientation == nullptr && m_frame == nullptr
        && m_uCoord == nullptr && !is_linegroup) {
        return;
    }

    unsigned int sub1_end;
    unsigned int sub2_start;

    if ((m_start < m_end) || ((m_start == m_end) && m_nonNewNum == 0)) {
        sub1_end = m_end;
        sub2_start = m_end;
    } else {
        sub1_end = m_maxNum;
        sub2_start = 0;
    }

    unsigned int current_time = W3D::Get_Sync_Time();
    unsigned int ckey = m_numColorKeyFrames - 1;
    unsigned int akey = m_numAlphaKeyFrames - 1;
    unsigned int skey = m_numSizeKeyFrames - 1;
    unsigned int rkey = m_numRotationKeyFrames - 1;
    unsigned int fkey = m_numFrameKeyFrames - 1;
    unsigned int bkey = m_numBlurTimeKeyFrames - 1;

    unsigned int part;
    Vector3 *color = m_color ? m_color->Get_Array() : nullptr;
    float *alpha = m_alpha ? m_alpha->Get_Array() : nullptr;
    float *size = m_size ? m_size->Get_Array() : nullptr;
    uint8_t *orientation = m_orientation ? m_orientation->Get_Array() : nullptr;
    uint8_t *frame = m_frame ? m_frame->Get_Array() : nullptr;
    float *ucoord = m_uCoord ? m_uCoord->Get_Array() : nullptr;
    Vector3 *tailposition = m_tailPosition ? m_tailPosition->Get_Array() : nullptr;
    Vector3 *position = nullptr;

    if (m_pingPongPosition) {
        int pingpong = W3D::Get_Frame_Count() & 0x1;
        position = m_position[pingpong]->Get_Array();
    } else {
        position = m_position[0]->Get_Array();
    }

    for (part = m_start; part < sub1_end; part++) {
        unsigned int part_age = current_time - m_timeStamp[part];

        if (color != nullptr) {
            for (; part_age < m_colorKeyFrameTimes[ckey]; ckey--) {
            }

            color[part] = m_colorKeyFrameValues[ckey]
                + m_colorKeyFrameDeltas[ckey] * (float)(part_age - m_colorKeyFrameTimes[ckey])
                + m_randomColorEntries[part & m_numRandomColorEntriesMinus1];
        }

        if (alpha != nullptr) {
            for (; part_age < m_alphaKeyFrameTimes[akey]; akey--) {
            }

            alpha[part] = m_alphaKeyFrameValues[akey]
                + m_alphaKeyFrameDeltas[akey] * (float)(part_age - m_alphaKeyFrameTimes[akey])
                + m_randomAlphaEntries[part & m_numRandomAlphaEntriesMinus1];
        }

        if (size != nullptr) {
            for (; part_age < m_sizeKeyFrameTimes[skey]; skey--) {
            }

            size[part] = m_sizeKeyFrameValues[skey]
                + m_sizeKeyFrameDeltas[skey] * (float)(part_age - m_sizeKeyFrameTimes[skey])
                + m_randomSizeEntries[part & m_numRandomSizeEntriesMinus1];
            size[part] = (size[part] >= 0.0f) ? size[part] : 0.0f;
        }

        if (orientation != nullptr) {
            for (; part_age < m_rotationKeyFrameTimes[rkey]; rkey--) {
            }

            float f_delta_t = (float)(part_age - m_rotationKeyFrameTimes[rkey]);
            float tmp_orient = m_orientationKeyFrameValues[rkey]
                + (m_rotationKeyFrameValues[rkey] + m_halfRotationKeyFrameDeltas[rkey] * f_delta_t) * f_delta_t
                + m_randomRotationEntries[part & m_numRandomRotationEntriesMinus1] * (float)part_age
                + m_randomOrientationEntries[part & m_numRandomOrientationEntriesMinus1];

            orientation[part] = (unsigned int)(((int)(tmp_orient * 256.0f)) & 0xFF);
        }

        if (frame != nullptr) {
            captainslog_assert(ucoord == nullptr);

            for (; part_age < m_frameKeyFrameTimes[fkey]; fkey--) {
            }

            float tmp_frame = m_frameKeyFrameValues[fkey]
                + m_frameKeyFrameDeltas[fkey] * (float)(part_age - m_frameKeyFrameTimes[fkey])
                + m_randomFrameEntries[part & m_numRandomFrameEntriesMinus1];

            frame[part] = (unsigned int)(((int)(tmp_frame)) & 0xFF);
        }

        if (ucoord != nullptr) {
            captainslog_assert(frame == nullptr);

            for (; part_age < m_frameKeyFrameTimes[fkey]; fkey--) {
            }

            ucoord[part] = m_frameKeyFrameValues[fkey]
                + m_frameKeyFrameDeltas[fkey] * (float)(part_age - m_frameKeyFrameTimes[fkey])
                + m_randomFrameEntries[part & m_numRandomFrameEntriesMinus1];
        }

        if (tailposition != nullptr) {
            float blur_time = m_blurTimeKeyFrameValues[0];

            if (m_blurTimeKeyFrameTimes != nullptr) {
                for (; part_age < m_blurTimeKeyFrameTimes[bkey]; bkey--) {
                }

                blur_time = m_blurTimeKeyFrameValues[bkey]
                    + m_blurTimeKeyFrameDeltas[bkey] * (float)(part_age - m_blurTimeKeyFrameTimes[bkey])
                    + m_randomBlurTimeEntries[part & m_numRandomBlurTimeEntriesMinus1];
            }

            tailposition[part] = position[part] - m_velocity[part] * blur_time * 1000;
        }
    }

    for (part = sub2_start; part < m_end; part++) {
        unsigned int part_age = current_time - m_timeStamp[part];

        if (color != nullptr) {
            for (; part_age < m_colorKeyFrameTimes[ckey]; ckey--) {
            }

            color[part] = m_colorKeyFrameValues[ckey]
                + m_colorKeyFrameDeltas[ckey] * (float)(part_age - m_colorKeyFrameTimes[ckey])
                + m_randomColorEntries[part & m_numRandomColorEntriesMinus1];
        }

        if (alpha != nullptr) {
            for (; part_age < m_alphaKeyFrameTimes[akey]; akey--) {
            }

            alpha[part] = m_alphaKeyFrameValues[akey]
                + m_alphaKeyFrameDeltas[akey] * (float)(part_age - m_alphaKeyFrameTimes[akey])
                + m_randomAlphaEntries[part & m_numRandomAlphaEntriesMinus1];
        }

        if (size != nullptr) {
            for (; part_age < m_sizeKeyFrameTimes[skey]; skey--) {
            }

            size[part] = m_sizeKeyFrameValues[skey]
                + m_sizeKeyFrameDeltas[skey] * (float)(part_age - m_sizeKeyFrameTimes[skey])
                + m_randomSizeEntries[part & m_numRandomSizeEntriesMinus1];
            size[part] = (size[part] >= 0.0f) ? size[part] : 0.0f;
        }

        if (orientation != nullptr) {
            for (; part_age < m_rotationKeyFrameTimes[rkey]; rkey--) {
            }

            float f_delta_t = (float)(part_age - m_rotationKeyFrameTimes[rkey]);
            float tmp_orient = m_orientationKeyFrameValues[rkey]
                + (m_rotationKeyFrameValues[rkey] + m_halfRotationKeyFrameDeltas[rkey] * f_delta_t) * f_delta_t
                + m_randomRotationEntries[part & m_numRandomRotationEntriesMinus1] * (float)part_age
                + m_randomOrientationEntries[part & m_numRandomOrientationEntriesMinus1];

            orientation[part] = (unsigned int)(((int)(tmp_orient * 256.0f)) & 0xFF);
        }

        if (frame != nullptr) {
            captainslog_assert(ucoord == nullptr);

            for (; part_age < m_frameKeyFrameTimes[fkey]; fkey--) {
            }

            float tmp_frame = m_frameKeyFrameValues[fkey]
                + m_frameKeyFrameDeltas[fkey] * (float)(part_age - m_frameKeyFrameTimes[fkey])
                + m_randomFrameEntries[part & m_numRandomFrameEntriesMinus1];

            frame[part] = (unsigned int)(((int)(tmp_frame)) & 0xFF);
        }

        if (ucoord != nullptr) {
            captainslog_assert(frame == nullptr);

            for (; part_age < m_frameKeyFrameTimes[fkey]; fkey--) {
            }

            ucoord[part] = m_frameKeyFrameValues[fkey]
                + m_frameKeyFrameDeltas[fkey] * (float)(part_age - m_frameKeyFrameTimes[fkey])
                + m_randomFrameEntries[part & m_numRandomFrameEntriesMinus1];
        }

        if (tailposition != nullptr) {
            float blur_time = m_blurTimeKeyFrameValues[0];

            if (m_blurTimeKeyFrameTimes != nullptr) {
                for (; part_age < m_blurTimeKeyFrameTimes[bkey]; bkey--) {
                }

                blur_time = m_blurTimeKeyFrameValues[bkey]
                    + m_blurTimeKeyFrameDeltas[bkey] * (float)(part_age - m_blurTimeKeyFrameTimes[bkey])
                    + m_randomBlurTimeEntries[part & m_numRandomBlurTimeEntriesMinus1];
            }

            tailposition[part] = position[part] - m_velocity[part] * blur_time * 1000;
        }
    }
}

void ParticleBufferClass::Update_Bounding_Box()
{
    Update_Kinematic_Particle_State();
    if (!m_boundingBoxDirty) {
        return;
    }

    if (m_nonNewNum == 0) {
        m_boundingBox.Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));
        m_boundingBoxDirty = false;
        return;
    }

    int pingpong = 0;

    if (m_pingPongPosition) {
        pingpong = W3D::Get_Frame_Count() & 0x1;
    }

    Vector3 *position = m_position[pingpong]->Get_Array();
    Vector3 max_coords = position[m_start];
    Vector3 min_coords = position[m_start];
    unsigned int sub1_end;
    unsigned int sub2_start;

    if ((m_start < m_end) || ((m_start == m_end) && m_nonNewNum == 0)) {
        sub1_end = m_end;
        sub2_start = m_end;
    } else {
        sub1_end = m_maxNum;
        sub2_start = 0;
    }

    for (unsigned int i = m_start; i < sub1_end; i++) {
        max_coords.X = max_coords.X >= position[i].X ? max_coords.X : position[i].X;
        max_coords.Y = max_coords.Y >= position[i].Y ? max_coords.Y : position[i].Y;
        max_coords.Z = max_coords.Z >= position[i].Z ? max_coords.Z : position[i].Z;
        min_coords.X = min_coords.X <= position[i].X ? min_coords.X : position[i].X;
        min_coords.Y = min_coords.Y <= position[i].Y ? min_coords.Y : position[i].Y;
        min_coords.Z = min_coords.Z <= position[i].Z ? min_coords.Z : position[i].Z;
    }

    for (unsigned int i = sub2_start; i < m_end; i++) {
        max_coords.X = max_coords.X >= position[i].X ? max_coords.X : position[i].X;
        max_coords.Y = max_coords.Y >= position[i].Y ? max_coords.Y : position[i].Y;
        max_coords.Z = max_coords.Z >= position[i].Z ? max_coords.Z : position[i].Z;
        min_coords.X = min_coords.X <= position[i].X ? min_coords.X : position[i].X;
        min_coords.Y = min_coords.Y <= position[i].Y ? min_coords.Y : position[i].Y;
        min_coords.Z = min_coords.Z <= position[i].Z ? min_coords.Z : position[i].Z;
    }

    Vector3 size(m_maxSize, m_maxSize, m_maxSize);
    max_coords += size;
    min_coords -= size;
    m_boundingBox.Init(MinMaxAABoxClass(min_coords, max_coords));
    m_boundingBoxDirty = false;
}

void ParticleBufferClass::Get_New_Particles()
{
    unsigned int current_time = W3D::Get_Sync_Time();
    Vector3 *position;
    Vector3 *prev_pos;

    if (m_pingPongPosition) {
        int pingpong = W3D::Get_Frame_Count() & 0x1;
        position = m_position[pingpong]->Get_Array();
        prev_pos = m_position[pingpong ^ 0x1]->Get_Array();
    } else {
        position = m_position[0]->Get_Array();
        prev_pos = nullptr;
    }

    for (; m_newParticleQueueCount;) {
        NewParticleStruct &new_particle = m_newParticleQueue[m_newParticleQueueStart];

        if (++m_newParticleQueueStart == m_maxNum) {
            m_newParticleQueueStart = 0;
        }

        unsigned char *id = m_groupID->Get_Array();

        m_newParticleQueueCount--;
        m_timeStamp[m_newEnd] = new_particle.m_timeStamp;
        unsigned int age = current_time - m_timeStamp[m_newEnd];

        if (age >= m_maxAge) {
            continue;
        }

        float fp_age = (float)age;

        if (m_hasAccel) {
            position[m_newEnd] = new_particle.m_position + (new_particle.m_velocity + 0.5f * m_accel * fp_age) * fp_age;
            m_velocity[m_newEnd] = new_particle.m_velocity + (m_accel * fp_age);
        } else {
            position[m_newEnd] = new_particle.m_position + (new_particle.m_velocity * fp_age);
            m_velocity[m_newEnd] = new_particle.m_velocity;
        }

        if (m_pingPongPosition) {
            prev_pos[m_newEnd] = new_particle.m_position;
        }

        id[m_newEnd] = new_particle.m_groupID;
        m_newEnd++;

        if (m_newEnd == m_maxNum) {
            m_newEnd = 0;
        }

        m_newNum++;

        if ((m_newNum + m_nonNewNum) == (signed)(m_maxNum + 1)) {
            m_start++;

            if (m_start == m_maxNum) {
                m_start = 0;
            }

            m_nonNewNum--;

            if (m_nonNewNum == -1) {
                m_end++;

                if (m_end == m_maxNum) {
                    m_end = 0;
                }

                m_nonNewNum = 0;
                m_newNum--;
            }
        }
    }
}

void ParticleBufferClass::Kill_Old_Particles()
{
    unsigned int sub1_end;
    unsigned int sub2_start;
    unsigned int i;

    if ((m_start < m_end) || ((m_start == m_end) && m_nonNewNum == 0)) {
        sub1_end = m_end;
        sub2_start = m_end;
    } else {
        sub1_end = m_maxNum;
        sub2_start = 0;
    }

    unsigned int current_time = W3D::Get_Sync_Time();
    bool broke = false;

    for (i = m_start; i < sub1_end; i++) {
        if ((current_time - m_timeStamp[i]) < m_maxAge) {
            broke = true;
            break;
        }

        m_nonNewNum--;
    }

    if (!broke) {
        for (i = sub2_start; i < m_end; i++) {
            if ((current_time - m_timeStamp[i]) < m_maxAge) {
                break;
            }

            m_nonNewNum--;
        }
    }

    m_start = i;
}

void ParticleBufferClass::Update_Non_New_Particles(unsigned int elapsed)
{
    unsigned int sub1_end;
    unsigned int sub2_start;

    if ((m_start < m_end) || ((m_start == m_end) && m_nonNewNum == 0)) {
        sub1_end = m_end;
        sub2_start = m_end;
    } else {
        sub1_end = m_maxNum;
        sub2_start = 0;
    }

    float fp_elapsed_time = (float)elapsed;

    if (m_pingPongPosition) {

        int pingpong = W3D::Get_Frame_Count() & 0x1;
        Vector3 *position = m_position[pingpong]->Get_Array();
        Vector3 *prev_pos = m_position[pingpong ^ 0x1]->Get_Array();

        if (m_hasAccel) {
            Vector3 delta_v = m_accel * fp_elapsed_time;
            Vector3 accel_p = m_accel * (0.5f * fp_elapsed_time * fp_elapsed_time);

            for (unsigned int i = m_start; i < sub1_end; i++) {
                position[i] = prev_pos[i] + m_velocity[i] * fp_elapsed_time + accel_p;
                m_velocity[i] += delta_v;
            }

            for (unsigned int i = sub2_start; i < m_end; i++) {
                position[i] = prev_pos[i] + m_velocity[i] * fp_elapsed_time + accel_p;
                m_velocity[i] += delta_v;
            }
        } else {
            for (unsigned int i = m_start; i < sub1_end; i++) {
                position[i] += m_velocity[i] * fp_elapsed_time;
            }

            for (unsigned int i = sub2_start; i < m_end; i++) {
                position[i] += m_velocity[i] * fp_elapsed_time;
            }
        }
    } else {

        Vector3 *position = m_position[0]->Get_Array();

        if (m_hasAccel) {
            Vector3 delta_v = m_accel * fp_elapsed_time;
            Vector3 accel_p = m_accel * (0.5f * fp_elapsed_time * fp_elapsed_time);

            for (unsigned int i = m_start; i < sub1_end; i++) {
                position[i] += m_velocity[i] * fp_elapsed_time + accel_p;
                m_velocity[i] += delta_v;
            }

            for (unsigned int i = sub2_start; i < m_end; i++) {
                position[i] += m_velocity[i] * fp_elapsed_time + accel_p;
                m_velocity[i] += delta_v;
            }
        } else {
            for (unsigned int i = m_start; i < sub1_end; i++) {
                position[i] += m_velocity[i] * fp_elapsed_time;
            }

            for (unsigned int i = sub2_start; i < m_end; i++) {
                position[i] += m_velocity[i] * fp_elapsed_time;
            }
        }
    }
}

void ParticleBufferClass::Get_Color_Key_Frames(ParticlePropertyStruct<Vector3> &colors) const
{
    int real_keyframe_count = (m_numColorKeyFrames > 0) ? (m_numColorKeyFrames - 1) : 0;
    bool create_last_keyframe = false;

    if ((m_colorKeyFrameDeltas != nullptr)
        && ((m_colorKeyFrameDeltas[m_numColorKeyFrames - 1].X != 0)
            || (m_colorKeyFrameDeltas[m_numColorKeyFrames - 1].Y != 0)
            || (m_colorKeyFrameDeltas[m_numColorKeyFrames - 1].Z != 0))) {
        real_keyframe_count++;
        create_last_keyframe = true;
    }

    colors.Start = m_colorKeyFrameValues[0];
    colors.Rand = m_colorRandom;
    colors.NumKeyFrames = real_keyframe_count;
    colors.KeyTimes = nullptr;
    colors.Values = nullptr;

    if (real_keyframe_count > 0) {
        colors.KeyTimes = new float[real_keyframe_count];
        colors.Values = new Vector3[real_keyframe_count];
        unsigned int index;

        for (index = 1; index < m_numColorKeyFrames; index++) {
            colors.KeyTimes[index - 1] = ((float)m_colorKeyFrameTimes[index]) / 1000;
            colors.Values[index - 1] = m_colorKeyFrameValues[index];
        }

        if (create_last_keyframe) {
            colors.KeyTimes[index - 1] = ((float)m_maxAge / 1000);
            Vector3 start_color = m_colorKeyFrameValues[index - 1];
            Vector3 &delta = m_colorKeyFrameDeltas[m_numColorKeyFrames - 1];
            float time_delta = m_maxAge - m_colorKeyFrameTimes[index - 1];
            colors.Values[index - 1] = start_color + (delta * time_delta);
        }
    }
}

void ParticleBufferClass::Get_Opacity_Key_Frames(ParticlePropertyStruct<float> &opacities) const
{
    int real_keyframe_count = (m_numAlphaKeyFrames > 0) ? (m_numAlphaKeyFrames - 1) : 0;
    bool create_last_keyframe = false;

    if ((m_alphaKeyFrameDeltas != nullptr) && (m_alphaKeyFrameDeltas[m_numAlphaKeyFrames - 1] != 0)) {
        real_keyframe_count++;
        create_last_keyframe = true;
    }

    opacities.Start = m_alphaKeyFrameValues[0];
    opacities.Rand = m_opacityRandom;
    opacities.NumKeyFrames = real_keyframe_count;
    opacities.KeyTimes = nullptr;
    opacities.Values = nullptr;

    if (real_keyframe_count > 0) {
        opacities.KeyTimes = new float[real_keyframe_count];
        opacities.Values = new float[real_keyframe_count];
        unsigned int index;

        for (index = 1; index < m_numAlphaKeyFrames; index++) {
            opacities.KeyTimes[index - 1] = ((float)m_alphaKeyFrameTimes[index]) / 1000;
            opacities.Values[index - 1] = m_alphaKeyFrameValues[index];
        }

        if (create_last_keyframe) {
            opacities.KeyTimes[index - 1] = ((float)m_maxAge / 1000);
            float start_alpha = m_alphaKeyFrameValues[index - 1];
            float &delta = m_alphaKeyFrameDeltas[m_numAlphaKeyFrames - 1];
            float time_delta = m_maxAge - m_alphaKeyFrameTimes[index - 1];
            opacities.Values[index - 1] = start_alpha + (delta * time_delta);
        }
    }
}

void ParticleBufferClass::Get_Size_Key_Frames(ParticlePropertyStruct<float> &sizes) const
{
    int real_keyframe_count = (m_numSizeKeyFrames > 0) ? (m_numSizeKeyFrames - 1) : 0;
    bool create_last_keyframe = false;

    if ((m_sizeKeyFrameDeltas != nullptr) && (m_sizeKeyFrameDeltas[m_numSizeKeyFrames - 1] != 0)) {
        real_keyframe_count++;
        create_last_keyframe = true;
    }

    sizes.Start = m_sizeKeyFrameValues[0];
    sizes.Rand = m_sizeRandom;
    sizes.NumKeyFrames = real_keyframe_count;
    sizes.KeyTimes = nullptr;
    sizes.Values = nullptr;

    if (real_keyframe_count > 0) {
        sizes.KeyTimes = new float[real_keyframe_count];
        sizes.Values = new float[real_keyframe_count];
        unsigned int index;

        for (index = 1; index < m_numSizeKeyFrames; index++) {
            sizes.KeyTimes[index - 1] = ((float)m_sizeKeyFrameTimes[index]) / 1000;
            sizes.Values[index - 1] = m_sizeKeyFrameValues[index];
        }

        if (create_last_keyframe) {
            sizes.KeyTimes[index - 1] = ((float)m_maxAge / 1000);
            float start_size = m_sizeKeyFrameValues[index - 1];
            float &delta = m_sizeKeyFrameDeltas[m_numSizeKeyFrames - 1];
            float time_delta = m_maxAge - m_sizeKeyFrameTimes[index - 1];
            sizes.Values[index - 1] = start_size + (delta * time_delta);
        }
    }
}

void ParticleBufferClass::Get_Rotation_Key_Frames(ParticlePropertyStruct<float> &rotations) const
{
    int real_keyframe_count = (m_numRotationKeyFrames > 0) ? (m_numRotationKeyFrames - 1) : 0;
    bool create_last_keyframe = false;

    if ((m_halfRotationKeyFrameDeltas != nullptr) && (m_halfRotationKeyFrameDeltas[m_numRotationKeyFrames - 1] != 0)) {
        real_keyframe_count++;
        create_last_keyframe = true;
    }

    rotations.Start = m_rotationKeyFrameValues ? m_rotationKeyFrameValues[0] * 1000.0f : 0;
    rotations.Rand = m_rotationRandom * 1000.0f;
    rotations.NumKeyFrames = real_keyframe_count;
    rotations.KeyTimes = nullptr;
    rotations.Values = nullptr;
    if (real_keyframe_count > 0) {
        rotations.KeyTimes = new float[real_keyframe_count];
        rotations.Values = new float[real_keyframe_count];
        unsigned int index;

        for (index = 1; index < m_numRotationKeyFrames; index++) {
            rotations.KeyTimes[index - 1] = ((float)m_rotationKeyFrameTimes[index]) / 1000;
            rotations.Values[index - 1] = m_rotationKeyFrameValues[index] * 1000.0f;
        }

        if (create_last_keyframe) {
            rotations.KeyTimes[index - 1] = ((float)m_maxAge / 1000);
            float start_rotation = m_rotationKeyFrameValues[index - 1];
            float delta = 2.0f * m_halfRotationKeyFrameDeltas[m_numRotationKeyFrames - 1];
            float time_delta = m_maxAge - m_rotationKeyFrameTimes[index - 1];
            rotations.Values[index - 1] = (start_rotation + (delta * time_delta)) * 1000.0f;
        }
    }
}

void ParticleBufferClass::Get_Frame_Key_Frames(ParticlePropertyStruct<float> &frames) const
{
    int real_keyframe_count = (m_numFrameKeyFrames > 0) ? (m_numFrameKeyFrames - 1) : 0;
    bool create_last_keyframe = false;

    if ((m_frameKeyFrameDeltas != nullptr) && (m_frameKeyFrameDeltas[m_numFrameKeyFrames - 1] != 0)) {
        real_keyframe_count++;
        create_last_keyframe = true;
    }

    frames.Start = m_frameKeyFrameValues[0];
    frames.Rand = m_frameRandom;
    frames.NumKeyFrames = real_keyframe_count;
    frames.KeyTimes = nullptr;
    frames.Values = nullptr;

    if (real_keyframe_count > 0) {
        frames.KeyTimes = new float[real_keyframe_count];
        frames.Values = new float[real_keyframe_count];
        unsigned int index;

        for (index = 1; index < m_numFrameKeyFrames; index++) {
            frames.KeyTimes[index - 1] = ((float)m_frameKeyFrameTimes[index]) / 1000;
            frames.Values[index - 1] = m_frameKeyFrameValues[index];
        }

        if (create_last_keyframe) {
            frames.KeyTimes[index - 1] = ((float)m_maxAge / 1000);
            float start_frame = m_frameKeyFrameValues[index - 1];
            float &delta = m_frameKeyFrameDeltas[m_numFrameKeyFrames - 1];
            float time_delta = m_maxAge - m_frameKeyFrameTimes[index - 1];
            frames.Values[index - 1] = start_frame + (delta * time_delta);
        }
    }
}

void ParticleBufferClass::Get_Blur_Time_Key_Frames(ParticlePropertyStruct<float> &blurtimes) const
{
    int real_keyframe_count = (m_numBlurTimeKeyFrames > 0) ? (m_numBlurTimeKeyFrames - 1) : 0;
    bool create_last_keyframe = false;

    if ((m_blurTimeKeyFrameDeltas != nullptr) && (m_blurTimeKeyFrameDeltas[m_numBlurTimeKeyFrames - 1] != 0)) {
        real_keyframe_count++;
        create_last_keyframe = true;
    }

    blurtimes.Start = m_blurTimeKeyFrameValues[0];
    blurtimes.Rand = m_blurTimeRandom;
    blurtimes.NumKeyFrames = real_keyframe_count;
    blurtimes.KeyTimes = nullptr;
    blurtimes.Values = nullptr;

    if (real_keyframe_count > 0) {
        blurtimes.KeyTimes = new float[real_keyframe_count];
        blurtimes.Values = new float[real_keyframe_count];
        unsigned int index;

        for (index = 1; index < m_numBlurTimeKeyFrames; index++) {
            blurtimes.KeyTimes[index - 1] = ((float)m_blurTimeKeyFrameTimes[index]) / 1000;
            blurtimes.Values[index - 1] = m_blurTimeKeyFrameValues[index];
        }

        if (create_last_keyframe) {
            blurtimes.KeyTimes[index - 1] = ((float)m_maxAge / 1000);
            float start_blurtime = m_blurTimeKeyFrameValues[index - 1];
            float &delta = m_blurTimeKeyFrameDeltas[m_numBlurTimeKeyFrames - 1];
            float time_delta = m_maxAge - m_blurTimeKeyFrameTimes[index - 1];
            blurtimes.Values[index - 1] = start_blurtime + (delta * time_delta);
        }
    }
}

void ParticleBufferClass::Set_LOD_Max_Screen_Size(int lod_level, float max_screen_size)
{
    if ((lod_level < 0) || (lod_level > 17)) {
        return;
    }

    s_LODMaxScreenSizes[lod_level] = max_screen_size;
}

float ParticleBufferClass::Get_LOD_Max_Screen_Size(int lod_level)
{
    if ((lod_level < 0) || (lod_level > 17)) {
        return NO_MAX_SCREEN_SIZE;
    }

    return s_LODMaxScreenSizes[lod_level];
}

int ParticleBufferClass::Get_Line_Texture_Mapping_Mode() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Get_Texture_Mapping_Mode();
    }

    return SegLineRendererClass::UNIFORM_WIDTH_TEXTURE_MAP;
}

int ParticleBufferClass::Is_Merge_Intersections() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Is_Merge_Intersections();
    }

    return false;
}

int ParticleBufferClass::Is_Freeze_Random() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Is_Freeze_Random();
    }

    return false;
}

int ParticleBufferClass::Is_Sorting_Disabled() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Is_Sorting_Disabled();
    }

    return false;
}

int ParticleBufferClass::Are_End_Caps_Enabled() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Are_End_Caps_Enabled();
    }

    return false;
}

int ParticleBufferClass::Get_Subdivision_Level() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Get_Current_Subdivision_Level();
    }

    return 0;
}

float ParticleBufferClass::Get_Noise_Amplitude() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Get_Noise_Amplitude();
    }

    return 0.0f;
}

float ParticleBufferClass::Get_Merge_Abort_Factor() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Get_Merge_Abort_Factor();
    }

    return 0.0f;
}

float ParticleBufferClass::Get_Texture_Tile_Factor() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Get_Texture_Tile_Factor();
    }

    return 1.0f;
}

Vector2 ParticleBufferClass::Get_UV_Offset_Rate() const
{
    if (m_lineRenderer != nullptr) {
        return m_lineRenderer->Get_UV_Offset_Rate();
    }

    return Vector2(0.0f, 0.0f);
}

ParticleBufferClass::TailDiffuseTypeEnum ParticleBufferClass::Determine_Tail_Diffuse()
{
    TextureClass *tex = Get_Texture();

    if (tex) {
        Ref_Ptr_Release(tex);
        return SAME_AS_HEAD;
    }

    ShaderClass shader = Get_Shader();

    if (shader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_SRC_COLOR) {
        return WHITE;
    } else if ((shader.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE)
        && (shader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE)) {
        return BLACK;
    } else if ((shader.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE)
        && (shader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE_MINUS_SRC_COLOR)) {
        return BLACK;
    } else if ((shader.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_SRC_ALPHA)
        && (shader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA)) {
        return SAME_AS_HEAD_ALPHA_ZERO;
    } else if (shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_ENABLE) {
        return SAME_AS_HEAD_ALPHA_ZERO;
    }

    return SAME_AS_HEAD;
}

TextureClass *ParticleBufferClass::Get_Texture() const
{
    if (m_pointGroup) {
        return m_pointGroup->Get_Texture();
    } else if (m_lineGroup) {
        return m_lineGroup->Get_Texture();
    } else if (m_lineRenderer) {
        return m_lineRenderer->Get_Texture();
    }

    return nullptr;
}

void ParticleBufferClass::Set_Texture(TextureClass *tex)
{
    if (m_pointGroup) {
        m_pointGroup->Set_Texture(tex);
    } else if (m_lineGroup) {
        m_lineGroup->Set_Texture(tex);
    } else if (m_lineRenderer) {
        m_lineRenderer->Set_Texture(tex);
    }
}

ShaderClass ParticleBufferClass::Get_Shader() const
{
    if (m_pointGroup) {
        return m_pointGroup->Get_Shader();
    } else if (m_lineGroup) {
        return m_lineGroup->Get_Shader();
    } else if (m_lineRenderer) {
        return m_lineRenderer->Get_Shader();
    }

    captainslog_assert(0);
    return ShaderClass::s_presetOpaqueShader;
}
