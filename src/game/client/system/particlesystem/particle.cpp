/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object holding a single particle instance.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particle.h"
#include "drawable.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "object.h"
#include "particlesys.h"
#include "particlesysmanager.h"
#include "xfer.h"
#include <captainslog.h>

/**
 * 0x004CCC30
 */
Particle::Particle(ParticleSystem *system, const ParticleInfo &info) :
    m_systemNext(nullptr),
    m_systemPrev(nullptr),
    m_overallNext(nullptr),
    m_overallPrev(nullptr),
    m_system(system),
    m_particleID(0),
    m_lifetimeLeft(info.m_lifetime),
    m_createTimestamp(g_theGameClient->Get_Frame()),
    m_alpha(info.m_alphaKey[0].value),
    m_alphaTargetKey(true),
    m_color(info.m_colorKey[0].color),
    m_colorTargetKey(true),
    m_drawable(false),
    m_inSystemList(false),
    m_inOverallList(false),
    m_systemUnderControl(nullptr)
{
    m_accel.x = 0.0f;
    m_accel.y = 0.0f;
    m_accel.z = 0.0f;
    m_lastPos.Zero();
    m_vel = info.m_vel;
    m_pos = info.m_pos;
    m_emitterPos = info.m_emitterPos;
    m_velDamping = info.m_velDamping;
#ifndef GAME_DLL
    m_angleX = info.m_angleX;
    m_angleY = info.m_angleY;
#endif
    m_angleZ = info.m_angleZ;
#ifndef GAME_DLL
    m_angularRateX = info.m_angularRateX;
    m_angularRateY = info.m_angularRateY;
#endif
    m_angularRateZ = info.m_angularRateZ;
    m_angularDamping = info.m_angularDamping;
    m_lifetime = info.m_lifetime;
    m_size = info.m_size;
    m_sizeRate = info.m_sizeRate;
    m_sizeRateDamping = info.m_sizeRateDamping;
    m_colorScale = info.m_colorScale;
    m_windRandomness = info.m_windRandomness;
    m_particleUpTowardsEmitter = info.m_particleUpTowardsEmitter;

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = info.m_alphaKey[i];
    }

    Compute_Alpha_Rate();

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_colorKey[i] = info.m_colorKey[i];
    }

    Compute_Color_Rate();
    g_theParticleSystemManager->Add_Particle(this, m_system->Get_Priority());
    m_system->Add_Particle(this);
}

/**
 * 0x004CD040
 */
Particle::~Particle()
{
    m_system->Remove_Particle(this);

    if (m_systemUnderControl != nullptr) {
        m_systemUnderControl->Detach_Control_Particle();
        m_systemUnderControl->Destroy();
    }

    m_systemUnderControl = nullptr;
    g_theParticleSystemManager->Remove_Particle(this);
}

/**
 * @brief Transfer the object data.
 *
 * 0x004CD2E0
 */
void Particle::Xfer_Snapshot(Xfer *xfer)
{
#define PARTICLE_XFER_VERSION 1
    uint8_t version = PARTICLE_XFER_VERSION;
    xfer->xferVersion(&version, PARTICLE_XFER_VERSION);
    ParticleInfo::Xfer_Snapshot(xfer);
    xfer->xferUnsignedInt(&m_particleID);
    xfer->xferCoord3D(&m_accel);
    xfer->xferCoord3D(&m_lastPos);
    xfer->xferUnsignedInt(&m_lifetimeLeft);
    xfer->xferUnsignedInt(&m_createTimestamp);
    xfer->xferReal(&m_alpha);
    xfer->xferReal(&m_alphaRate);
    xfer->xferInt(&m_alphaTargetKey);
    xfer->xferRGBColor(&m_color);
    xfer->xferRGBColor(&m_colorRate);
    xfer->xferInt(&m_colorTargetKey);

    DrawableID drawable_id_fake =
        INVALID_DRAWABLE_ID; // Looks like this was removed and a fake entry added to avoid version bump.
    xfer->xferDrawableID(&drawable_id_fake);

    // This looks like it will save/process the value, but not restore it?
    ParticleSystemID id = m_systemUnderControl != nullptr ? m_systemUnderControl->Get_System_ID() : PARTSYS_ID_NONE;
    xfer->xferInt(reinterpret_cast<int32_t *>(&id));
}

/**
 * @brief Performs additional post data load house keeping.
 *
 * 0x004CD3F0
 */
