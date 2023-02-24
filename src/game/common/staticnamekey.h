/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Holds static mappings of names to a name key.
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
#include "namekeygenerator.h"

class StaticNameKey
{
public:
    StaticNameKey(const char *name) : m_key(NAMEKEY_INVALID), m_name(name) {}

    operator NameKeyType() { return Key(); }

    NameKeyType Key();
    const char *Name() { return m_name; }

private:
    NameKeyType m_key;
    const char *m_name;
};

#ifdef GAME_DLL
extern StaticNameKey &g_teamNameKey;
extern StaticNameKey &g_theInitialCameraPositionKey;
extern StaticNameKey &g_playerNameKey;
extern StaticNameKey &g_playerIsHumanKey;
extern StaticNameKey &g_playerDisplayNameKey;
extern StaticNameKey &g_playerFactionKey;
extern StaticNameKey &g_playerAlliesKey;
extern StaticNameKey &g_playerEnemiesKey;
extern StaticNameKey &g_teamOwnerKey;
extern StaticNameKey &g_teamIsSingletonKey;
extern StaticNameKey &g_objectInitialHealthKey;
extern StaticNameKey &g_objectEnabledKey;
extern StaticNameKey &g_objectIndestructibleKey;
extern StaticNameKey &g_objectUnsellableKey;
extern StaticNameKey &g_objectPoweredKey;
extern StaticNameKey &g_objectRecruitableAIKey;
extern StaticNameKey &g_objectTargetableKey;
extern StaticNameKey &g_originalOwnerKey;
extern StaticNameKey &g_uniqueIDKey;
extern StaticNameKey &g_waypointIDKey;
extern StaticNameKey &g_waypointNameKey;
extern StaticNameKey &g_weatherKey;
extern StaticNameKey &g_lightHeightAboveTerrainKey;
extern StaticNameKey &g_scorchTypeKey;
#else
extern StaticNameKey g_teamNameKey;
extern StaticNameKey g_theInitialCameraPositionKey;
extern StaticNameKey g_playerNameKey;
extern StaticNameKey g_playerIsHumanKey;
extern StaticNameKey g_playerDisplayNameKey;
extern StaticNameKey g_playerFactionKey;
extern StaticNameKey g_playerAlliesKey;
extern StaticNameKey g_playerEnemiesKey;
extern StaticNameKey g_teamOwnerKey;
extern StaticNameKey g_teamIsSingletonKey;
extern StaticNameKey g_objectInitialHealthKey;
extern StaticNameKey g_objectEnabledKey;
extern StaticNameKey g_objectIndestructibleKey;
extern StaticNameKey g_objectUnsellableKey;
extern StaticNameKey g_objectPoweredKey;
extern StaticNameKey g_objectRecruitableAIKey;
extern StaticNameKey g_objectTargetableKey;
extern StaticNameKey g_originalOwnerKey;
extern StaticNameKey g_uniqueIDKey;
extern StaticNameKey g_waypointIDKey;
extern StaticNameKey g_waypointNameKey;
extern StaticNameKey g_weatherKey;
extern StaticNameKey g_lightHeightAboveTerrainKey;
extern StaticNameKey g_scorchTypeKey;
#endif

extern StaticNameKey g_teamUnitMinCount1Key;
extern StaticNameKey g_teamUnitMaxCount1Key;
extern StaticNameKey g_teamUnitType1Key;
extern StaticNameKey g_teamUnitMinCount2Key;
extern StaticNameKey g_teamUnitMaxCount2Key;
extern StaticNameKey g_teamUnitType2Key;
extern StaticNameKey g_teamUnitMinCount3Key;
extern StaticNameKey g_teamUnitMaxCount3Key;
extern StaticNameKey g_teamUnitType3Key;
extern StaticNameKey g_teamUnitMinCount4Key;
extern StaticNameKey g_teamUnitMaxCount4Key;
extern StaticNameKey g_teamUnitType4Key;
extern StaticNameKey g_teamUnitMinCount5Key;
extern StaticNameKey g_teamUnitMaxCount5Key;
extern StaticNameKey g_teamUnitType5Key;
extern StaticNameKey g_teamUnitMinCount6Key;
extern StaticNameKey g_teamUnitMaxCount6Key;
extern StaticNameKey g_teamUnitType6Key;
extern StaticNameKey g_teamUnitMinCount7Key;
extern StaticNameKey g_teamUnitMaxCount7Key;
extern StaticNameKey g_teamUnitType7Key;
extern StaticNameKey g_teamHomeKey;
extern StaticNameKey g_teamOnCreateScriptKey;
extern StaticNameKey g_teamIsAIRecruitableKey;
extern StaticNameKey g_teamIsBaseDefenseKey;
extern StaticNameKey g_teamIsPerimeterDefenseKey;
extern StaticNameKey g_teamAutoReinforceKey;
extern StaticNameKey g_teamAggressivenessKey;
extern StaticNameKey g_teamTransportsReturnKey;
extern StaticNameKey g_teamAvoidThreatsKey;
extern StaticNameKey g_teamAttackCommonTargetKey;
extern StaticNameKey g_teamMaxInstancesKey;
extern StaticNameKey g_teamOnIdleScriptKey;
extern StaticNameKey g_teamInitialIdleFramesKey;
extern StaticNameKey g_teamEnemySightedScriptKey;
extern StaticNameKey g_teamAllClearScriptKey;
extern StaticNameKey g_teamOnDestroyedScriptKey;
extern StaticNameKey g_teamDestroyedThresholdKey;
extern StaticNameKey g_teamOnUnitDestroyedScriptKey;
extern StaticNameKey g_teamProductionPriorityKey;
extern StaticNameKey g_teamProductionPrioritySuccessIncreaseKey;
extern StaticNameKey g_teamProductionPriorityFailureDecreaseKey;
extern StaticNameKey g_teamProductionConditionKey;
extern StaticNameKey g_teamExecutesActionsOnCreateKey;
extern StaticNameKey g_teamGenericScriptHookKey;
extern StaticNameKey g_teamTransportKey;
extern StaticNameKey g_teamTransportsExitKey;
extern StaticNameKey g_teamStartsFullKey;
extern StaticNameKey g_teamReinforcementOriginKey;
extern StaticNameKey g_teamVeterancyKey;
extern StaticNameKey g_multiplayerIsLocalKey;
