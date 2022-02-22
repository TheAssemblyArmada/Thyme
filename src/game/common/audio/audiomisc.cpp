/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding a collection of miscellaneous audio events.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "audiomisc.h"
#include "audiomanager.h"
#include <cstddef>

// clang-format off
const FieldParse MiscAudio::s_fieldParseTable[] = {
    { "RadarNotifyUnitUnderAttackSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_radarNotifyUnitUnderAttack) },
    { "RadarNotifyHarvesterUnderAttackSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_radarNotifyHarvesterUnderAttack) },
    { "RadarNotifyStructureUnderAttackSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_radarNotifyStructureUnderAttack) },
    { "RadarNotifyUnderAttackSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_radarNotifyUnderAttack) },
    { "RadarNotifyInfiltrationSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_radarNotifyInfiltration) },
    { "RadarNotifyOnlineSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_radarNotifyOnline) },
    { "RadarNotifyOfflineSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_radarNotifyOffline) },
    { "DefectorTimerTickSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_defectorTimerTick) },
    { "DefectorTimerDingSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_defectorTimerDing) },
    { "LockonTickSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_lockonTick) },
    { "AllCheerSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_allCheer) },
    { "BattleCrySound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_battleCry) },
    { "GUIClickSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_guiClick) },
    { "NoCanDoSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_noCanDo) },
    { "StealthDiscoveredSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_stealthDiscovered) },
    { "StealthNeutralizedSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_stealthNeautralized) },
    { "MoneyDepositSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_moneyDeposit) },
    { "MoneyWithdrawSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_moneyWithdrawn) },
    { "BuildingDisabled", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_buildingDisabled) },
    { "BuildingReenabled", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_buildingReenabled) },
    { "VehicleDisabled", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_vehicleDisabled) },
    { "VehicleReenabled", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_vehicleReenabled) },
    { "SplatterVehiclePilotsBrain", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_splatterVehiclePilotsBrain) },
    { "TerroristInCarMoveVoice", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_terroristInCarMove) },
    { "TerroristInCarAttackVoice", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_terroristInCarAttackMove) },
    { "TerroristInCarSelectVoice", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_terroristInCarSelect) },
    { "CrateHeal", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_crateHeal) },
    { "CrateShroud", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_crateShroud) },
    { "CrateSalvage", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_crateSalvage) },
    { "CrateFreeUnit", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_crateFreeUnit) },
    { "CrateMoney", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_crateMoney) },
    { "UnitPromoted", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_unitPromoted) },
    { "RepairSparks", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_repairSparks) },
    { "SabotageShutDownBuilding", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_sabotageShutdownBuilding) },
    { "SabotageResetTimeBuilding", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_sabotageResetTimeBuilding) },
    { "AircraftWheelScreech", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(MiscAudio, m_aircraftWheelScreech) },
    { nullptr, nullptr, nullptr, 0 } };
// clang-format on

// Was originally INI::parseMiscAudio
void MiscAudio::Parse_Misc_Audio(INI *ini)
{
    ini->Init_From_INI(g_theAudio->Get_Misc_Audio(), s_fieldParseTable);
}
