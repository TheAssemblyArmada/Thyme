/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object holding a particle system instance.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamelod.h"
#include "gamemath.h"
#include "globaldata.h"
#include "minmax.h"
#include "particle.h"
#include "particlesys.h"
#include "particlesystemplate.h"
#include "randomvalue.h"
#include "xfer.h"

using GameMath::Cos;
using GameMath::Sin;
using GameMath::Sqrt;

ParticleSystem::ParticleSystem(const ParticleSystemTemplate &temp, ParticleSystemID id, bool create_slaves) :
    m_systemParticlesHead(nullptr),
    m_systemParticlesTail(nullptr),
    m_particleCount(),
    m_systemID(id),
    m_attachedToDrawableID(DRAWABLE_UNK),
    m_attachedToObjectID(OBJECT_UNK),
    m_localTransform(true),
    m_transform(true),
    m_burstDelayLeft(),
    m_delayLeft(),
    m_startTimestamp(),
    m_systemLifetimeLeft(),
    m_nextParticleIDMaybe(),
    m_accumulatedSizeBonus(0.0f),
    m_velCoefficient(1.0f, 1.0f, 1.0f),
    m_countCoefficient(1.0f),
    m_delayCoefficient(1.0f),
    m_sizeCoefficient(1.0f),
    m_pos(),
    m_lastPos(),
    m_slaveSystem(nullptr),
    m_slaveID(PARTSYS_ID_NONE),
    m_masterSystem(nullptr),
    m_masterID(PARTSYS_ID_NONE),
    m_template(&temp),
    m_controlParticle(),
    m_isLocalIdentity(true),
    m_isIdentity(true),
    m_isForever(false),
    m_isStopped(false),
    m_isDestroyed(false),
    m_isFirstPos(true),
    m_saveable(true),
    m_unkBool1(false)
{
    m_slavePosOffset = temp.m_slavePosOffset;
    m_unkValue = 0;
    m_driftVelocity = temp.m_driftVelocity;
    m_gravity = temp.m_gravity;
    m_lifetime = temp.m_lifetime;
    m_startSize = temp.m_startSize;
    m_startSizeRate = temp.m_startSizeRate;
    m_sizeRate = temp.m_sizeRate;
    m_sizeRateDamping = temp.m_sizeRateDamping;

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = temp.m_alphaKey[i];
        m_colorKey[i] = temp.m_colorKey[i];
    }

    m_colorScale.Set_Range(
        temp.m_colorScale.Get_Min() / 255.0f, temp.m_colorScale.Get_Max() / 255.0f, GameClientRandomVariable::UNIFORM);
    m_burstDelay = temp.m_burstDelay;
    m_burstDelayLeft = 0;
    m_burstCount = temp.m_burstCount;
    m_isOneShot = temp.m_isOneShot;
    m_delayLeft = m_initialDelay;
    /*m_startTimestamp = g_theGameClient->*/ // TODO Needs GameClient
    m_systemLifetimeLeft = temp.m_systemLifetime;
    m_isForever = m_systemLifetime == 0;
    m_velDamping = temp.m_velDamping;
#ifdef THYME_STANDALONE
    m_angleX = temp.m_angleX;
    m_angularRateX = temp.m_angularRateX;
    m_angleY = temp.m_angleY;
    m_angularRateY = temp.m_angularRateY;
