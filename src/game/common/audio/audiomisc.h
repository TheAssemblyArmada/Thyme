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
#pragma once

#include "always.h"
#include "audioeventrts.h"
#include "ini.h"

class MiscAudio
{
public:
    static void Parse_Misc_Audio(INI *ini);

    AudioEventRTS m_radarNotifyUnitUnderAttack;
    AudioEventRTS m_radarNotifyHarvesterUnderAttack;
    AudioEventRTS m_radarNotifyStructureUnderAttack;
    AudioEventRTS m_radarNotifyUnderAttack;
    AudioEventRTS m_radarNotifyInfiltration;
    AudioEventRTS m_radarNotifyOnline;
    AudioEventRTS m_radarNotifyOffline;
    AudioEventRTS m_defectorTimerTick;
    AudioEventRTS m_defectorTimerDing;
    AudioEventRTS m_lockonTick;
    AudioEventRTS m_allCheer;
    AudioEventRTS m_battleCry;
    AudioEventRTS m_guiClick;
    AudioEventRTS m_noCanDo;
    AudioEventRTS m_stealthDiscovered;
    AudioEventRTS m_stealthNeautralized;
    AudioEventRTS m_moneyDeposit;
    AudioEventRTS m_moneyWithdrawn;
    AudioEventRTS m_buildingDisabled;
    AudioEventRTS m_buildingReenabled;
    AudioEventRTS m_vehicleDisabled;
    AudioEventRTS m_vehicleReenabled;
    AudioEventRTS m_splatterVehiclePilotsBrain;
    AudioEventRTS m_terroristInCarMove;
    AudioEventRTS m_terroristInCarAttackMove;
    AudioEventRTS m_terroristInCarSelect;
    AudioEventRTS m_crateHeal;
    AudioEventRTS m_crateShroud;
    AudioEventRTS m_crateSalvage;
    AudioEventRTS m_crateFreeUnit;
    AudioEventRTS m_crateMoney;
    AudioEventRTS m_unitPromoted;
    AudioEventRTS m_repairSparks;
    AudioEventRTS m_sabotageShutdownBuilding;
    AudioEventRTS m_sabotageResetTimeBuilding;
    AudioEventRTS m_aircraftWheelScreech;

    static const FieldParse s_fieldParseTable[];
};