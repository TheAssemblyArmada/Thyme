/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Battle Plan Update
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
#include "player.h"
#include "specialpowerupdatemodule.h"

enum TransitionStatus
{
    TRANSITION_STATUS_NONE,
    TRANSITION_STATUS_UNPACKING,
    TRANSITION_STATUS_IDLE,
    TRANSITION_STATUS_PACKING,
};

class BattlePlanUpdate : public SpecialPowerUpdateModule
{
    IMPLEMENT_POOL(BattlePlanUpdate)
public:
    virtual ~BattlePlanUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Object_Created() override;
    virtual void On_Delete() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual SpecialPowerUpdateInterface *Get_Special_Power_Update_Interface() override;

    virtual UpdateSleepTime Update() override;

    virtual void Initiate_Intent_To_Do_Special_Power(const SpecialPowerTemplate *special_power_template,
        const Object *target_obj,
        const Coord3D *target_pos,
        const Waypoint *waypoint,
        unsigned int options) override;
    virtual bool Is_Special_Ability() const override;
    virtual bool Is_Special_Power() const override;
    virtual bool Is_Active() const override;
    virtual CommandOption Get_Command_Option() const override;
    virtual bool Does_Special_Power_Have_Overridable_Destination_Active() const override;
    virtual bool Does_Special_Power_Have_Overridable_Destination() const override;
    virtual void Set_Special_Power_Overridable_Destination(const Coord3D *destination) override;
    virtual bool Is_Power_Currently_In_Use(const CommandButton *button) const override;

    BattlePlanStatus Get_Active_Battle_Plan();

private:
    BattlePlanStatus m_battlePlanUnk1;
    BattlePlanStatus m_battlePlanUnk2;
    BattlePlanStatus m_activeBattlePlan;
    TransitionStatus m_transitionStatus;
    unsigned int m_updateFrame;
    SpecialPowerModuleInterface *m_specialPower;
    bool m_objectCreated;
    bool m_turretOutOfPosition;
    BattlePlanBonuses *m_battlePlanBonuses;
    AudioEventRTS m_bombardmentPlanUnpackSound;
    AudioEventRTS m_bombardmentPlanPackSound;
    AudioEventRTS m_bombardmentAnnouncement;
    AudioEventRTS m_searchAndDestroyPlanUnpackSound;
    AudioEventRTS m_searchAndDestroyPlanIdleLoopSound;
    AudioEventRTS m_searchAndDestroyPlanPackSound;
    AudioEventRTS m_searchAndDestroyAnnouncement;
    AudioEventRTS m_holdTheLinePlanUnpackSound;
    AudioEventRTS m_holdTheLinePlanPackSound;
    AudioEventRTS m_holdTheLineAnnouncement;
    ObjectID m_visionObject;
};