#endif
    m_angleZ = temp.m_angleZ;
    m_angularRateZ = temp.m_angularRateZ;
    m_angularDamping = temp.m_angularDamping;
    m_priority = temp.m_priority;
    m_emissionVelocityType = temp.m_emissionVelocityType;
    memcpy(&m_emissionVelocity, &temp.m_emissionVelocity, sizeof(m_emissionVelocity));
    m_emissionVolumeType = temp.m_emissionVolumeType;
    memcpy(&m_emissionVolume, &temp.m_emissionVolume, sizeof(m_emissionVelocity));
    m_isEmissionVolumeHollow = temp.m_isEmissionVolumeHollow;
    m_isGroundAligned = temp.m_isGroundAligned;
    m_isEmitAboveGroundOnly = temp.m_isEmitAboveGroundOnly;
    m_isParticleUpTowardsEmitter = temp.m_isParticleUpTowardsEmitter;
    m_windMotion = temp.m_windMotion;
    m_windAngleChange = temp.m_windAngleChange;
    m_windAngleChangeMin = temp.m_windAngleChangeMin;
    m_windAngleChangeMax = temp.m_windAngleChangeMax;
    m_windMotionStartAngleMin = temp.m_windMotionStartAngleMin;
    m_windMotionStartAngleMax = temp.m_windMotionStartAngleMax;
    m_windMotionEndAngleMin = temp.m_windMotionEndAngleMin;
    m_windMotionEndAngleMax = temp.m_windMotionEndAngleMax;
    m_windMotionMovingToEndAngle = temp.m_windMotionMovingToEndAngle;
    m_windMotionStartAngle = Get_Client_Random_Value_Real(temp.m_windMotionStartAngleMin, temp.m_windMotionStartAngleMax);
    m_windMotionEndAngle = Get_Client_Random_Value_Real(temp.m_windMotionEndAngleMin, temp.m_windMotionEndAngleMax);
    m_windAngle = Get_Client_Random_Value_Real(m_windMotionStartAngle, m_windMotionEndAngle);
    m_shaderType = temp.m_shaderType;
    m_particleType = temp.m_particleType;
    m_particleTypeName = temp.m_particleTypeName;
}

ParticleSystem::~ParticleSystem()
{
    Remove_Slave();
    Remove_Master();

    for (Particle *i = m_systemParticlesHead; i != nullptr; i = m_systemParticlesHead) {
        Delete_Instance(i);
    }

    m_attachedToDrawableID = DRAWABLE_UNK;
    m_attachedToObjectID = OBJECT_UNK;

    if (m_controlParticle != nullptr) {
        m_controlParticle->m_systemUnderControl = nullptr;
    }

    m_controlParticle = nullptr;

    g_theParticleSystemManager->Remove_Particle_System(this);
}

void ParticleSystem::Update(int unk) {}

void ParticleSystem::Xfer_Snapshot(Xfer *xfer)
{
#define PARTICLESYS_XFER_VERSION 1
    uint8_t version = PARTICLESYS_XFER_VERSION;
    xfer->xferVersion(&version, PARTICLESYS_XFER_VERSION);
    ParticleSystemInfo::Xfer_Snapshot(xfer);
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_systemID)); // Was xferVoid
    xfer->xferDrawableID(&m_attachedToDrawableID);
    xfer->xferObjectID(&m_attachedToObjectID);
    xfer->xferBool(&m_isLocalIdentity);
    xfer->xferMatrix3D(&m_localTransform); // Was xferVoid
    xfer->xferBool(&m_isIdentity);
    xfer->xferMatrix3D(&m_transform); // Was xferVoid
    xfer->xferUnsignedInt(&m_burstDelayLeft);
    xfer->xferUnsignedInt(&m_delayLeft);
    xfer->xferUnsignedInt(&m_startTimestamp);
    xfer->xferUnsignedInt(&m_systemLifetimeLeft);
    xfer->xferUnsignedInt(&m_nextParticleIDMaybe);
    xfer->xferBool(&m_isForever);
    xfer->xferReal(&m_accumulatedSizeBonus);
    xfer->xferBool(&m_isStopped);
    xfer->xferCoord3D(&m_velCoefficient);
    xfer->xferReal(&m_countCoefficient);
    xfer->xferReal(&m_delayCoefficient);
    xfer->xferReal(&m_sizeCoefficient);
    xfer->xferCoord3D(&m_pos);
    xfer->xferCoord3D(&m_lastPos);
    xfer->xferBool(&m_isFirstPos);
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_slaveID)); // Was xferVoid
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_masterID)); // Was xferVoid
    uint32_t count = m_particleCount;
    xfer->xferUnsignedInt(&count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (Particle *part = m_systemParticlesHead; part != nullptr; part = part->m_systemNext) {
            part->Xfer_Snapshot(xfer);
        }
    } else {
        ParticleInfo *info = Generate_Particle_Info(0, 1);

        for (unsigned i = 0; i < count; ++i) {
            xfer->xferSnapshot(Create_Particle(*info, m_priority, true));
        }
    }
}

