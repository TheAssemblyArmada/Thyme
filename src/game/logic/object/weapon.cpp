/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Weapon objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "weapon.h"
#include "ai.h"
#include "aipathfind.h"
#include "aiupdate.h"
#include "assistedtargetingupdate.h"
#include "audiomanager.h"
#include "behaviormodule.h"
#include "damage.h"
#include "drawable.h"
#include "experiencetracker.h"
#include "fxlist.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "ini.h"
#include "laserupdate.h"
#include "object.h"
#include "objectcreationlist.h"
#include "opencontain.h"
#include "parkingplacebehavior.h"
#include "particlesystemplate.h"
#include "partitionmanager.h"
#include "player.h"
#include "projectilestreamupdate.h"
#include "simpleobjectiterator.h"
#include "specialpowercompletiondie.h"
#include "terrainlogic.h"
#include "thingfactory.h"
#include "weaponset.h"

void Do_FX_Pos(FXList const *list,
    const Coord3D *primary,
    const Matrix3D *primary_mtx,
    float primary_speed,
    const Coord3D *secondary,
    float radius);

#ifndef GAME_DLL
WeaponStore *g_theWeaponStore = nullptr;
#endif

const char *TheWeaponBonusNames[] = {
    "GARRISONED",
    "HORDE",
    "CONTINUOUS_FIRE_MEAN",
    "CONTINUOUS_FIRE_FAST",
    "NATIONALISM",
    "PLAYER_UPGRADE",
    "DRONE_SPOTTING",
    "DEMORALIZED_OBSOLETE",
    "ENTHUSIASTIC",
    "VETERAN",
    "ELITE",
    "HERO",
    "BATTLEPLAN_BOMBARDMENT",
    "BATTLEPLAN_HOLDTHELINE",
    "BATTLEPLAN_SEARCHANDDESTROY",
    "SUBLIMINAL",
    "SOLO_HUMAN_EASY",
    "SOLO_HUMAN_NORMAL",
    "SOLO_HUMAN_HARD",
    "SOLO_AI_EASY",
    "SOLO_AI_NORMAL",
    "SOLO_AI_HARD",
    "TARGET_FAERIE_FIRE",
    "FANATICISM",
    "FRENZY_ONE",
    "FRENZY_TWO",
    "FRENZY_THREE",
    nullptr,
};

const char *TheWeaponBonusFieldNames[] = {
    "DAMAGE",
    "RADIUS",
    "RANGE",
    "RATE_OF_FIRE",
    "PRE_ATTACK",
    nullptr,
};

void WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponBonusSet *wbs = *static_cast<WeaponBonusSet **>(store);
    wbs->Parse_Weapon_Bonus_Set(ini);
}

void WeaponBonusSet::Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponBonusSet *wbs = static_cast<WeaponBonusSet *>(store);
    wbs->Parse_Weapon_Bonus_Set(ini);
}

void WeaponBonusSet::Parse_Weapon_Bonus_Set(INI *ini)
{
    int set = INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusNames);
    WeaponBonus::Field field = WeaponBonus::Field(INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusFieldNames));
    m_bonus[set].Set_Field(field, INI::Scan_PercentToReal(ini->Get_Next_Token()));
}

const WeaponTemplate *WeaponStore::Find_Weapon_Template(Utf8String name) const
{
    if (strcasecmp(name.Str(), "None") == 0) {
        return nullptr;
    } else {
        WeaponTemplate *tmplate = Find_Weapon_Template_Private(g_theNameKeyGenerator->Name_To_Key(name.Str()));
        captainslog_dbgassert(tmplate != nullptr, "Weapon %s not found!", name.Str());
        return tmplate;
    }
}

void WeaponStore::Set_Delayed_Damage(const WeaponTemplate *weapon,
    const Coord3D *pos,
    unsigned int which_frame,
    ObjectID source_id,
    ObjectID victim_id,
    const WeaponBonus &bonus)
{
    WeaponDelayedDamageInfo info;
    info.m_delayedWeapon = weapon;
    info.m_delayDamagePos = *pos;
    info.m_delayDamageFrame = which_frame;
    info.m_delaySourceID = source_id;
    info.m_delayIntendedVictimID = victim_id;
    info.m_bonus = bonus;
    m_weaponDDI.push_back(info);
}

void WeaponStore::Parse_Weapon_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    const WeaponTemplate *w = g_theWeaponStore->Find_Weapon_Template(name);
    captainslog_dbgassert(w != nullptr || strcasecmp(name, "None") == 0, "WeaponTemplate %s not found!", name);
    *static_cast<const WeaponTemplate **>(store) = w;
}

Weapon *WeaponStore::Allocate_New_Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot) const
{
    return new Weapon(tmpl, wslot);
}

WeaponStore::~WeaponStore()
{
    Delete_All_Delayed_Damage();

    for (unsigned int i = 0; i < m_weaponTemplateVector.size(); i++) {
        WeaponTemplate *tmplate = m_weaponTemplateVector[i];

        if (tmplate != nullptr) {
            tmplate->Delete_Instance();
        }
    }

    m_weaponTemplateVector.clear();
}

void WeaponStore::Handle_Projectile_Detonation(const WeaponTemplate *tmplate,
    const Object *source_obj,
    const Coord3D *victim_pos,
    int bonus_condition,
    bool do_damage) const
{
    Weapon *weapon = g_theWeaponStore->Allocate_New_Weapon(tmplate, WEAPONSLOT_PRIMARY);
    weapon->Load_Ammo_Now(source_obj);
    weapon->Fire_Projectile_Detonation_Weapon(source_obj, victim_pos, bonus_condition, do_damage);
    weapon->Delete_Instance();
}

void WeaponStore::Create_And_Fire_Temp_Weapon(
    const WeaponTemplate *tmplate, const Object *source_obj, const Coord3D *victim_pos)
{
    if (tmplate) {
        Weapon *weapon = g_theWeaponStore->Allocate_New_Weapon(tmplate, WEAPONSLOT_PRIMARY);
        weapon->Load_Ammo_Now(source_obj);
        weapon->Fire_Weapon(source_obj, victim_pos, nullptr);
        weapon->Delete_Instance();
    }
}

WeaponTemplate *WeaponStore::Find_Weapon_Template_Private(NameKeyType key) const
{
    for (unsigned int i = 0; i < m_weaponTemplateVector.size(); i++) {
        WeaponTemplate *tmplate = m_weaponTemplateVector[i];

        if (tmplate->Get_Name_Key() == key) {
            return tmplate;
        }
    }

    return nullptr;
}

WeaponTemplate *WeaponStore::New_Weapon_Template(Utf8String name)
{
    if (name.Is_Empty()) {
        return nullptr;
    } else {
        WeaponTemplate *tmplate = new WeaponTemplate();
        tmplate->m_name = name;
        tmplate->m_nameKey = g_theNameKeyGenerator->Name_To_Key(name.Str());
        m_weaponTemplateVector.push_back(tmplate);
        return tmplate;
    }
}

WeaponTemplate *WeaponStore::New_Override(WeaponTemplate *tmplate)
{
    if (tmplate == nullptr) {
        return nullptr;
    }

    WeaponTemplate *t = new WeaponTemplate();
    *t = *tmplate;
    t->Friend_Set_Next_Template(tmplate);
    return t;
}

void WeaponStore::Update()
{
    for (auto i = m_weaponDDI.begin(); i != m_weaponDDI.end();) {
        if (g_theGameLogic->Get_Frame() >= i->m_delayDamageFrame) {
            i->m_delayedWeapon->Deal_Damage_Internal(
                i->m_delaySourceID, i->m_delayIntendedVictimID, &i->m_delayDamagePos, i->m_bonus, false);
            i = m_weaponDDI.erase(i);
        } else {
            i++;
        }
    }
}

void WeaponStore::Delete_All_Delayed_Damage()
{
    m_weaponDDI.clear();
}

void WeaponStore::Reset_Weapon_Templates()
{
    for (unsigned int i = 0; i < m_weaponTemplateVector.size(); i++) {
        m_weaponTemplateVector[i]->Reset();
    }
}

void WeaponStore::Reset()
{
    for (unsigned int i = 0; i < m_weaponTemplateVector.size(); i++) {
        WeaponTemplate *tmplate = m_weaponTemplateVector[i];

        if (tmplate->Is_Override()) {
            tmplate->Friend_Clear_Next_Template();
            tmplate->Delete_Instance();
        }
    }

    Delete_All_Delayed_Damage();
    Reset_Weapon_Templates();
}

void WeaponStore::PostProcessLoad()
{
    if (g_theThingFactory != nullptr) {
        for (unsigned int i = 0; i < m_weaponTemplateVector.size(); i++) {
            WeaponTemplate *tmplate = m_weaponTemplateVector[i];

            if (tmplate != nullptr) {
                tmplate->Post_Process_Load();
            }
        }
    } else {
        captainslog_dbgassert(false, "you must call this after TheThingFactory is inited");
    }
}

void WeaponStore::Parse_Weapon_Template_Definition(INI *ini)
{
    Utf8String str;
    str.Set(ini->Get_Next_Token());
    WeaponTemplate *tmplate = g_theWeaponStore->Find_Weapon_Template_Private(g_theNameKeyGenerator->Name_To_Key(str.Str()));

    if (tmplate == nullptr) {
        tmplate = g_theWeaponStore->New_Weapon_Template(str);
    } else if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        tmplate = g_theWeaponStore->New_Override(tmplate);
    } else {
        captainslog_dbgassert(false, "Weapon '%s' already exists, but OVERRIDE not specified", str.Str());
    }

    if (tmplate != nullptr) {
        ini->Init_From_INI(tmplate, WeaponTemplate::Get_Field_Parse());

        if (tmplate->m_projectileObject.Is_None()) {
            tmplate->m_projectileObject.Clear();
        }

        if (!tmplate->Get_Fire_Sound()->Get_Event_Name().Is_Empty()) {
            if (tmplate->Get_Fire_Sound()->Get_Event_Name().Compare_No_Case("NoSound")) {
                captainslog_dbgassert(g_theAudio->Is_Valid_Audio_Event(tmplate->Get_Fire_Sound()),
                    "Invalid FireSound %s in Weapon '%s'.",
                    tmplate->Get_Fire_Sound()->Get_Event_Name().Str(),
                    tmplate->Get_Name().Str());
            }
        }
    }
}

Weapon::Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot) :
    m_template(tmpl),
    m_wslot(wslot),
    m_status(OUT_OF_AMMO),
    m_ammoInClip(0),
    m_whenWeCanFireAgain(0),
    m_whenPreAttackFinished(0),
    m_whenLastReloadStarted(0),
    m_lastFireFrame(0),
    m_projectileStreamID(INVALID_OBJECT_ID),
    m_maxShotCount(0x7FFFFFFF),
    m_curBarrel(0),
    m_leechWeaponRangeActive(false)
{
    m_pitchLimited = m_template->Get_Min_Target_Pitch() > -GAMEMATH_PI || m_template->Get_Max_Target_Pitch() < GAMEMATH_PI;
    m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
    m_suspendFXDelay = m_template->Get_Suspend_FX_Delay() + g_theGameLogic->Get_Frame();
}

Weapon::Weapon(const Weapon &that) :
    m_template(that.m_template),
    m_wslot(that.m_wslot),
    m_status(OUT_OF_AMMO),
    m_ammoInClip(0),
    m_whenWeCanFireAgain(0),
    m_whenPreAttackFinished(0),
    m_whenLastReloadStarted(0),
    m_lastFireFrame(0),
    m_projectileStreamID(INVALID_OBJECT_ID),
    m_maxShotCount(0x7FFFFFFF),
    m_curBarrel(0),
    m_leechWeaponRangeActive(false)
{
    m_pitchLimited = m_template->Get_Min_Target_Pitch() > -GAMEMATH_PI || m_template->Get_Max_Target_Pitch() < GAMEMATH_PI;
    m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
    m_suspendFXDelay = that.Get_Suspend_FX_Delay();
}

Weapon &Weapon::operator=(const Weapon &that)
{
    if (this != &that) {
        m_template = that.m_template;
        m_wslot = that.m_wslot;
        m_status = OUT_OF_AMMO;
        m_ammoInClip = 0;
        m_whenPreAttackFinished = 0;
        m_whenLastReloadStarted = 0;
        m_whenWeCanFireAgain = 0;
        m_leechWeaponRangeActive = 0;
        m_pitchLimited =
            m_template->Get_Min_Target_Pitch() > -GAMEMATH_PI || m_template->Get_Max_Target_Pitch() < GAMEMATH_PI;
        m_maxShotCount = 0x7FFFFFFF;
        m_curBarrel = 0;
        m_lastFireFrame = 0;
        m_suspendFXDelay = that.Get_Suspend_FX_Delay();
        m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
        m_projectileStreamID = INVALID_OBJECT_ID;
    }

    return *this;
}

