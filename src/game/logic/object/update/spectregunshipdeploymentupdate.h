/**
 * @file
 *
 * @author xezon
 *
 * @brief Specter Gunship Deployment Update Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "specialpowertemplate.h"
#include "specialpowerupdatemodule.h"
#include "radiusdecal.h"

class SpectreGunshipDeploymentUpdate;

class SpectreGunshipDeploymentUpdateModuleData : public ModuleData
{
    friend class SpectreGunshipDeploymentUpdate;

public:
    SpectreGunshipDeploymentUpdateModuleData();
    virtual ~SpectreGunshipDeploymentUpdateModuleData() override;

    static void Build_Field_Parse(MultiIniFieldParse &parse);

private:
    enum GunshipCreateLocType
    {
        GUNSHIP_CREATE_AT_EDGE_NEAR_SOURCE,
        GUNSHIP_CREATE_AT_EDGE_FARTHEST_FROM_SOURCE,
        GUNSHIP_CREATE_AT_EDGE_NEAR_TARGET,
        GUNSHIP_CREATE_AT_EDGE_FARTHEST_FROM_TARGET,

        GUNSHIP_CREATE_LOC_COUNT
    };

    SpecialPowerTemplate *m_specialPowerTemplate;
    ScienceType m_requiredScience;
    int m_unk_0010;
    Utf8String m_gunshipTemplateName;
    Utf8String m_unk_0018;
    RadiusDecalTemplate m_radiusDecal1;
    RadiusDecalTemplate m_radiusDecal2;
    int m_unk_0054;
    float m_attackAreaRadius;
    int m_unk_005C;
    int m_unk_0060;
    GunshipCreateLocType m_createLocType;
    int m_unk_0068;
};

class SpectreGunshipDeploymentUpdate : public SpecialPowerUpdateModule
{
    IMPLEMENT_POOL(SpectreGunshipDeploymentUpdate)

protected:
    SpectreGunshipDeploymentUpdate(Thing *thing, const ModuleData *module_data);
    virtual ~SpectreGunshipDeploymentUpdate() override {}

public:
    static SpectreGunshipDeploymentUpdate *Friend_New_Module_Instance(Thing *thing, const ModuleData *module_data);
    static SpectreGunshipDeploymentUpdateModuleData *Friend_New_Module_Data(INI *ini);

    virtual NameKeyType Get_Module_Name_Key(void) const override;
    virtual void On_Object_Created() override;

    // Snapshot
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    //~Snapshot

    // BehaviorModule
    virtual SpecialPowerUpdateInterface *Get_Special_Power_Update_Interface() override;
    //~BehaviorModule

    // UpdateModule
    virtual UpdateSleepTime Update() override;
    virtual DisabledBitFlags Get_Disabled_Types_To_Process() const override;
    //~UpdateModule

    // SpecialPowerUpdateInterface
    virtual bool Does_Special_Power_Update_Pass_Science_Test(void) const override;
    virtual ScienceType Get_Extra_Required_Science() const override;
    virtual void Initiate_Intent_To_Do_Special_Power(
        const SpecialPowerTemplate *, const Object *, const Coord3D *, const Waypoint *, unsigned int) override;
    virtual bool Is_Special_Ability() const override;
    virtual bool Is_Special_Power() const override;
    virtual bool Is_Active() const override;
    virtual CommandOption Get_Command_Option() const override;
    virtual bool Does_Special_Power_Have_Overridable_Destination_Active() const override;
    virtual bool Does_Special_Power_Have_Overridable_Destination() const override;
    virtual void Set_Special_Power_Overridable_Destination(const Coord3D *) override;
    virtual bool Is_Power_Currently_In_Use(const CommandButton *) const override;
    //~SpecialPowerUpdateInterface
};
