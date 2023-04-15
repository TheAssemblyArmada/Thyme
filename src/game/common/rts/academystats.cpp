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
#include "academystats.h"
#include "controlbar.h"
#include "gamelogic.h"
#include "gametext.h"
#include "globaldata.h"
#include "opencontain.h"
#include "player.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "randomvalue.h"
#include "specialpower.h"
#include "upgrade.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void Find_Dozer_Command_Set(Object *obj, void *data)
{
    if (data == nullptr && obj != nullptr && obj->Is_KindOf(KINDOF_DOZER)) {
        // TODO: Investigate possible bug here, the code does nothing with the return value of Find_Command_Set
        g_theControlBar->Find_Command_Set(obj->Get_Command_Set_String());
    }
}

void AcademyStats::Init(const Player *player)
{
    if (g_theGameLogic != nullptr) {
        m_lastUpdateFrame = g_theGameLogic->Get_Frame() + 30;
        m_needsUpdate = true;
        m_skipCalc = false;
        m_player = player;
        const PlayerTemplate *tmplate = player->Get_Player_Template();

        if (tmplate != nullptr) {
            bool skip_calc = true;

            if (tmplate->Get_Base_Side().Compare_No_Case("USA") == 0) {
                skip_calc = false;
            }

            if (tmplate->Get_Base_Side().Compare_No_Case("China") == 0) {
                skip_calc = false;
            }

            if (tmplate->Get_Base_Side().Compare_No_Case("GLA") == 0) {
                skip_calc = false;
            }

            if (skip_calc) {
                m_skipCalc = true;
            }
        } else {
            m_skipCalc = true;
        }

        m_commandSet = nullptr;
        player->Iterate_Objects(Find_Dozer_Command_Set, m_commandSet);
        m_commandCenterTemplate = nullptr;
        m_supplyCenterTemplate = nullptr;

        if (m_commandSet != nullptr) {
            for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                const CommandButton *button = m_commandSet->Get_Command_Button(i);

                if (button != nullptr) {
                    const ThingTemplate *thing = *button->Get_Template();

                    if (thing != nullptr) {
                        if (thing->Is_KindOf(KINDOF_FS_SUPPLY_CENTER)) {
                            m_supplyCenterTemplate = thing;
                            m_money = m_supplyCenterTemplate->Calc_Cost_To_Build(player);
                        } else if (thing->Is_KindOf(KINDOF_COMMANDCENTER)) {
                            m_commandCenterTemplate = thing;
                        }
                    }
                }
            }
        }

        m_buildSupplyCenterOnTime = false;
        m_supplyCenterCount = 0;

        if (m_supplyCenterTemplate == nullptr) {
            m_money = 1000;
        }

        m_hasBuiltRadar = false;
        m_dozerCount = 0;
        m_capturedStructureCount = 0;
        m_sciencePointsSpentCount = 0;
        m_supersUsedCount = 0;
        m_garrisonCount = 0;
        m_idleFrame1 = 0;
        m_idleFrame2 = 0;
        m_dragSelectCount = 0;
        m_upgradeResearchCount = 0;
        m_powerFrame1 = 0;
        m_powerFrame2 = 0;
        m_powerState = false;
        m_gathererCount = 0;
        m_heroesCount = 0;
        m_builtStrategyCenter = false;
        m_pickedStrategyPlan = false;
        m_usedTunnels = false;
        m_tunnelCount = 0;
        m_controlGroupCount = 0;
        m_secondaryIncomeCount = 0;
        m_clearedBuildingCount = 0;
        m_pickedUpSalvageCount = 0;
        m_guardAbilityUseCount = 0;
        m_usedDoubleClickMove = 0;
        m_builtBarracksOnTime = false;
        m_builtWarFactoryOnTime = false;
        m_builtTechStructureOnTime = false;
        m_incomeFrame1 = 0;
        m_incomeFrame2 = 0;
        m_trapsUsed = 0;
        m_minesClearedCount = 0;
        m_unmannedVehiclesStolenCount = 0;
        m_killedPilotCount = 0;
        m_disguiserCount = 0;
        m_disguisedUnitCount = 0;
        m_firestormCount = 0;
    }
}

void Update_Academy_Stats(Object *obj, void *data)
{
    AcademyStats *stats = static_cast<AcademyStats *>(data);

    if (obj != nullptr && stats != nullptr && stats->Needs_Update()) {
        stats->Record_Production(obj, nullptr);
    }
}