void Weapon::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DEBUG_STRUCTS
    Utf8String str1;
    Utf8String str2;
    // todo
#endif

    Utf8String name = m_template->Get_Name();
    xfer->xferAsciiString(&name);
    xfer->xferUser(&m_wslot, sizeof(m_wslot));

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_ammoInClip);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_whenWeCanFireAgain);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_whenPreAttackFinished);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_whenLastReloadStarted);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_lastFireFrame);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferObjectID(&m_projectileStreamID);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    ObjectID laser = INVALID_OBJECT_ID;
    xfer->xferObjectID(&laser);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferInt(&m_maxShotCount);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferInt(&m_curBarrel);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferInt(&m_numShotsForCurBarrel);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    unsigned short scatter_count = static_cast<unsigned short>(m_scatterTargetIndexes.size());
    xfer->xferUnsignedShort(&scatter_count);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    for (auto i = m_scatterTargetIndexes.begin(); i != m_scatterTargetIndexes.end(); i++) {
        int index = *i;
        xfer->xferInt(&index);

#ifdef GAME_DEBUG_STRUCTS
        // todo
#endif
    }

    xfer->xferBool(&m_pitchLimited);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferBool(&m_leechWeaponRangeActive);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif
}

void Weapon::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 3;
    xfer->xferVersion(&version, 3);

    if (version >= 2) {
        Utf8String name;
        name = m_template->Get_Name();
        xfer->xferAsciiString(&name);

        if (xfer->Get_Mode() == XFER_LOAD) {
            m_template = g_theWeaponStore->Find_Weapon_Template(name);

            if (m_template == nullptr) {
                throw CODE_06;
            }
        }
    }

    xfer->xferUser(&m_wslot, sizeof(m_wslot));
    xfer->xferUser(&m_status, sizeof(m_status));
    xfer->xferUnsignedInt(&m_ammoInClip);
    xfer->xferUnsignedInt(&m_whenWeCanFireAgain);
    xfer->xferUnsignedInt(&m_whenPreAttackFinished);
    xfer->xferUnsignedInt(&m_whenLastReloadStarted);
    xfer->xferUnsignedInt(&m_lastFireFrame);

    if (version < 3) {
        m_suspendFXDelay = 0;
    } else {
        xfer->xferUnsignedInt(&m_suspendFXDelay);
    }

    xfer->xferObjectID(&m_projectileStreamID);
    ObjectID laser = INVALID_OBJECT_ID;
    xfer->xferObjectID(&laser);
    xfer->xferInt(&m_maxShotCount);
    xfer->xferInt(&m_curBarrel);
    xfer->xferInt(&m_numShotsForCurBarrel);

    unsigned short size = static_cast<unsigned short>(m_scatterTargetIndexes.size());
    xfer->xferUnsignedShort(&size);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto i = m_scatterTargetIndexes.begin(); i != m_scatterTargetIndexes.end(); i++) {
            int index = *i;
            xfer->xferInt(&index);
        }
    } else {
        m_scatterTargetIndexes.clear();
        for (unsigned short i = 0; i < size; i++) {
            int index;
            xfer->xferInt(&index);
            m_scatterTargetIndexes.push_back(index);
        }
    }

    xfer->xferBool(&m_pitchLimited);
    xfer->xferBool(&m_leechWeaponRangeActive);
}

void Weapon::Load_Post_Process()
{
    if (m_projectileStreamID != INVALID_OBJECT_ID) {
        if (g_theGameLogic->Find_Object_By_ID(m_projectileStreamID) == nullptr) {
            m_projectileStreamID = INVALID_OBJECT_ID;
        }
    }
}

bool Weapon::Fire_Projectile_Detonation_Weapon(
    const Object *source_obj, const Coord3D *victim_pos, unsigned int bonus_condition, bool do_damage)
{
    return Private_Fire_Weapon(source_obj, nullptr, victim_pos, true, false, bonus_condition, nullptr, do_damage);
}

bool Weapon::Fire_Weapon(const Object *source_obj, Object *victim_obj, ObjectID *projectile_id)
{
    return Private_Fire_Weapon(source_obj, victim_obj, nullptr, false, false, 0, projectile_id, true);
}

bool Weapon::Fire_Weapon(const Object *source_obj, const Coord3D *victim_pos, ObjectID *projectile_id)
{
    return Private_Fire_Weapon(source_obj, nullptr, victim_pos, false, false, 0, projectile_id, true);
}

bool Weapon::Private_Fire_Weapon(const Object *source_obj,
    Object *victim_obj,
    const Coord3D *victim_pos,
    bool is_projectile_detonation,
    bool ignore_ranges,
    unsigned int bonus_condition,
    ObjectID *projectile_id,
    bool do_damage)
{
    if (projectile_id != nullptr) {
        *projectile_id = INVALID_OBJECT_ID;
    }

    if (m_template == nullptr) {
        return false;
    }

    if (m_template->Get_Request_Assist_Range() != 0.0f && victim_obj != nullptr) {
        Process_Request_Assistance(source_obj, victim_obj);
    }

    if (m_template->Is_Leech_Range_Weapon()) {
        Set_Leech_Range_Active(true);
    }

    DamageType damage = m_template->Get_Damage_Type();

    if (damage == DAMAGE_DEPLOY) {
        const AIUpdateInterface *update = source_obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            const AssaultTransportAIInterface *assault = update->Get_Assault_Transport_AI_Interface();

            if (assault != nullptr) {
                assault->Begin_Assault(victim_obj);
            }
        }

        goto l1;
    }

    if (damage == DAMAGE_DISARM) {
        if (source_obj != nullptr && victim_obj != nullptr) {
            bool mine_disarmed = false;

            for (BehaviorModule **i = victim_obj->Get_All_Modules(); *i != nullptr; i++) {
                LandMineInterface *mine = (*i)->Get_Land_Mine_Interface();

                if (mine != nullptr) {
                    Do_FX_Pos(m_template->Get_Fire_FX(source_obj->Get_Veterancy_Level()),
                        victim_obj->Get_Position(),
                        victim_obj->Get_Transform_Matrix(),
                        0.0f,
                        victim_obj->Get_Position(),
                        0.0f);
                    mine->Disarm();
                    mine_disarmed = true;
                }

                if (!mine_disarmed
                    && (victim_obj->Is_KindOf(KINDOF_MINE) || victim_obj->Is_KindOf(KINDOF_BOOBY_TRAP)
                        || victim_obj->Is_KindOf(KINDOF_DEMOTRAP))) {
                    Do_FX_Pos(m_template->Get_Fire_FX(source_obj->Get_Veterancy_Level()),
                        victim_obj->Get_Position(),
                        victim_obj->Get_Transform_Matrix(),
                        0.0f,
                        victim_obj->Get_Position(),
                        0.0f);
                    g_theGameLogic->Destroy_Object(victim_obj);
                    mine_disarmed = true;
                }

                if (mine_disarmed) {
                    source_obj->Get_Controlling_Player()->Get_Academy_Stats()->Increment_Mines_Disarmed();
                }
            }
        }

        m_maxShotCount--;
        m_ammoInClip--;

        if (m_ammoInClip != 0 || !m_template->Is_Auto_Reloads_Clip()) {
            return false;
        }

        Reload_Ammo(source_obj);
        return true;
    } else {
    l1:
        WeaponBonus bonus;
        Compute_Bonus(source_obj, bonus_condition, bonus);

        captainslog_dbgassert(Get_Status() != OUT_OF_AMMO, "Hmm, firing weapon that is OUT_OF_AMMO");
        captainslog_dbgassert(Get_Status() == READY_TO_FIRE, "Hmm, Weapon is firing more often than should be possible");
        captainslog_dbgassert(m_ammoInClip != 0, "Hmm, firing an empty weapon");

        if (Get_Status() != READY_TO_FIRE) {
            return false;
        }

        unsigned int frame = g_theGameLogic->Get_Frame();

        if (m_ammoInClip != 0) {
            if (m_curBarrel >= source_obj->Get_Drawable()->Get_Barrel_Count(m_wslot)) {
                m_curBarrel = 0;
                m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
            }

            if (m_scatterTargetIndexes.size() != 0) {
                if (victim_obj != nullptr) {
                    victim_pos = victim_obj->Get_Position();
                    victim_obj = nullptr;
                }

                Coord3D pos = *victim_pos;
                int random_target_index = Get_Logic_Random_Value(0, m_scatterTargetIndexes.size() - 1);
                int scatter_target_index = m_scatterTargetIndexes[random_target_index];
                float scatter_target_scalar = Get_Scatter_Target_Scalar();
                const std::vector<Coord2D> &targets = m_template->Get_Scatter_Targets();
                Coord2D target = targets[scatter_target_index];
                pos.x += target.x * scatter_target_scalar;
                pos.y += target.y * scatter_target_scalar;
                pos.z = g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr);
                m_scatterTargetIndexes[random_target_index] = m_scatterTargetIndexes.back();
                m_scatterTargetIndexes.pop_back();
                m_template->Fire_Weapon_Template(source_obj,
                    m_wslot,
                    m_curBarrel,
                    victim_obj,
                    &pos,
                    bonus,
                    is_projectile_detonation,
                    ignore_ranges,
                    this,
                    projectile_id,
                    do_damage);
            } else {
                m_template->Fire_Weapon_Template(source_obj,
                    m_wslot,
                    m_curBarrel,
                    victim_obj,
                    victim_pos,
                    bonus,
                    is_projectile_detonation,
                    ignore_ranges,
                    this,
                    projectile_id,
                    do_damage);
            }

            m_lastFireFrame = frame;
            m_ammoInClip--;
            m_maxShotCount--;
            m_numShotsForCurBarrel--;

            if (m_numShotsForCurBarrel <= 0) {
                m_curBarrel++;
                m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
            }

            if (m_ammoInClip != 0) {
                m_status = BETWEEN_FIRING_SHOTS;
                m_whenLastReloadStarted = frame;
                m_whenWeCanFireAgain = m_template->Get_Delay_Between_Shots(bonus) + frame;

                if (source_obj->Is_Share_Weapon_Reload_Time()) {
                    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
                        Weapon *weapon =
                            const_cast<Weapon *>(source_obj->Get_Weapon_In_Weapon_Slot(static_cast<WeaponSlotType>(i)));

                        if (weapon != nullptr) {
                            weapon->Set_Next_Shot(m_whenWeCanFireAgain);
                            weapon->Set_Status(BETWEEN_FIRING_SHOTS);
                        }
                    }
                }
            } else if (m_template->Is_Auto_Reloads_Clip()) {
                Reload_Ammo(source_obj);
                return true;
            } else {
                m_status = OUT_OF_AMMO;
                m_whenWeCanFireAgain = 0x7FFFFFFF;
            }
        }

        return false;
    }
}

bool Weapon::Is_Within_Attack_Range(const Object *source_obj, const Object *target_obj) const
{
    float range_sqr = GameMath::Square(Get_Attack_Range(source_obj));
    float distance_sqr;

    if (target_obj->Is_KindOf(KINDOF_BRIDGE)) {
        TBridgeAttackInfo info;
        g_theTerrainLogic->Get_Bridge_Attack_Points(target_obj, &info);
        distance_sqr =
            g_thePartitionManager->Get_Distance_Squared(source_obj, &info.m_attackPoint1, FROM_BOUNDINGSPHERE_2D, nullptr);

        if (distance_sqr > range_sqr) {
            distance_sqr = g_thePartitionManager->Get_Distance_Squared(
                source_obj, &info.m_attackPoint2, FROM_BOUNDINGSPHERE_2D, nullptr);
        }
    } else {
        distance_sqr = g_thePartitionManager->Get_Distance_Squared(source_obj, target_obj, FROM_BOUNDINGSPHERE_2D, nullptr);
    }

    if (distance_sqr < GameMath::Square(m_template->Get_Minimum_Attack_Range())) {
        return false;
    }

    if (distance_sqr > range_sqr) {
        return false;
    }

    if (!Is_Contact_Weapon() || !target_obj->Is_KindOf(KINDOF_STRUCTURE)) {
        return true;
    }

    SimpleObjectIterator *iter = g_thePartitionManager->Iterate_Potential_Collisions(
        source_obj->Get_Position(), source_obj->Get_Geometry_Info(), 0.0f, false);
    MemoryPoolObjectHolder holder(iter);

    for (Object *o = iter->First(); o != nullptr; o = iter->Next()) {
        if (target_obj == o) {
            return true;
        }
    }

    return false;
}