void Particle::Load_Post_Process()
{
    // When is the system ID loaded to the memory for the pointer? Some unswizzling scheme?
    if (m_systemUnderControl != nullptr) {
        ParticleSystemID system_id = ParticleSystemID(reinterpret_cast<uintptr_t>(m_systemUnderControl));
        ParticleSystem *system = g_theParticleSystemManager->Find_Particle_System(system_id);
        system->Set_Control_Particle(this);
        Control_Particle_System(system);
        captainslog_relassert(m_systemUnderControl != nullptr,
            6,
            "Particle::Load_Post_Process - Unable to find system under control pointer",
            system_id);
    }
}

/**
 * @brief Computes rate based on current target key.
 *
 * 0x004CCB50
 */
void Particle::Compute_Alpha_Rate()
{
    if (m_alphaKey[m_alphaTargetKey].frame != 0) {
        float val_diff = m_alphaKey[m_alphaTargetKey].value - m_alphaKey[m_alphaTargetKey - 1].value;
        float frame_diff = float(int(m_alphaKey[m_alphaTargetKey].frame - m_alphaKey[m_alphaTargetKey - 1].frame));
        m_alphaRate = val_diff / frame_diff;
    } else {
        m_alphaRate = 0.0f;
    }
}

/**
 * @brief Computes rate based on current target key.
 *
 * 0x004CCBA0
 */
void Particle::Compute_Color_Rate()
{
    if (m_colorKey[m_colorTargetKey].frame != 0) {
        float frame_diff = float(int(m_colorKey[m_colorTargetKey].frame - m_colorKey[m_colorTargetKey - 1].frame));
        m_colorRate.red =
            float(m_colorKey[m_colorTargetKey].color.red - m_colorKey[m_colorTargetKey - 1].color.red) / frame_diff;
        m_colorRate.green =
            float(m_colorKey[m_colorTargetKey].color.green - m_colorKey[m_colorTargetKey - 1].color.green) / frame_diff;
        m_colorRate.blue =
            float(m_colorKey[m_colorTargetKey].color.blue - m_colorKey[m_colorTargetKey - 1].color.blue) / frame_diff;
    } else {
        m_colorRate = { 0.0f, 0.0f, 0.0f };
    }
}

/**
 * @brief Applies some force to the accelration vector.
 */
void Particle::Apply_Force(const Coord3D &force)
{
    m_accel += force;
}

/**
 * @brief Applies wind motion logic to the particle.
 *
 * 0x004CD160
 */
void Particle::Do_Wind_Motion()
{
    float wind_angle = m_system->Get_Wind_Angle();
    Coord3D system_pos;
    m_system->Get_Position(&system_pos);
    ObjectID object_id = m_system->Get_Attached_Object();

    if (object_id != INVALID_OBJECT_ID) {
        Object *obj = g_theGameLogic->Find_Object_By_ID(object_id);

        if (obj != nullptr) {
            system_pos += *obj->Get_Position();
        }
    } else {
        DrawableID drawable_id = m_system->Get_Attached_Drawable();

        if (drawable_id != INVALID_DRAWABLE_ID) {
            Drawable *drawable = g_theGameClient->Find_Drawable_By_ID(drawable_id);

            if (drawable != nullptr) {
                system_pos += *drawable->Get_Position();
            }
        }
    }

    Coord3D coords = m_pos - system_pos;
    float dist_from_wind = coords.Length();

    if (dist_from_wind < 200.0f) {
        float wind_force_strength = 2.0f * m_windRandomness;

        if (dist_from_wind > 75.0f) {
            wind_force_strength = (1.0f - (dist_from_wind - 75.0f) / (200.0f - 75.0f)) * wind_force_strength;
        }

        m_pos.x += GameMath::Cos(wind_angle) * wind_force_strength;
        m_pos.y += GameMath::Sin(wind_angle) * wind_force_strength;
    }
}

/**
 * @brief Gets the priority of the system this particle belongs to.
 */
ParticlePriorityType Particle::Get_Priority() const
{
    return m_system->Get_Priority();
}

float Angle_Between(const Coord2D *veca, const Coord2D *vecb)
{
    if (veca == nullptr || veca->Length() == 0.0f || vecb == nullptr || vecb->Length() == 0.0f) {
        return 0.0f;
    }

    float lena = veca->Length();
    float lenb = vecb->Length();

    float len = veca->x * vecb->x + veca->y * vecb->y;

    if (len == 0.0f) {
        if (vecb->x <= 0.0f) {
            return 0.0f;
        } else {
            return GAMEMATH_PI;
        }
    } else {
        float cos = GameMath::Acos(len / (lena * lenb));

        if (vecb->x <= 0.0f) {
            return -cos;
        } else {
            return cos;
        }
    }
}

