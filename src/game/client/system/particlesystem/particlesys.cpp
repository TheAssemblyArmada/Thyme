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
#include "particlesys.h"
#include "drawable.h"
#include "gameclient.h"
#include "gamelod.h"
#include "gamelogic.h"
#include "gamemath.h"
#include "globaldata.h"
#include "object.h"
#include "particle.h"
#include "particlesystemplate.h"
#include "randomvalue.h"
#include "terrainlogic.h"
#include "xfer.h"
#include <algorithm>
#include <captainslog.h>

/**
 * 0x004CDA10
 */
ParticleSystem::ParticleSystem(const ParticleSystemTemplate *temp, ParticleSystemID id, bool create_slaves) :
    m_systemParticlesHead(nullptr),
    m_systemParticlesTail(nullptr),
    m_particleCount(0),
    m_systemID(id),
    m_attachedToDrawableID(INVALID_DRAWABLE_ID),
    m_attachedToObjectID(INVALID_OBJECT_ID),
    m_localTransform(true),
    m_transform(true),
    m_burstDelayLeft(0),
    m_delayLeft(0),
    m_startTimestamp(g_theGameClient->Get_Frame()),
    m_lastParticleID(0),
    m_accumulatedSizeBonus(0.0f),
    m_countCoefficient(1.0f),
    m_delayCoefficient(1.0f),
    m_sizeCoefficient(1.0f),
    m_slaveSystem(nullptr),
    m_slaveID(PARTSYS_ID_NONE),
    m_masterSystem(nullptr),
    m_masterID(PARTSYS_ID_NONE),
    m_template(temp),
    m_controlParticle(nullptr),
    m_isLocalIdentity(true),
    m_isIdentity(true),
    m_isForever(false),
    m_isStopped(false),
    m_isDestroyed(false),
    m_isFirstPos(true),
    m_saveable(true),
    m_unkBool1(false)
{
    m_lastPos.Zero();
    m_pos.Zero();
    m_velCoefficient.x = 1.0f;
    m_velCoefficient.y = 1.0f;
    m_velCoefficient.z = 1.0f;
    m_slavePosOffset = temp->m_slavePosOffset;
    m_unkValue = 0;
    m_driftVelocity = temp->m_driftVelocity;
    m_gravity = temp->m_gravity;
    m_lifetime = temp->m_lifetime;
    m_startSize = temp->m_startSize;
    m_startSizeRate = temp->m_startSizeRate;
    m_sizeRate = temp->m_sizeRate;
    m_sizeRateDamping = temp->m_sizeRateDamping;

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = temp->m_alphaKey[i];
        m_colorKey[i] = temp->m_colorKey[i];
    }

    m_colorScale.Set_Range(
        temp->m_colorScale.Get_Min() / 255.0f, temp->m_colorScale.Get_Max() / 255.0f, GameClientRandomVariable::UNIFORM);
    m_burstDelay = temp->m_burstDelay;
    m_burstCount = temp->m_burstCount;
    m_isOneShot = temp->m_isOneShot;
    m_delayLeft = temp->m_initialDelay.Get_Value();
    m_systemLifetimeLeft = temp->m_systemLifetime;
    m_isForever = temp->m_systemLifetime == 0;
    m_velDamping = temp->m_velDamping;
#ifndef GAME_DLL
    m_angleX = temp->m_angleX;
    m_angularRateX = temp->m_angularRateX;
    m_angleY = temp->m_angleY;
    m_angularRateY = temp->m_angularRateY;