bool Weapon::Is_Within_Attack_Range(const Object *source_obj, const Coord3D *target_pos) const
{
    float distance_sqr =
        g_thePartitionManager->Get_Distance_Squared(source_obj, target_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
    return distance_sqr >= GameMath::Square(m_template->Get_Minimum_Attack_Range())
        && distance_sqr <= GameMath::Square(Get_Attack_Range(source_obj));
}

float Weapon::Get_Attack_Range(const Object *source_obj) const
{
    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    return m_template->Get_Attack_Range(bonus);
}

void Weapon::Compute_Bonus(const Object *source_obj, unsigned int flags, WeaponBonus &bonus) const
{
    bonus.Clear();
    flags |= source_obj->Get_Weapon_Bonus_Condition();

    if (source_obj->Get_Contained_By() != nullptr) {
        ContainModuleInterface *contain = source_obj->Get_Contained_By()->Get_Contain();

        if (contain != nullptr) {
            if (contain->Is_Weapon_Bonus_Passed_To_Passengers()) {
                flags |= contain->Get_Weapon_Bonus_Passed_To_Passengers();
            }
        }
    }

    if (g_theWriteableGlobalData->m_weaponBonusSet != nullptr) {
        g_theWriteableGlobalData->m_weaponBonusSet->Append_Bonuses(flags, bonus);
    }

    if (m_template->Get_Extra_Bonus() != nullptr) {
        m_template->Get_Extra_Bonus()->Append_Bonuses(flags, bonus);
    }
}

void Weapon::On_Weapon_Bonus_Change(const Object *source_obj)
{
    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    bool time_set = false;
    int time;

    if (Get_Status() == RELOADING_CLIP) {
        time = m_template->Get_Clip_Reload_Time(bonus);
        time_set = true;
    } else if (Get_Status() == BETWEEN_FIRING_SHOTS) {
        time = m_template->Get_Delay_Between_Shots(bonus);
        time_set = true;
    }

    if (time_set) {
        m_whenLastReloadStarted = g_theGameLogic->Get_Frame();
        m_whenWeCanFireAgain = time + m_whenLastReloadStarted;

        if (source_obj->Is_Share_Weapon_Reload_Time()) {
            for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
                Weapon *weapon = const_cast<Weapon *>(source_obj->Get_Weapon_In_Weapon_Slot(static_cast<WeaponSlotType>(i)));

                if (weapon != nullptr) {
                    weapon->Set_Next_Shot(m_whenWeCanFireAgain);
                    weapon->Set_Status(RELOADING_CLIP);
                }
            }
        }
    }
}

bool Weapon::Is_Within_Target_Pitch(const Object *source_obj, const Object *target_obj) const
{
    if (Is_Contact_Weapon() || !Is_Pitch_Limited()) {
        return true;
    }

    const Coord3D *source_pos = source_obj->Get_Position();
    const Coord3D *target_pos = target_obj->Get_Position();

    if (GameMath::Fabs(target_pos->z - source_pos->z) < 10.0f) {
        return true;
    }

    float pitch_above;
    float pitch_below;
    source_obj->Get_Geometry_Info().Calc_Pitches(
        *source_pos, target_obj->Get_Geometry_Info(), *target_pos, pitch_above, pitch_below);

    if (m_template->Get_Min_Target_Pitch() > pitch_above || m_template->Get_Max_Target_Pitch() < pitch_above) {
        if (m_template->Get_Min_Target_Pitch() > pitch_below || m_template->Get_Max_Target_Pitch() < pitch_below) {
            if (m_template->Get_Min_Target_Pitch() < pitch_above) {
                return false;
            }

            if (m_template->Get_Max_Target_Pitch() > pitch_below) {
                return false;
            }
        }
    }

    return true;
}

void Weapon::Load_Ammo_Now(const Object *source_obj)
{
    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    Reload_With_Bonus(source_obj, bonus, true);
}

void Weapon::Reload_With_Bonus(const Object *source_obj, const WeaponBonus &bonus, bool load_instantly)
{
    if (m_template->Get_Clip_Size() <= 0 || m_ammoInClip != m_template->Get_Clip_Size()
        || source_obj->Is_Share_Weapon_Reload_Time()) {
        m_ammoInClip = m_template->Get_Clip_Size();

        if (m_ammoInClip == 0) {
            m_ammoInClip = INT_MAX;
        }

        m_status = RELOADING_CLIP;
        int time;

        if (load_instantly) {
            time = 0;
        } else {
            time = m_template->Get_Clip_Reload_Time(bonus);
        }

        m_whenLastReloadStarted = g_theGameLogic->Get_Frame();
        m_whenWeCanFireAgain = m_whenLastReloadStarted + time;

        if (source_obj->Is_Share_Weapon_Reload_Time()) {
            for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
                Weapon *weapon = const_cast<Weapon *>(source_obj->Get_Weapon_In_Weapon_Slot(static_cast<WeaponSlotType>(i)));

                if (weapon != nullptr) {
                    weapon->Set_Next_Shot(m_whenWeCanFireAgain);
                    weapon->Set_Status(RELOADING_CLIP);
                }
            }
        }

        Rebuild_Scatter_Targets();
    }
}

void Weapon::Rebuild_Scatter_Targets()
{
    m_scatterTargetIndexes.clear();
    const std::vector<Coord2D> &targets = m_template->Get_Scatter_Targets();

    for (size_t i = 0; i < targets.size(); i++) {
        m_scatterTargetIndexes.push_back(static_cast<int>(i));
    }
}

bool Weapon::Is_Damage_Weapon() const
{
    DamageType type = m_template->Get_Damage_Type();

    switch (type) {
        case DAMAGE_DEPLOY:
            return true;
        case DAMAGE_HACK:
            return false;
        case DAMAGE_DISARM:
            return true;
    }

    WeaponBonus bonus;
    return m_template->Get_Primary_Damage(bonus) > 0.0f || m_template->Get_Secondary_Damage(bonus) > 0.0f;
}

void Weapon::Reload_Ammo(const Object *source_obj)
{
    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    Reload_With_Bonus(source_obj, bonus, false);
}

Weapon::WeaponStatus Weapon::Get_Status() const
{
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (frame < m_whenPreAttackFinished) {
        return PRE_ATTACK;
    }

    if (frame >= m_whenWeCanFireAgain) {
        if (m_ammoInClip != 0) {
            m_status = READY_TO_FIRE;
        } else {
            m_status = OUT_OF_AMMO;
        }
    }

    return m_status;
}

float Weapon::Get_Percent_Ready_To_Fire() const
{
    switch (Get_Status()) {
        case READY_TO_FIRE:
            return 1.0f;
        case OUT_OF_AMMO:
        case PRE_ATTACK:
            return 0.0f;
        case BETWEEN_FIRING_SHOTS:
        case RELOADING_CLIP: {
            unsigned int now = g_theGameLogic->Get_Frame();
            unsigned int next_shot = Get_Next_Shot();
            captainslog_dbgassert(now >= m_whenLastReloadStarted, "now >= m_whenLastReloadStarted");

            if (now >= next_shot) {
                return 1.0f;
            }

            captainslog_dbgassert(next_shot >= m_whenLastReloadStarted, "next_shot >= m_whenLastReloadStarted");
            unsigned int total_time = next_shot - m_whenLastReloadStarted;

            if (total_time == 0) {
                return 1.0f;
            }

            unsigned int time_left = next_shot - now;
            captainslog_dbgassert(time_left <= total_time, "time_left <= total_time");

            if (total_time - time_left >= total_time) {
                return 1.0f;
            }

            return (float)(total_time - time_left) / (float)total_time;
        }
        default:
            captainslog_dbgassert(false, "should not get here");
            return 0.0f;
    }
}

float Weapon::Estimate_Weapon_Damage(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos)
{
    if (m_template == nullptr) {
        return 0.0f;
    }

    if (Get_Status() == OUT_OF_AMMO && !m_template->Is_Auto_Reloads_Clip()) {
        return 0.0f;
    }

    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    return m_template->Estimate_Weapon_Template_Damage(source_obj, victim_obj, victim_pos, bonus);
}