void AcademyStats::Update()
{
    if (!m_skipCalc) {
        unsigned int frame = g_theGameLogic->Get_Frame();

        if (m_lastUpdateFrame >= frame) {
            m_lastUpdateFrame = frame;
            m_player->Iterate_Objects(Update_Academy_Stats, this);

            if (m_supplyCenterCount == 0 && !m_buildSupplyCenterOnTime) {
                const Money *money = m_player->Get_Money();

                if (money != nullptr) {
                    if (money->Count_Money() < m_money) {
                        m_buildSupplyCenterOnTime = true;
                    }
                }
            }

            const Energy *energy = m_player->Get_Energy();

            if (energy != nullptr) {
                bool has_power = energy->Has_Sufficient_Power();

                if (has_power != m_powerState) {
                    if (has_power) {
                        unsigned int power_frame = frame - m_powerFrame2;

                        if (power_frame > m_powerFrame1) {
                            m_powerFrame1 = power_frame;
                        }
                    } else {
                        m_powerFrame2 = frame;
                    }

                    m_powerState = has_power;
                }
            }

            if (Needs_Update()) {
                Set_Needs_Update(false);
            }
        }
    }
}

void AcademyStats::Record_Production(const Object *produced, const Object *producer)
{
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (produced->Is_KindOf(KINDOF_FS_SUPPLY_CENTER)) {
        m_supplyCenterCount++;
    }

    if (produced->Is_KindOf(KINDOF_DOZER)) {
        m_dozerCount++;
    }

    if (produced->Is_KindOf(KINDOF_INFANTRY) || produced->Is_KindOf(KINDOF_VEHICLE) || produced->Is_KindOf(KINDOF_DOZER)
        || produced->Is_KindOf(KINDOF_HARVESTER)) {
        unsigned int idle_frame = frame - m_idleFrame2;

        if (idle_frame > m_idleFrame1) {
            m_idleFrame1 = idle_frame;
        }

        m_idleFrame2 = frame;
    }

    if (produced->Is_KindOf(KINDOF_HARVESTER)) {
        m_gathererCount++;
    }

    if (produced->Is_KindOf(KINDOF_HERO)) {
        m_heroesCount++;
    }

    if (produced->Is_KindOf(KINDOF_FS_STRATEGY_CENTER)) {
        m_builtStrategyCenter = true;
    }

    if (produced->Get_Contain() != nullptr) {
        if (produced->Get_Contain()->Is_Tunnel_Contain()) {
            m_usedTunnels = true;
        }
    }

    if (produced->Is_KindOf(KINDOF_MONEY_HACKER) || produced->Is_KindOf(KINDOF_FS_BLACK_MARKET)
        || produced->Is_KindOf(KINDOF_FS_SUPPLY_DROPZONE)) {
        m_secondaryIncomeCount++;
    }

    if (produced->Is_KindOf(KINDOF_FS_BARRACKS) && g_theGameLogic->Get_Frame() <= 9000) {
        m_builtBarracksOnTime = true;
    }

    if (produced->Is_KindOf(KINDOF_FS_WARFACTORY) && g_theGameLogic->Get_Frame() <= 18000) {
        m_builtWarFactoryOnTime = true;
    }

    if (produced->Is_KindOf(KINDOF_FS_ADVANCED_TECH) && g_theGameLogic->Get_Frame() <= 27000) {
        m_builtTechStructureOnTime = true;
    }

    if (produced->Is_KindOf(KINDOF_DISGUISER)) {
        m_disguiserCount++;
    }
}

void AcademyStats::Record_Upgrade(const UpgradeTemplate *upgrade, bool is_grant)
{
    if (upgrade->Get_Academy_Classify() == ACT_UPGRADE_RADAR) {
        m_hasBuiltRadar = true;
    }

    if (!is_grant) {
        m_upgradeResearchCount++;
    }
}

void AcademyStats::Record_Special_Power_Used(const SpecialPowerTemplate *power)
{
    if (power->Get_Academy_Classify() == ACT_SUPERPOWER) {
        m_supersUsedCount++;
    }
}

void AcademyStats::Record_Income()
{
    unsigned int frame = g_theGameLogic->Get_Frame();

    unsigned int income_frame = m_incomeFrame1 - frame;

    if (income_frame > m_incomeFrame2) {
        m_incomeFrame2 = income_frame;
    }

    m_incomeFrame1 = frame;
}