#endif
    m_angleZ = temp->m_angleZ;
    m_angularRateZ = temp->m_angularRateZ;
    m_angularDamping = temp->m_angularDamping;
    m_priority = temp->m_priority;
    m_emissionVelocityType = temp->m_emissionVelocityType;
    memcpy(&m_emissionVelocity, &temp->m_emissionVelocity, sizeof(m_emissionVelocity));
    m_emissionVolumeType = temp->m_emissionVolumeType;
    memcpy(&m_emissionVolume, &temp->m_emissionVolume, sizeof(m_emissionVolume));
    m_isEmissionVolumeHollow = temp->m_isEmissionVolumeHollow;
    m_isGroundAligned = temp->m_isGroundAligned;
    m_isEmitAboveGroundOnly = temp->m_isEmitAboveGroundOnly;
    m_isParticleUpTowardsEmitter = temp->m_isParticleUpTowardsEmitter;
    m_windMotion = temp->m_windMotion;
    m_windAngleChange = temp->m_windAngleChange;
    m_windAngleChangeMin = temp->m_windAngleChangeMin;
    m_windAngleChangeMax = temp->m_windAngleChangeMax;
    m_windMotionStartAngleMin = temp->m_windMotionStartAngleMin;
    m_windMotionStartAngleMax = temp->m_windMotionStartAngleMax;
    m_windMotionEndAngleMin = temp->m_windMotionEndAngleMin;
    m_windMotionEndAngleMax = temp->m_windMotionEndAngleMax;
    m_windMotionMovingToEndAngle = temp->m_windMotionMovingToEndAngle;
    m_windMotionStartAngle = Get_Client_Random_Value_Real(m_windMotionStartAngleMin, m_windMotionStartAngleMax);
    m_windMotionEndAngle = Get_Client_Random_Value_Real(m_windMotionEndAngleMin, m_windMotionEndAngleMax);
    m_windAngle = Get_Client_Random_Value_Real(m_windMotionStartAngle, m_windMotionEndAngle);
    m_shaderType = temp->m_shaderType;
    m_particleType = temp->m_particleType;
    m_particleTypeName = temp->m_particleTypeName;

    if (create_slaves) {
        ParticleSystem *slave = temp->Create_Slave_System(create_slaves);

        if (slave != nullptr) {
            Set_Slave(slave);
            slave->Set_Master(this);
        }
    }

    m_attachedSystemName = temp->m_attachedSystemName;

    g_theParticleSystemManager->Add_Particle_System(this);
}

/**
 * 0x004CE310
 */
ParticleSystem::~ParticleSystem()
{
    if (m_slaveSystem != nullptr) {
        captainslog_dbgassert(
            m_slaveSystem->Get_Master() == this, "~ParticleSystem: Our slave doesn't have us as a master!");
        m_slaveSystem->Set_Master(nullptr);
        Set_Slave(nullptr);
    }

    if (m_masterSystem != nullptr) {
        captainslog_dbgassert(
            m_masterSystem->Get_Slave() == this, "~ParticleSystem: Our master doesn't have us as a slave!");
        m_masterSystem->Set_Slave(nullptr);
        Set_Master(nullptr);
    }

    while (m_systemParticlesHead != nullptr) {
        m_systemParticlesHead->Delete_Instance();
    }

    m_attachedToDrawableID = INVALID_DRAWABLE_ID;
    m_attachedToObjectID = INVALID_OBJECT_ID;

    if (m_controlParticle != nullptr) {
        m_controlParticle->Detach_Controlled_Particle_System();
    }

    m_controlParticle = nullptr;

    g_theParticleSystemManager->Remove_Particle_System(this);
}

/**
 * @brief Performs logic tick update.
 *
 * 0x004CFB80
 */
