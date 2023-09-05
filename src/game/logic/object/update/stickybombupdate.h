/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Sticky Bomb Update
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
#include "gamelogic.h"
#include "updatemodule.h"

struct StickyBombUpdateModuleData : public UpdateModuleData
{
public:
    ~StickyBombUpdateModuleData() override;

private:
    Utf8String m_attachToBone;
    float m_offsetZ;
    WeaponTemplate *m_geometryBasedDamageWeapon;
    FXList *m_geometryBasedDamageFX;
};

class StickyBombUpdate : public UpdateModule
{
    IMPLEMENT_POOL(StickyBombUpdate);

public:
    ~StickyBombUpdate() override;
    NameKeyType Get_Module_Name_Key() const override;
    void On_Object_Created() override;
    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;
    UpdateSleepTime Update() override;

    unsigned int Get_Die_Frame() const { return m_dieFrame; }
    bool Has_Die_Frame() const { return m_dieFrame != 0; }
    Object *Get_Target_Object() const { return g_theGameLogic->Find_Object_By_ID(m_targetID); }

    void Detonate();

private:
    ObjectID m_targetID;
    unsigned int m_dieFrame;
    unsigned int m_unk;
};