void ParticleSystem::Load_Post_Process() {}

void ParticleSystem::Set_Saveable(bool saveable)
{
    m_saveable = saveable;

    for (ParticleSystem *sys = m_slaveSystem; sys != nullptr; sys = sys->m_slaveSystem) {
        sys->m_saveable = saveable;
    }
}

void ParticleSystem::Destroy()
{
    m_isDestroyed = true;

    for (ParticleSystem *sys = m_slaveSystem; sys != nullptr; sys = sys->m_slaveSystem) {
        sys->m_isDestroyed = true;
    }
}

void ParticleSystem::Get_Position(Coord3D *pos) const
{
    if (pos != nullptr) {
        *pos = {m_localTransform[0][3], m_localTransform[1][3], m_localTransform[2][3]};
    }
}

void ParticleSystem::Set_Position(const Coord3D &pos)
{
    m_localTransform[0][3] = pos.x;
    m_localTransform[1][3] = pos.y;
    m_localTransform[2][3] = pos.z;
    m_isLocalIdentity = false;
}

void ParticleSystem::Set_Local_Transform(const Matrix3D &transform)
{
    m_localTransform = transform;
    m_isLocalIdentity = false;
}

void ParticleSystem::Rotate_Local_Transform_X(float theta)
{
    m_localTransform.Rotate_X(theta);
}

void ParticleSystem::Rotate_Local_Transform_Y(float theta)
{
    m_localTransform.Rotate_Y(theta);
}

void ParticleSystem::Rotate_Local_Transform_Z(float theta)
{
    m_localTransform.Rotate_Z(theta);
}

void ParticleSystem::Attach_To_Drawable(const Drawable *drawable)
{
// TODO requires Drawable.
#ifndef THYME_STANDALONE
    Call_Method<void, ParticleSystem, const Drawable *>(0x004CE800, this, drawable);
#endif
}

void ParticleSystem::Attach_To_Object(const Object *object)
{
// TODO requires Object.
#ifndef THYME_STANDALONE
    Call_Method<void, ParticleSystem, const Object *>(0x004CE830, this, object);
#endif
}

