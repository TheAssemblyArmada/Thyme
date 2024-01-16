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
extern StaticNameKey g_playerIsSkirmishKey;
extern StaticNameKey g_playerIsPreorderKey;
extern StaticNameKey g_multiplayerStartIndexKey;
extern StaticNameKey g_skirmishDifficultyKey;
extern StaticNameKey g_playerColorKey;
extern StaticNameKey g_playerNightColorKey;
extern StaticNameKey g_playerStartMoneyKey;
extern StaticNameKey g_objectNameKey;
extern StaticNameKey g_objectMaxHPs;
extern StaticNameKey g_objectInitialHealth;
extern StaticNameKey g_objectVeterancy;
extern StaticNameKey g_objectAggressiveness;
extern StaticNameKey g_objectRecruitableAI;
extern StaticNameKey g_objectSelectable;
extern StaticNameKey g_objectStoppingDistance;
extern StaticNameKey g_objectEnabled;
extern StaticNameKey g_objectPowered;
extern StaticNameKey g_objectIndestructible;
extern StaticNameKey g_objectUnsellable;
extern StaticNameKey g_objectTargetable;
extern StaticNameKey g_objectVisualRange;
extern StaticNameKey g_objectShroudClearingDistance;
extern StaticNameKey g_objectGrantUpgrade;
extern StaticNameKey g_objectTime;
extern StaticNameKey g_objectWeather;
extern StaticNameKey g_objectSoundAmbientEnabled;
extern StaticNameKey g_objectSoundAmbient;
extern StaticNameKey g_objectSoundAmbientCustomized;
extern StaticNameKey g_objectSoundAmbientLooping;
extern StaticNameKey g_objectSoundAmbientLoopCount;
extern StaticNameKey g_objectSoundAmbientMinVolume;
extern StaticNameKey g_objectSoundAmbientVolume;
extern StaticNameKey g_objectSoundAmbientMinRange;
extern StaticNameKey g_objectSoundAmbientMaxRange;
extern StaticNameKey g_objectSoundAmbientPriority;
extern StaticNameKey g_waypointPathLabel1;
extern StaticNameKey g_waypointPathLabel2;
extern StaticNameKey g_waypointPathLabel3;
extern StaticNameKey g_waypointPathBiDirectional;
extern StaticNameKey g_mapName;
extern StaticNameKey g_lightAmbientColor;
extern StaticNameKey g_lightDiffuseColor;
extern StaticNameKey g_lightOuterRadius;
extern StaticNameKey g_lightInnerRadius;
extern StaticNameKey g_objectRadius;