bool Weapon::Is_Source_Object_With_Goal_Position_Within_Attack_Range(
    const Object *source_obj, const Coord3D *goal_pos, const Object *target_obj, const Coord3D *target_pos) const
{
    float goal_squared;

    if (target_obj != nullptr) {
        goal_squared = g_thePartitionManager->Get_Goal_Distance_Squared(
            source_obj, goal_pos, target_obj, FROM_BOUNDINGSPHERE_2D, nullptr);
    } else {
        if (target_pos == nullptr) {
            return false;
        }

        goal_squared = g_thePartitionManager->Get_Goal_Distance_Squared(
            source_obj, goal_pos, target_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
    }

    return goal_squared >= GameMath::Square(m_template->Get_Minimum_Attack_Range())
        && goal_squared <= GameMath::Square(Get_Attack_Range(source_obj));
}

void Weapon::New_Projectile_Fired(
    const Object *source_obj, const Object *projectile_obj, const Object *target_obj, const Coord3D *target_pos)
{
    if (!m_template->Get_Projectile_Stream_Name().Is_Empty()) {
        Object *obj = g_theGameLogic->Find_Object_By_ID(m_projectileStreamID);

        if (obj == nullptr) {
            m_projectileStreamID = INVALID_OBJECT_ID;
            ThingTemplate *tmplate = g_theThingFactory->Find_Template(m_template->Get_Projectile_Stream_Name(), true);
            obj = g_theThingFactory->New_Object(
                tmplate, source_obj->Get_Controlling_Player()->Get_Default_Team(), OBJECT_STATUS_MASK_NONE);

            if (obj == nullptr) {
                return;
            }

            m_projectileStreamID = obj->Get_ID();
        }

        static NameKeyType key_ProjectileStreamUpdate = Name_To_Key("ProjectileStreamUpdate");
        ProjectileStreamUpdate *module = static_cast<ProjectileStreamUpdate *>(obj->Find_Module(key_ProjectileStreamUpdate));

        if (module != nullptr) {
            module->Set_Position(source_obj->Get_Position());
            ObjectID id;

            if (target_obj != nullptr) {
                id = target_obj->Get_ID();
            } else {
                id = INVALID_OBJECT_ID;
            }

            module->Add_Projectile(source_obj->Get_ID(), projectile_obj->Get_ID(), id, target_pos);
        }
    }
}

void Weapon::Create_Laser(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos)
{
    ThingTemplate *tmplate = g_theThingFactory->Find_Template(m_template->Get_Laser_Name(), true);

    if (tmplate != nullptr) {
        Object *laser = g_theThingFactory->New_Object(
            tmplate, source_obj->Get_Controlling_Player()->Get_Default_Team(), OBJECT_STATUS_MASK_NONE);

        if (laser != nullptr) {
            laser->Set_Position(source_obj->Get_Position());
            Drawable *drawable = laser->Get_Drawable();

            if (drawable != nullptr) {
                static NameKeyType key_LaserUpdate = Name_To_Key("LaserUpdate");
                LaserUpdate *update = static_cast<LaserUpdate *>(drawable->Find_Client_Update_Module(key_LaserUpdate));

                if (update != nullptr) {
                    Coord3D pos = *victim_pos;

                    if (victim_obj != nullptr && !victim_obj->Is_KindOf(KINDOF_PROJECTILE)
                        && !victim_obj->Is_Airborne_Target()) {
                        pos.z += 10.0f;
                    }

                    update->Init_Laser(
                        source_obj, victim_obj, source_obj->Get_Position(), &pos, m_template->Get_Laser_Bone_Name(), 0);
                }
            }
        }
    } else {
        captainslog_dbgassert(false,
            "Weapon::Create_Laser(). %s could not find template for its laser %s.",
            source_obj->Get_Template()->Get_Name().Str(),
            m_template->Get_Laser_Name().Str());
    }
}

int Weapon::Get_Clip_Reload_Time(const Object *source_obj) const
{
    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    return m_template->Get_Clip_Reload_Time(bonus);
}

void Weapon::Set_Clip_Percent_Full(float percent, bool b)
{
    if (m_template->Get_Clip_Size() != 0) {
        unsigned int size = GameMath::Fast_To_Int_Floor(m_template->Get_Clip_Size() * percent);

        if (size > m_ammoInClip || (b && size < m_ammoInClip)) {
            m_ammoInClip = size;
            m_status = m_ammoInClip != 0 ? READY_TO_FIRE : OUT_OF_AMMO;
            m_whenLastReloadStarted = g_theGameLogic->Get_Frame();
            m_whenWeCanFireAgain = m_whenLastReloadStarted;
            Rebuild_Scatter_Targets();
        }
    }
}

bool Weapon::Is_Too_Close(const Object *source_obj, const Object *target_obj) const
{
    float range = m_template->Get_Minimum_Attack_Range();

    if (range == 0.0f) {
        return false;
    }

    return GameMath::Square(range)
        > g_thePartitionManager->Get_Distance_Squared(source_obj, target_obj, FROM_BOUNDINGSPHERE_2D, nullptr);
}

bool Weapon::Is_Too_Close(const Object *source_obj, const Coord3D *target_pos) const
{
    float range = m_template->Get_Minimum_Attack_Range();

    if (range == 0.0f) {
        return false;
    }

    return GameMath::Square(range)
        > g_thePartitionManager->Get_Distance_Squared(source_obj, target_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
}

bool Weapon::Is_Goal_Pos_Within_Attack_Range(
    const Object *source_obj, const Coord3D *goal_pos, const Object *target_obj, const Coord3D *target_pos) const
{
    float max_attack_range = GameMath::Square(Get_Attack_Range(source_obj) - 2.5f);
    float distance_sqr;

    if (target_obj != nullptr) {
        if (target_obj->Is_KindOf(KINDOF_BRIDGE)) {
            TBridgeAttackInfo info;
            g_theTerrainLogic->Get_Bridge_Attack_Points(target_obj, &info);
            distance_sqr = g_thePartitionManager->Get_Goal_Distance_Squared(
                source_obj, goal_pos, &info.m_attackPoint1, FROM_BOUNDINGSPHERE_2D, nullptr);

            if (distance_sqr > max_attack_range) {
                distance_sqr = g_thePartitionManager->Get_Goal_Distance_Squared(
                    source_obj, goal_pos, &info.m_attackPoint2, FROM_BOUNDINGSPHERE_2D, nullptr);
            }
        } else {
            distance_sqr = g_thePartitionManager->Get_Goal_Distance_Squared(
                source_obj, goal_pos, target_obj, FROM_BOUNDINGSPHERE_2D, nullptr);
        }
    } else {
        distance_sqr = g_thePartitionManager->Get_Goal_Distance_Squared(
            source_obj, goal_pos, target_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
    }

    float min_attack_range = GameMath::Square(m_template->Get_Minimum_Attack_Range() + 2.5f);
    return distance_sqr >= min_attack_range && distance_sqr <= max_attack_range;
}

float Weapon::Get_Attack_Distance(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos) const
{
    float range = Get_Attack_Range(source_obj);

    if (victim_obj != nullptr) {
        return victim_obj->Get_Geometry_Info().Get_Bounding_Circle_Radius()
            + source_obj->Get_Geometry_Info().Get_Bounding_Circle_Radius() + range;
    }

    return range;
}

void Weapon::Pre_Fire_Weapon(const Object *source_obj, const Object *victim_obj)
{
    int delay = Get_Pre_Attack_Delay(source_obj, victim_obj);

    if (delay > 0) {
        Set_Status(PRE_ATTACK);
        Set_Pre_Attack_Finished(delay + g_theGameLogic->Get_Frame());

        if (m_template->Is_Leech_Range_Weapon()) {
            Set_Leech_Range_Active(true);
        }
    }
}

int Weapon::Get_Pre_Attack_Delay(const Object *source_obj, const Object *target_obj) const
{
    AttackType type = m_template->Get_Pre_Attack_Type();

    if (type == ATTACK_TYPE_PER_CLIP) {
        if (m_template->Get_Clip_Size() > 0 && m_ammoInClip < (unsigned int)m_template->Get_Clip_Size()) {
            return 0;
        }
    } else if (type == ATTACK_TYPE_PER_ATTACK && source_obj->Get_Num_Consecutive_Shots_Fired_At_Target(target_obj) > 0) {
        return 0;
    }

    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    return m_template->Get_Pre_Attack_Delay(bonus);
}

bool Weapon::Fire_Projectile_Detonation_Weapon(
    const Object *source_obj, Object *victim_obj, unsigned int bonus_condition, bool do_damage)
{
    return Private_Fire_Weapon(source_obj, victim_obj, nullptr, true, false, bonus_condition, nullptr, do_damage);
}

Object *Weapon::Force_Fire_Weapon(const Object *source_obj, const Coord3D *victim_pos)
{
    ObjectID id = INVALID_OBJECT_ID;
    Private_Fire_Weapon(source_obj, nullptr, victim_pos, false, true, 0, &id, true);
    return g_theGameLogic->Find_Object_By_ID(id);
}

float Weapon::Get_Primary_Damage_Radius(const Object *source_obj) const
{
    WeaponBonus bonus;
    Compute_Bonus(source_obj, 0, bonus);
    return m_template->Get_Primary_Damage_Radius(bonus);
}

void Weapon::Get_Firing_Line_Of_Sight_Origin(const Object *source_obj, Coord3D &pos) const
{
    pos.z += source_obj->Get_Geometry_Info().Get_Max_Height_Above_Position();
}

bool Weapon::Is_Clear_Firing_Line_Of_Sight_Terrain(const Object *source_obj, const Object *target_obj) const
{
    Coord3D source_pos = *source_obj->Get_Position();
    Get_Firing_Line_Of_Sight_Origin(source_obj, source_pos);
    const Coord3D *target_pos = target_obj->Get_Position();
    Coord3D center_pos;
    target_obj->Get_Geometry_Info().Get_Center_Position(*target_pos, center_pos);
    return g_thePartitionManager->Is_Clear_Line_Of_Sight_Terrain(nullptr, source_pos, nullptr, center_pos);
}

bool Weapon::Is_Clear_Firing_Line_Of_Sight_Terrain(const Object *source_obj, const Coord3D &target_pos) const
{
    Coord3D source_pos = *source_obj->Get_Position();
    Get_Firing_Line_Of_Sight_Origin(source_obj, source_pos);
    return g_thePartitionManager->Is_Clear_Line_Of_Sight_Terrain(nullptr, source_pos, nullptr, target_pos);
}

bool Weapon::Is_Clear_Goal_Firing_Line_Of_Sight_Terrain(
    const Object *source_obj, const Coord3D &goal_pos, const Object *target_obj) const
{
    Coord3D source_pos = goal_pos;
    Get_Firing_Line_Of_Sight_Origin(source_obj, source_pos);
    const Coord3D *target_pos = target_obj->Get_Position();
    Coord3D center_pos;
    target_obj->Get_Geometry_Info().Get_Center_Position(*target_pos, center_pos);
    return g_thePartitionManager->Is_Clear_Line_Of_Sight_Terrain(nullptr, source_pos, nullptr, center_pos);
}

bool Weapon::Is_Clear_Goal_Firing_Line_Of_Sight_Terrain(
    const Object *source_obj, const Coord3D &goal_pos, const Coord3D &target_pos) const
{
    Coord3D source_pos = goal_pos;
    Get_Firing_Line_Of_Sight_Origin(source_obj, source_pos);
    return g_thePartitionManager->Is_Clear_Line_Of_Sight_Terrain(nullptr, source_pos, nullptr, target_pos);
}

void Weapon::Transfer_Next_Shot_Stats_From(const Weapon &weapon)
{
    m_whenWeCanFireAgain = weapon.Get_Next_Shot();
    m_whenLastReloadStarted = weapon.Get_Last_Reload_Started();
    m_status = weapon.Get_Status();
}

void Clip_To_Terrain_Extent(Coord3D &pos)
{
    Region3D extent;
    g_theTerrainLogic->Get_Extent(&extent);

    if (extent.lo.x + 10.0f > pos.x) {
        pos.x = extent.lo.x + 10.0f;
    }

    if (extent.lo.y + 10.0f > pos.y) {
        pos.y = extent.lo.y + 10.0f;
    }

    if (extent.hi.x - 10.0f < pos.x) {
        pos.x = extent.hi.x - 10.0f;
    }

    if (extent.hi.y - 10.0f < pos.y) {
        pos.y = extent.hi.y - 10.0f;
    }
}

bool Weapon::Compute_Approach_Target(const Object *source_obj,
    const Object *target_obj,
    const Coord3D *target_pos,
    float angle_offset,
    Coord3D &approach_target_pos) const
{
    const Coord3D *pos;
    Coord3D vector;

    if (target_obj != nullptr) {
        pos = target_obj->Get_Position();
        g_thePartitionManager->Get_Vector_To(target_obj, source_obj, FROM_BOUNDINGSPHERE_2D, vector);
    } else if (target_pos != nullptr) {
        pos = target_pos;
        g_thePartitionManager->Get_Vector_To(source_obj, target_pos, FROM_BOUNDINGSPHERE_2D, vector);
        vector.x = -vector.x;
        vector.y = -vector.y;
        vector.z = -vector.z;
    } else {
        captainslog_dbgassert(false, "error");
        approach_target_pos.Zero();
        return false;
    }

    float length = vector.Length();
    float min_range = m_template->Get_Minimum_Attack_Range();

    if (min_range <= 10.0f || length >= min_range) {
        if (length >= 0.001f) {
            if (Is_Contact_Weapon()) {
                approach_target_pos = *pos;
                return false;
            } else {
                vector.x = vector.x / length;
                vector.y = vector.y / length;
                vector.z = vector.z / length;

                if (angle_offset != 0.0f) {
                    float angle = GameMath::Atan2(vector.y, vector.x);
                    vector.x = GameMath::Cos(angle + angle_offset);
                    vector.y = GameMath::Sin(angle + angle_offset);
                }

                float range = Get_Attack_Range(source_obj) * 0.9f;
                approach_target_pos.x = range * vector.x + pos->x;
                approach_target_pos.y = range * vector.y + pos->y;
                approach_target_pos.z = range * vector.z + pos->z;

                if (source_obj->Get_AI_Update_Interface() != nullptr) {
                    const AIUpdateInterface *update = source_obj->Get_AI_Update_Interface();

                    if (update->Is_Aircraft_That_Adjusts_Destination()) {
                        g_theAI->Get_Pathfinder()->Adjust_Target_Destination(
                            source_obj, target_obj, target_pos, this, &approach_target_pos);
                    }
                }

                return false;
            }
        } else {
            approach_target_pos = *source_obj->Get_Position();
            return true;
        }
    } else {
        captainslog_dbgassert(Get_Attack_Range(source_obj) * 0.9f > min_range, "Min attack range is too near attack range.");
        const Coord3D *source_pos = source_obj->Get_Position();
        vector.x = source_pos->x - pos->x;
        vector.x = source_pos->y - pos->y;
        vector.z = 0.0f;
        vector.Normalize();

        if (source_obj->Is_Above_Terrain()) {
            float angle = source_obj->Get_Orientation() - GameMath::Atan2(-vector.y, -vector.x);

            if (angle > DEG_TO_RADF(360.0f)) {
                angle -= DEG_TO_RADF(360.0f);
            }

            if (angle < DEG_TO_RADF(-360.0f)) {
                angle += DEG_TO_RADF(360.0f);
            }

            if (GameMath::Fabs(angle) < DEG_TO_RADF(90.0f)) {
                vector.x = -vector.x;
                vector.y = -vector.y;
                vector.z = -vector.z;
            }
        }

        if (angle_offset != 0.0f) {
            float angle = GameMath::Atan2(vector.y, vector.x);
            vector.x = GameMath::Cos(angle + angle_offset);
            vector.y = GameMath::Sin(angle + angle_offset);
        }

        float range2 = (Get_Attack_Range(source_obj) - min_range) / 2.0f;

        if (target_obj != nullptr) {
            range2 += target_obj->Get_Geometry_Info().Get_Bounding_Circle_Radius();
        }

        range2 += source_obj->Get_Geometry_Info().Get_Bounding_Circle_Radius();
        approach_target_pos.x = range2 * vector.x + pos->x;
        approach_target_pos.y = range2 * vector.y + pos->y;
        approach_target_pos.z = range2 * vector.z + pos->z;
        Clip_To_Terrain_Extent(approach_target_pos);
        return false;
    }
}

void Weapon::Calc_Projectile_Launch_Position(
    const Object *source_obj, WeaponSlotType wslot, int ammo_index, Matrix3D &launch_transform, Coord3D &launch_pos)
{
    if (source_obj->Get_Contained_By()
        && source_obj->Get_Contained_By()->Get_Contain()->Is_Enclosing_Container_For(source_obj)) {
        const Matrix3D *tm = source_obj->Get_Transform_Matrix();
        launch_transform = *tm;
        Vector3 pos;
        tm->Get_Translation(&pos);
        launch_pos.x = pos.X;
        launch_pos.y = pos.Y;
        launch_pos.z = pos.Z;
    } else {
        float turret_yaw = 0.0f;
        float turret_pitch = 0.0f;
        WhichTurretType turret;
        const AIUpdateInterface *update = source_obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            turret = update->Get_Which_Turret_For_Weapon_Slot(wslot, &turret_yaw, &turret_pitch);
        } else {
            turret = TURRET_INVALID;
        }

        Matrix3D tm(true);
        Coord3D turret_yaw_pos;
        Coord3D turret_pitch_pos;
        turret_yaw_pos.Zero();
        turret_pitch_pos.Zero();
        Drawable *drawable = source_obj->Get_Drawable();

        if (drawable == nullptr
            || !drawable->Get_Projectile_Launch_Offset(wslot, ammo_index, &tm, turret, &turret_yaw_pos, &turret_pitch_pos)) {
            captainslog_dbgassert(false, "ProjectileLaunchPos %d %d not found!", wslot, ammo_index);
            tm.Make_Identity();
            turret_yaw_pos.Zero();
            turret_pitch_pos.Zero();
        }

        if (turret != TURRET_INVALID) {
            Matrix3D yaw_tm(true);
            Matrix3D pitch_tm(true);

            pitch_tm.Translate(turret_pitch_pos.x, turret_pitch_pos.y, turret_pitch_pos.z);
            pitch_tm.In_Place_Pre_Rotate_Y(-turret_pitch);
            pitch_tm.Translate(-turret_pitch_pos.x, -turret_pitch_pos.y, -turret_pitch_pos.z);

            yaw_tm.Translate(turret_yaw_pos.x, turret_yaw_pos.y, turret_yaw_pos.z);
            yaw_tm.In_Place_Pre_Rotate_Z(turret_yaw);
            yaw_tm.Translate(-turret_yaw_pos.x, -turret_yaw_pos.y, -turret_yaw_pos.z);

            Matrix3D tm2(tm);
            tm.Mul(yaw_tm, pitch_tm);
            tm.Post_Mul(tm2);
        }

        source_obj->Convert_Bone_Pos_To_World_Pos(nullptr, &tm, nullptr, &launch_transform);
        Vector3 pos;
        launch_transform.Get_Translation(&pos);
        launch_pos.x = pos.X;
        launch_pos.y = pos.Y;
        launch_pos.z = pos.Z;
    }
}

void Weapon::Position_Projectile_For_Launch(
    Object *projectile_obj, const Object *source_obj, WeaponSlotType wslot, int ammo_index)
{
    if (source_obj != nullptr) {
        Matrix3D tm(true);
        Coord3D pos;
        Calc_Projectile_Launch_Position(source_obj, wslot, ammo_index, tm, pos);
        projectile_obj->Get_Drawable()->Set_Drawable_Hidden(false);
        projectile_obj->Set_Transform_Matrix(&tm);
        projectile_obj->Set_Position(&pos);
        projectile_obj->Get_Experience_Tracker()->Set_Experience_Sink(source_obj->Get_ID());
        const PhysicsBehavior *source_phys = source_obj->Get_Physics();
        PhysicsBehavior *projectile_phys = projectile_obj->Get_Physics();

        if (source_phys != nullptr) {
            if (projectile_phys != nullptr) {
                source_phys->Transfer_Velocity_To(projectile_phys);
                projectile_phys->Set_Ignore_Collisions_With(source_obj);
            }
        }
    } else {
        g_theGameLogic->Destroy_Object(projectile_obj);
    }
}

void Make_Assistance_Request(Object *object, void *data)
{
    AssistanceRequestData *request = static_cast<AssistanceRequestData *>(data);
    if (object != request->source_obj) {
        if (object->Get_Template()->Is_Equivalent_To(request->source_obj->Get_Template())) {
            if (g_thePartitionManager->Get_Distance_Squared(object, request->source_obj, FROM_CENTER_2D, nullptr)
                <= request->request_assist_range_sqr) {
                static NameKeyType key_assistUpdate = Name_To_Key("AssistedTargetingUpdate");
                AssistedTargetingUpdate *update =
                    static_cast<AssistedTargetingUpdate *>(object->Find_Module(key_assistUpdate));

                if (update != nullptr) {
                    if (update->Is_Free_To_Assist()) {
                        update->Assist_Attack(request->source_obj, request->target_obj);
                    }
                }
            }
        }
    }
}

void Weapon::Process_Request_Assistance(const Object *source_obj, Object *target_obj)
{
    Player *player = source_obj->Get_Controlling_Player();

    if (player != nullptr) {
        AssistanceRequestData data;
        data.source_obj = source_obj;
        data.target_obj = target_obj;
        data.request_assist_range_sqr = m_template->Get_Request_Assist_Range() * m_template->Get_Request_Assist_Range();
        player->Iterate_Objects(Make_Assistance_Request, &data);
    }
}

const char *s_theDeathTypeNames[] = {
    "NORMAL",
    "NONE",
    "CRUSHED",
    "BURNED",
    "EXPLODED",
    "POISONED",
    "TOPPLED",
    "FLOODED",
    "SUICIDED",
    "LASERED",
    "DETONATED",
    "SPLATTED,",
    "POISONED_BETA",
    "EXTRA_2",
    "EXTRA_3",
    "EXTRA_4",
    "EXTRA_5",
    "EXTRA_6",
    "EXTRA_7",
    "EXTRA_8",
    "POISONED_GAMMA",
    nullptr,
};

const char *s_theAutoReloadsClipNames[] = {
    "YES",
    "NO",
    "RETURN_TO_BASE",
    nullptr,
};

const char *s_theAttackTypeNames[] = {
    "PER_SHOT",
    "PER_ATTACK",
    "PER_CLIP",
    nullptr,
};

const char *s_theRadiusDamageAffectsNames[] = {
    "SELF",
    "ALLIES",
    "ENEMIES",
    "NEUTRALS",
    "SUICIDE",
    "NOT_SIMILAR",
    "NOT_AIRBORNE",
    nullptr,
};

const char *s_theProjectileCollidesWithNames[] = {
    "ALLIES",
    "ENEMIES",
    "STRUCTURES",
    "SHRUBBERY",
    "PROJECTILES",
    "WALLS",
    "SMALL_MISSILES",
    "BALLISTIC_MISSILES",
    "CONTROLLED_STRUCTURES",
    nullptr,
};

void Parse_All_Vet_Levels_ASCII_String(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf8String OCL_str = ini->Get_Next_Ascii_String();
    Utf8String *OCL_vet_array = static_cast<Utf8String *>(store);

    for (auto i = 0; i < VETERANCY_COUNT; ++i) {
        OCL_vet_array[i] = OCL_str;
    }
}

void Parse_Per_Vet_Level_ASCII_String(INI *ini, void *formal, void *store, const void *user_data)
{
    int vet_level = ini->Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);
    Utf8String *OCL_vet_array = static_cast<Utf8String *>(store);
    OCL_vet_array[vet_level] = ini->Get_Next_Ascii_String();
}

void Parse_All_Vet_Levels_FXList(INI *ini, void *formal, void *store, const void *user_data)
{
    FXList **lists = static_cast<FXList **>(store);
    FXList *list = nullptr;
    FXList::Parse(ini, nullptr, &list, nullptr);

    for (int i = 0; i < VETERANCY_COUNT; i++) {
        lists[i] = list;
    }
}

void Parse_Per_Vet_Level_FXList(INI *ini, void *formal, void *store, const void *user_data)
{
    FXList **lists = static_cast<FXList **>(store);
    int index = INI::Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);
    FXList *list = nullptr;
    FXList::Parse(ini, nullptr, &list, nullptr);
    lists[index] = list;
}