Coord3D *ParticleSystem::Compute_Particle_Velocity(const Coord3D *pos)
{
    static Coord3D _vel;

    switch (m_emissionVelocityType) {
        case EMISSION_VELOCITY_ORTHO:
            _vel.x = m_emissionVelocity.ortho.x.Get_Value();
            _vel.y = m_emissionVelocity.ortho.y.Get_Value();
            _vel.z = m_emissionVelocity.ortho.z.Get_Value();
            break;
        case EMISSION_VELOCITY_SPHERICAL: {
            float scale = m_emissionVelocity.spherical.Get_Value();
            _vel = *Compute_Point_On_Sphere();
            _vel *= scale;
        } break;
        case EMISSION_VELOCITY_HEMISPHERICAL: {
            float scale = m_emissionVelocity.hemispherical.Get_Value();
            // Randomize our point, only from 0.0 for Z as only half sphere.
            do {
                _vel.x = Get_Client_Random_Value_Real(-1.0f, 1.0f);
                _vel.y = Get_Client_Random_Value_Real(-1.0f, 1.0f);
                _vel.z = Get_Client_Random_Value_Real(0.0f, 1.0f);
            } while (_vel.x == 0.0f && _vel.y == 0.0f && _vel.z == 0.0f);

            float length = _vel.Length();

            if (length != 0.0f) {
                _vel /= length;
            }

            _vel *= scale;
        } break;
        case EMISSION_VELOCITY_CYLINDRICAL: {
            float radius = m_emissionVelocity.cylindrical.radial.Get_Value();
            float two_pi = Get_Client_Random_Value_Real(0.0f, GAMEMATH_PI * 2);
            _vel.x = Cos(two_pi) * radius;
            _vel.y = Sin(two_pi) * radius;
            _vel.z = m_emissionVelocity.cylindrical.normal.Get_Value();
        } break;
        case EMISSION_VELOCITY_OUTWARD: {
            float speed = m_emissionVelocity.outward.outward.Get_Value();
            float other_speed = m_emissionVelocity.outward.other.Get_Value();

            switch (m_emissionVolumeType) {
                case EMISSION_VOLUME_POINT: {
                    _vel = *Compute_Point_On_Sphere();
                    _vel *= speed;
                } break;
                case EMISSION_VOLUME_LINE: {
                    Coord3D distance = m_emissionVolume.line.end - m_emissionVolume.line.start;
                    float length = distance.Length();

                    if (length != 0.0f) {
                        distance /= length;
                    }

                    float tmp1 = 0.0f - distance.y;
                    float tmp2 = distance.x - 0.0f;
                    float tmp3 = float(0.0f * distance.y) - float(tmp2 * distance.z);
                    float tmp4 = float(tmp1 * distance.z) - float(0.0f * distance.x);

                    _vel.x = float(tmp1 * speed) + float(tmp3 * other_speed);
                    _vel.y = float(tmp2 * speed) + float(tmp4 * other_speed);
                    _vel.z = float(float(0.0f * speed) + float(float(distance.x * tmp2) - float(tmp1 - distance.y)))
                        * other_speed;
                } break;
                case EMISSION_VOLUME_BOX: {
                    _vel = *pos;
                    float length = _vel.Length();

                    if (length != 0.0f) {
                        _vel /= length;
                    }

                    _vel *= speed;
                }

                break;
                case EMISSION_VOLUME_SPHERE: {
                    _vel = *pos;
                    float length = _vel.Length();

                    if (length != 0.0f) {
                        _vel /= length;
                    }

                    _vel *= speed;
                } break;
                case EMISSION_VOLUME_CYLINDER: {
                    Coord2D cyl_pos(pos->x, pos->y);
                    float length = cyl_pos.Length();

                    if (length != 0.0f) {
                        cyl_pos /= length;
                    }

                    cyl_pos *= speed;
                    _vel.x = cyl_pos.x;
                    _vel.y = cyl_pos.y;
                    _vel.z = other_speed;
                } break;
                default:
                    break;
            }
        } break;
        default:
            _vel.x = 0.0f;
            _vel.y = 0.0f;
            _vel.z = 0.0f;
            break;
    }

    _vel.x *= float(float(float(g_theWriteableGlobalData->m_particleScale * 0.5f) + 0.5f) * m_velCoefficient.x);
    _vel.y *= float(float(float(g_theWriteableGlobalData->m_particleScale * 0.5f) + 0.5f) * m_velCoefficient.y);
    _vel.z *= float(float(float(g_theWriteableGlobalData->m_particleScale * 0.5f) + 0.5f) * m_velCoefficient.z);

    return &_vel;
}

