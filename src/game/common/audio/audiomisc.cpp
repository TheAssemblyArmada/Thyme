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
    FIELD_PARSE_AUDIO_EVENT_RTS("RadarNotifyUnitUnderAttackSound", MiscAudio, m_radarNotifyUnitUnderAttack),
    FIELD_PARSE_AUDIO_EVENT_RTS("RadarNotifyHarvesterUnderAttackSound", MiscAudio, m_radarNotifyHarvesterUnderAttack),
    FIELD_PARSE_AUDIO_EVENT_RTS("RadarNotifyStructureUnderAttackSound", MiscAudio, m_radarNotifyStructureUnderAttack),
    FIELD_PARSE_AUDIO_EVENT_RTS("RadarNotifyUnderAttackSound", MiscAudio, m_radarNotifyUnderAttack),
    FIELD_PARSE_AUDIO_EVENT_RTS("RadarNotifyInfiltrationSound", MiscAudio, m_radarNotifyInfiltration),
    FIELD_PARSE_AUDIO_EVENT_RTS("RadarNotifyOnlineSound", MiscAudio, m_radarNotifyOnline),
    FIELD_PARSE_AUDIO_EVENT_RTS("RadarNotifyOfflineSound", MiscAudio, m_radarNotifyOffline),
    FIELD_PARSE_AUDIO_EVENT_RTS("DefectorTimerTickSound", MiscAudio, m_defectorTimerTick),
    FIELD_PARSE_AUDIO_EVENT_RTS("DefectorTimerDingSound", MiscAudio, m_defectorTimerDing),
    FIELD_PARSE_AUDIO_EVENT_RTS("LockonTickSound", MiscAudio, m_lockonTick),
    FIELD_PARSE_AUDIO_EVENT_RTS("AllCheerSound", MiscAudio, m_allCheer),
    FIELD_PARSE_AUDIO_EVENT_RTS("BattleCrySound", MiscAudio, m_battleCry),
    FIELD_PARSE_AUDIO_EVENT_RTS("GUIClickSound", MiscAudio, m_guiClick),
    FIELD_PARSE_AUDIO_EVENT_RTS("NoCanDoSound", MiscAudio, m_noCanDo),
    FIELD_PARSE_AUDIO_EVENT_RTS("StealthDiscoveredSound", MiscAudio, m_stealthDiscovered),
    FIELD_PARSE_AUDIO_EVENT_RTS("StealthNeutralizedSound", MiscAudio, m_stealthNeautralized),
    FIELD_PARSE_AUDIO_EVENT_RTS("MoneyDepositSound", MiscAudio, m_moneyDeposit),
    FIELD_PARSE_AUDIO_EVENT_RTS("MoneyWithdrawSound", MiscAudio, m_moneyWithdrawn),
    FIELD_PARSE_AUDIO_EVENT_RTS("BuildingDisabled", MiscAudio, m_buildingDisabled),
    FIELD_PARSE_AUDIO_EVENT_RTS("BuildingReenabled", MiscAudio, m_buildingReenabled),
    FIELD_PARSE_AUDIO_EVENT_RTS("VehicleDisabled", MiscAudio, m_vehicleDisabled),
    FIELD_PARSE_AUDIO_EVENT_RTS("VehicleReenabled", MiscAudio, m_vehicleReenabled),
    FIELD_PARSE_AUDIO_EVENT_RTS("SplatterVehiclePilotsBrain", MiscAudio, m_splatterVehiclePilotsBrain),
    FIELD_PARSE_AUDIO_EVENT_RTS("TerroristInCarMoveVoice", MiscAudio, m_terroristInCarMove),
    FIELD_PARSE_AUDIO_EVENT_RTS("TerroristInCarAttackVoice", MiscAudio, m_terroristInCarAttackMove),
    FIELD_PARSE_AUDIO_EVENT_RTS("TerroristInCarSelectVoice", MiscAudio, m_terroristInCarSelect),
    FIELD_PARSE_AUDIO_EVENT_RTS("CrateHeal", MiscAudio, m_crateHeal),
    FIELD_PARSE_AUDIO_EVENT_RTS("CrateShroud", MiscAudio, m_crateShroud),
    FIELD_PARSE_AUDIO_EVENT_RTS("CrateSalvage", MiscAudio, m_crateSalvage),
    FIELD_PARSE_AUDIO_EVENT_RTS("CrateFreeUnit", MiscAudio, m_crateFreeUnit),
    FIELD_PARSE_AUDIO_EVENT_RTS("CrateMoney", MiscAudio, m_crateMoney),
    FIELD_PARSE_AUDIO_EVENT_RTS("UnitPromoted", MiscAudio, m_unitPromoted),
    FIELD_PARSE_AUDIO_EVENT_RTS("RepairSparks", MiscAudio, m_repairSparks),
    FIELD_PARSE_AUDIO_EVENT_RTS("SabotageShutDownBuilding", MiscAudio, m_sabotageShutdownBuilding),
    FIELD_PARSE_AUDIO_EVENT_RTS("SabotageResetTimeBuilding", MiscAudio, m_sabotageResetTimeBuilding),
    FIELD_PARSE_AUDIO_EVENT_RTS("AircraftWheelScreech", MiscAudio, m_aircraftWheelScreech),
    FIELD_PARSE_LAST
};
// clang-format on

// Was originally INI::parseMiscAudio
void MiscAudio::Parse_Misc_Audio(INI *ini)
{
    ini->Init_From_INI(g_theAudio->Get_Misc_Audio(), s_fieldParseTable);
}
