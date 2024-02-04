/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief EVA
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "eva.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
Eva *g_theEva;
#endif

// clang-format off
const FieldParse EvaSideSounds::s_evaSideSounds[] = {
    {"Side", &INI::Parse_AsciiString, nullptr, offsetof(EvaSideSounds, m_side)},
    {"Sounds", &INI::Parse_Sounds_List, nullptr, offsetof(EvaSideSounds, m_sounds)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

void Parse_Side_Sounds_List(INI *ini, void *formal, void *store, const void *user_data)
{
    EvaSideSounds sounds;
    ini->Init_From_INI(&sounds, EvaSideSounds::Get_Field_Parse());
    static_cast<std::vector<EvaSideSounds> *>(store)->push_back(sounds);
}

// clang-format off
const FieldParse EvaCheckInfo::s_evaEventInfo[] = {
    {"Priority", &INI::Parse_Unsigned_Int, nullptr, offsetof(EvaCheckInfo, m_priority)},
    {"TimeBetweenChecksMS", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(EvaCheckInfo, m_timeBetweenChecksMS)},
    {"ExpirationTimeMS", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(EvaCheckInfo, m_expirationTimeMS)},
    {"SideSounds", &Parse_Side_Sounds_List, nullptr, offsetof(EvaCheckInfo, m_sideSounds)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

EvaCheckInfo *Eva::New_Eva_Check_Info(Utf8String name)
{
    EvaMessage message = Name_To_Message(name);

    for (auto it = m_checkInfo.begin(); it != m_checkInfo.end(); it++) {
        if (*it != nullptr && (*it)->m_messageType == message) {
            return nullptr;
        }
    }

    EvaCheckInfo *info = new EvaCheckInfo();
    m_checkInfo.push_back(info);
    info->m_messageType = message;
    return info;
}

Eva::Eva() : m_player(nullptr), m_unk1(0), m_unk2(0), m_currentMessageType(NUM_EVA_MESSAGES), m_evaEnabled(true)
{
    for (int i = 0; i < NUM_EVA_MESSAGES; i++) {
        m_shouldPlay[i] = false;
    }
}

Eva::~Eva()
{
    for (auto it = m_checkInfo.begin(); it != m_checkInfo.end(); it++) {
        if (*it != nullptr) {
            (*it)->Delete_Instance();
        }
    }
}

void Eva::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Eva.ini", INI_LOAD_OVERWRITE, nullptr);
}

void Eva::Reset()
{
    m_unk2 = 0;
    m_unk1 = 0;

    for (auto it = m_check.begin(); it != m_check.end(); it = m_check.erase(it)) {
    }

    for (int i = 0; i < NUM_EVA_MESSAGES; i++) {
        m_shouldPlay[i] = false;
    }

    m_evaEnabled = true;
}

void Eva::Update()
{
#ifdef GAME_DLL
    Call_Method<void, Eva>(PICK_ADDRESS(0x00513340, 0x009BB4DA), this);
#endif
}

void Eva::Set_Should_Play(EvaMessage message)
{
    m_shouldPlay[message] = true;
}

static const char *s_theEvaMessageNames[] = { "LOWPOWER",
    "INSUFFICIENTFUNDS",
    "SUPERWEAPONDETECTED_OWN_PARTICLECANNON",
    "SUPERWEAPONDETECTED_OWN_NUKE",
    "SUPERWEAPONDETECTED_OWN_SCUDSTORM",
    "SUPERWEAPONDETECTED_ALLY_PARTICLECANNON",
    "SUPERWEAPONDETECTED_ALLY_NUKE",
    "SUPERWEAPONDETECTED_ALLY_SCUDSTORM",
    "SUPERWEAPONDETECTED_ENEMY_PARTICLECANNON",
    "SUPERWEAPONDETECTED_ENEMY_NUKE",
    "SUPERWEAPONDETECTED_ENEMY_SCUDSTORM",
    "SUPERWEAPONLAUNCHED_OWN_PARTICLECANNON",
    "SUPERWEAPONLAUNCHED_OWN_NUKE",
    "SUPERWEAPONLAUNCHED_OWN_SCUDSTORM",
    "SUPERWEAPONLAUNCHED_ALLY_PARTICLECANNON",
    "SUPERWEAPONLAUNCHED_ALLY_NUKE",
    "SUPERWEAPONLAUNCHED_ALLY_SCUDSTORM",
    "SUPERWEAPONLAUNCHED_ENEMY_PARTICLECANNON",
    "SUPERWEAPONLAUNCHED_ENEMY_NUKE",
    "SUPERWEAPONLAUNCHED_ENEMY_SCUDSTORM",
    "SUPERWEAPONREADY_OWN_PARTICLECANNON",
    "SUPERWEAPONREADY_OWN_NUKE",
    "SUPERWEAPONREADY_OWN_SCUDSTORM",
    "SUPERWEAPONREADY_ALLY_PARTICLECANNON",
    "SUPERWEAPONREADY_ALLY_NUKE",
    "SUPERWEAPONREADY_ALLY_SCUDSTORM",
    "SUPERWEAPONREADY_ENEMY_PARTICLECANNON",
    "SUPERWEAPONREADY_ENEMY_NUKE",
    "SUPERWEAPONREADY_ENEMY_SCUDSTORM",
    "BUILDINGLOST",
    "BASEUNDERATTACK",
    "ALLYUNDERATTACK",
    "BEACONDETECTED",
    "ENEMYBLACKLOTUSDETECTED",
    "ENEMYJARMENKELLDETECTED",
    "ENEMYCOLONELBURTONDETECTED",
    "OWNBLACKLOTUSDETECTED",
    "OWNJARMENKELLDETECTED",
    "OWNCOLONELBURTONDETECTED",
    "UNITLOST",
    "GENERALLEVELUP",
    "VEHICLESTOLEN",
    "BUILDINGSTOLEN",
    "CASHSTOLEN",
    "UPGRADECOMPLETE",
    "BUILDINGBEINGSTOLEN",
    "BUILDINGSABOTAGED",
    "SUPERWEAPONLAUNCHED_OWN_GPS_SCRAMBLER",
    "SUPERWEAPONLAUNCHED_ALLY_GPS_SCRAMBLER",
    "SUPERWEAPONLAUNCHED_ENEMY_GPS_SCRAMBLER",
    "SUPERWEAPONLAUNCHED_OWN_SNEAK_ATTACK",
    "SUPERWEAPONLAUNCHED_ALLY_SNEAK_ATTACK",
    "SUPERWEAPONLAUNCHED_ENEMY_SNEAK_ATTACK" };

EvaMessage Eva::Name_To_Message(Utf8String const &name)
{
    for (int i = 0; i < NUM_EVA_MESSAGES; i++) {
        if (name.Compare_No_Case(s_theEvaMessageNames[i]) == 0) {
            return static_cast<EvaMessage>(i);
        }
    }

    return EVA_MESSAGE_INVALID;
}

void Eva::Parse(INI *ini)
{
    Utf8String name;
    name.Set(ini->Get_Next_Token());
    EvaCheckInfo *info = g_theEva->New_Eva_Check_Info(name);

    if (info != nullptr) {
        ini->Init_From_INI(info, EvaCheckInfo::Get_Field_Parse());
    }
}

void Eva::Parse_Eva_Message_From_INI(INI *ini, void *formal, void *store, const void *user_data)
{
    EvaMessage message = Name_To_Message(ini->Get_Next_Token());

    if (message == EVA_MESSAGE_INVALID) {
        throw CODE_06;
    }

    *static_cast<EvaMessage *>(store) = message;
}