void Parse_All_Vet_Levels_Particle_System(INI *ini, void *formal, void *store, const void *user_data)
{
    ParticleSystemTemplate *particle_template = nullptr;
    ParticleSystemTemplate::Parse(ini, nullptr, &particle_template, nullptr);
    ParticleSystemTemplate **particle_syst_vet_array = static_cast<ParticleSystemTemplate **>(store);

    for (int i = 0; i < VETERANCY_COUNT; ++i) {
        particle_syst_vet_array[i] = particle_template;
    }
}

void Parse_Per_Vet_Level_Particle_System(INI *ini, void *formal, void *store, const void *user_data)
{
    int vet_level = ini->Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);
    ParticleSystemTemplate **particle_syst_vet_array = static_cast<ParticleSystemTemplate **>(store);
    ParticleSystemTemplate *particle_template = nullptr;
    ParticleSystemTemplate::Parse(ini, nullptr, &particle_template, nullptr);
    particle_syst_vet_array[vet_level] = particle_template;
}

// clang-format off
FieldParse WeaponTemplate::s_fieldParseTable[] = {
     { "ProjectileStreamName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_projectileStreamName) },
     { "LaserName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_laserName) },
     { "LaserBoneName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_laserBoneName) },
     { "PrimaryDamage", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_primaryDamage) },
     { "PrimaryDamageRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_primaryDamageRadius) },
     { "SecondaryDamage", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_secondaryDamage) },
     { "SecondaryDamageRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_secondaryDamageRadius) },
     { "ShockWaveAmount", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_shockWaveAmount) },
     { "ShockWaveRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_shockWaveRadius) },
     { "ShockWaveTaperOff", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_shockWaveTaperOff) },
     { "AttackRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_attackRange) },
     { "MinimumAttackRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_minimumAttackRange) },
     { "RequestAssistRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_requestAssistRange) },
     { "AcceptableAimDelta", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_aimDelta) },
     { "ScatterRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_scatterRadius) },
     { "ScatterTargetScalar", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_scatterTargetScalar) },
     { "ScatterRadiusVsInfantry", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_scatterRadiusVsInfantry) },
     { "DamageType", &BitFlags<DAMAGE_NUM_TYPES>::Parse_Single_Bit_From_INI, nullptr, offsetof(WeaponTemplate, m_damageType) },
     { "DamageStatusType", &BitFlags<OBJECT_STATUS_COUNT>::Parse_Single_Bit_From_INI, nullptr, offsetof(WeaponTemplate, m_damageStatusType) },
     { "DeathType", &INI::Parse_Index_List, s_theDeathTypeNames, offsetof(WeaponTemplate, m_deathType) },
     { "WeaponSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(WeaponTemplate, m_weaponSpeed) },
     { "MinWeaponSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(WeaponTemplate, m_minWeaponSpeed) },
     { "ScaleWeaponSpeed", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_scaleWeaponSpeed) },
     { "WeaponRecoil", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_weaponRecoil) },
     { "MinTargetPitch", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_minTargetPitch) },
     { "MaxTargetPitch", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_maxTargetPitch) },
     { "RadiusDamageAngle", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_radiusDamageAngle) },
     { "ProjectileObject", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_projectileObject) },
     { "FireSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(WeaponTemplate, m_fireSound) },
     { "FireSoundLoopTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_fireSoundLoopTime) },
     { "FireFX", &Parse_All_Vet_Levels_FXList, nullptr, offsetof(WeaponTemplate, m_fireFX) },
     { "ProjectileDetonationFX", &Parse_All_Vet_Levels_FXList, nullptr, offsetof(WeaponTemplate, m_projectileDetonateFX) },
     { "FireOCL", &Parse_All_Vet_Levels_ASCII_String, nullptr, offsetof(WeaponTemplate, m_fireOCLName) },
     { "ProjectileDetonationOCL", &Parse_All_Vet_Levels_ASCII_String, nullptr, offsetof(WeaponTemplate, m_projectileDetonationOCLName) },
     { "ProjectileExhaust", &Parse_All_Vet_Levels_Particle_System, nullptr, offsetof(WeaponTemplate, m_projectileExhaust) },
     { "VeterancyFireFX", &Parse_Per_Vet_Level_FXList, nullptr, offsetof(WeaponTemplate, m_fireFX) },
     { "VeterancyProjectileDetonationFX", &Parse_Per_Vet_Level_FXList, nullptr, offsetof(WeaponTemplate, m_projectileDetonateFX) },
     { "VeterancyFireOCL", &Parse_Per_Vet_Level_ASCII_String, nullptr, offsetof(WeaponTemplate, m_fireOCLName) },
     { "VeterancyProjectileDetonationOCL", &Parse_Per_Vet_Level_ASCII_String, nullptr, offsetof(WeaponTemplate, m_projectileDetonationOCLName) },
     { "VeterancyProjectileExhaust", &Parse_Per_Vet_Level_Particle_System, nullptr, offsetof(WeaponTemplate, m_projectileExhaust) },
     { "ClipSize", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_clipSize) },
     { "ContinuousFireOne", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_continuousFireOneShotsNeeded) },
     { "ContinuousFireTwo", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_continuousFireTwoShotsNeeded) },
     { "ContinuousFireCoast", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_continuousFireCoastFrames) },
     { "AutoReloadWhenIdle", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_autoReloadWhenIdle) },
     { "ClipReloadTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_clipReloadTime) },
     { "DelayBetweenShots", &WeaponTemplate::Parse_Shot_Delay, nullptr, 0 },
     { "ShotsPerBarrel", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_shotsPerBarrel) },
     { "DamageDealtAtSelfPosition", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_damageDealtAtSelfPosition) },
     { "RadiusDamageAffects", &INI::Parse_Bitstring32, s_theRadiusDamageAffectsNames, offsetof(WeaponTemplate, m_affectsMask) },
     { "ProjectileCollidesWith", &INI::Parse_Bitstring32, s_theProjectileCollidesWithNames, offsetof(WeaponTemplate, m_collideMask) },
     { "AntiAirborneVehicle", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_AIRBORNE_VEHICLE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiGround", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_GROUND), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiProjectile", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_PROJECTILE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiSmallMissile", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_SMALL_MISSILE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiMine", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_MINE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiParachute", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_PARACHUTE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiAirborneInfantry", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_AIRBORNE_INFANTRY), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiBallisticMissile", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_BALLISTIC_MISSILE), offsetof(WeaponTemplate, m_antiMask) },
     { "AutoReloadsClip", &INI::Parse_Index_List, s_theAutoReloadsClipNames, offsetof(WeaponTemplate, m_autoReloadsClip) },
     { "ProjectileStreamName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_projectileStreamName) },
     { "LaserName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_laserName) },
     { "LaserBoneName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_laserBoneName) },
     { "WeaponBonus", &WeaponTemplate::Parse_Weapon_Bonus_Set, nullptr, 0 },
     { "HistoricBonusTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_historicBonusTime) },
     { "HistoricBonusRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_historicBonusRadius) },
     { "HistoricBonusCount", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_historicBonusCount) },
     { "HistoricBonusWeapon", &WeaponStore::Parse_Weapon_Template, nullptr, offsetof(WeaponTemplate, m_historicBonusWeapon) },
     { "LeechRangeWeapon", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_leechRangeWeapon) },
     { "ScatterTarget", &WeaponTemplate::Parse_Scatter_Target, nullptr, 0 },
     { "CapableOfFollowingWaypoints", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_capableOfFollowingWaypoint) },
     { "ShowsAmmoPips", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_showAmmoPips) },
     { "AllowAttackGarrisonedBldgs", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_allowAttackGarrisonedBldgs) },
     { "PlayFXWhenStealthed", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_playFXWhenStealthed) },
     { "PreAttackDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_preAttackDelay) },
     { "PreAttackType", &INI::Parse_Index_List, s_theAttackTypeNames, offsetof(WeaponTemplate, m_preAttackType) },
     { "ContinueAttackRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_continueAttackRange) },
     { "SuspendFXDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_suspendFXDelay) },
     { "MissileCallsOnDie", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_missileCallsOnDie) },
     { nullptr, nullptr, nullptr, 0 },
};
// clang-format on

WeaponTemplate::WeaponTemplate() :
    m_nextTemplate(nullptr),
    m_name("NoNameWeapon"),
    m_nameKey(NAMEKEY_INVALID),
    m_primaryDamage(0.0f),
    m_primaryDamageRadius(0.0f),
    m_secondaryDamage(0.0f),
    m_secondaryDamageRadius(0.0f),
    m_shockWaveAmount(0.0f),
    m_shockWaveRadius(0.0f),
    m_shockWaveTaperOff(0.0f),
    m_attackRange(0.0f),
    m_minimumAttackRange(0.0f),
    m_requestAssistRange(0.0f),
    m_aimDelta(0.0f),
    m_scatterRadius(0.0f),
    m_scatterTargetScalar(0.0f),
    m_damageType(DAMAGE_EXPLOSION),
    m_deathType(DEATH_NORMAL),
    m_weaponSpeed(999999.0f),
    m_minWeaponSpeed(999999.0f),
    m_scaleWeaponSpeed(false),
    m_weaponRecoil(0.0f),
    m_minTargetPitch(DEG_TO_RADF(-180.0f)),
    m_maxTargetPitch(DEG_TO_RADF(180.0f)),
    m_radiusDamageAngle(DEG_TO_RADF(180.0f)),
    m_projectileTemplate(nullptr),
    m_fireSoundLoopTime(0),
    m_extraBonus(nullptr),
    m_clipSize(0),
    m_clipReloadTime(0),
    m_minDelayBetweenShots(0),
    m_maxDelayBetweenShots(0),
    m_continuousFireOneShotsNeeded(0x7FFFFFFF),
    m_continuousFireTwoShotsNeeded(0x7FFFFFFF),
    m_continuousFireCoastFrames(0),
    m_autoReloadWhenIdle(0),
    m_shotsPerBarrel(1),
    m_antiMask(ANTI_GROUND),
    m_affectsMask(AFFECTS_NEUTRALS | AFFECTS_ENEMIES | AFFECTS_ALLIES),
    m_collideMask(COLLIDE_STRUCTURES),
    m_damageDealtAtSelfPosition(false),
    m_autoReloadsClip(AUTO_RELOADS_CLIP_YES),
    m_preAttackType(ATTACK_TYPE_PER_SHOT),
    m_historicBonusTime(0),
    m_historicBonusRadius(0.0f),
    m_historicBonusCount(0),
    m_historicBonusWeapon(nullptr),
    m_leechRangeWeapon(false),
    m_capableOfFollowingWaypoint(false),
    m_showAmmoPips(false),
    m_allowAttackGarrisonedBldgs(false),
    m_playFXWhenStealthed(false),
    m_preAttackDelay(0),
    m_continueAttackRange(0.0f),
    m_scatterRadiusVsInfantry(0.0f),
    m_damageStatusType(OBJECT_STATUS_NONE),
    m_suspendFXDelay(0),
    m_missileCallsOnDie(false)
{
    m_projectileObject.Clear();

    for (int i = 0; i < VETERANCY_COUNT; i++) {
        m_fireOCLName[i].Clear();
        m_projectileDetonationOCLName[i].Clear();
        m_projectileExhaust[i] = nullptr;
        m_fireOCL[i] = nullptr;
        m_projectileDetonationOCL[i] = nullptr;
        m_fireFX[i] = nullptr;
        m_projectileDetonateFX[i] = nullptr;
    }

    m_projectileStreamName.Clear();
    m_laserName.Clear();
    m_laserBoneName.Clear();
}

WeaponTemplate::~WeaponTemplate()
{
    if (m_nextTemplate != nullptr) {
        m_nextTemplate->Delete_Instance();
    }

    if (m_extraBonus != nullptr) {
        m_extraBonus->Delete_Instance();
    }
}

void WeaponTemplate::Reset()
{
    m_historicDamage.clear();
}

void WeaponTemplate::Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponTemplate *tmplate = static_cast<WeaponTemplate *>(formal);

    if (tmplate->m_extraBonus == nullptr) {
        tmplate->m_extraBonus = new WeaponBonusSet();
    }

    tmplate->m_extraBonus->Parse_Weapon_Bonus_Set(ini);
}