bool Particle::Update()
{
    m_vel += m_accel;
    m_vel *= m_velDamping;
    m_pos += m_vel + *m_system->Get_Drift_Velocity();

    if (m_system->Get_Wind_Motion() != ParticleSystemInfo::WIND_MOTION_UNUSED) {
        Do_Wind_Motion();
    }

    m_angleZ += m_angularRateZ;
    m_angularRateZ *= m_angularDamping;

    if (m_particleUpTowardsEmitter) {
        Coord2D coord_2d;
        coord_2d.x = m_pos.x - m_emitterPos.x;
        coord_2d.y = m_pos.y - m_emitterPos.y;
        static Coord2D upVec{ 0.0f, 1.0f };
        m_angleZ = Angle_Between(&upVec, &coord_2d) + GAMEMATH_PI;
    }

    m_size += m_sizeRate;
    m_sizeRate *= m_sizeRateDamping;

    if (m_system->Get_Shader_Type() != ParticleSystemInfo::PARTICLE_SHADER_ADDITIVE) {
        m_alpha += m_alphaRate;

        if (m_alphaTargetKey < KEYFRAME_COUNT && m_alphaKey[m_alphaTargetKey].frame != 0) {
            if (g_theGameClient->Get_Frame() - m_createTimestamp >= m_alphaKey[m_alphaTargetKey].frame) {
                m_alpha = m_alphaKey[m_alphaTargetKey++].value;
                Compute_Alpha_Rate();
            }
        } else {
            m_alphaRate = 0.0f;
        }

        if (m_alpha >= 0.0f) {
            if (m_alpha > 1.0f) {
                m_alpha = 1.0f;
            }
        } else {
            m_alpha = 0.0f;
        }
    }

    m_color.red += m_colorRate.red;
    m_color.green += m_colorRate.green;
    m_color.blue += m_colorRate.blue;

    if (m_colorTargetKey < KEYFRAME_COUNT && m_colorKey[m_colorTargetKey].frame != 0) {
        if (g_theGameClient->Get_Frame() - m_createTimestamp >= m_colorKey[m_colorTargetKey].frame) {
            m_colorTargetKey++;
            Compute_Color_Rate();
        }
    } else {
        m_colorRate.red = 0.0f;
        m_colorRate.green = 0.0f;
        m_colorRate.blue = 0.0f;
    }

    m_color.red += m_colorScale;
    m_color.green += m_colorScale;
    m_color.blue += m_colorScale;

    if (m_color.red >= 0.0f) {
        if (m_color.red > 1.0f) {
            m_color.red = 1.0f;
        }
    } else {
        m_color.red = 0.0f;
    }

    if (m_color.red >= 0.0f) {
        if (m_color.green > 1.0f) {
            m_color.green = 1.0f;
        }
    } else {
        m_color.green = 0.0f;
    }

    if (m_color.blue >= 0.0f) {
        if (m_color.blue > 1.0f) {
            m_color.blue = 1.0f;
        }
    } else {
        m_color.blue = 0.0f;
    }

    m_accel.x = 0.0f;
    m_accel.y = 0.0f;
    m_accel.z = 0.0f;

    if (m_lifetimeLeft != 0) {
        if (--m_lifetimeLeft == 0) {
            return false;
        }
    }

    captainslog_dbgassert(m_lifetimeLeft != 0, "A particle has an infinite lifetime...");
    return !Is_Invisible();
}

bool Particle::Is_Invisible()
{
    switch (m_system->Get_Shader_Type()) {
        case ParticleSystemInfo::PARTICLE_SHADER_ADDITIVE:
            return m_colorKey[m_colorTargetKey].frame == 0 && m_color.red + m_color.green + m_color.blue <= 0.059999999f;
        case ParticleSystemInfo::PARTICLE_SHADER_ALPHA:
            return m_alpha < 0.02f;
        case ParticleSystemInfo::PARTICLE_SHADER_ALPHA_TEST:
            return false;
        case ParticleSystemInfo::PARTICLE_SHADER_MULTIPLY:
            return m_colorKey[m_colorTargetKey].frame == 0 && m_color.red * m_color.green * m_color.blue > 0.94999999f;
        default:
            return true;
    }
}