bool ParticleSystem::Update(int index)
{
    if (!g_theWriteableGlobalData->m_useFX) {
        return false;
    }

    if (m_delayLeft != 0) {
        if (--m_delayLeft == 0) {
            m_startTimestamp = g_theGameClient->Get_Frame();
        }

        return true;
    } else {
        if (m_windMotion != WIND_MOTION_UNUSED) {
            Update_Wind_Motion();
        }

        bool matrix_set = false;
        const Matrix3D *matrix = nullptr;
        bool shrouded = false;

        if (m_attachedToDrawableID != INVALID_DRAWABLE_ID) {
            Drawable *drawable = g_theGameClient->Find_Drawable_By_ID(m_attachedToDrawableID);

            if (drawable != nullptr) {
                if (drawable->Is_Fully_Obscured_By_Shroud()) {
                    shrouded = true;
                }

                matrix = drawable->Get_Transform_Matrix();
                m_lastPos = m_pos;
                m_pos = *drawable->Get_Position();
            } else {
                m_attachedToDrawableID = INVALID_DRAWABLE_ID;
                Destroy();
            }
        } else if (m_attachedToObjectID != INVALID_OBJECT_ID) {
            Object *object = g_theGameLogic->Find_Object_By_ID(m_attachedToObjectID);

            if (object != nullptr) {
                shrouded = object->Get_Shrouded_Status(index) >= SHROUDED_SEEN;

                Drawable *drawable = object->Get_Drawable();

                if (drawable != nullptr) {
                    matrix = drawable->Get_Transform_Matrix();
                } else {
                    matrix = object->Get_Transform_Matrix();
                }

                m_lastPos = m_pos;
                m_pos = *object->Get_Position();
            } else {
                m_attachedToObjectID = INVALID_OBJECT_ID;
                Destroy();
            }
        }

        if (matrix != nullptr) {
            if (m_unkBool1) {
                m_transform = m_localTransform;
            } else if (m_isLocalIdentity) {
                m_transform = *matrix;
            } else {
                m_transform.Mul(*matrix, m_localTransform);
            }

            m_isIdentity = false;
            matrix_set = true;
        }

        if (!matrix_set) {
            if (m_isLocalIdentity) {
                m_isIdentity = true;
            } else {
                m_transform = m_localTransform;
                m_isIdentity = false;
            }
        }

        if (m_controlParticle != nullptr) {
            const Coord3D *pos = m_controlParticle->Get_Position();
            m_transform.Set_X_Translation(pos->x);
            m_transform.Set_Y_Translation(pos->y);
            m_transform.Set_Z_Translation(pos->z);
            m_isIdentity = false;
            m_lastPos = m_pos;
            m_pos = *pos;
        }

        if (!m_isDestroyed && (m_isForever || m_systemLifetimeLeft != 0) && !shrouded && !m_isStopped
            && m_masterSystem == nullptr) {
            if (m_burstDelayLeft != 0) {
                m_burstDelayLeft--;
            } else {
                ParticlePriorityType priority = Get_Priority();
                int particle_count = GameMath::Fast_To_Int_Truncate(m_burstCount.Get_Value()) * m_countCoefficient;

                for (int particle_num = 0; particle_num < particle_count; particle_num++) {
                    ParticleInfo *info = Generate_Particle_Info(particle_num, particle_count);

                    if (m_isEmitAboveGroundOnly) {
                        float ground_height = g_theTerrainLogic->Get_Ground_Height(info->m_pos.x, info->m_pos.y, nullptr);

                        if (ground_height > info->m_pos.z) {
                            continue;
                        }
                    }

                    Particle *control_particle = Create_Particle(*info, priority, false);

                    if (control_particle != nullptr) {
                        if (!m_attachedSystemName.Is_Empty()) {
                            ParticleSystemTemplate *system_template =
                                g_theParticleSystemManager->Find_Template(m_attachedSystemName);

                            if (system_template != nullptr) {
                                ParticleSystem *control_system =
                                    g_theParticleSystemManager->Create_Particle_System(system_template, false);
                                control_system->Set_Control_Particle(control_particle);
                                control_particle->Control_Particle_System(control_system);
                            }
                        }

                        if (m_slaveSystem != nullptr) {
                            m_slaveSystem->Create_Particle(
                                Merge_Related_Systems(this, m_slaveSystem, false), priority, false);
                        }
                    }
                }

                m_burstDelayLeft = m_burstDelay.Get_Value();
                m_burstDelayLeft *= m_delayCoefficient;
            }
        }

        Particle *particle = m_systemParticlesHead;

        while (particle != nullptr) {
            if (m_gravity != 0.0f) {
                Coord3D force;
                force.x = 0.0f;
                force.y = 0.0f;
                force.z = m_gravity;
                particle->Apply_Force(force);
            }

            if (particle->Update()) {
                particle = particle->m_systemNext;
            } else {
                Particle *old_particle = particle;
                particle = particle->m_systemNext;
                old_particle->Delete_Instance();
            }
        }

        if (m_isDestroyed && m_systemParticlesHead == nullptr) {
            return false;
        }

        if (m_isForever) {
            return true;
        }

        if (m_systemLifetimeLeft != 0) {
            m_systemLifetimeLeft--;
        }

        if (Get_Particle_Count() != 0) {
            return true;
        }

        return m_systemLifetimeLeft != 0;
    }
}

/**
 * @brief Performs transfer logic on the class.
 *
 * 0x004D11E0
 */
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
    xfer->xferUnsignedInt(&m_lastParticleID);
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
            Particle *particle = Create_Particle(*info, m_priority, true);
            captainslog_dbgassert(
                particle != nullptr, "ParticleSystem::Xfer_Snapshot - Unable to create particle for loading");
            xfer->xferSnapshot(particle);
        }
    }
}

/**
 * @brief Performs additional post data load house keeping.
 *
 * 0x004D1430
 */