void AcademyStats::Evaluate_Tier_1_Advice(AcademyAdviceInfo *info, int count)
{
    int max_advice_count = AcademyAdviceInfo::MAX_ACADEMY_ADVICE;
    unsigned int frame = g_theGameLogic->Get_Frame();
    bool give_advice = count != -1;
    int advice_count = 0;

    if (m_buildSupplyCenterOnTime) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:BuildSupplyCenterEarlier"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!m_hasBuiltRadar) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:TryBuildingRadar"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_dozerCount < 2) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:BuildMorePeons"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_capturedStructureCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:TryCapturingStructures"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_sciencePointsSpentCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:SpendGeneralsPoints"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_supersUsedCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:TryUsingSuperweapons"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_garrisonCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:TryGarrisoningAStructure"));
            info->m_adviceCount++;
        }

        count--;
    }

    unsigned int idle_frame = frame - m_idleFrame2;

    if (idle_frame > m_idleFrame1) {
        m_idleFrame1 = idle_frame;
    }

    if (m_idleFrame1 > 9000 || m_idleFrame2 == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:IdleBuildingUnits"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_dragSelectCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:TryDragSelectingUnits"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_upgradeResearchCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:ResearchUpgrades"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!m_powerState) {
        unsigned int power_frame = frame - m_powerFrame2;

        if (power_frame > m_powerFrame1) {
            m_powerFrame1 = power_frame;
        }
    }

    if (m_powerFrame1 > 18000) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:RanOutOfPower"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_gathererCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:BuildMoreGatherers"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_heroesCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:BuildAHero"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!give_advice && advice_count > 0) {
        Evaluate_Tier_1_Advice(info, advice_count);
    }
}

void AcademyStats::Evaluate_Tier_2_Advice(AcademyAdviceInfo *info, int count)
{
    int max_advice_count = AcademyAdviceInfo::MAX_ACADEMY_ADVICE;
    bool give_advice = count != -1;
    int advice_count = 0;

    if (m_builtStrategyCenter && !m_pickedStrategyPlan) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:PickStrategyCenterPlan"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_usedTunnels && m_tunnelCount != 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:UseTunnelNetwork"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_controlGroupCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:UseControlGroups"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_secondaryIncomeCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:UseSecondaryIncomeMethods"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_clearedBuildingCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:ClearBuildings"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_player->Get_Player_Template() != nullptr) {
        if (m_player->Get_Player_Template()->Get_Base_Side().Compare_No_Case("GLA") && m_pickedUpSalvageCount == 0) {
            advice_count++;

            if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
                info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:PickUpSalvage"));
                info->m_adviceCount++;
            }

            count--;
        }
    }

    if (m_guardAbilityUseCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:UseGuardAbility"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_supplyCenterCount < 2) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:MultipleSupplyCenters"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!give_advice && advice_count > 0) {
        Evaluate_Tier_2_Advice(info, advice_count);
    }
}

void AcademyStats::Evaluate_Tier_3_Advice(AcademyAdviceInfo *info, int count)
{
    int max_advice_count = AcademyAdviceInfo::MAX_ACADEMY_ADVICE;
    unsigned int frame = g_theGameLogic->Get_Frame();
    bool give_advice = count != -1;
    int advice_count = 0;

    if (!g_theWriteableGlobalData->m_alternateMouseEnabled) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:AlternateMouseInterface"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!m_usedDoubleClickMove) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:DoubleClickAttackMoveGuard"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!m_builtBarracksOnTime) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:BuildBarracksSooner"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!m_builtWarFactoryOnTime) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:BuildWarFactorySooner"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (!m_builtTechStructureOnTime) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:BuildTechStructureSooner"));
            info->m_adviceCount++;
        }

        count--;
    }

    unsigned int income_frame = m_incomeFrame1 - frame;

    if (income_frame > m_incomeFrame2) {
        m_incomeFrame2 = income_frame;
    }

    if (m_incomeFrame2 > 3600 || m_incomeFrame1 == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:NoIncome"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (g_thePlayerList->Get_Local_Player()->Get_Academy_Stats()->Get_Traps_Used() != 0 && m_minesClearedCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            // BUGFIX Change string here from NoIncome to ClearMines (clearly the intended string)
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:ClearMines"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (g_thePlayerList->Get_Neutral_Player()->Get_Academy_Stats()->Get_Killed_Pilot_Count() != 0
        && m_unmannedVehiclesStolenCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:UnmannedVehicles"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_disguiserCount != 0 && m_disguisedUnitCount == 0) {
        advice_count++;

        if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
            info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:DisguisedUnits"));
            info->m_adviceCount++;
        }

        count--;
    }

    if (m_player->Get_Player_Template() != nullptr) {
        if (m_player->Get_Player_Template()->Get_Base_Side().Compare_No_Case("China") && m_firestormCount == 0) {
            advice_count++;

            if (give_advice && Get_Client_Random_Value(0, count - 1) < max_advice_count - info->m_adviceCount) {
                info->m_adviceString[info->m_adviceCount].Concat(g_theGameText->Fetch("ACADEMY:Firestorm"));
                info->m_adviceCount++;
            }

            count--;
        }
    }

    if (!give_advice && advice_count > 0) {
        Evaluate_Tier_3_Advice(info, advice_count);
    }
}

