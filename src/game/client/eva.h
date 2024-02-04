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
#pragma once
#include "always.h"
#include "asciistring.h"
#include "audioeventrts.h"
#include "mempoolobj.h"
#include "subsysteminterface.h"
#include <vector>

enum EvaMessage
{
    EVA_MESSAGE_INVALID = -1,
    EVA_MESSAGE_LOWPOWER,
    EVA_MESSAGE_INSUFFICIENTFUNDS,
    EVA_MESSAGE_SUPERWEAPONDETECTED_OWN_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONDETECTED_OWN_NUKE,
    EVA_MESSAGE_SUPERWEAPONDETECTED_OWN_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONDETECTED_ALLY_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONDETECTED_ALLY_NUKE,
    EVA_MESSAGE_SUPERWEAPONDETECTED_ALLY_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONDETECTED_ENEMY_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONDETECTED_ENEMY_NUKE,
    EVA_MESSAGE_SUPERWEAPONDETECTED_ENEMY_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_OWN_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_OWN_NUKE,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_OWN_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ALLY_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ALLY_NUKE,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ALLY_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ENEMY_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ENEMY_NUKE,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ENEMY_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONREADY_OWN_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONREADY_OWN_NUKE,
    EVA_MESSAGE_SUPERWEAPONREADY_OWN_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONREADY_ALLY_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONREADY_ALLY_NUKE,
    EVA_MESSAGE_SUPERWEAPONREADY_ALLY_SCUDSTORM,
    EVA_MESSAGE_SUPERWEAPONREADY_ENEMY_PARTICLECANNON,
    EVA_MESSAGE_SUPERWEAPONREADY_ENEMY_NUKE,
    EVA_MESSAGE_SUPERWEAPONREADY_ENEMY_SCUDSTORM,
    EVA_MESSAGE_BUILDINGLOST,
    EVA_MESSAGE_BASEUNDERATTACK,
    EVA_MESSAGE_ALLYUNDERATTACK,
    EVA_MESSAGE_BEACONDETECTED,
    EVA_MESSAGE_ENEMYBLACKLOTUSDETECTED,
    EVA_MESSAGE_ENEMYJARMENKELLDETECTED,
    EVA_MESSAGE_ENEMYCOLONELBURTONDETECTED,
    EVA_MESSAGE_OWNBLACKLOTUSDETECTED,
    EVA_MESSAGE_OWNJARMENKELLDETECTED,
    EVA_MESSAGE_OWNCOLONELBURTONDETECTED,
    EVA_MESSAGE_UNITLOST,
    EVA_MESSAGE_GENERALLEVELUP,
    EVA_MESSAGE_VEHICLESTOLEN,
    EVA_MESSAGE_BUILDINGSTOLEN,
    EVA_MESSAGE_CASHSTOLEN,
    EVA_MESSAGE_UPGRADECOMPLETE,
    EVA_MESSAGE_BUILDINGBEINGSTOLEN,
    EVA_MESSAGE_BUILDINGSABOTAGED,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_OWN_GPS_SCRAMBLER,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ALLY_GPS_SCRAMBLER,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ENEMY_GPS_SCRAMBLER,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_OWN_SNEAK_ATTACK,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ALLY_SNEAK_ATTACK,
    EVA_MESSAGE_SUPERWEAPONLAUNCHED_ENEMY_SNEAK_ATTACK,
    NUM_EVA_MESSAGES,
};

struct EvaSideSounds
{
    Utf8String m_side;
    std::vector<Utf8String> m_sounds;

    static const FieldParse *Get_Field_Parse() { return s_evaSideSounds; }
    static const FieldParse s_evaSideSounds[];
};

class EvaCheckInfo : public MemoryPoolObject
{
    IMPLEMENT_POOL(EvaCheckInfo)

public:
    EvaCheckInfo() : m_messageType(NUM_EVA_MESSAGES), m_timeBetweenChecksMS(900), m_expirationTimeMS(150), m_priority(1) {}

private:
    static const FieldParse *Get_Field_Parse() { return s_evaEventInfo; }
    static const FieldParse s_evaEventInfo[];

    EvaMessage m_messageType;
    unsigned int m_timeBetweenChecksMS;
    unsigned int m_expirationTimeMS;
    unsigned int m_priority;
    std::vector<EvaSideSounds> m_sideSounds;
    friend class Eva;
};

struct EvaCheck
{
    EvaCheck() : check_info(nullptr), unk1(0xFFFFFFFF), unk2(0), unk3(0) {}

    EvaCheckInfo *check_info;
    unsigned int unk1;
    unsigned int unk2;
    bool unk3;
};

class Eva : public SubsystemInterface
{
public:
    Eva();
    virtual ~Eva() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    EvaCheckInfo *New_Eva_Check_Info(Utf8String name);
    EvaCheckInfo *Get_Eva_Check_Info(Utf8String name);
    void Set_Should_Play(EvaMessage message);
    void Set_Eva_Enabled(bool enabled);
    bool Is_Time_For_Check(EvaMessage message, unsigned int frame);
    bool Message_Should_Play(EvaMessage message, unsigned int frame);
    void Play_Message(EvaMessage message, unsigned int frame);
    void Process_Playing_Message(unsigned int frame);

    static void Parse(INI *ini);
    static EvaMessage Name_To_Message(Utf8String const &name);
    static Utf8String Message_To_Name(EvaMessage message);
    static bool Should_Play_Low_Power(Player *player);
    static bool Should_Play_Generic_Handler(Player *player);
    static void Parse_Eva_Message_From_INI(INI *ini, void *formal, void *store, const void *user_data);

private:
    std::vector<EvaCheckInfo *> m_checkInfo;
    std::vector<EvaCheck> m_check;
    AudioEventRTS m_sound;
    Player *m_player;
    int m_unk1;
    int m_unk2;
    EvaMessage m_currentMessageType;
    bool m_shouldPlay[NUM_EVA_MESSAGES];
    bool m_evaEnabled;
};

#ifdef GAME_DLL
extern Eva *&g_theEva;
#else
extern Eva *g_theEva;
#endif
