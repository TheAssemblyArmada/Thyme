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

class Player;
class CommandSet;
class ThingTemplate;

enum AcademyClassificationType
{
    ACT_NONE,
    ACT_UPGRADE_RADAR,
    ACT_SUPERPOWER,
};

class AcademyStats : public SnapShot
{
public:
    void Init(const Player *player);
    void Update();
    void Record_Income();
    void Increment_Mines_Disarmed() { m_minesClearedCount++; }
    void Increment_Used_Control_Groups() { m_controlGroupCount++; }
    void Add_Science_Points_Spent(int points) { m_sciencePointsSpentCount += points; }

    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}

private:
    Player *m_player;
    unsigned int m_lastUupdateFrame;
    bool m_needsUpdate;
    CommandSet *m_commandSet;
    bool m_skipCalc;
    ThingTemplate *m_commandCenterTemplate;
    bool m_buildSupplyCenterOnTime;
    unsigned int m_supplyCenterCount;
    ThingTemplate *m_supplyCenterTemplate;
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