void ParticleSystem::Load_Post_Process()
{
    if (m_slaveID != PARTSYS_ID_NONE) {
        captainslog_relassert(
            m_slaveSystem == nullptr, 6, "ParticleSystem::Load_Post_Process - m_slaveSystem is not NULL but should be");
        m_slaveSystem = g_theParticleSystemManager->Find_Particle_System(m_slaveID);
        captainslog_relassert(m_slaveSystem != nullptr && !m_slaveSystem->Is_Destroyed(),
            6,
            "ParticleSystem::Load_Post_Process - m_slaveSystem is NULL or destroyed");
    }

    if (m_masterID != PARTSYS_ID_NONE) {
        captainslog_relassert(
            m_masterSystem == nullptr, 6, "ParticleSystem::Load_Post_Process - m_masterSystem is not NULL but should be");
        m_masterSystem = g_theParticleSystemManager->Find_Particle_System(m_masterID);
        captainslog_relassert(m_masterSystem != nullptr && !m_masterSystem->Is_Destroyed(),
            6,
            "ParticleSystem::Load_Post_Process - m_masterSystem is NULL or destroyed");
    }
}

/**
 * @brief Sets this object and all slave objects as saveable.
 */
void ParticleSystem::Set_Saveable(bool saveable)
{
    m_saveable = saveable;

    if (m_slaveSystem != nullptr) {
        m_slaveSystem->Set_Saveable(saveable);
    }
}

/**
 * @brief Sets this and all slave systems as destroyed.
 *
 * 0x004CE500
 */
void ParticleSystem::Destroy()
{
    m_isDestroyed = true;

    if (m_slaveSystem != nullptr) {
        m_slaveSystem->Destroy();
    }
}

/**
 * @brief Gets the position of this particle system.
 *
 * 0x004CE530
 */
void ParticleSystem::Get_Position(Coord3D *pos) const
{
    Vector3 trans;
    m_localTransform.Get_Translation(&trans);

    if (pos != nullptr) {
        pos->x = trans.X;
        pos->y = trans.Y;
        pos->z = trans.Z;
    }
}

/**
 * @brief Sets the position of this particle system.
 *
 * 0x004CE570
 */
void ParticleSystem::Set_Position(const Coord3D &pos)
{
    m_localTransform.Set_X_Translation(pos.x);
    m_localTransform.Set_Y_Translation(pos.y);
    m_localTransform.Set_Z_Translation(pos.z);
    m_isLocalIdentity = false;
}

/**
 * @brief Sets the local transform matrix.
 *
 * 0x004CE5A0
 */
void ParticleSystem::Set_Local_Transform(const Matrix3D &transform)
{
    m_localTransform = transform;
    m_isLocalIdentity = false;
}

/**
 * @brief Rotates the local transform matrix around X.
 *
 * 0x004CE620
 */
void ParticleSystem::Rotate_Local_Transform_X(float theta)
{
    m_localTransform.Rotate_X(theta);
}

/**
 * @brief Rotates the local transform matrix around Y.
 *
 * 0x004CE6C0
 */
void ParticleSystem::Rotate_Local_Transform_Y(float theta)
{
    m_localTransform.Rotate_Y(theta);
}

/**
 * @brief Rotates the local transform matrix around Z.
 *
 * 0x004CE760
 */
void ParticleSystem::Rotate_Local_Transform_Z(float theta)
{
    m_localTransform.Rotate_Z(theta);
}

/**
 * @brief Attaches system to a drawable object.
 *
 * 0x004CE800
 */
void ParticleSystem::Attach_To_Drawable(const Drawable *drawable)
{
    if (drawable != nullptr) {
        m_attachedToDrawableID = drawable->Get_ID();
    } else {
        m_attachedToDrawableID = INVALID_DRAWABLE_ID;
    }
}

/**
 * @brief Attaches system to an Object object.
 *
 * 0x004CE830
 */
void ParticleSystem::Attach_To_Object(const Object *object)
{
    if (object != nullptr) {
        m_attachedToObjectID = object->Get_ID();
    } else {
        m_attachedToObjectID = INVALID_OBJECT_ID;
    }
}