bool AcademyStats::Calculate_Academy_Advice(AcademyAdviceInfo *info)
{
    int max_advice_count = AcademyAdviceInfo::MAX_ACADEMY_ADVICE;

    if (m_skipCalc) {
        return false;
    }

    if (info == nullptr) {
        captainslog_dbgassert(false, "AcademyStats::Calculate_Academy_Advice() was passed in NULL AcademyAdviceInfo.");
        return false;
    } else {
        info->m_adviceCount = 0;

        for (int i = 0; i < max_advice_count; i++) {
            Utf16String str;
            str = "\n\n";
            info->m_adviceString[i].Format(str);
        }

        Evaluate_Tier_1_Advice(info, -1);

        if (info->m_adviceCount < max_advice_count) {
            Evaluate_Tier_2_Advice(info, -1);

            if (info->m_adviceCount < max_advice_count) {
                Evaluate_Tier_3_Advice(info, -1);
            }
        }

        return info->m_adviceCount != 0;
    }
}

void AcademyStats::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUnsignedInt(&m_lastUpdateFrame);
    xfer->xferBool(&m_needsUpdate);
    xfer->xferBool(&m_skipCalc);
    xfer->xferBool(&m_buildSupplyCenterOnTime);
    xfer->xferUnsignedInt(&m_supplyCenterCount);
    xfer->xferUnsignedInt(&m_money);
    xfer->xferBool(&m_hasBuiltRadar);
    xfer->xferUnsignedInt(&m_dozerCount);
    xfer->xferUnsignedInt(&m_capturedStructureCount);
    xfer->xferUnsignedInt(&m_sciencePointsSpentCount);
    xfer->xferUnsignedInt(&m_supersUsedCount);
    xfer->xferUnsignedInt(&m_garrisonCount);
    xfer->xferUnsignedInt(&m_idleFrame1);
    xfer->xferUnsignedInt(&m_idleFrame2);
    xfer->xferUnsignedInt(&m_dragSelectCount);
    xfer->xferUnsignedInt(&m_upgradeResearchCount);
    xfer->xferUnsignedInt(&m_powerFrame1);
    xfer->xferUnsignedInt(&m_powerFrame2);
    xfer->xferBool(&m_powerState);
    xfer->xferUnsignedInt(&m_gathererCount);
    xfer->xferUnsignedInt(&m_heroesCount);
    xfer->xferBool(&m_builtStrategyCenter);
    xfer->xferBool(&m_pickedStrategyPlan);
    xfer->xferUnsignedInt(&m_tunnelCount);
    xfer->xferBool(&m_usedTunnels);
    xfer->xferUnsignedInt(&m_controlGroupCount);
    xfer->xferUnsignedInt(&m_secondaryIncomeCount);
    xfer->xferUnsignedInt(&m_clearedBuildingCount);
    xfer->xferUnsignedInt(&m_pickedUpSalvageCount);
    xfer->xferUnsignedInt(&m_guardAbilityUseCount);
    xfer->xferUnsignedInt(&m_usedDoubleClickMove);
    xfer->xferBool(&m_builtBarracksOnTime);
    xfer->xferBool(&m_builtWarFactoryOnTime);
    xfer->xferBool(&m_builtTechStructureOnTime);
    xfer->xferUnsignedInt(&m_incomeFrame1);
    xfer->xferUnsignedInt(&m_incomeFrame2);
    xfer->xferUnsignedInt(&m_trapsUsed);
    xfer->xferUnsignedInt(&m_minesClearedCount);
    xfer->xferUnsignedInt(&m_unmannedVehiclesStolenCount);
    xfer->xferUnsignedInt(&m_killedPilotCount);
    xfer->xferUnsignedInt(&m_disguiserCount);
    xfer->xferUnsignedInt(&m_disguisedUnitCount);
    xfer->xferUnsignedInt(&m_firestormCount);
}