Coord3D *ParticleSystem::Compute_Particle_Position()
{
    static Coord3D _pos;

    switch (m_emissionVolumeType) {
        case EMISSION_VOLUME_LINE: {
            float scale = Get_Client_Random_Value_Real(0.0f, 1.0f);

            _pos.x = float(scale * float(m_emissionVolume.line.end.x - m_emissionVolume.line.start.x))
                + m_emissionVolume.line.start.x;
            _pos.y = float(scale * float(m_emissionVolume.line.end.y - m_emissionVolume.line.start.y))
                + m_emissionVolume.line.start.y;
            _pos.z = float(scale * float(m_emissionVolume.line.end.z - m_emissionVolume.line.start.z))
                + m_emissionVolume.line.start.z;
        }

        break;

        case EMISSION_VOLUME_BOX:
            // If its hollow, final value must be on a surface.
            if (m_isEmissionVolumeHollow) {
                int random_val = Get_Client_Random_Value(0, 6);

                switch (random_val % 3) {
                    case 0:
                        _pos.x = Get_Client_Random_Value_Real(-m_emissionVolume.box.x, m_emissionVolume.box.x);
                        _pos.y = Get_Client_Random_Value_Real(-m_emissionVolume.box.y, m_emissionVolume.box.y);

                        if (random_val != 0) {
                            _pos.z = m_emissionVolume.box.z;
                        } else {
                            _pos.z = -m_emissionVolume.box.z;
                        }

                        break;

                    case 1:
                        _pos.y = Get_Client_Random_Value_Real(-m_emissionVolume.box.y, m_emissionVolume.box.y);
                        _pos.z = Get_Client_Random_Value_Real(-m_emissionVolume.box.z, m_emissionVolume.box.z);

                        if (random_val != 1) {
                            _pos.z = m_emissionVolume.box.x;
                        } else {
                            _pos.z = -m_emissionVolume.box.x;
                        }

                        break;

                    case 2:
                        _pos.x = Get_Client_Random_Value_Real(-m_emissionVolume.box.x, m_emissionVolume.box.x);
                        _pos.z = Get_Client_Random_Value_Real(-m_emissionVolume.box.z, m_emissionVolume.box.z);

                        if (random_val != 2) {
                            _pos.y = m_emissionVolume.box.y;
                        } else {
                            _pos.y = -m_emissionVolume.box.y;
                        }
                        break;

                    default:
                        break;
                }

                // The original code looks like it accidentally missed a break here and fell through into the VOLUME_LINE
                // code.
            } else {
                _pos.x = Get_Client_Random_Value_Real(-m_emissionVolume.box.x, m_emissionVolume.box.x);
                _pos.y = Get_Client_Random_Value_Real(-m_emissionVolume.box.y, m_emissionVolume.box.y);
                _pos.z = Get_Client_Random_Value_Real(-m_emissionVolume.box.z, m_emissionVolume.box.z);
            }

            break;

        case EMISSION_VOLUME_SPHERE: {
            float radius = m_isEmissionVolumeHollow ? m_emissionVolume.sphere :
                                                      Get_Client_Random_Value_Real(0.0f, m_emissionVolume.sphere);
            _pos = *Compute_Point_On_Sphere();
            _pos *= radius;
        } break;

        case EMISSION_VOLUME_CYLINDER: {
            float two_pi = Get_Client_Random_Value_Real(0.0f, GAMEMATH_PI * 2);
            float radius = m_isEmissionVolumeHollow ? m_emissionVolume.cylinder.radius :
                                                      Get_Client_Random_Value_Real(0.0f, m_emissionVolume.cylinder.radius);
            float height = m_emissionVolume.cylinder.length * 0.5f;
            _pos.x = Cos(two_pi) * radius;
            _pos.y = Sin(two_pi) * radius;
            _pos.z = Get_Client_Random_Value_Real(-height, height);
        } break;

        default:
            _pos.x = 0.0f;
            _pos.y = 0.0f;
            _pos.z = 0.0f;
            break;
    }

    _pos *= float(float(g_theWriteableGlobalData->m_particleScale * 0.5f) + 0.5f);

    return &_pos;
}