/**
 * @brief Computes an initial velocity for a particle.
 *
 * 0x004CE860
 */
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
            Coord3D *point = Compute_Point_On_Sphere();
            _vel = scale * (*point);
        } break;
        case EMISSION_VELOCITY_HEMISPHERICAL: {
            float scale = m_emissionVelocity.hemispherical.Get_Value();
            Coord3D rand;

            // Randomize our point, only from 0.0 for Z as only half sphere.
            do {
                rand.x = Get_Client_Random_Value_Real(-1.0f, 1.0f);
                rand.y = Get_Client_Random_Value_Real(-1.0f, 1.0f);
                rand.z = Get_Client_Random_Value_Real(0.0f, 1.0f);
            } while (rand.x == 0.0f && rand.y == 0.0f && rand.z == 0.0f);

            rand.Normalize();
            _vel = scale * rand;
        } break;
        case EMISSION_VELOCITY_CYLINDRICAL: {
            float radius = m_emissionVelocity.cylindrical.radial.Get_Value();
            float two_pi = Get_Client_Random_Value_Real(0.0f, GAMEMATH_PI * 2);
            _vel.x = GameMath::Cos(two_pi) * radius;
            _vel.y = GameMath::Sin(two_pi) * radius;
            _vel.z = m_emissionVelocity.cylindrical.normal.Get_Value();
        } break;
        case EMISSION_VELOCITY_OUTWARD: {
            float speed = m_emissionVelocity.outward.outward.Get_Value();
            float other_speed = m_emissionVelocity.outward.other.Get_Value();
            Coord3D zero;
            zero.Zero();

            switch (m_emissionVolumeType) {
                case EMISSION_VOLUME_POINT: {
                    Coord3D *point = Compute_Point_On_Sphere();
                    _vel = speed * (*point);
                } break;
                case EMISSION_VOLUME_LINE: {
                    Coord3D distance;
                    distance.x = m_emissionVolume.line.end.x - m_emissionVolume.line.start.x;
                    distance.y = m_emissionVolume.line.end.y - m_emissionVolume.line.start.y;
                    distance.z = m_emissionVolume.line.end.z - m_emissionVolume.line.start.z;
                    distance.Normalize();

                    Coord3D c1;
                    c1.x = 0.0f;
                    c1.y = 0.0f;
                    c1.z = 1.0f;

                    Coord3D c2;
                    Coord3D::Cross_Product(&c1, &distance, &c2);
                    Coord3D::Cross_Product(&distance, &c2, &c1);

                    _vel = speed * c2 + other_speed * c1;
                } break;
                case EMISSION_VOLUME_BOX:
                case EMISSION_VOLUME_SPHERE: {
                    _vel = (*pos) - zero;
                    _vel.Normalize();
                    _vel *= speed;
                } break;
                case EMISSION_VOLUME_CYLINDER: {
                    Coord2D cyl_pos;
                    cyl_pos.x = pos->x - zero.x;
                    cyl_pos.y = pos->y - zero.y;
                    cyl_pos.Normalize();

                    _vel.x = speed * cyl_pos.x;
                    _vel.y = speed * cyl_pos.y;
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

    _vel.x *= (g_theWriteableGlobalData->m_particleScale / 2.0f + 0.5f) * m_velCoefficient.x;
    _vel.y *= (g_theWriteableGlobalData->m_particleScale / 2.0f + 0.5f) * m_velCoefficient.y;
    _vel.z *= (g_theWriteableGlobalData->m_particleScale / 2.0f + 0.5f) * m_velCoefficient.z;

    return &_vel;
}

/**
 * @brief Computes an initial position for a particle.
 *
 * 0x004CEF60
 */
Coord3D *ParticleSystem::Compute_Particle_Position()
{
    static Coord3D _pos;

    switch (m_emissionVolumeType) {
        case EMISSION_VOLUME_LINE: {
            float scale = Get_Client_Random_Value_Real(0.0f, 1.0f);
            _pos = scale * (m_emissionVolume.line.end - m_emissionVolume.line.start) + m_emissionVolume.line.start;
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
            Coord3D *point = Compute_Point_On_Sphere();
            _pos = radius * (*point);
        } break;

        case EMISSION_VOLUME_CYLINDER: {
            float two_pi = Get_Client_Random_Value_Real(0.0f, GAMEMATH_PI * 2.0f);
            float radius = m_isEmissionVolumeHollow ? m_emissionVolume.cylinder.radius :
                                                      Get_Client_Random_Value_Real(0.0f, m_emissionVolume.cylinder.radius);
            float height = m_emissionVolume.cylinder.length * 0.5f;
            _pos.x = GameMath::Cos(two_pi) * radius;
            _pos.y = GameMath::Sin(two_pi) * radius;
            _pos.z = Get_Client_Random_Value_Real(-height, height);
        } break;

        default:
            _pos.x = 0.0f;
            _pos.y = 0.0f;
            _pos.z = 0.0f;
            break;
    }

    _pos = (g_theWriteableGlobalData->m_particleScale / 2.0f + 0.5f) * _pos;

    return &_pos;
}

/**
 * @brief Creates a particle from provided info.
 *
 * 0x004CF530
 */
Particle *ParticleSystem::Create_Particle(const ParticleInfo &info, ParticlePriorityType priority, bool always_render)
{
    if (always_render) {
        return NEW_POOL_OBJ(Particle, this, info);
    }

    if (!g_theWriteableGlobalData->m_useFX) {
        return nullptr;
    }

    // Don't render if priority is lower than LOD minimum priority.
    // If skip priority is lower than LOD minimum, workout if we skip this particular particle.
    if (priority < g_theGameLODManager->Min_Particle_Priority()
        || (priority < g_theGameLODManager->Min_Particle_Skip_Priority() && g_theGameLODManager->Is_Particle_Skipped())) {
        return nullptr;
    }

    if (Get_Particle_Count() != 0 && priority == PARTICLE_PRIORITY_AREA_EFFECT) {
        if (m_isGroundAligned
            && g_theParticleSystemManager->Get_Field_Particle_Count() > g_theWriteableGlobalData->m_maxFieldParticleCount) {
            return nullptr;
        }
    } else if (priority == PARTICLE_PRIORITY_ALWAYS_RENDER) {
        return NEW_POOL_OBJ(Particle, this, info);
    }

    int excess = g_theParticleSystemManager->Get_Particle_Count() - g_theWriteableGlobalData->m_maxParticleCount;

    if (excess > 0 && g_theParticleSystemManager->Remove_Oldest_Particles(excess, priority) != unsigned(excess)) {
        return nullptr;
    }

    if (g_theWriteableGlobalData->m_maxParticleCount == 0) {
        return nullptr;
    }

    return NEW_POOL_OBJ(Particle, this, info);
}

/**
 * @brief Generates info used to create particles for this system.
 *
 * 0x004CF750
 */
ParticleInfo *ParticleSystem::Generate_Particle_Info(int id, int count)
{
    static ParticleInfo _info;

    if (count == 0) {
        captainslog_dbgassert(false, "particleCount must NOT be 0. Set to 1 or greater.");
        return &_info;
    } else {
        _info.m_pos = *Compute_Particle_Position();
        _info.m_vel = *Compute_Particle_Velocity(&_info.m_pos);

        if (!m_isIdentity) {
            Vector3 pos;
            Vector3 transform;

            if (m_isFirstPos) {
                m_lastPos = m_pos;
                m_isFirstPos = false;
            }

            pos.X = _info.m_pos.x;
            pos.Y = _info.m_pos.y;
            pos.Z = _info.m_pos.z;
            m_transform.Mul_Vector3(pos, transform);
            _info.m_pos.x = transform.X - (1.0f - (float)id / (float)count) * (m_pos.x - m_lastPos.x);
            _info.m_pos.y = transform.Y - (1.0f - (float)id / (float)count) * (m_pos.y - m_lastPos.y);
            _info.m_pos.z = transform.Z - (1.0f - (float)id / (float)count) * (m_pos.z - m_lastPos.z);

            Vector3 vel;
            Vector3 rotate;
            vel.X = _info.m_vel.x;
            vel.Y = _info.m_vel.y;
            vel.Z = _info.m_vel.z;
            Matrix3D::Rotate_Vector(m_transform, vel, &rotate);
            _info.m_vel.x = rotate.X;
            _info.m_vel.y = rotate.Y;
            _info.m_vel.z = rotate.Z;
        }

        _info.m_velDamping = m_velDamping.Get_Value();
        _info.m_angularDamping = m_angularDamping.Get_Value();
#ifndef GAME_DLL
        _info.m_angleX = m_angleX.Get_Value();
        _info.m_angleY = m_angleY.Get_Value();
#endif
        _info.m_angleZ = m_angleZ.Get_Value();
#ifndef GAME_DLL
        _info.m_angularRateX = m_angularRateX.Get_Value();
        _info.m_angularRateY = m_angularRateY.Get_Value();
#endif
        _info.m_angularRateZ = m_angularRateZ.Get_Value();
        _info.m_lifetime = m_lifetime.Get_Value();
        _info.m_size = float(m_startSize.Get_Value() * m_sizeCoefficient) * g_theWriteableGlobalData->m_particleScale;
        _info.m_sizeRate = float(m_sizeRate.Get_Value() * m_sizeCoefficient) * g_theWriteableGlobalData->m_particleScale;
        _info.m_sizeRateDamping = m_sizeRateDamping.Get_Value();
        _info.m_size += m_accumulatedSizeBonus;
        m_accumulatedSizeBonus += m_startSizeRate.Get_Value();

        if (m_accumulatedSizeBonus != 0.0f) {
            m_accumulatedSizeBonus = std::min(m_accumulatedSizeBonus, 50.0f);
        }

        for (int i = 0; i < KEYFRAME_COUNT; ++i) {
            _info.m_alphaKey[i].value = m_alphaKey[i].var.Get_Value();
            _info.m_alphaKey[i].frame = m_alphaKey[i].frame;
            _info.m_colorKey[i].color = m_colorKey[i].color;
            _info.m_colorKey[i].frame = m_colorKey[i].frame;
        }

        _info.m_colorScale = m_colorScale.Get_Value();

        Vector3 pos;
        Vector3 trans(0.0f, 0.0f, 0.0f);
        m_transform.Mul_Vector3(trans, pos);
        _info.m_emitterPos.x = pos.X;
        _info.m_emitterPos.y = pos.Y;
        _info.m_emitterPos.z = pos.Z;

        _info.m_particleUpTowardsEmitter = m_isParticleUpTowardsEmitter;
        _info.m_windRandomness = Get_Client_Random_Value_Real(0.7f, 1.3f);
        return &_info;
    }
}

/**
 * @brief Adds a particle to this systems lists.
 */
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
        particle->Set_ID(m_lastParticleID++);
        ++m_particleCount;
    }
}

