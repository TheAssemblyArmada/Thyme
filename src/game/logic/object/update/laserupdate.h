/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Laser Update
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
#include "clientupdatemodule.h"
#include "coord.h"
#include "particlesysmanager.h"

class LaserUpdateModuleData : public ModuleData
{
public:
    LaserUpdateModuleData();
    virtual ~LaserUpdateModuleData() override;
    static void Build_Field_Parse(MultiIniFieldParse &p);

private:
    Utf8String m_muzzleParticleSystem;
    Utf8String m_targetParticleSystem;
    float m_punchThroughScalar;
    friend class LaserUpdate;
};

class LaserUpdate : public ClientUpdateModule
{
    IMPLEMENT_POOL(LaserUpdate);

public:
    LaserUpdate(Thing *thing, ModuleData const *module_data);
    virtual ~LaserUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void Client_Update() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);
    float Get_Current_Laser_Radius() const;
    void Init_Laser(Object const *source_obj,
        Object const *victim_obj,
        Coord3D const *start_pos,
        Coord3D const *end_pos,
        Utf8String master_bone,
        int width);
    void Update_End_Pos();
    void Update_Start_Pos();
    void Set_Decay_Frames(unsigned int frames);

    const Coord3D &Get_Start_Pos() const { return m_startPos; }
    const Coord3D &Get_End_Pos() const { return m_endPos; }
    void Set_Dirty(bool dirty) { m_dirty = dirty; }
    bool Is_Dirty() const { return m_dirty; }
    float Get_Width() const { return m_width; }
    const LaserUpdateModuleData *Get_Laser_Update_Module_Data() const
    {
        return static_cast<const LaserUpdateModuleData *>(Module::Get_Module_Data());
    }

private:
    Coord3D m_startPos;
    Coord3D m_endPos;
    DrawableID m_sourceDrawableID;
    DrawableID m_victimDrawableID;
    bool m_dirty;
    ParticleSystemID m_muzzleParticleSystemID;
    ParticleSystemID m_targetParticleSystemID;
    bool m_grow;
    bool m_shrink;
    unsigned int m_growStartFrame;
    unsigned int m_growEndFrame;
    float m_width;
    unsigned int m_shrinkStartFrame;
    unsigned int m_shrinkEndFrame;
    Utf8String m_masterBone;
};