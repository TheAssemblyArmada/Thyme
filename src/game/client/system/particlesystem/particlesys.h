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
    IMPLEMENT_NAMED_POOL(ParticleSystem, ParticleSystemPool);
    friend class ParticleSystemManager;

public:
    ParticleSystem(const ParticleSystemTemplate *temp, ParticleSystemID id, bool create_slaves);
    virtual ~ParticleSystem();

    virtual void Update(int unk);
    virtual Particle *Create_Particle(const ParticleInfo &info, ParticlePriorityType priority, bool always_render);
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    ParticleSystemID System_ID() const { return m_systemID; }
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
    ParticlePriorityType Get_Priority() const { return m_priority; }
    void Set_Lifetime_Range(float min, float max) { m_lifetime.Set_Range(min, max, GameClientRandomVariable::UNIFORM); }

    static ParticleInfo Merge_Related_Systems(ParticleSystem *master, ParticleSystem *slave, bool promote_slave);

#ifdef GAME_DLL
    static void Hook_Me();
    ParticleSystem *Hook_Ctor(const ParticleSystemTemplate *temp, ParticleSystemID id, bool create_slaves)
    {
        return new (this) ParticleSystem(temp, id, create_slaves);
    }
    void Hook_Dtor() { ParticleSystem::~ParticleSystem(); }
    //void Hook_Update(int unk) { ParticleSystem::Update(unk); }
    Particle *Hook_Create_Particle(const ParticleInfo &info, ParticlePriorityType priority, bool always_render)
    {
        return ParticleSystem::Create_Particle(info, priority, always_render);
    }
    void Hook_Xfer(Xfer *xfer) { ParticleSystem::Xfer_Snapshot(xfer); }
    void Hook_LoadPP() { ParticleSystem::Load_Post_Process(); }
#endif
private:
    ParticleInfo *Generate_Particle_Info(int id, int count);
    Coord3D *Compute_Particle_Velocity(const Coord3D *pos);
    Coord3D *Compute_Particle_Position();
    void Update_Wind_Motion();
    void Set_Master(ParticleSystem *master);
    void Set_Slave(ParticleSystem *slave);
    void Remove_Master();
    void Remove_Slave();
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

#ifdef GAME_DLL
#include "hooker.h"

inline void ParticleSystem::Hook_Me()
{
    Hook_Method(0x004CDA10, &ParticleSystem::Hook_Ctor);
    Hook_Method(0x004CE310, &ParticleSystem::Hook_Dtor);
    Hook_Method(0x004D11E0, &ParticleSystem::Hook_Xfer);
    Hook_Method(0x004D1430, &ParticleSystem::Hook_LoadPP);
    Hook_Method(0x004CE500, &ParticleSystem::Destroy);
    Hook_Method(0x004CE530, &ParticleSystem::Get_Position);
    Hook_Method(0x004CE570, &ParticleSystem::Set_Position);
    Hook_Method(0x004CE5A0, &ParticleSystem::Set_Local_Transform);
    Hook_Method(0x004CE620, &ParticleSystem::Rotate_Local_Transform_X);
    Hook_Method(0x004CE6C0, &ParticleSystem::Rotate_Local_Transform_Y);
    Hook_Method(0x004CE760, &ParticleSystem::Rotate_Local_Transform_Z);
    Hook_Method(0x004CE860, &ParticleSystem::Compute_Particle_Velocity);
    Hook_Method(0x004CEF60, &ParticleSystem::Compute_Particle_Position);
    Hook_Method(0x004CF530, &ParticleSystem::Hook_Create_Particle);
    Hook_Method(0x004CF750, &ParticleSystem::Generate_Particle_Info);
    Hook_Method(0x004D0920, &ParticleSystem::Update_Wind_Motion);
    Hook_Function(0x004D0B30, &ParticleSystem::Merge_Related_Systems);
}

#endif