/**
 * @brief Removes a particle from this systems lists
 */
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

/**
 * @brief Computes a point on a sphere.
 */
Coord3D *ParticleSystem::Compute_Point_On_Sphere()
{
    static Coord3D _point;

    // Randomize our static point.
    do {
        _point.x = Get_Client_Random_Value_Real(-1.0f, 1.0f);
        _point.y = Get_Client_Random_Value_Real(-1.0f, 1.0f);
        _point.z = Get_Client_Random_Value_Real(-1.0f, 1.0f);
    } while (_point.x == 0.0f && _point.y == 0.0f && _point.z == 0.0f);

    _point.Normalize();

    return &_point;
}

/**
 * @brief Updates system to take account of wind simulation.
 *
 * 0x004D0920
 */
void ParticleSystem::Update_Wind_Motion()
{
    switch (m_windMotion) {
        case WIND_MOTION_PING_PONG: {
            captainslog_dbgassert(
                m_windMotionStartAngle < m_windMotionEndAngle, "Update_Wind_Motion: startAngle must be < endAngle");
            // Mathsy stuff here, not sure what the actual formula being used here is for, harmonic motion perhaps?
            float tmp = float(m_windMotionStartAngle - m_windMotionEndAngle) * 0.5f;
            float change = float(1.0f - float(GameMath::Fabs(float(tmp - m_windAngle) + m_windMotionEndAngle) / tmp))
                * m_windAngleChange;
            change = std::max(change, 0.005f);

            // Apply change in correct direction.
            if (m_windMotionMovingToEndAngle) {
                m_windAngle += change;

                if (m_windAngle < m_windMotionStartAngle) {
                    return;
                }

                m_windMotionMovingToEndAngle = false;
            } else {
                m_windAngle -= change;

                if (m_windAngle > m_windMotionEndAngle) {
                    return;
                }

                m_windMotionMovingToEndAngle = true;
            }

            m_windAngleChange = Get_Client_Random_Value_Real(m_windAngleChangeMin, m_windAngleChangeMax);
            m_windMotionStartAngle = Get_Client_Random_Value_Real(m_windMotionStartAngleMin, m_windMotionStartAngleMax);
            m_windMotionEndAngle = Get_Client_Random_Value_Real(m_windMotionEndAngleMin, m_windMotionEndAngleMax);
        }

        break;
        case WIND_MOTION_CIRCULAR:
            if (m_windAngleChange == 0.0f) {
                m_windAngleChange = Get_Client_Random_Value_Real(m_windAngleChangeMin, m_windAngleChangeMax);
            }

            m_windAngle += m_windAngleChange;

            if (m_windAngle > GAMEMATH_PI * 2) {
                m_windAngle -= GAMEMATH_PI * 2;
            } else if (m_windAngle < 0.0f) {
                m_windAngle += GAMEMATH_PI * 2;
            }

            break;
        default:
            break;
    }
}

