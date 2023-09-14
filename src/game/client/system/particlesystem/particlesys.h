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
#pragma once

#include "always.h"
#include "matrix3d.h"
#include "mempoolobj.h"
#include "particlesysinfo.h"
#include "particlesysmanager.h"

class Particle;
class ParticleInfo;
class ParticleSystemTemplate;
class Drawable;
class Object;

class ParticleSystem : public MemoryPoolObject, public ParticleSystemInfo
{
    ALLOW_HOOKING
    IMPLEMENT_NAMED_POOL(ParticleSystem, ParticleSystemPool);

protected:
    virtual ~ParticleSystem() override;

public:
    ParticleSystem(const ParticleSystemTemplate *temp, ParticleSystemID id, bool create_slaves);

    virtual bool Update(int index);
    virtual Particle *Create_Particle(const ParticleInfo &info, ParticlePriorityType priority, bool always_render);

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    ParticleSystemID Get_System_ID() const { return m_systemID; }
    void Set_Control_Particle(Particle *particle) { m_controlParticle = particle; }
    void Start() { m_isStopped = false; }
    void Stop() { m_isStopped = true; }
    void Set_Saveable(bool saveable);
    void Destroy();
    void Get_Position(Coord3D *pos) const;
    void Set_Position(const Coord3D &pos);
    void Set_Local_Transform(const Matrix3D &transform);
    void Rotate_Local_Transform_X(float theta);
    void Rotate_Local_Transform_Y(float theta);
    void Rotate_Local_Transform_Z(float theta);
    void Attach_To_Drawable(const Drawable *drawable);
    void Attach_To_Object(const Object *object);
    void Add_Particle(Particle *particle);
    void Remove_Particle(Particle *particle);

    void Set_Lifetime_Range(float min, float max) { m_lifetime.Set_Range(min, max, GameClientRandomVariable::UNIFORM); }
    void Set_Unk(bool set) { m_unkBool1 = set; }
    void Set_Velocity_Multiplier(Coord3D *mul) { m_velCoefficient = *mul; }
    void Set_Burst_Count_Multiplier(float mul) { m_countCoefficient = mul; }
    void Set_Size_Multiplier(float mul) { m_sizeCoefficient = mul; }
    void Set_Delay_Left(uint32_t delay) { m_delayLeft = delay; }
    void Set_System_Life_Left(uint32_t life) { m_systemLifetimeLeft = life; }

    void Set_Emission_Volume_Sphere(float volume)
    {
        if (m_emissionVolumeType == EMISSION_VOLUME_SPHERE) {
            m_emissionVolume.sphere = volume;
        }
    }

    void Set_Emission_Volume_Cylinder(float volume)
    {
        if (m_emissionVolumeType == EMISSION_VOLUME_CYLINDER) {
            m_emissionVolume.cylinder.radius = volume;
        }
    }

    void Detach_Control_Particle() { m_controlParticle = nullptr; }

    bool Is_Forever() const { return m_isForever; }
    bool Is_Drawable() const { return m_particleType == PARTICLE_TYPE_DRAWABLE; }
    bool Is_Streak() const { return m_particleType == PARTICLE_TYPE_STREAK; }
    unsigned int Is_Volume_Particle() const { return m_particleType != PARTICLE_TYPE_VOLUME_PARTICLE ? 0 : 6; }
    bool Is_Ground_Aligned() const { return m_isGroundAligned; }
    bool Is_Not_Ground_Aligned() const { return !m_isGroundAligned; }
    bool Is_Destroyed() const { return m_isDestroyed; }
    bool Is_Saveable() const { return m_saveable; }

    ParticlePriorityType Get_Priority() const { return m_priority; }
    float Get_Wind_Angle() const { return m_windAngle; }
    ObjectID Get_Attached_Object() const { return m_attachedToObjectID; }
    DrawableID Get_Attached_Drawable() const { return m_attachedToDrawableID; }
    const Coord3D *Get_Drift_Velocity() const { return &m_driftVelocity; }
    WindMotion Get_Wind_Motion() const { return m_windMotion; }
    ParticleShaderType Get_Shader_Type() const { return m_shaderType; }
    Utf8String Get_Particle_Type_Name() const { return m_particleTypeName; }
    Particle *Get_First_Particle() const { return m_systemParticlesHead; }
    EmissionVolumeType Get_Emission_Volume_Type() const { return m_emissionVolumeType; }
    const ParticleSystemTemplate *Get_Template() const { return m_template; }
    const ParticleSystem *Get_Slave() const { return m_slaveSystem; }
    const ParticleSystem *Get_Master() const { return m_masterSystem; }
    uint32_t Get_Particle_Count() const { return m_particleCount; }
    const Coord3D *Get_Slave_Position_Offset() const { return &m_slavePosOffset; }

    void Reset_Delay()
    {
        m_burstDelayLeft = 0;
        m_delayLeft = 0;
    }

    static ParticleInfo Merge_Related_Systems(ParticleSystem *master, ParticleSystem *slave, bool promote_slave);

#ifdef GAME_DLL
    ParticleSystem *Hook_Ctor(const ParticleSystemTemplate *temp, ParticleSystemID id, bool create_slaves)
    {
        return new (this) ParticleSystem(temp, id, create_slaves);
    }
    void Hook_Dtor() { ParticleSystem::~ParticleSystem(); }
#endif
private:
    ParticleInfo *Generate_Particle_Info(int id, int count);
    Coord3D *Compute_Particle_Velocity(const Coord3D *pos);
    Coord3D *Compute_Particle_Position();
    void Update_Wind_Motion();
    void Set_Master(ParticleSystem *master);
    void Set_Slave(ParticleSystem *slave);
    static Coord3D *Compute_Point_On_Sphere();

private:
    Particle *m_systemParticlesHead;
    Particle *m_systemParticlesTail;
    uint32_t m_particleCount;
    ParticleSystemID m_systemID;
    DrawableID m_attachedToDrawableID;
    ObjectID m_attachedToObjectID;
    Matrix3D m_localTransform;
    Matrix3D m_transform;
    uint32_t m_burstDelayLeft;
    uint32_t m_delayLeft;
    uint32_t m_startTimestamp;
    uint32_t m_systemLifetimeLeft;
    uint32_t m_lastParticleID;
    float m_accumulatedSizeBonus;
    Coord3D m_velCoefficient;
    float m_countCoefficient;
    float m_delayCoefficient;
    float m_sizeCoefficient;
    Coord3D m_pos;
    Coord3D m_lastPos;
    ParticleSystem *m_slaveSystem;
    ParticleSystemID m_slaveID;
    ParticleSystem *m_masterSystem;
    ParticleSystemID m_masterID;
    const ParticleSystemTemplate *m_template;
    Particle *m_controlParticle;
    bool m_isLocalIdentity;
    bool m_isIdentity;
    bool m_isForever;
    bool m_isStopped;
    bool m_isDestroyed;
    bool m_isFirstPos;
    bool m_saveable;
    bool m_unkBool1;
};