Particle *ParticleSystem::Create_Particle(const ParticleInfo &info, ParticlePriorityType priority, bool always_render)
{
    if (!always_render) {
        if (!g_theWriteableGlobalData->m_useFX) {
            return nullptr;
        }

        // Don't render if priority is lower than LOD minimum priority.
        if (priority < g_theGameLODManager->Min_Particle_Priority()) {
            return nullptr;
        }

        // If skip priority is lower than LOD minimum, workout if we skip this particular particle.
        if (priority < g_theGameLODManager->Min_Particle_Skip_Priority()) {
            g_theGameLODManager->Increment_Particle_Count();

            if ((g_theGameLODManager->Particle_Count() & g_theGameLODManager->Particle_Skip_Mask())
                != g_theGameLODManager->Particle_Skip_Mask()) {
                return nullptr;
            }
        }

        if (m_particleCount != 0 && priority == PARTPRIORITY_AREA_EFFECT) {
            if (m_isGroundAligned
                && g_theParticleSystemManager->Field_Particle_Count() > g_theWriteableGlobalData->m_maxFieldParticleCount) {
                return nullptr;
            }
        } else if (priority != PARTPRIORITY_ALWAYS_RENDER) {
            int excess_particles =
                g_theParticleSystemManager->Particle_Count() - g_theWriteableGlobalData->m_maxFieldParticleCount;

            if (excess_particles > 0) {
                while (excess_particles != 0 && g_theParticleSystemManager->Particle_Count() != 0) {
                    for (ParticlePriorityType i = PARTPRIORITY_WEAPON_EXPLOSION; i < priority; ++i) {
                        if (g_theParticleSystemManager->Get_Particle_Head(i) != nullptr) {
                            Delete_Instance(g_theParticleSystemManager->Get_Particle_Head(i));
                        }
                    }
                }
            }
        }
    }

    return new Particle(this, info);
}

ParticleInfo *ParticleSystem::Generate_Particle_Info(int id, int count)
{
    static ParticleInfo _info;

    if (count != 0) {
        _info.m_pos = *Compute_Particle_Position();
        _info.m_vel = *Compute_Particle_Velocity(&_info.m_pos);

        if (!m_isIdentity) {
            if (m_isFirstPos) {
                m_lastPos = m_pos;
                m_isFirstPos = false;
            }

            float scale = 1.0f - float(float(id) / float(count));
            Coord3D tmp = m_pos - m_lastPos;
            tmp *= scale;

            // Looks like a matrix transfom... original code perhaps casted Coord3D to Vector3?
            _info.m_pos.x = float(float(float(_info.m_pos.x * m_transform[0][0]) + float(_info.m_pos.y * m_transform[0][1]))
                                + float(_info.m_pos.z * m_transform[0][2])) + m_transform[0][3];
            _info.m_pos.y = float(float(float(_info.m_pos.x * m_transform[1][0]) + float(_info.m_pos.y * m_transform[1][1]))
                                + float(_info.m_pos.z * m_transform[1][2]))
                + m_transform[1][3];
            _info.m_pos.z = float(float(float(_info.m_pos.x * m_transform[2][0]) + float(_info.m_pos.y * m_transform[2][1]))
                                + float(_info.m_pos.z * m_transform[2][2]))
                + m_transform[2][3];
            _info.m_pos -= tmp;

            // Looks like a matrix rotate... original code perhaps casted Coord3D to Vector3?
            _info.m_vel.x = float(float(_info.m_vel.x * m_transform[0][0]) + float(_info.m_vel.y * m_transform[0][1]))
                                + float(_info.m_vel.z * m_transform[0][2]);
            _info.m_vel.y = float(float(_info.m_vel.x * m_transform[1][0]) + float(_info.m_vel.y * m_transform[1][1]))
                                + float(_info.m_vel.z * m_transform[1][2]);
            _info.m_vel.z = float(float(_info.m_vel.x * m_transform[2][0]) + float(_info.m_vel.y * m_transform[2][1]))
                                + float(_info.m_vel.z * m_transform[2][2]);
        }

        _info.m_velDamping = m_velDamping.Get_Value();
        _info.m_angularDamping = m_angularDamping.Get_Value();
#ifdef THYME_STANDALONE
        _info.m_angleX = m_angleX.Get_Value();
        _info.m_angleY = m_angleY.Get_Value();
#endif
        _info.m_angleZ = m_angleZ.Get_Value();
#ifdef THYME_STANDALONE
        _info.m_angularRateX = m_angularRateX.Get_Value();
        _info.m_angularRateY = m_angularRateY.Get_Value();
#endif
        _info.m_angularRateZ = m_angularRateZ.Get_Value();
        _info.m_lifetime = m_lifetime.Get_Value();
        _info.m_size = float(m_startSize.Get_Value() * g_theWriteableGlobalData->m_particleScale) * m_sizeCoefficient;
        _info.m_sizeRate = float(m_sizeRate.Get_Value() * g_theWriteableGlobalData->m_particleScale) * m_sizeCoefficient;
        _info.m_sizeRateDamping = m_sizeRateDamping.Get_Value();
        _info.m_size += m_accumulatedSizeBonus;
        m_accumulatedSizeBonus = Clamp(m_accumulatedSizeBonus + m_startSizeRate.Get_Value(), 0.0f, 50.0f);
        
        for (int i = 0; i < KEYFRAME_COUNT; ++i) {
            _info.m_alphaKey[i].value = m_alphaKey[i].var.Get_Value();
            _info.m_alphaKey[i].frame = m_alphaKey[i].frame;
            _info.m_colorKey[i].color = m_colorKey[i].color;
            _info.m_colorKey[i].frame = m_colorKey[i].frame;
        }

        _info.m_colorScale = m_colorScale.Get_Value();
        _info.m_emitterPos = {
            m_transform[0][3],
            m_transform[1][3],
            m_transform[2][3],
        };

        _info.m_particleUpTowardsEmitter = m_isParticleUpTowardsEmitter;
        _info.m_windRandomness = Get_Client_Random_Value_Real(0.7f, 1.3f);
    }

    return &_info;
}