/**
 * @brief Adds the master system to this particle system.
 */
void ParticleSystem::Set_Master(ParticleSystem *master)
{
    m_masterSystem = master;

    if (master != nullptr) {
        m_masterID = master->Get_System_ID();
    } else {
        m_masterID = PARTSYS_ID_NONE;
    }
}

/**
 * @brief Adds the slave system to this particle system.
 */
void ParticleSystem::Set_Slave(ParticleSystem *slave)
{
    m_slaveSystem = slave;

    if (slave != nullptr) {
        m_slaveID = slave->Get_System_ID();
    } else {
        m_slaveID = PARTSYS_ID_NONE;
    }
}

/**
 * @brief Creates a particle info from the information in two systems.
 *
 * 0x004D0B30
 */
ParticleInfo ParticleSystem::Merge_Related_Systems(ParticleSystem *master, ParticleSystem *slave, bool promote_slave)
{
    if (master == nullptr || slave == nullptr) {
        captainslog_dbgassert(false, "masterParticleSystem or slaveParticleSystem was NULL. Should not happen.");
        return ParticleInfo();
    } else {
        ParticleInfo tmp = *master->Generate_Particle_Info(1, 1);
        ParticleInfo *slave_info = slave->Generate_Particle_Info(1, 1);

        tmp.m_lifetime = slave_info->m_lifetime;
        tmp.m_size *= slave_info->m_size;
        tmp.m_sizeRate *= slave_info->m_sizeRate;
        tmp.m_sizeRateDamping *= slave_info->m_sizeRateDamping;
#ifndef GAME_DLL
        tmp.m_angleX = slave_info->m_angleX;
        tmp.m_angleY = slave_info->m_angleY;
#endif
        tmp.m_angleZ = slave_info->m_angleZ;
#ifndef GAME_DLL
        tmp.m_angularRateX = slave_info->m_angularRateX;
        tmp.m_angularRateY = slave_info->m_angularRateY;
#endif
        tmp.m_angularRateZ = slave_info->m_angularRateZ;
        tmp.m_angularDamping = slave_info->m_angularDamping;

        for (int i = 0; i < KEYFRAME_COUNT; ++i) {
            tmp.m_alphaKey[i] = slave_info->m_alphaKey[i];
            tmp.m_colorKey[i] = slave_info->m_colorKey[i];
        }

        tmp.m_colorScale = slave_info->m_colorScale;
        tmp.m_pos += *slave->Get_Slave_Position_Offset();

        if (promote_slave) {
            slave->m_burstCount = master->m_burstCount;
            slave->m_burstDelay = master->m_burstDelay;
            slave->m_priority = master->m_priority;
            memcpy(&slave->m_emissionVelocity, &master->m_emissionVelocity, sizeof(slave->m_emissionVelocity));
            slave->m_emissionVelocityType = master->m_emissionVelocityType;
            memcpy(&slave->m_emissionVolume, &master->m_emissionVolume, sizeof(slave->m_emissionVolume));
            slave->m_emissionVolumeType = master->m_emissionVolumeType;
            slave->m_isEmissionVolumeHollow = master->m_isEmissionVolumeHollow;
            slave->m_startSize.Set_Range(slave->m_startSize.Get_Min() * master->m_startSize.Get_Min(),
                slave->m_startSize.Get_Max() * master->m_startSize.Get_Max(),
                master->m_startSize.Get_Type());
            slave->m_sizeRate.Set_Range(slave->m_sizeRate.Get_Min() * master->m_sizeRate.Get_Min(),
                slave->m_sizeRate.Get_Max() * master->m_sizeRate.Get_Max(),
                master->m_sizeRate.Get_Type());
            slave->m_sizeRateDamping.Set_Range(slave->m_sizeRateDamping.Get_Min() * master->m_sizeRateDamping.Get_Min(),
                slave->m_sizeRateDamping.Get_Max() * master->m_sizeRateDamping.Get_Max(),
                master->m_sizeRateDamping.Get_Type());
        }

        return tmp;
    }
}