void WeaponTemplate::Parse_Scatter_Target(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponTemplate *tmplate = static_cast<WeaponTemplate *>(formal);
    Coord2D target;
    INI::Parse_Coord2D(ini, nullptr, &target, nullptr);
    tmplate->m_scatterTarget.push_back(target);
}

const float _DURATION_MULT = 0.03f;

void WeaponTemplate::Parse_Shot_Delay(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponTemplate *tmplate = static_cast<WeaponTemplate *>(formal);
    const char *str = ini->Get_Next_Token_Or_Null(ini->Get_Seps_Colon());

    if (strcasecmp(str, "Min") == 0) {
        tmplate->m_minDelayBetweenShots = INI::Scan_Int(ini->Get_Next_Token(ini->Get_Seps_Colon()));
        const char *str2 = ini->Get_Next_Token_Or_Null(ini->Get_Seps_Colon());

        if (strcasecmp(str2, "Max") == 0) {
            tmplate->m_maxDelayBetweenShots = INI::Scan_Int(ini->Get_Next_Token(ini->Get_Seps_Colon()));
        } else {
            tmplate->m_maxDelayBetweenShots = tmplate->m_minDelayBetweenShots;
        }
    } else {
        tmplate->m_minDelayBetweenShots = INI::Scan_Int(str);
        tmplate->m_maxDelayBetweenShots = tmplate->m_minDelayBetweenShots;
    }

    tmplate->m_minDelayBetweenShots = GameMath::Ceil(tmplate->m_minDelayBetweenShots * _DURATION_MULT);
    tmplate->m_maxDelayBetweenShots = GameMath::Ceil(tmplate->m_maxDelayBetweenShots * _DURATION_MULT);
}

void WeaponTemplate::Post_Process_Load()
{
    if (g_theThingFactory != nullptr) {
        if (m_projectileObject.Is_Empty()) {
            m_projectileTemplate = nullptr;
        } else {
            m_projectileTemplate = g_theThingFactory->Find_Template(m_projectileObject, true);
            captainslog_dbgassert(m_projectileTemplate != nullptr, "projectile %s not found!", m_projectileObject.Str());
        }

        for (int i = 0; i < VETERANCY_COUNT; i++) {
            if (m_fireOCLName[i].Is_Empty()) {
                m_fireOCL[i] = nullptr;
            } else {
                m_fireOCL[i] = g_theObjectCreationListStore->Find_Object_Creation_List(m_fireOCLName[i].Str());
                captainslog_dbgassert(m_fireOCL[i] != nullptr, "OCL %s not found in a weapon!", m_fireOCLName[i].Str());
            }

            if (m_projectileDetonationOCLName[i].Is_Empty()) {
                m_projectileDetonationOCL[i] = nullptr;
            } else {
                m_projectileDetonationOCL[i] =
                    g_theObjectCreationListStore->Find_Object_Creation_List(m_projectileDetonationOCLName[i].Str());
                captainslog_dbgassert(m_projectileDetonationOCL[i] != nullptr,
                    "OCL %s not found in a weapon!",
                    m_projectileDetonationOCLName[i].Str());
            }
        }
    } else {
        captainslog_dbgassert(false, "you must call this after TheThingFactory is inited");
    }
}

float WeaponTemplate::Get_Attack_Range(const WeaponBonus &bonus) const
{
    float range = bonus.Get_Field(WeaponBonus::RANGE) * m_attackRange - 2.5f;

    if (range < 0.0f) {
        return 0.0f;
    }

    return range;
}

float WeaponTemplate::Get_Minimum_Attack_Range() const
{
    float range = m_minimumAttackRange - 2.5f;

    if (range < 0.0f) {
        return 0.0f;
    }

    return range;
}

float WeaponTemplate::Get_Unmodified_Attack_Range() const
{
    return m_attackRange;
}

int WeaponTemplate::Get_Delay_Between_Shots(const WeaponBonus &bonus) const
{
    float delay;

    if (m_minDelayBetweenShots == m_maxDelayBetweenShots) {
        delay = m_minDelayBetweenShots;
    } else {
        delay = Get_Logic_Random_Value(m_minDelayBetweenShots, m_maxDelayBetweenShots);
    }

    return GameMath::Fast_To_Int_Floor(delay / bonus.Get_Field(WeaponBonus::RATE_OF_FIRE));
}

int WeaponTemplate::Get_Clip_Reload_Time(const WeaponBonus &bonus) const
{
    return GameMath::Fast_To_Int_Floor(m_clipReloadTime / bonus.Get_Field(WeaponBonus::RATE_OF_FIRE));
}

int WeaponTemplate::Get_Pre_Attack_Delay(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::PRE_ATTACK) * m_preAttackDelay;
}

float WeaponTemplate::Get_Primary_Damage(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::DAMAGE) * m_primaryDamage;
}

float WeaponTemplate::Get_Primary_Damage_Radius(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::RADIUS) * m_primaryDamageRadius;
}

float WeaponTemplate::Get_Secondary_Damage(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::DAMAGE) * m_secondaryDamage;
}

float WeaponTemplate::Get_Secondary_Damage_Radius(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::RADIUS) * m_secondaryDamageRadius;
}

bool WeaponTemplate::Is_Contact_Weapon() const
{
    return m_attackRange - 2.5f < 10.0f;
}

void WeaponTemplate::Trim_Old_Historic_Damage() const
{
    unsigned int frame = g_theGameLogic->Get_Frame() - g_theWriteableGlobalData->m_historicDamageLimit;

    while (m_historicDamage.size() != 0 && m_historicDamage.front().frame <= frame) {
        m_historicDamage.pop_front();
    }
}

bool WeaponTemplate::Should_Projectile_Collide_With(
    const Object *source_obj, const Object *projectile_obj, const Object *collide_obj, ObjectID id) const
{
    if (projectile_obj == nullptr || collide_obj == nullptr) {
        return false;
    }

    if (id == collide_obj->Get_ID()) {
        return true;
    }

    if (source_obj != nullptr) {
        if (source_obj == collide_obj) {
            return false;
        }

        if (source_obj->Get_Contained_By() == collide_obj) {
            return false;
        }
    }

    if ((Get_Damage_Type() == DAMAGE_FLAME || Get_Damage_Type() == DAMAGE_PARTICLE_BEAM)
        && collide_obj->Get_Status(OBJECT_STATUS_BURNED)) {
        return false;
    }

    if (collide_obj->Is_KindOf(KINDOF_FS_AIRFIELD)) {
        for (BehaviorModule **i = collide_obj->Get_All_Modules(); *i != nullptr; i++) {
            ParkingPlaceBehaviorInterface *park = (*i)->Get_Parking_Place_Behavior_Interface();

            if (park != nullptr && park->Has_Reserved_Space(id)) {
                return false;
            }
        }
    }

    const AIUpdateInterface *update = collide_obj->Get_AI_Update_Interface();

    if (update != nullptr && update->Get_Sneaky_Targeting_Offset(nullptr)) {
        return false;
    }

    int mask = 0;
    Relationship r = projectile_obj->Get_Relationship(collide_obj);

    if (r == ALLIES) {
        mask = COLLIDE_ALLIES;
    } else if (r == ENEMIES) {
        mask = COLLIDE_ENEMIES;
    }

    if (collide_obj->Is_KindOf(KINDOF_STRUCTURE)) {
        if (collide_obj->Get_Controlling_Player() == projectile_obj->Get_Controlling_Player()) {
            mask |= COLLIDE_CONTROLLED_STRUCTURES;
        } else {
            mask |= COLLIDE_STRUCTURES;
        }
    }

    if (collide_obj->Is_KindOf(KINDOF_SHRUBBERY)) {
        mask |= COLLIDE_SHRUBBERY;
    }

    if (collide_obj->Is_KindOf(KINDOF_PROJECTILE)) {
        mask |= COLLIDE_PROJECTILES;
    }

    if (collide_obj->Get_Template()->Get_Fence_Width() > 0.0f) {
        mask |= COLLIDE_WALLS;
    }

    if (collide_obj->Is_KindOf(KINDOF_SMALL_MISSILE)) {
        mask |= COLLIDE_SMALL_MISSILES;
    }

    if (collide_obj->Is_KindOf(KINDOF_BALLISTIC_MISSILE)) {
        mask |= COLLIDE_BALLISTIC_MISSILES;
    }

    return (mask & Get_Collide_Mask()) != 0;
}