void ParticleSystem::Add_Particle(Particle *particle)
{
    if (!particle->m_inSystemList) {
        if (m_systemParticlesHead == nullptr) {
            m_systemParticlesHead = particle;
        }

        if (m_systemParticlesTail != nullptr) {
            m_systemParticlesTail->m_systemNext = particle;
        }

        particle->m_systemPrev = m_systemParticlesTail;
        m_systemParticlesTail = particle;
        particle->m_systemNext = nullptr;
        particle->m_inSystemList = true;
        particle->m_particleIDMaybe = m_nextParticleIDMaybe++;
        ++m_particleCount;
    }
}

void ParticleSystem::Remove_Particle(Particle *particle)
{
    if (particle->m_inSystemList) {
        if (particle->m_systemNext != nullptr) {
            particle->m_systemNext->m_systemPrev = particle->m_systemPrev;
        }

        if (particle->m_systemPrev != nullptr) {
            particle->m_systemPrev->m_systemNext = particle->m_systemNext;
        }

        if (particle == m_systemParticlesHead) {
            m_systemParticlesHead = particle->m_systemNext;
        }

        if (particle == m_systemParticlesTail) {
            m_systemParticlesTail = particle->m_systemPrev;
        }

        particle->m_systemNext = nullptr;
        particle->m_systemPrev = nullptr;
        particle->m_inSystemList = false;
        --m_particleCount;
    }
}

Coord3D *ParticleSystem::Compute_Point_On_Sphere()
{
    static Coord3D _point;

    // Randomize our static point.
    do {
        _point.x = Get_Client_Random_Value_Real(-1.0f, 1.0f);
        _point.y = Get_Client_Random_Value_Real(-1.0f, 1.0f);
        _point.z = Get_Client_Random_Value_Real(-1.0f, 1.0f);
    } while (_point.x == 0.0f && _point.y == 0.0f && _point.z == 0.0f);

    float length = _point.Length();

    if (length != 0.0f) {
        _point /= length;
    }

    return &_point;
}

void ParticleSystem::Remove_Master()
{
    if (m_masterSystem != nullptr) {
        m_masterSystem->m_slaveSystem = nullptr;
        m_masterSystem->m_slaveID = PARTSYS_ID_NONE;
        m_masterSystem = nullptr;
        m_masterID = PARTSYS_ID_NONE;
    }
}

void ParticleSystem::Remove_Slave()
{
    if (m_slaveSystem != nullptr) {
        m_slaveSystem->m_masterSystem = nullptr;
        m_slaveSystem->m_masterID = PARTSYS_ID_NONE;
        m_slaveSystem = nullptr;
        m_slaveID = PARTSYS_ID_NONE;
    }
}
