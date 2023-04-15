/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Academy Stats
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
#include "snapshot.h"
#include "unicodestring.h"

class Player;
class CommandSet;
class ThingTemplate;
class Object;
class UpgradeTemplate;
class SpecialPowerTemplate;

enum AcademyClassificationType
{
    ACT_NONE,
    ACT_UPGRADE_RADAR,
    ACT_SUPERPOWER,
};

struct AcademyAdviceInfo
{
    enum
    {
        MAX_ACADEMY_ADVICE = 1
    };

    Utf16String m_adviceString[MAX_ACADEMY_ADVICE];
    int m_adviceCount;
};

class AcademyStats : public SnapShot
{
public:
    AcademyStats() {}
    ~AcademyStats() {}

    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}

    void Init(const Player *player);
    void Update();
    void Record_Income();
    void Record_Production(const Object *produced, const Object *producer);
    void Record_Upgrade(const UpgradeTemplate *upgrade, bool is_grant);
    void Record_Special_Power_Used(const SpecialPowerTemplate *power);
    void Evaluate_Tier_1_Advice(AcademyAdviceInfo *info, int count);
    void Evaluate_Tier_2_Advice(AcademyAdviceInfo *info, int count);
    void Evaluate_Tier_3_Advice(AcademyAdviceInfo *info, int count);
    bool Calculate_Academy_Advice(AcademyAdviceInfo *info);

    void Increment_Mines_Disarmed() { m_minesClearedCount++; }
    void Increment_Used_Control_Groups() { m_controlGroupCount++; }
    void Increment_Traps_Used() { m_trapsUsed++; }
    void Increment_Double_Click_Moves_Used() { m_usedDoubleClickMove++; }
    void Increment_Cleared_Building_Count() { m_clearedBuildingCount++; }
    void Increment_Garrison_Count() { m_garrisonCount++; }
    void Increment_Tunnel_Count() { m_tunnelCount++; }
    void Increment_Firestorm_Count() { m_firestormCount++; }
    void Increment_Captured_Structure_Count() { m_capturedStructureCount++; }
    void Increment_Disguised_Unit_Count() { m_disguisedUnitCount++; }
    void Increment_Picked_Up_Salvage_Count() { m_pickedUpSalvageCount++; }
    void Increment_Killed_Pilot_Count() { m_killedPilotCount++; }
    void Increment_Drag_Select_Count() { m_dragSelectCount++; }

    void Add_Science_Points_Spent(int points) { m_sciencePointsSpentCount += points; }
    void Picked_Strategy_Plan() { m_pickedStrategyPlan = true; }
    void Set_Needs_Update(bool update) { m_needsUpdate = update; }

    unsigned int Get_Traps_Used() const { return m_trapsUsed; }
    unsigned int Get_Killed_Pilot_Count() const { return m_killedPilotCount; }
    bool Needs_Update() const { return m_needsUpdate; }

private:
    const Player *m_player;
    unsigned int m_lastUpdateFrame;
    bool m_needsUpdate;
    CommandSet *m_commandSet;
    bool m_skipCalc;
    const ThingTemplate *m_commandCenterTemplate;
    bool m_buildSupplyCenterOnTime;
    unsigned int m_supplyCenterCount;
    const ThingTemplate *m_supplyCenterTemplate;
    unsigned int m_money;
    bool m_hasBuiltRadar;
    unsigned int m_dozerCount;
    unsigned int m_capturedStructureCount;
    unsigned int m_sciencePointsSpentCount;
    unsigned int m_supersUsedCount;
    unsigned int m_garrisonCount;
    unsigned int m_idleFrame1;
    unsigned int m_idleFrame2;
    unsigned int m_dragSelectCount;
    unsigned int m_upgradeResearchCount;
    unsigned int m_powerFrame1;
    unsigned int m_powerFrame2;
    bool m_powerState;
    unsigned int m_gathererCount;
    unsigned int m_heroesCount;
    bool m_builtStrategyCenter;
    bool m_pickedStrategyPlan;
    unsigned int m_tunnelCount;
    bool m_usedTunnels;
    unsigned int m_controlGroupCount;
    unsigned int m_secondaryIncomeCount;
    unsigned int m_clearedBuildingCount;
    unsigned int m_pickedUpSalvageCount;
    unsigned int m_guardAbilityUseCount;
    unsigned int m_usedDoubleClickMove;
    bool m_builtBarracksOnTime;
    bool m_builtWarFactoryOnTime;
    bool m_builtTechStructureOnTime;
    unsigned int m_incomeFrame1;
    unsigned int m_incomeFrame2;
    unsigned int m_trapsUsed;
    unsigned int m_minesClearedCount;
    unsigned int m_unmannedVehiclesStolenCount;
    unsigned int m_killedPilotCount;
    unsigned int m_disguiserCount;
    unsigned int m_disguisedUnitCount;
    unsigned int m_firestormCount;
};