float WeaponTemplate::Estimate_Weapon_Template_Damage(
    const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos, const WeaponBonus &bonus) const
{
    if (source_obj == nullptr || (victim_obj == nullptr && victim_pos == nullptr)) {
        captainslog_dbgassert(false, "bad args to estimate");
        return 0.0f;
    }

    DamageType damage = Get_Damage_Type();
    DeathType death = Get_Death_Type();

    if (victim_obj != nullptr && victim_obj->Is_KindOf(KINDOF_SHRUBBERY)) {
        if (death == DEATH_BURNED) {
            return 1.0f;
        } else {
            return 0.0f;
        }
    }

    if (victim_obj->Is_KindOf(KINDOF_STRUCTURE) && damage == DAMAGE_SNIPER) {
        if (victim_obj->Get_Contain() != nullptr) {
            ContainModuleInterface *contain = victim_obj->Get_Contain();

            if (contain->Get_Contain_Count() == 0) {
                return 0.0f;
            }
        }
    }

    if (damage == DAMAGE_SURRENDER || m_allowAttackGarrisonedBldgs) {
        ContainModuleInterface *contain = victim_obj->Get_Contain();

        if (contain != nullptr) {
            if (contain->Get_Contain_Count() != 0 && contain->Is_Garrisonable()
                && !contain->Is_Immune_To_Clear_Building_Attacks()) {
                return 1.0f;
            }
        }
    }

    if (victim_obj != nullptr) {
        if (damage == DAMAGE_DISARM) {
            if (victim_obj->Is_KindOf(KINDOF_MINE) || victim_obj->Is_KindOf(KINDOF_BOOBY_TRAP)
                || victim_obj->Is_KindOf(KINDOF_DEMOTRAP)) {
                return 1.0f;
            } else {
                return 0.0f;
            }
        }

        if (damage == DAMAGE_DEPLOY && !victim_obj->Is_Airborne_Target()) {
            return 1.0f;
        }
    }

    if (victim_pos == nullptr) {
        victim_pos = victim_obj->Get_Position();
    }

    float primary = Get_Primary_Damage(bonus);

    if (victim_obj == nullptr) {
        return primary;
    }

    DamageInfoInput info;
    info.m_damageType = damage;
    info.m_deathType = death;
    info.m_sourceID = source_obj->Get_ID();
    info.m_amount = primary;
    return victim_obj->Estimate_Damage(info);
}

unsigned int WeaponTemplate::Fire_Weapon_Template(const Object *source_obj,
    WeaponSlotType wslot,
    int specific_barrel_to_use,
    Object *victim_obj,
    const Coord3D *victim_pos,
    const WeaponBonus &bonus,
    bool is_projectile_detonation,
    bool ignore_ranges,
    Weapon *firing_weapon,
    ObjectID *projectile_id,
    bool do_damage) const
{
    Utf8String str;

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_extraLogging) {
        if (victim_obj != nullptr) {
            str.Format("%s", victim_obj->Get_Template()->Get_Name().Str());
        } else if (victim_pos != nullptr) {
            str.Format("%d,%d,%d", (int)victim_pos->x, (int)victim_pos->y, (int)victim_pos->z);
        }

        captainslog_debug("%d - WeaponTemplate::fireWeaponTemplate() begin - %s attacking %s - ",
            (int)g_theGameLogic->Get_Frame(),
            source_obj->Get_Template()->Get_Name().Str(),
            str.Str());
    }
#endif

    captainslog_dbgassert(specific_barrel_to_use >= 0, "specific_barrel_to_use should no longer be -1");

    if (source_obj == nullptr || (victim_obj == nullptr && victim_pos == nullptr)) {
#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData->m_extraLogging) {
            captainslog_debug("FAIL 1 (sourceObj %d == nullptr || (victimObj %d == nullptr && victimPos %d == nullptr)",
                source_obj != nullptr,
                victim_obj != nullptr,
                victim_pos != nullptr);
        }
#endif
        return 0;
    }

    // assert disabled due to stock issue with ScorpionTankGunFXWeapon
    // captainslog_dbgassert(m_primaryDamage > 0.0f, "You can't really shoot a zero damage weapon at an Object.");
    ObjectID source_id = source_obj->Get_ID();
    const Coord3D *source_pos = source_obj->Get_Position();
    ObjectID victim_id;
    Coord3D new_pos;
    float dist_sqr;

    if (victim_obj != nullptr) {
        if (source_obj == victim_obj) {
            captainslog_debug("*** firing weapon at self -- is this really what you want?");
        }

        victim_pos = victim_obj->Get_Position();
        victim_id = victim_obj->Get_ID();
        const AIUpdateInterface *update = victim_obj->Get_AI_Update_Interface();
        Coord3D offset;

        if (update != nullptr && update->Get_Sneaky_Targeting_Offset(&offset)) {
            new_pos = *victim_pos;
            new_pos += offset;
            victim_pos = &new_pos;
            victim_obj = nullptr;
            victim_id = INVALID_OBJECT_ID;
            dist_sqr = g_thePartitionManager->Get_Distance_Squared(source_obj, &new_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
        } else if (victim_obj->Is_KindOf(KINDOF_BRIDGE)) {
            TBridgeAttackInfo info;
            g_theTerrainLogic->Get_Bridge_Attack_Points(victim_obj, &info);
            dist_sqr = g_thePartitionManager->Get_Distance_Squared(
                source_obj, &info.m_attackPoint1, FROM_BOUNDINGSPHERE_2D, nullptr);
            victim_pos = &info.m_attackPoint1;
            float dist2_sqr = g_thePartitionManager->Get_Distance_Squared(
                source_obj, &info.m_attackPoint2, FROM_BOUNDINGSPHERE_2D, nullptr);

            if (dist_sqr > dist2_sqr) {
                dist_sqr = dist2_sqr;
                victim_pos = &info.m_attackPoint2;
            }
        } else {
            dist_sqr = g_thePartitionManager->Get_Distance_Squared(source_obj, victim_obj, FROM_BOUNDINGSPHERE_2D, nullptr);
        }
    } else {
        victim_id = INVALID_OBJECT_ID;
        dist_sqr = g_thePartitionManager->Get_Distance_Squared(source_obj, victim_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
    }

    if (!ignore_ranges && !Is_Leech_Range_Weapon()) {
        float attack_range_sqr = GameMath::Square(Get_Attack_Range(bonus));

        if (dist_sqr > attack_range_sqr) {
#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData->m_extraLogging) {
                captainslog_debug("FAIL 2 (dist_sqr %.2f > attack_range_sqr %.2f)", dist_sqr, attack_range_sqr);
            }
#endif

            return 0;
        }
    }

    if (!ignore_ranges) {
        float min_attack_range_sqr = GameMath::Square(Get_Minimum_Attack_Range());

        if (dist_sqr < min_attack_range_sqr && !is_projectile_detonation) {
            captainslog_dbgassert(min_attack_range_sqr * 0.8f < dist_sqr,
                "*** victim is closer than min attack range (%f vs %f) of this weapon -- why did we attempt to fire?",
                GameMath::Sqrt(dist_sqr),
                GameMath::Sqrt(min_attack_range_sqr));
#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData->m_extraLogging) {
                captainslog_debug("FAIL 3 (dist_sqr %.2f< min_attack_range_sqr %.2f - 0.5f && !is_projectile_detonation %d)",
                    dist_sqr,
                    min_attack_range_sqr,
                    0);
            }
#endif

            return 0;
        }
    }

    Coord3D secondary_pos;

    if (source_obj->Get_Drawable()) {
        if (victim_obj != nullptr) {
            victim_obj->Get_Geometry_Info().Get_Center_Position(*victim_obj->Get_Position(), secondary_pos);
        } else {
            secondary_pos.Set(victim_pos);
        }

        float recoil_amount = Get_Weapon_Recoil();
        float recoil_angle;

        if (recoil_amount == 0.0f) {
            recoil_angle = 0.0f;
        } else {
            recoil_angle = GameMath::Atan2(victim_pos->y - source_pos->y, victim_pos->x - source_pos->x);
        }

        VeterancyLevel veterancy = source_obj->Get_Veterancy_Level();
        FXList *fx;

        if (is_projectile_detonation) {
            fx = Get_Projectile_Detonate_FX(veterancy);
        } else {
            fx = Get_Fire_FX(veterancy);
        }

        if (g_theGameLogic->Get_Frame() < firing_weapon->Get_Suspend_FX_Delay()) {
            fx = nullptr;
        }

        bool done;

        if (source_obj->Is_Locally_Controlled() || !source_obj->Get_Status(OBJECT_STATUS_STEALTHED)
            || source_obj->Get_Status(OBJECT_STATUS_DETECTED) || source_obj->Get_Status(OBJECT_STATUS_DISGUISED)
            || source_obj->Is_KindOf(KINDOF_MINE) || Is_Play_FX_When_Stealthed()) {
            done = source_obj->Get_Drawable()->Handle_Weapon_Fire_FX(wslot,
                specific_barrel_to_use,
                fx,
                Get_Weapon_Speed(),
                recoil_amount,
                recoil_angle,
                &secondary_pos,
                Get_Primary_Damage_Radius(bonus));
        } else {
            done = true;
        }

        if (!done && fx != nullptr) {
            const Coord3D *pos2;

            if (Is_Contact_Weapon()) {
                pos2 = &secondary_pos;
            } else {
                pos2 = source_obj->Get_Drawable()->Get_Position();
            }

            Do_FX_Pos(fx,
                pos2,
                source_obj->Get_Drawable()->Get_Transform_Matrix(),
                Get_Weapon_Speed(),
                &secondary_pos,
                Get_Primary_Damage_Radius(bonus));
        }
    }

    VeterancyLevel veterancy = source_obj->Get_Veterancy_Level();
    ObjectCreationList *ocl;

    if (is_projectile_detonation) {
        ocl = Get_Projectile_Detonation_OCL(veterancy);
    } else {
        ocl = Get_Fire_OCL(veterancy);
    }

    if (ocl != nullptr) {
        ocl->Create(source_obj, nullptr, 0);
    }

    Coord3D pos = *victim_pos;
    float scatter = 0.0f;

    if (m_scatterRadius > 0.0f
        || (m_scatterRadiusVsInfantry > 0.0f && victim_obj != nullptr && victim_obj->Is_KindOf(KINDOF_INFANTRY))) {
        scatter = m_scatterRadius;
        PathfindLayerEnum layer = LAYER_GROUND;

        if (victim_obj != nullptr) {
            if (victim_obj->Is_KindOf(KINDOF_STRUCTURE)) {
                victim_obj->Get_Geometry_Info().Get_Center_Position(*victim_obj->Get_Position(), pos);
            }

            if (m_scatterRadiusVsInfantry > 0.0f && victim_obj->Is_KindOf(KINDOF_INFANTRY)) {
                scatter += m_scatterRadiusVsInfantry;
            }

            layer = victim_obj->Get_Layer();
        }

        scatter = Get_Logic_Random_Value_Real(0.0f, scatter);
        float angle = Get_Logic_Random_Value_Real(0.0f, DEG_TO_RADF(360.0f));
        Coord3D pos3;
        pos3.Zero();
        pos3.x = GameMath::Cos(angle) * scatter;
        pos3.y = GameMath::Sin(angle) * scatter;
        pos.x += pos3.x;
        pos.y += pos3.y;
        pos.z = g_theTerrainLogic->Get_Layer_Height(pos.x, pos.y, layer, nullptr, true);
    }

    if (Get_Projectile_Template() != nullptr && !is_projectile_detonation) {
        Player *player = source_obj->Get_Controlling_Player();

        Object *projectile_obj =
            g_theThingFactory->New_Object(Get_Projectile_Template(), player->Get_Default_Team(), OBJECT_STATUS_MASK_NONE);
        projectile_obj->Set_Producer(source_obj);

        if (player->Get_Total_Battle_Plan_Count() > 0) {
            player->Apply_Battle_Plan_Bonuses_For_Object(projectile_obj);
        }

        if (projectile_id != nullptr) {
            *projectile_id = projectile_obj->Get_ID();
        }

        SpecialPowerCompletionDie *source_die = source_obj->Find_Special_Power_Completion_Die();

        if (source_die != nullptr) {
            source_die->Notify_Script_Engine();
            SpecialPowerCompletionDie *projectile_die = projectile_obj->Find_Special_Power_Completion_Die();

            if (projectile_die != nullptr) {
                projectile_die->Set_Creator(INVALID_OBJECT_ID);
            }
        } else {
            SpecialPowerCompletionDie *projectile_die = projectile_obj->Find_Special_Power_Completion_Die();

            if (projectile_die != nullptr) {
                projectile_die->Set_Creator(source_obj->Get_ID());
            }
        }

        firing_weapon->New_Projectile_Fired(source_obj, projectile_obj, victim_obj, victim_pos);
        ProjectileUpdateInterface *projectile = nullptr;

        for (BehaviorModule **i = projectile_obj->Get_All_Modules(); *i != nullptr; i++) {
            projectile = (*i)->Get_Projectile_Update_Interface();

            if (projectile != nullptr) {
                break;
            }
        }

        if (projectile != nullptr) {
            if (scatter <= 0.0f) {
                projectile->Projectile_Launch_At_Object_Or_Position(victim_obj,
                    &pos,
                    source_obj,
                    wslot,
                    specific_barrel_to_use,
                    this,
                    m_projectileExhaust[source_obj->Get_Veterancy_Level()]);
            } else {
                projectile->Projectile_Launch_At_Object_Or_Position(nullptr,
                    &pos,
                    source_obj,
                    wslot,
                    specific_barrel_to_use,
                    this,
                    m_projectileExhaust[source_obj->Get_Veterancy_Level()]);
            }
        } else {
            projectile_obj->Set_Position(&pos);
        }

        if (projectile_obj->Is_KindOf(KINDOF_SMALL_MISSILE)) {
            if (victim_obj != nullptr) {
                if (victim_obj->Has_Countermeasures()) {
                    const AIUpdateInterface *update = victim_obj->Get_AI_Update_Interface();

                    if (update != nullptr) {
                        if (update->Get_Cur_Locomotor_Set() != LOCOMOTORSET_SUPERSONIC) {
                            victim_obj->Report_Missile_For_Countermeasures(projectile_obj);
                        }
                    }
                }
            }
        }

#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData->m_extraLogging) {
            captainslog_debug("DONE");
        }
