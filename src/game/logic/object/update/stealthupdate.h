/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Stealth Update
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
#include "updatemodule.h"
class FXList;

class StealthUpdateModuleData : public UpdateModuleData
{
    friend class StealthUpdate;

public:
    virtual ~StealthUpdateModuleData() override;

private:
    BitFlags<OBJECT_STATUS_COUNT> m_hintDetectableConditions;
    BitFlags<OBJECT_STATUS_COUNT> m_requiredStatus;
    BitFlags<OBJECT_STATUS_COUNT> m_forbiddenStatus;
    FXList *m_disguiseRevealFX;
    FXList *m_disguiseFX;
    float m_moveThresholdSpeed;
    float m_friendlyOpacityMin;
    float m_friendlyOpacityMax;
    float m_revealDistanceFromTarget;
    unsigned int m_disguiseTransitionTime;
    unsigned int m_disguiseRevealTransitionTime;
    unsigned int m_pulseFrequency;
    unsigned int m_stealthDelay;
    int m_stealthLevel;
    unsigned int m_blackMarketCheckDelay;
    int m_enemyDetectionEvaEvent;
    int m_ownDetectionEvaEvent;
    bool m_innateStealth;
    bool m_orderIdleEnemiesToAttackMeUponReveal;
    bool m_disguisesAsTeam;
    bool m_useRiderStealth;
    bool m_grantedBySpecialPower;
};

class StealthUpdate : public UpdateModule
{
    IMPLEMENT_POOL(StealthUpdate)

public:
    virtual ~StealthUpdate() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual StealthUpdate *Get_Steath() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual UpdateSleepTime Update() override;
    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;

    const StealthUpdateModuleData *Get_Stealth_Update_Module_Data() const
    {
        return static_cast<const StealthUpdateModuleData *>(Module::Get_Module_Data());
    }

    float Get_Friendly_Opacity() const { return Get_Stealth_Update_Module_Data()->m_friendlyOpacityMin; }
    bool Get_Granted_By_Special_Power() const { return Get_Stealth_Update_Module_Data()->m_grantedBySpecialPower; }
    bool Has_Disguised_Template() const { return m_disguisedTemplate != nullptr; }
    ThingTemplate *Get_Disguised_Template() const { return m_disguisedTemplate; }
    int Get_Player_Index() const { return m_playerIndex; }
    unsigned int Get_Stealth_Delay() const { return Get_Stealth_Update_Module_Data()->m_stealthDelay; }

private:
    unsigned int m_framesTillStealthAllowed;
    unsigned int m_framesTillDetectionExpires;
    unsigned int m_blackMarketCheckFrame;
    bool m_unk4;
    float m_unk5;
    float m_unk6;
    int m_playerIndex;
    ThingTemplate *m_disguisedTemplate;
    unsigned int m_disguiseTransitionTime;
    bool m_disguised;
    bool m_disguising;
    bool m_revealDisguise;
    unsigned int m_receiveGrantTime;
    bool m_unk7;
    int m_unk8;
};