#endif

        return 0;
    } else {
        Coord3D firing_vector = *victim_pos - *source_pos;
        float firing_duration = firing_vector.Length() / Get_Weapon_Speed();
        ObjectID target_id = !Is_Damage_Dealt_At_Self_Position() ? victim_id : INVALID_OBJECT_ID;

        if (firing_weapon->Has_Laser()) {
            if (Get_Primary_Damage_Radius(bonus) < scatter && Get_Secondary_Damage_Radius(bonus) < scatter) {
                target_id = INVALID_OBJECT_ID;
                firing_weapon->Create_Laser(source_obj, nullptr, &pos);
            } else {
                if (victim_obj != nullptr) {
                    pos.Set(victim_obj->Get_Position());
                }

                firing_weapon->Create_Laser(source_obj, victim_obj, &pos);
            }

            if (do_damage) {
                Deal_Damage_Internal(source_id, target_id, &pos, bonus, is_projectile_detonation);
            }

            return g_theGameLogic->Get_Frame();
        } else {
            const Coord3D *pos3;
            if (Is_Damage_Dealt_At_Self_Position()) {
                pos3 = source_pos;
            } else {
                pos3 = victim_pos;
            }

            if (firing_duration > 1.0f) {
                unsigned int frame = 0;

                if (g_theWeaponStore && do_damage) {
                    frame = GameMath::Fast_To_Int_Ceil(firing_duration) + g_theGameLogic->Get_Frame();
                    g_theWeaponStore->Set_Delayed_Damage(this, pos3, frame, source_id, target_id, bonus);
                }

#ifdef GAME_DEBUG_STRUCTS
                if (g_theWriteableGlobalData->m_extraLogging) {
                    captainslog_debug("EARLY 5 (delaying damage applied until frame %d)", (int)frame);
                }
#endif

                return frame;
            } else {
                if (do_damage) {
                    Deal_Damage_Internal(source_id, target_id, pos3, bonus, is_projectile_detonation);
                }

#ifdef GAME_DEBUG_STRUCTS
                if (g_theWriteableGlobalData->m_extraLogging) {
                    captainslog_debug("EARLY 4 (delayed damage applied now)");
                }
#endif

                return g_theGameLogic->Get_Frame();
            }
        }
    }
}

bool Is_2D_Dist_Squared_Less_Than(const Coord3D *pos, const Coord3D *pos2, float dist_sqr)
{
    return GameMath::Square(pos->x - pos2->x) + GameMath::Square(pos->y - pos2->y) <= dist_sqr;
}

void WeaponTemplate::Deal_Damage_Internal(ObjectID source_id,
    ObjectID victim_id,
    const Coord3D *pos,
    const WeaponBonus &bonus,
    bool is_projectile_detonation) const
{
    if (source_id != INVALID_OBJECT_ID && (victim_id != INVALID_OBJECT_ID || pos != nullptr)) {
        Object *source = g_theGameLogic->Find_Object_By_ID(source_id);
        Trim_Old_Historic_Damage();

        if (m_historicBonusCount > 0 && m_historicBonusWeapon != this) {
            float radius_sqr = m_historicBonusRadius * m_historicBonusRadius;
            int count = 0;
            unsigned int frame = g_theGameLogic->Get_Frame();
            unsigned int frame2 = frame - m_historicBonusTime;

            for (auto it = m_historicDamage.begin(); it != m_historicDamage.end(); it++) {
                if (it->frame >= frame2) {
                    if (Is_2D_Dist_Squared_Less_Than(pos, &it->location, radius_sqr)) {
                        count++;
                    }
                }
            }

            if (count < m_historicBonusCount - 1) {
                HistoricWeaponDamageInfo info(frame, *pos);
                m_historicDamage.push_back(info);
            } else {
                g_theWeaponStore->Create_And_Fire_Temp_Weapon(m_historicBonusWeapon, source, pos);
                m_historicDamage.clear();
            }
        }

        Object *victim;

        if (victim_id != INVALID_OBJECT_ID) {
            victim = g_theGameLogic->Find_Object_By_ID(victim_id);
        } else {
            victim = nullptr;
        }

        if (victim != nullptr) {
            pos = victim->Get_Position();
        }

        DamageType damage = Get_Damage_Type();
        DeathType death = Get_Death_Type();
        ObjectStatusTypes status = Get_Damage_Status_Type();

        if (Get_Projectile_Template() == nullptr || is_projectile_detonation) {
            float primary_radius = Get_Primary_Damage_Radius(bonus);
            float secondary_radius = Get_Secondary_Damage_Radius(bonus);
            float primary_damage = Get_Primary_Damage(bonus);
            float secondary_damage = Get_Secondary_Damage(bonus);
            int affects_mask = Get_Affects_Mask();

            captainslog_dbgassert(secondary_radius >= primary_radius || secondary_radius == 0.0f,
                "secondary radius should be >= primary radius (or zero)");

            float primary_radius_sqr = GameMath::Square(primary_radius);
            float max_radius = std::max(primary_radius, secondary_radius);
            SimpleObjectIterator *iter;
            float numeric;
            Object *target;

            if (max_radius <= 0.0f) {
                iter = nullptr;
                numeric = 0.0f;
                target = victim;
                if ((affects_mask & AFFECTS_SUICIDE) != 0) {
                    DamageInfo info;
                    info.m_in.m_damageType = damage;
                    info.m_in.m_deathType = death;
                    info.m_in.m_sourceID = source_id;
                    info.m_in.m_playerMask = 0;
                    info.m_in.m_damageStatusType = status;
                    info.m_in.m_amount = 999999.0f;
                    source->Attempt_Damage(&info);
                }
            } else {
                iter = g_thePartitionManager->Iterate_Objects_In_Range(
                    pos, max_radius, FROM_BOUNDINGSPHERE_3D, nullptr, ITER_FASTEST);
                target = iter->First_With_Numeric(&numeric);
            }

            MemoryPoolObjectHolder holder(iter);

            for (;;) {
                if (target == nullptr) {
                    return;
                }

                bool suicide = false;
                DamageInfo info;
                float angle;
                Coord3D source_to_target_vec;

                if (source != nullptr && target != victim) {
                    if ((affects_mask & AFFECTS_SUICIDE) != 0 && source == target) {
                        suicide = true;
                    } else {
                        if ((affects_mask & AFFECTS_SELF) == 0) {
                            if (source == target) {
                                goto next;
                            }

                            if (source->Get_Producer_ID() == target->Get_ID()) {
                                goto next;
                            }
                        }

                        if ((affects_mask & AFFECTS_NOT_SIMILAR) != 0) {
                            if (source->Get_Template()->Is_Equivalent_To(target->Get_Template())) {
                                if (source->Get_Relationship(target) == ALLIES) {
                                    goto next;
                                }
                            }
                        }

                        if ((affects_mask & AFFECTS_NOT_AIRBORNE) != 0 && target->Is_Significantly_Above_Terrain()) {
                            goto next;
                        }

                        Relationship relationship = source->Get_Relationship(target);
                        WeaponAffectsType affects;

                        if (relationship == ALLIES) {
                            affects = AFFECTS_ALLIES;
                        } else if (relationship == ENEMIES) {
                            affects = AFFECTS_ENEMIES;
                        } else {
                            affects = AFFECTS_NEUTRALS;
                        }

                        if ((affects & affects_mask) == 0) {
                            goto next;
                        }
                    }
                }

                info.m_in.m_damageType = damage;
                info.m_in.m_deathType = death;
                info.m_in.m_sourceID = source_id;
                info.m_in.m_playerMask = 0;
                info.m_in.m_damageStatusType = status;
                source_to_target_vec.Zero();

                if (target != nullptr && source != nullptr) {
                    source_to_target_vec.Set(target->Get_Position());
                    source_to_target_vec.Sub(source->Get_Position());
                }

                angle = Get_Radius_Damage_Angle();

                if ((angle >= DEG_TO_RADF(180.0f) || target != nullptr) && source != nullptr) {
                    Vector3 source_forward_dir;
                    source->Get_Transform_Matrix()->Get_X_Vector(&source_forward_dir);
                    Vector3 source_to_target_dir(source_to_target_vec.x, source_to_target_vec.y, source_to_target_vec.z);
                    source_forward_dir.Normalize();
                    source_to_target_dir.Normalize();

                    if (GameMath::Cos(angle) <= source_forward_dir * source_to_target_dir) {
                        info.m_in.m_shockWaveAmount = m_shockWaveAmount;

                        if (info.m_in.m_shockWaveAmount > 0.0f) {
                            if (GameMath::Fabs(source_to_target_vec.x) < GAMEMATH_EPSILON
                                && GameMath::Fabs(source_to_target_vec.y) < GAMEMATH_EPSILON
                                && GameMath::Fabs(source_to_target_vec.z) < GAMEMATH_EPSILON) {
                                source_to_target_vec.z = 1.0f;
                            }

                            info.m_in.m_shockWavePos = source_to_target_vec;
                            info.m_in.m_shockWaveRadius = m_shockWaveRadius;
                            info.m_in.m_shockWaveTaperOff = m_shockWaveTaperOff;
                        }

                        if (source != nullptr && source->Get_Controlling_Player() != nullptr) {
                            info.m_in.m_playerMask = source->Get_Controlling_Player()->Get_Player_Mask();
                        }

                        if (numeric > primary_radius_sqr) {
                            info.m_in.m_amount = secondary_damage;
                        } else {
                            info.m_in.m_amount = primary_damage;
                        }

                        if (suicide) {
                            info.m_in.m_amount = 999999.0f;
                        }

                        if (source != nullptr && source->Is_KindOf(KINDOF_PROJECTILE)) {
                            ProjectileUpdateInterface *projectile = nullptr;

                            for (BehaviorModule **i = target->Get_All_Modules(); *i != nullptr; i++) {
                                projectile = (*i)->Get_Projectile_Update_Interface();

                                if (projectile != nullptr) {
                                    info.m_in.m_sourceID = projectile->Projectile_Get_Launcher_ID();
                                    break;
                                }
                            }
                        }

                        target->Attempt_Damage(&info);
                    }
                }

            next:
                if (iter != nullptr) {
                    target = iter->Next_With_Numeric(&numeric);
                } else {
                    target = nullptr;
                }
            }
        }

        captainslog_dbgassert(false, "projectile weapons should never get Deal_Damage_Internal called directly");
    }
}
