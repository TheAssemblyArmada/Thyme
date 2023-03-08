/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "object.h"
#include "ai.h"
#include "aipathfind.h"
#include "behaviormodule.h"
#include "bodymodule.h"
#include "controlbar.h"
#include "drawable.h"
#include "experiencetracker.h"
#include "firingtracker.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "ingameui.h"
#include "modulefactory.h"
#include "objectdefectionhelper.h"
#include "objectrepulsorhelper.h"
#include "objectsmchelper.h"
#include "objectweaponstatushelper.h"
#include "opencontain.h"
#include "partitionmanager.h"
#include "playerlist.h"
#include "polygontrigger.h"
#include "radar.h"
#include "scriptengine.h"
#include "specialpower.h"
#include "statusdamagehelper.h"
#include "subdualdamagehelper.h"
#include "team.h"
#include "tempweaponbonushelper.h"
#include "terrainlogic.h"
#include "updatemodule.h"
#include "w3ddebugicons.h"
#include "weaponset.h"

template<>
const char *BitFlags<SPECIAL_POWER_COUNT>::s_bitNamesList[] = { "SPECIAL_INVALID",
    "SPECIAL_DAISY_CUTTER",
    "SPECIAL_PARADROP_AMERICA",
    "SPECIAL_CARPET_BOMB",
    "SPECIAL_CLUSTER_MINES",
    "SPECIAL_EMP_PULSE",
    "SPECIAL_NAPALM_STRIKE",
    "SPECIAL_CASH_HACK",
    "SPECIAL_NEUTRON_MISSILE",
    "SPECIAL_SPY_SATELLITE",
    "SPECIAL_DEFECTOR",
    "SPECIAL_TERROR_CELL",
    "SPECIAL_AMBUSH",
    "SPECIAL_BLACK_MARKET_NUKE",
    "SPECIAL_ANTHRAX_BOMB",
    "SPECIAL_SCUD_STORM",
    "SPECIAL_DEMORALIZE_OBSOLETE",
    "SPECIAL_CRATE_DROP",
    "SPECIAL_A10_THUNDERBOLT_STRIKE",
    "SPECIAL_DETONATE_DIRTY_NUKE",
    "SPECIAL_ARTILLERY_BARRAGE",
    "SPECIAL_MISSILE_DEFENDER_LASER_GUIDED_MISSILES",
    "SPECIAL_REMOTE_CHARGES",
    "SPECIAL_TIMED_CHARGES",
    "SPECIAL_HELIX_NAPALM_BOMB",
    "SPECIAL_HACKER_DISABLE_BUILDING",
    "SPECIAL_TANKHUNTER_TNT_ATTACK",
    "SPECIAL_BLACKLOTUS_CAPTURE_BUILDING",
    "SPECIAL_BLACKLOTUS_DISABLE_VEHICLE_HACK",
    "SPECIAL_BLACKLOTUS_STEAL_CASH_HACK",
    "SPECIAL_INFANTRY_CAPTURE_BUILDING",
    "SPECIAL_RADAR_VAN_SCAN",
    "SPECIAL_SPY_DRONE",
    "SPECIAL_DISGUISE_AS_VEHICLE",
    "SPECIAL_BOOBY_TRAP",
    "SPECIAL_REPAIR_VEHICLES",
    "SPECIAL_PARTICLE_UPLINK_CANNON",
    "SPECIAL_CASH_BOUNTY",
    "SPECIAL_CHANGE_BATTLE_PLANS",
    "SPECIAL_CIA_INTELLIGENCE",
    "SPECIAL_CLEANUP_AREA",
    "SPECIAL_LAUNCH_BAIKONUR_ROCKET",
    "SPECIAL_SPECTRE_GUNSHIP",
    "SPECIAL_GPS_SCRAMBLER",
    "SPECIAL_FRENZY",
    "SPECIAL_SNEAK_ATTACK",
    "SPECIAL_CHINA_CARPET_BOMB",
    "EARLY_SPECIAL_CHINA_CARPET_BOMB",
    "SPECIAL_LEAFLET_DROP",
    "EARLY_SPECIAL_LEAFLET_DROP",
    "EARLY_SPECIAL_FRENZY",
    "SPECIAL_COMMUNICATIONS_DOWNLOAD",
    "EARLY_SPECIAL_REPAIR_VEHICLES",
    "SPECIAL_TANK_PARADROP",
    "SUPW_SPECIAL_PARTICLE_UPLINK_CANNON",
    "AIRF_SPECIAL_DAISY_CUTTER",
    "NUKE_SPECIAL_CLUSTER_MINES",
    "NUKE_SPECIAL_NEUTRON_MISSILE",
    "AIRF_SPECIAL_A10_THUNDERBOLT_STRIKE",
    "AIRF_SPECIAL_SPECTRE_GUNSHIP",
    "INFA_SPECIAL_PARADROP_AMERICA",
    "SLTH_SPECIAL_GPS_SCRAMBLER",
    "AIRF_SPECIAL_CARPET_BOMB",
    "SUPR_SPECIAL_CRUISE_MISSILE",
    "LAZR_SPECIAL_PARTICLE_UPLINK_CANNON",
    "SUPW_SPECIAL_NEUTRON_MISSILE",
    "SPECIAL_BATTLESHIP_BOMBARDMENT",
    nullptr };

BitFlags<MODELCONDITION_COUNT> s_allWeaponFireFlags[3] = { BitFlags<MODELCONDITION_COUNT>(
                                                               BitFlags<MODELCONDITION_COUNT>::kInit,
                                                               MODELCONDITION_FIRING_A,
                                                               MODELCONDITION_BETWEEN_FIRING_SHOTS_A,
                                                               MODELCONDITION_RELOADING_A,
                                                               MODELCONDITION_PREATTACK_A,
                                                               MODELCONDITION_USING_WEAPON_A),
    BitFlags<MODELCONDITION_COUNT>(BitFlags<MODELCONDITION_COUNT>::kInit,
        MODELCONDITION_FIRING_B,
        MODELCONDITION_BETWEEN_FIRING_SHOTS_B,
        MODELCONDITION_RELOADING_B,
        MODELCONDITION_PREATTACK_B,
        MODELCONDITION_USING_WEAPON_B),
    BitFlags<MODELCONDITION_COUNT>(BitFlags<MODELCONDITION_COUNT>::kInit,
        MODELCONDITION_FIRING_C,
        MODELCONDITION_BETWEEN_FIRING_SHOTS_C,
        MODELCONDITION_RELOADING_C,
        MODELCONDITION_PREATTACK_C,
        MODELCONDITION_USING_WEAPON_C) };

ModelConditionFlagType s_theWeaponSetTypeToModelConditionTypeMap[] = { MODELCONDITION_WEAPONSET_VETERAN,
    MODELCONDITION_WEAPONSET_ELITE,
    MODELCONDITION_WEAPONSET_HERO,
    MODELCONDITION_WEAPONSET_PLAYER_UPGRADE,
    MODELCONDITION_WEAPONSET_CRATEUPGRADE_ONE,
    MODELCONDITION_WEAPONSET_CRATEUPGRADE_TWO,
    MODELCONDITION_INVALID,
    MODELCONDITION_INVALID,
    MODELCONDITION_INVALID,
    MODELCONDITION_RIDER1,
    MODELCONDITION_RIDER2,
    MODELCONDITION_RIDER3,
    MODELCONDITION_RIDER4,
    MODELCONDITION_RIDER5,
    MODELCONDITION_RIDER6,
    MODELCONDITION_RIDER7,
    MODELCONDITION_RIDER8 };

Object::Object(const ThingTemplate *tt, BitFlags<OBJECT_STATUS_COUNT> status_bits, Team *team) :
    Thing(tt),
    m_drawable(nullptr),
    m_next(nullptr),
    m_prev(nullptr),
    m_geometryInfo(tt->Get_Template_Geometry_Info()),
    m_friendlyLookSighting(nullptr),
    m_allLookSighting(nullptr),
    m_spiedOnPlayers(0),
    m_shroudSighting(nullptr),
    m_threatSighting(nullptr),
    m_valueSighting(nullptr),
    m_specialModelConditionSleepFrame(0),
    m_objectRepulsorHelper(nullptr),
    m_objectSMCHelper(nullptr),
    m_objectWeaponStatusHelper(nullptr),
    m_objectDefectionHelper(nullptr),
    m_statusDamageHelper(nullptr),
    m_subdualDamageHelper(nullptr),
    m_tempWeaponBonusHelper(nullptr),
    m_firingTracker(nullptr),
    m_allModules(nullptr),
    m_contain(nullptr),
    m_body(nullptr),
    m_stealth(nullptr),
    m_ai(nullptr),
    m_physics(nullptr),
    m_partitionData(nullptr),
    m_radarData(nullptr),
    m_experienceTracker(nullptr),
    m_containedBy(nullptr),
    m_containedByID(OBJECT_UNK),
    m_containedByFrame(0),
    m_team(nullptr),
    m_customIndicatorColor(0),
    m_enteredOrExited(0),
    m_formationID(FORMATION_UNK),
    m_scriptStatus(0),
    m_privateStatus(0),
    m_numTriggerAreasActive(0),
    m_singleUseCommand(false),
    m_receivingDifficultyBonus(false)
{
#ifdef GAME_DEBUG_STRUCTS
    m_hasDiedAlready = false;
#endif
    m_applyBattlePlanBonuses = false;
    const ThingTemplate *tmplate = static_cast<const ThingTemplate *>(tt->Get_Final_Override());
    Utf8String module_name;
    m_formationOffset.y = 0.0f;
    m_formationOffset.x = 0.0f;
    m_iPos.Zero();

    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        m_spiedOnByPlayer[i] = 0;
    }

    for (int i = 0; i < DISABLED_TYPE_COUNT; i++) {
        m_disabledStateFrames[i] = 0;
    }

    m_weaponBonusCondition = 0;
    m_curWeaponSetFlags.Clear();

    if (g_theGameLogic != nullptr && tmplate != nullptr) {
        m_friendlyLookSighting = new SightingInfo();
        m_friendlyLookSighting->Reset();
        m_allLookSighting = new SightingInfo();
        m_allLookSighting->Reset();
        m_shroudSighting = new SightingInfo();
        m_shroudSighting->Reset();
        m_threatSighting = new SightingInfo();
        m_threatSighting->Reset();
        m_valueSighting = new SightingInfo();
        m_valueSighting->Reset();
        m_id = OBJECT_UNK;
        m_producerID = OBJECT_UNK;
        m_builderID = OBJECT_UNK;
        m_status = status_bits;
        m_layer = LAYER_GROUND;
        m_aiGroup = nullptr;
        m_constructionPercent = -1.0f;
        m_visionRange = tmplate->Get_Vision_Range();
        m_shroudClearingRange = tmplate->Get_Shroud_Clearing_Range();

        if (m_shroudClearingRange == -1.0f) {
            m_shroudClearingRange = m_visionRange;
        }

        m_shroudRange = 0.0f;
        m_singleUseCommand = false;
        Set_ID(g_theGameLogic->Allocate_Object_ID());
        m_allModules = new BehaviorModule *[tmplate->Get_Body_Modules()->Get_Count() + 8];
        BehaviorModule **modules = m_allModules;
        const ModuleInfo *module_info = tmplate->Get_Body_Modules();

        if (team != nullptr) {
            Set_Team(team);
        } else {
            Set_Team(g_thePlayerList->Get_Neutral_Player()->Get_Default_Team());
        }

        static NameKeyType smcHelperModuleDataTagNameKey = g_theNameKeyGenerator->Name_To_Key("ModuleTag_SMCHelper");
        static ObjectSMCHelperModuleData smcModuleData;
        smcModuleData.Set_Tag_Key(smcHelperModuleDataTagNameKey);
        m_objectSMCHelper = new ObjectSMCHelper(this, &smcModuleData);
        *modules++ = m_objectSMCHelper;
        bool is_inactive = false;

        for (unsigned int i = 0; i < module_info->Get_Count(); i++) {
            module_name = module_info->Get_Nth_Name(i);
            if (!module_name.Is_Empty() && module_name.Compare("InactiveBody") == 0) {
                is_inactive = true;
                break;
            }
        }

        if (!is_inactive) {
            static NameKeyType statusHelperModuleDataTagNameKey =
                g_theNameKeyGenerator->Name_To_Key("ModuleTag_StatusDamageHelper");
            static StatusDamageHelperModuleData statusModuleData;
            statusModuleData.Set_Tag_Key(statusHelperModuleDataTagNameKey);
            m_statusDamageHelper = new StatusDamageHelper(this, &statusModuleData);
            *modules++ = m_statusDamageHelper;

            static NameKeyType subdualHelperModuleDataTagNameKey =
                g_theNameKeyGenerator->Name_To_Key("ModuleTag_SubdualDamageHelper");
            static SubdualDamageHelperModuleData subdualModuleData;
            subdualModuleData.Set_Tag_Key(subdualHelperModuleDataTagNameKey);
            m_subdualDamageHelper = new SubdualDamageHelper(this, &subdualModuleData);
            *modules++ = m_subdualDamageHelper;
        }

        if (g_theAI != nullptr && g_theAI->Get_AI_Data()->m_enableRepulsors && Is_KindOf(KINDOF_CAN_BE_REPULSED)) {
            static NameKeyType repulsorHelperModuleDataTagNameKey =
                g_theNameKeyGenerator->Name_To_Key("ModuleTag_RepulsorHelper");
            static ObjectRepulsorHelperModuleData repulsorModuleData;
            repulsorModuleData.Set_Tag_Key(repulsorHelperModuleDataTagNameKey);
            m_objectRepulsorHelper = new ObjectRepulsorHelper(this, &repulsorModuleData);
            *modules++ = m_objectRepulsorHelper;
        }

        if (!Is_KindOf(KINDOF_SHRUBBERY)) {
            static NameKeyType defectionModuleDataTagNameKey =
                g_theNameKeyGenerator->Name_To_Key("ModuleTag_DefectionHelper");
            static ObjectDefectionHelperModuleData defectionModuleData;
            defectionModuleData.Set_Tag_Key(defectionModuleDataTagNameKey);
            m_objectDefectionHelper = new ObjectDefectionHelper(this, &defectionModuleData);
            *modules++ = m_objectDefectionHelper;
        }

        if (tmplate->Can_Possibly_Have_Any_Weapon()) {
            static NameKeyType weaponStatusModuleDataTagNameKey =
                g_theNameKeyGenerator->Name_To_Key("ModuleTag_WeaponStatusHelper");
            static ObjectWeaponStatusHelperModuleData weaponStatusModuleData;
            weaponStatusModuleData.Set_Tag_Key(weaponStatusModuleDataTagNameKey);
            m_objectWeaponStatusHelper = new ObjectWeaponStatusHelper(this, &weaponStatusModuleData);
            *modules++ = m_objectWeaponStatusHelper;

            static NameKeyType firingTrackerModuleDataTagNameKey =
                g_theNameKeyGenerator->Name_To_Key("ModuleTag_FiringTrackerHelper");
            static FiringTrackerModuleData firingTrackerModuleData;
            firingTrackerModuleData.Set_Tag_Key(firingTrackerModuleDataTagNameKey);
            m_firingTracker = new FiringTracker(this, &firingTrackerModuleData);
            *modules++ = m_firingTracker;

            static NameKeyType tempWeaponBonusHelperModuleDataTagNameKey =
                g_theNameKeyGenerator->Name_To_Key("ModuleTag_TempWeaponBonusHelper");
            static TempWeaponBonusHelperModuleData tempWeaponBonusModuleData;
            tempWeaponBonusModuleData.Set_Tag_Key(tempWeaponBonusHelperModuleDataTagNameKey);
            m_tempWeaponBonusHelper = new TempWeaponBonusHelper(this, &tempWeaponBonusModuleData);
            *modules++ = m_tempWeaponBonusHelper;
        }

        for (unsigned int i = 0; i < module_info->Get_Count(); i++) {
            module_name = module_info->Get_Nth_Name(i);

            if (!module_name.Is_Empty()) {
                BehaviorModule *module = static_cast<BehaviorModule *>(
                    g_theModuleFactory->New_Module(this, module_name, module_info->Get_Nth_Data(i), MODULE_DEFAULT));
                *modules++ = module;
                BodyModuleInterface *body = module->Get_Body();

                if (body != nullptr) {
                    captainslog_dbgassert(m_body == nullptr, "Duplicate bodies");
                    m_body = body;
                }

                ContainModuleInterface *contain = module->Get_Contain();

                if (contain != nullptr) {
                    captainslog_dbgassert(m_contain == nullptr, "Duplicate containers");
                    m_contain = contain;
                }

                StealthUpdate *stealth = module->Get_Steath();

                if (stealth != nullptr) {
#if 0
                    // disabled because this is triggered by the stock ini files.
                    captainslog_dbgassert(m_stealth == nullptr, "DuplicateStealthUpdates!");
#endif
                    m_stealth = stealth;
                }

                AIUpdateInterface *update = module->Get_AI_Update_Interface();

                if (update != nullptr) {
                    captainslog_dbgassert(m_ai == nullptr,
                        "%s has more than one AI module. This is illegal!",
                        Get_Template()->Get_Name().Str());
                    m_ai = update;
                }

                static NameKeyType key_PhysicsUpdate = g_theNameKeyGenerator->Name_To_Key("PhysicsBehavior");

                if (module->Get_Module_Name_Key() == key_PhysicsUpdate) {
                    captainslog_dbgassert(m_physics == nullptr,
                        "You should never have more than one Physics module (%s)",
                        Get_Template()->Get_Name().Str());
                    m_physics = static_cast<PhysicsBehavior *>(module);
                }
            }
        }

        *modules = nullptr;
        AIUpdateInterface *update = Get_AI_Update_Interface();

        if (update != nullptr) {
            update->Set_Attitude(Get_Team()->Get_Prototype()->Get_Template_Info()->m_initialTeamAttitude);

            if (m_team != nullptr) {
                if (m_team->Get_Prototype() != nullptr) {
                    if (m_team->Get_Prototype()->Get_Attack_Priority_Name().Is_Not_Empty()) {
                        const AttackPriorityInfo *attack_info =
                            g_theScriptEngine->Get_Attack_Info(m_team->Get_Prototype()->Get_Attack_Priority_Name());

                        if (attack_info != nullptr) {
                            if (attack_info->Get_Name().Is_Not_Empty()) {
                                update->Set_Attack_Priority_Info(attack_info);
                            }
                        }
                    }
                }
            }
        }

        m_experienceTracker = new ExperienceTracker(this);
        m_experienceTracker->Set_Veterency_Level(
            Get_Controlling_Player()->Get_Production_Veterancy_Level(Get_Template()->Get_Name()), true);

        for (BehaviorModule **m = m_allModules; *m != nullptr; m++) {
            (*m)->On_Object_Created();
        }

        m_numTriggerAreasActive = 0;
        m_enteredOrExited = 0;
        m_isSelectable = tmplate->Is_KindOf(KINDOF_SELECTABLE);
        m_healthBoxOffset.Zero();
        m_applyBattlePlanBonuses = true;
        g_theRadar->Add_Object(this);
        g_theGameLogic->Register_Object(this);
        m_occlusionDelayFrame = tmplate->Get_Occlusion_Delay() + g_theGameLogic->Get_Frame();
        m_soleHealingBenefactor = OBJECT_UNK;
        m_soleHealingEndFrame = 0;
    }
}

Object::~Object()
{
    g_theAI->Get_Pathfinder()->Remove_Object_From_Pathfind_Map(this);

    if (!Is_KindOf(KINDOF_PROJECTILE) && !Is_KindOf(KINDOF_INERT)) {
        g_theGameLogic->Save_Frame();
        g_theScriptEngine->Notify_Of_Object_Creation_Or_Destruction();
    }

    if (m_radarData != nullptr) {
        g_theRadar->Remove_Object(this);
    }

    g_theGameLogic->Send_Object_Destroyed(this);
    Set_Team(nullptr);
    m_friendlyLookSighting->Delete_Instance();
    m_friendlyLookSighting = nullptr;
    m_allLookSighting->Delete_Instance();
    m_allLookSighting = nullptr;
    m_shroudSighting->Delete_Instance();
    m_shroudSighting = nullptr;
    m_threatSighting->Delete_Instance();
    m_threatSighting = nullptr;
    m_valueSighting->Delete_Instance();
    m_valueSighting = nullptr;

    if (m_partitionData != nullptr) {
        g_thePartitionManager->Unregister_Object(this);
    }

    if (m_aiGroup != nullptr) {
        m_aiGroup->Remove(this);
    }

    m_ai = nullptr;
    m_physics = nullptr;

    for (BehaviorModule **m = m_allModules; *m != nullptr; m++) {
        (*m)->Delete_Instance();
        *m = nullptr;
    }

    delete[] m_allModules;
    m_allModules = nullptr;

    if (m_experienceTracker != nullptr) {
        m_experienceTracker->Delete_Instance();
    }

    m_experienceTracker = nullptr;
    m_firingTracker = nullptr;
    m_objectRepulsorHelper = nullptr;
    m_statusDamageHelper = nullptr;
    m_tempWeaponBonusHelper = nullptr;
    m_subdualDamageHelper = nullptr;
    m_objectSMCHelper = nullptr;
    m_objectWeaponStatusHelper = nullptr;
    m_objectDefectionHelper = nullptr;
    m_id = OBJECT_UNK;
    g_theScriptEngine->Notify_Of_Object_Destruction(this);
}

float Object::Calculate_Height_Above_Terrain() const
{
    const Coord3D *pos = Get_Position();
    return pos->z - g_theTerrainLogic->Get_Layer_Height(pos->x, pos->y, m_layer, nullptr, true);
}

bool Pos_Changed(const Coord3D *c1, const Coord3D *c2)
{
    return GameMath::Fabs(c1->x - c2->x) > 0.01f || GameMath::Fabs(c1->y - c2->y) > 0.01f
        || GameMath::Fabs(c1->z - c2->z) > 0.01f;
}

bool Angle_Changed(float f1, float f2)
{
    return GameMath::Fabs(f1 - f2) > 0.01f;
}

void Object::React_To_Transform_Change(const Matrix3D *tm, const Coord3D *pos, float angle)
{
    if (gm_isnanf(Get_Position()->x) || gm_isnanf(Get_Position()->y) || gm_isnanf(Get_Position()->z)) {
        captainslog_dbgassert(false, "Object pos is nan.");
        g_theGameLogic->Destroy_Object(this);
    }

    if (m_drawable != nullptr) {
        m_drawable->Set_Transform_Matrix(Get_Transform_Matrix());
    }

    bool pos_changed = Pos_Changed(pos, Get_Position());
    bool angle_changed = Angle_Changed(angle, Get_Orientation());

    if (pos_changed || angle_changed) {
        if (m_partitionData != nullptr) {
            m_partitionData->Make_Dirty(true);
        }

        if (Get_Contain()) {
            Get_Contain()->Contain_React_To_Transform_Change();
        }
    }

    if (pos_changed) {
        Set_Trigger_Area_Flags_For_Change_In_Position();
        Region3D extent;
        g_theTerrainLogic->Get_Extent(&extent);

        if (extent.Is_In_Region_No_Z(Get_Position())) {
            m_privateStatus &= ~STATUS_OUTSIDE_MAP;
        } else {
            m_privateStatus |= STATUS_OUTSIDE_MAP;
        }
    }
}

void Object::React_To_Turret(WhichTurretType turret, float angle, float pitch)
{
    float turret_angle = 0.0f;
    float turret_pitch = 0.0f;

    if (Get_AI_Update_Interface() != nullptr) {
        Get_AI_Update_Interface()->Get_Turret_Rot_And_Pitch(turret, &turret_angle, &turret_pitch);
    }

    if (turret_angle != angle) {
        if (Get_Contain() != nullptr) {
            Get_Contain()->Contain_React_To_Transform_Change();
        }
    }
}

void Object::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    xfer->xferUnsignedByte(&m_privateStatus);
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    Matrix3D *tm = const_cast<Matrix3D *>(Get_Transform_Matrix());
    xfer->xferUser(tm, sizeof(tm));
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    xfer->xferUser(&m_id, sizeof(m_id));
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    xfer->xferUser(&m_objectUpgradesCompleted, sizeof(m_objectUpgradesCompleted));
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    if (m_experienceTracker != nullptr) {
        xfer->xferSnapshot(m_experienceTracker);
    }
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    float health = Get_Body_Module()->Get_Health();
    xfer->xferUser(&health, sizeof(health));
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    xfer->xferUnsignedInt(&m_weaponBonusCondition);
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif
    float scalar = Get_Body_Module()->Get_Damage_Scalar();
    xfer->xferUser(&scalar, sizeof(scalar));
#ifdef GAME_DEBUG_STRUCTS
    // TODO LogObjectCRCs
#endif

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        const Weapon *weapon = Get_Weapon_In_Weapon_Slot(static_cast<WeaponSlotType>(i));

        if (weapon != nullptr) {
            xfer->xferSnapshot(const_cast<Weapon *>(weapon));
        }
    }
}

void Object::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 9;
    xfer->xferVersion(&version, 9);
    ObjectID id = Get_ID();
    xfer->xferObjectID(&id);
    Set_ID(id);
    captainslog_debug("Xfer Object %s id=%d", Get_Template()->Get_Name().Str(), id);

    if (version < 7) {
        Coord3D pos = *Get_Position();
        xfer->xferCoord3D(&pos);
        Set_Position(&pos);
        float angle = Get_Orientation();
        xfer->xferReal(&angle);
        Set_Orientation(angle);
    } else {
        Matrix3D tm(*Get_Transform_Matrix());
        xfer->xferMatrix3D(&tm);
        Set_Transform_Matrix(&tm);
    }

    unsigned int teamid;
    if (m_team != nullptr) {
        teamid = m_team->Get_Team_ID();
    } else {
        teamid = 0;
    }

    xfer->xferUser(&teamid, sizeof(teamid));
    xfer->xferObjectID(&m_producerID);
    xfer->xferObjectID(&m_builderID);
    DrawableID drawableid;
    Drawable *drawable = Get_Drawable();

    if (drawable != nullptr) {
        drawableid = drawable->Get_ID();
    } else {
        drawableid = DRAWABLE_UNK;
    }

    xfer->xferDrawableID(&drawableid);

    if (xfer->Get_Mode() == XFER_LOAD) {
        drawable->Set_ID(drawableid);
    }

    xfer->xferAsciiString(&m_name);

    if (version < 8) {
        unsigned int status;
        xfer->xferUnsignedInt(&status);

        for (int i = 0; i < 32; i++) {
            if (((1 << i) & status) != 0) {
                BitFlags<OBJECT_STATUS_COUNT> flags(BitFlags<OBJECT_STATUS_COUNT>::kInit, i + 1);
                m_status.Set(flags);
            }
        }
    } else {
        m_status.Xfer(xfer);
    }

    xfer->xferUnsignedByte(&m_scriptStatus);
    xfer->xferUnsignedByte(&m_privateStatus);

    if (xfer->Get_Mode() == XFER_LOAD) {
        Team *team = g_theTeamFactory->Find_Team_By_ID(teamid);
        captainslog_relassert(team != nullptr, 6, "Object::xfer - Unable to load team");
        Set_Or_Restore_Team(team, true);
    }

    xfer->xferSnapshot(&m_geometryInfo);
    xfer->xferSnapshot(m_friendlyLookSighting);

    if (version >= 9) {
        xfer->xferSnapshot(m_allLookSighting);
    }

    xfer->xferSnapshot(m_shroudSighting);
    xfer->xferUser(&m_spiedOnByPlayer, sizeof(m_spiedOnByPlayer));
    xfer->xferUser(&m_spiedOnPlayers, sizeof(m_spiedOnPlayers));
    xfer->xferReal(&m_visionRange);
    xfer->xferReal(&m_shroudClearingRange);
    xfer->xferReal(&m_shroudRange);
    m_disabledStates.Xfer(xfer);

    if (xfer->Get_Mode() == XFER_SAVE || version >= 2) {
        xfer->xferBool(&m_singleUseCommand);
    } else {
        m_singleUseCommand = false;
    }

    xfer->xferUser(&m_disabledStateFrames, sizeof(m_disabledStateFrames));
    xfer->xferUnsignedInt(&m_specialModelConditionSleepFrame);

    if (xfer->Get_Mode() == XFER_LOAD && m_radarData != nullptr) {
        g_theRadar->Remove_Object(this);
    }

    xfer->xferSnapshot(m_experienceTracker);

    if (version >= 6) {
        if (xfer->Get_Mode() == XFER_SAVE) {
            if (m_containedBy != nullptr) {
                m_containedByID = m_containedBy->Get_ID();
            } else {
                m_containedByID = OBJECT_UNK;
            }
        }

        xfer->xferObjectID(&m_containedByID);
    }

    xfer->xferUnsignedInt(&m_containedByFrame);
    xfer->xferReal(&m_constructionPercent);
    xfer->xferUpgradeMask(&m_objectUpgradesCompleted);
    xfer->xferAsciiString(&m_originalTeamName);
    xfer->xferColor(&m_customIndicatorColor);
    xfer->xferCoord3D(&m_healthBoxOffset);
    xfer->xferByte(&m_numTriggerAreasActive);
    xfer->xferUnsignedInt(&m_enteredOrExited);
    xfer->xferICoord3D(&m_iPos);
    captainslog_relassert(
        m_numTriggerAreasActive < 6, 6, "Invalid m_numTriggerAreasActive = %d, max is %d", m_numTriggerAreasActive, 5);

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        Utf8String trigger_name;

        if (m_triggerInfo[i].polygon_trigger != nullptr) {
            trigger_name = m_triggerInfo[i].polygon_trigger->Get_Trigger_Name();
        }

        xfer->xferAsciiString(&trigger_name);

        if (xfer->Get_Mode() == XFER_LOAD) {
            m_triggerInfo[i].polygon_trigger = g_theTerrainLogic->Get_Trigger_Area_By_Name(trigger_name);
        }

        xfer->xferByte(reinterpret_cast<int8_t *>(&m_triggerInfo[i].entered));
        xfer->xferByte(reinterpret_cast<int8_t *>(&m_triggerInfo[i].exited));
        xfer->xferByte(reinterpret_cast<int8_t *>(&m_triggerInfo[i].inside));
    }

    xfer->xferUser(&m_layer, sizeof(m_layer));
    xfer->xferUser(&m_destinationLayer, sizeof(m_destinationLayer));
    xfer->xferBool(&m_isSelectable);
    xfer->xferUnsignedInt(&m_occlusionDelayFrame);
    xfer->xferUser(&m_formationID, sizeof(m_formationID));

    if (m_formationID != FORMATION_UNK) {
        xfer->xferCoord2D(&m_formationOffset);
    }

    unsigned short module_count = 0;

    for (BehaviorModule **m = m_allModules; *m != nullptr; m++) {
        module_count++;
    }

    xfer->xferUnsignedShort(&module_count);
    Utf8String module_name;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (BehaviorModule **m = m_allModules; *m != nullptr; m++) {
            BehaviorModule *module = (*m);
            module_name = g_theNameKeyGenerator->Key_To_Name(module->Get_Tag_Key());
            captainslog_dbgassert(
                module_name != Utf8String::s_emptyString, "Object::xfer - Module tag key does not translate to a string!");
            xfer->xferAsciiString(&module_name);
            xfer->Begin_Block();
            xfer->xferSnapshot(module);
            xfer->End_Block();
        }
    } else {
        for (unsigned short i = 0; i < module_count; i++) {
            xfer->xferAsciiString(&module_name);
            NameKeyType key = g_theNameKeyGenerator->Name_To_Key(module_name.Str());
            BehaviorModule *module = nullptr;

            for (BehaviorModule **m = m_allModules; *m != nullptr; m++) {
                if (key == (*m)->Get_Tag_Key()) {
                    module = *m;
                    break;
                }
            }

            int size = xfer->Begin_Block();

            if (module != nullptr) {
                xfer->xferSnapshot(module);
            } else {
                xfer->Skip(size);
            }

            xfer->End_Block();
        }
    }

    if (version < 3) {
        if (xfer->Get_Mode() == XFER_LOAD) {
            m_soleHealingBenefactor = OBJECT_UNK;
            m_soleHealingEndFrame = 0;
        }
    } else {
        xfer->xferObjectID(&m_soleHealingBenefactor);
        xfer->xferUnsignedInt(&m_soleHealingEndFrame);
    }

    captainslog_dbgassert(xfer->Get_Mode() != XFER_LOAD || m_partitionData != nullptr, "should not be in partitionmgr yet");

    if (version >= 4) {
        m_curWeaponSetFlags.Xfer(xfer);
        xfer->xferUnsignedInt(&m_weaponBonusCondition);
        xfer->xferUser(&m_weaponSetConditions, sizeof(m_weaponSetConditions));
        xfer->xferSnapshot(&m_weaponSet);
        m_specialPowers.Xfer(xfer);
        xfer->xferAsciiString(&m_commandSetStringOverride);
        xfer->xferBool(&m_applyBattlePlanBonuses);
    }

    if (version < 5) {
        m_receivingDifficultyBonus = false;
    } else {
        xfer->xferBool(&m_receivingDifficultyBonus);
    }
}

void Object::Load_Post_Process()
{
    if (m_containedByID != OBJECT_UNK) {
        m_containedBy = g_theGameLogic->Find_Object_By_ID(m_containedByID);
    } else {
        m_containedBy = nullptr;
    }
}

ObjectShroudStatus Object::Get_Shrouded_Status(int index) const
{
#ifdef GAME_DLL
    return Call_Method<ObjectShroudStatus, const Object, int>(PICK_ADDRESS(0x00547D60, 0x007D12FB), this, index);
#else
    return ObjectShroudStatus(0);
#endif
}

// zh: 0x005479B0 wb: 0x007D0E3D

Relationship Object::Get_Relationship(const Object *that) const
{
#ifdef GAME_DLL
    return Call_Method<Relationship, const Object, const Object *>(PICK_ADDRESS(0x005479B0, 0x007D0E3D), this, that);
#else
    return Relationship();
#endif
}

Player *Object::Get_Controlling_Player() const
{
#ifdef GAME_DLL
    return Call_Method<Player *, const Object>(PICK_ADDRESS(0x00547A00, 0x007D0EAB), this);
#endif
    return nullptr;
}

bool Object::Is_In_List(Object **list) const
{
    return m_prev != nullptr || m_next != nullptr || *list == this;
}

// zh: 0x00549BD0 wb: 0x007D370C
void Object::On_Veterancy_Level_Changed(VeterancyLevel v1, VeterancyLevel v2, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, Object, VeterancyLevel, VeterancyLevel, bool>(PICK_ADDRESS(0x00549BD0, 0x007D370C), this, v1, v2, b);
#endif
}

int Object::Get_Night_Indicator_Color() const
{
#ifdef GAME_DLL
    return Call_Method<int, const Object>(PICK_ADDRESS(0x00547AF0, 0x007D0FEC), this);
#else
    return 0;
#endif
}

int Object::Get_Indicator_Color() const
{
#ifdef GAME_DLL
    return Call_Method<int, const Object>(PICK_ADDRESS(0x00547AC0, 0x007D0F89), this);
#else
    return 0;
#endif
}

bool Object::Is_Locally_Controlled() const
{
    return g_thePlayerList->Get_Local_Player() == Get_Controlling_Player();
}

void Object::Get_Health_Box_Position(Coord3D &pos) const
{
    pos = *Get_Position();
    pos.z += Get_Geometry_Info().Get_Max_Height_Above_Position() + 10.0f;
    pos.Add(&m_healthBoxOffset);

    if (Is_KindOf(KINDOF_MOB_NEXUS)) {
        pos.z += 20.0f;
    }
}

bool Object::Get_Health_Box_Dimensions(float &width, float &height) const
{
    if (Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
        width = 0.0f;
        height = 0.0f;
        return false;
    }

    float f1;

    if (Get_Geometry_Info().Get_Minor_Radius() + Get_Geometry_Info().Get_Major_Radius() > 150.0f) {
        f1 = 150.0f;
    } else {
        f1 = Get_Geometry_Info().Get_Minor_Radius() + Get_Geometry_Info().Get_Major_Radius();
    }

    float f2 = std::max(f1, 20.0f);

    width = 3.0f;

    if (f2 + f2 < 20.0f) {
        height = 20.0f;
    } else {
        height = f2 + f2;
    }

    return true;
}

bool Object::Get_Ammo_Pip_Showing_Info(int &clip_size, int &ammo_in_clip) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Object, int &, int &>(
        PICK_ADDRESS(0x00547760, 0x007D0A7F), this, clip_size, ammo_in_clip);
#else
    return false;
#endif
}

BehaviorModule *Object::Find_Module(NameKeyType type) const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        if ((*module)->Get_Module_Name_Key() == type) {
            return *module;
        }
    }

    return nullptr;
}

// #TODO This will cause compile error as soon as DamageModule is properly implemented elsewhere. Delete this then.
class DamageModule : public BehaviorModule
{
};

DamageModule *Object::Find_Damage_Module(NameKeyType type) const
{
    return static_cast<DamageModule *>(Find_Module(type));
}

UpdateModule *Object::Find_Update_Module(NameKeyType type) const
{
    return static_cast<UpdateModule *>(Find_Module(type));
}

VeterancyLevel Object::Get_Veterancy_Level() const
{
    if (m_experienceTracker) {
        return m_experienceTracker->Get_Current_Level();
    } else {
        return VETERANCY_REGULAR;
    }
}

bool Object::Is_Selectable() const
{
    if (Get_Template()->Is_KindOf(KINDOF_ALWAYS_SELECTABLE)) {
        return true;
    }

    return m_isSelectable && !Get_Status(OBJECT_STATUS_UNSELECTABLE) && !Is_Effectively_Dead();
}

bool Object::Is_Mass_Selectable() const
{
    return Is_Selectable() && !Is_KindOf(KINDOF_STRUCTURE);
}

void Object::Init_Object()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x00545D90, 0x007CF205), this);
#endif
}

float Object::Get_Carrier_Deck_Height() const
{
#ifdef GAME_DLL
    return Call_Method<float, const Object>(PICK_ADDRESS(0x0054F3D0, 0x007D96FE), this);
#else
    return 0.0f;
#endif
}

void Object::Set_Status(BitFlags<OBJECT_STATUS_COUNT> bits, bool set)
{
#ifdef GAME_DLL
    Call_Method<void, Object, BitFlags<OBJECT_STATUS_COUNT>, bool>(PICK_ADDRESS(0x00546E20, 0x007D011C), this, bits, set);
#endif
}

void Object::Set_Model_Condition_State(ModelConditionFlagType a)
{
    if (m_drawable != nullptr) {
        m_drawable->Set_Model_Condition_State(a);
    }
}

void Object::Clear_Model_Condition_State(ModelConditionFlagType a)
{
    if (m_drawable != nullptr) {
        m_drawable->Clear_Model_Condition_State(a);
    }
}

void Object::Attempt_Damage(DamageInfo *info)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DamageInfo *>(PICK_ADDRESS(0x00547DA0, 0x007D134E), this, info);
#endif
}

void Object::Clear_Disabled(DisabledType type)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DisabledType>(PICK_ADDRESS(0x005489F0, 0x007D1FE8), this, type);
#endif
}

bool Object::Can_Crush_Or_Squish(Object *obj, CrushSquishTestType type)
{
#ifdef GAME_DLL
    return Call_Method<bool, Object, Object *, CrushSquishTestType>(PICK_ADDRESS(0x005471D0, 0x007D0486), this, obj, type);
#else
    return false;
#endif
}

void Object::Defect(Team *team, unsigned int i)
{
#ifdef GAME_DLL
    Call_Method<void, Object, Team *, unsigned int>(PICK_ADDRESS(0x0054EEC0, 0x007D924B), this, team, i);
#endif
}

unsigned char Object::Get_Crushable_Level() const
{
    return Get_Template()->Get_Crushable_Level();
}

unsigned char Object::Get_Crusher_Level() const
{
    return Get_Template()->Get_Crusher_Level();
}

void Object::Kill(DamageType damage, DeathType death)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DamageType, DeathType>(PICK_ADDRESS(0x00548300, 0x007D1733), this, damage, death);
#endif
}

void Object::On_Collide(Object *other, const Coord3D *loc, const Coord3D *normal)
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        CollideModuleInterface *collide = (*module)->Get_Collide();

        if (collide != nullptr) {
            if (Get_Status_Bits().Test(OBJECT_STATUS_NO_COLLISIONS)) {
                return;
            }

            collide->On_Collide(other, loc, normal);
        }
    }
}

ExitInterface *Object::Get_Object_Exit_Interface() const
{
    ExitInterface *exit = nullptr;

    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        exit = (*module)->Get_Update_Exit_Interface();

        if (exit != nullptr) {
            break;
        }
    }

    if (exit == nullptr) {
        ContainModuleInterface *contain = Get_Contain();

        if (contain != nullptr) {
            return contain->Get_Contain_Exit_Interface();
        }
    }

    return exit;
}

void Object::Set_Disabled(DisabledType type)
{
    Set_Disabled_Until(type, 0x3FFFFFFF);
}

void Object::Set_Disabled_Until(DisabledType type, unsigned int i)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DisabledType, unsigned int>(PICK_ADDRESS(0x005485C0, 0x007D1A3F), this, type, i);
#endif
}

void Object::Set_Captured(bool captured)
{
    if (captured) {
        m_privateStatus |= STATUS_CAPTURED;
    } else {
        captainslog_debug("Clearing Captured Status. This should never happen.");
        m_privateStatus &= ~STATUS_CAPTURED;
    }
}

const Weapon *Object::Get_Current_Weapon(WeaponSlotType *wslot) const
{
    if (!m_weaponSet.Has_Any_Weapon()) {
        return nullptr;
    }

    if (wslot) {
        *wslot = m_weaponSet.Get_Cur_Weapon_Slot();
    }

    return m_weaponSet.Get_Cur_Weapon();
}

Weapon *Object::Get_Current_Weapon(WeaponSlotType *wslot)
{
    if (!m_weaponSet.Has_Any_Weapon()) {
        return nullptr;
    }

    if (wslot) {
        *wslot = m_weaponSet.Get_Cur_Weapon_Slot();
    }

    return m_weaponSet.Get_Cur_Weapon();
}

bool Object::Is_Able_To_Attack() const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Object>(PICK_ADDRESS(0x0054A0C0, 0x007D3B0D), this);
#else
    return false;
#endif
}

bool Object::Choose_Best_Weapon_For_Target(const Object *target, WeaponChoiceCriteria criteria, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, Object, const Object *, WeaponChoiceCriteria, CommandSourceType>(
        PICK_ADDRESS(0x00547810, 0x007D0B27), this, target, criteria, source);
#else
    return false;
#endif
}

CanAttackResult Object::Get_Able_To_Attack_Specific_Object(
    AbleToAttackType type, const Object *obj, CommandSourceType source, WeaponSlotType slot) const
{
#ifdef GAME_DLL
    return Call_Method<CanAttackResult, const Object, AbleToAttackType, const Object *, CommandSourceType, WeaponSlotType>(
        PICK_ADDRESS(0x005477B0, 0x007D0AC5), this, type, obj, source, slot);
#else
    return ATTACK_RESULT_0;
#endif
}

void Object::Set_Team(Team *team)
{
#ifdef GAME_DLL
    Call_Method<void, Object, Team *>(PICK_ADDRESS(0x00546800, 0x007CFB7A), this, team);
#endif
}

void Object::Heal_Completely()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x00548430, 0x007D1800), this);
#endif
}

float Object::Get_Vision_Range() const
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_debugVisibility) {
        Vector3 v(m_visionRange, 0.0f, 0.0f);

        for (int i = 0; i < g_theWriteableGlobalData->m_debugVisibilityTileCount; i++) {
            float f1 = i * 1.0f / g_theWriteableGlobalData->m_debugVisibilityTileCount;
            float angle = (f1 + f1) * GAMEMATH_PI;
            v.Rotate_Z(angle);
            Coord3D pos;
            pos.x = v.X + Get_Position()->x;
            pos.y = v.Y + Get_Position()->y;
            pos.z = v.Z + Get_Position()->z;
            Add_Icon(&pos,
                g_theWriteableGlobalData->m_debugVisibilityTileWidth,
                g_theWriteableGlobalData->m_debugVisibilityTileDuration,
                g_theWriteableGlobalData->m_debugVisibilityTileTargettableColor);
        }
    }
#endif
    return m_visionRange;
}

void Object::Handle_Partition_Cell_Maintenance()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x0054CD30, 0x007D6DFE), this);
#endif
}

bool Object::Is_Inside(const PolygonTrigger *trigger) const
{
#if 0
    // disabled since this triggers on stock (incorrect if statement order in ScriptConditions::evaluateSkirmishPlayerHasUnitsInArea)
    captainslog_dbgassert(!Is_KindOf(KINDOF_INERT), "Asking whether an inert is inside a trigger area. This is invalid.");
#endif

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        if (m_triggerInfo[i].inside && m_triggerInfo[i].polygon_trigger == trigger) {
            return true;
        }
    }

    return false;
}

bool Object::Did_Enter(const PolygonTrigger *trigger) const
{
    if (!Did_Enter_Or_Exit()) {
        return false;
    }

    captainslog_dbgassert(!Is_KindOf(KINDOF_INERT), "Asking whether an inert object entered or exited. This is invalid.");

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        if (m_triggerInfo[i].entered && m_triggerInfo[i].polygon_trigger == trigger) {
            return true;
        }
    }

    return false;
}

bool Object::Did_Enter_Or_Exit() const
{
    if (Is_KindOf(KINDOF_INERT)) {
        return false;
    }

    unsigned int frame = g_theGameLogic->Get_Frame();
    return m_enteredOrExited == frame || m_enteredOrExited == frame - 1;
}

bool Object::Did_Exit(const PolygonTrigger *trigger) const
{
    if (!Did_Enter_Or_Exit()) {
        return false;
    }

    captainslog_dbgassert(!Is_KindOf(KINDOF_INERT), "Asking whether an inert object entered or exited. This is invalid.");

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        if (m_triggerInfo[i].exited && m_triggerInfo[i].polygon_trigger == trigger) {
            return true;
        }
    }

    return false;
}

CanAttackResult Object::Get_Able_To_Use_Weapon_Against_Target(
    AbleToAttackType type, const Object *obj, const Coord3D *pos, CommandSourceType source, WeaponSlotType slot) const
{
    return m_weaponSet.Get_Able_To_Use_Weapon_Against_Target(type, this, obj, pos, source, slot);
}

SpawnBehaviorInterface *Object::Get_Spawn_Behavior_Interface() const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        SpawnBehaviorInterface *behavior = (*module)->Get_Spawn_Behavior_Interface();

        if (behavior != nullptr) {
            return behavior;
        }
    }

    return nullptr;
}

bool Object::Has_Countermeasures() const
{
#ifdef GAME_DLL
    return Call_Method<bool, Object const>(PICK_ADDRESS(0x0054F460, 0x007D981D), this);
#else
    return false;
#endif
}

SpecialPowerCompletionDie *Object::Find_Special_Power_Completion_Die() const
{
#ifdef GAME_DLL
    return Call_Method<SpecialPowerCompletionDie *, Object const>(PICK_ADDRESS(0x0054E8E0, 0x007D8D93), this);
#else
    return nullptr;
#endif
}

void Object::Set_Producer(const Object *obj)
{
    if (obj != nullptr) {
        m_producerID = obj->Get_ID();
    } else {
        m_producerID = OBJECT_UNK;
    }
}

void Object::Report_Missile_For_Countermeasures(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Object, Object *>(PICK_ADDRESS(0x0054F4A0, 0x007D9855), this, obj);
#endif
}

float Object::Estimate_Damage(DamageInfoInput &info) const
{
    const BodyModuleInterface *body = Get_Body_Module();

    if (body == nullptr) {
        return 0.0f;
    } else {
        return body->Estimate_Damage(info);
    }
}

int Object::Get_Num_Consecutive_Shots_Fired_At_Target(const Object *target) const
{
    if (m_firingTracker != nullptr) {
        return m_firingTracker->Get_Num_Concecutive_Shots_At_Victim(target);
    } else {
        return 0;
    }
}

void Object::Update_Upgrade_Modules()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x00548E90, 0x007D25F5), this);
#endif
}

void Object::Friend_Adjust_Power_For_Player(bool power)
{
    if (!Is_Disabled() || Get_Template()->Get_Energy_Production() <= 0) {
        Energy *energy = Get_Controlling_Player()->Get_Energy();

        if (power) {
            energy->Object_Entering_Influence(this);
        } else {
            energy->Object_Leaving_Influence(this);
        }
    }
}

void Object::Set_Weapon_Bonus_Condition(WeaponBonusConditionType bonus)
{
    unsigned int condition = m_weaponBonusCondition;
    m_weaponBonusCondition = (1 << bonus) | condition;

    if (condition != m_weaponBonusCondition) {
        m_weaponSet.Weapon_Set_On_Weapon_Bonus_Change(this);
    }
}

void Object::Set_Vision_Range(float range)
{
    m_visionRange = range;
}

void Object::Set_Shroud_Clearing_Range(float range)
{
    if (range != m_shroudClearingRange) {
        m_shroudClearingRange = range;
        const Coord3D *pos = Get_Position();

        if (pos->x != 0.0f || pos->y != 0.0f || pos->z != 0.0f) {
            Handle_Partition_Cell_Maintenance();
        }
    }
}

void Object::Clear_Weapon_Bonus_Condition(WeaponBonusConditionType bonus)
{
    unsigned int condition = m_weaponBonusCondition;
    m_weaponBonusCondition = ~(1 << bonus) & condition;

    if (condition != m_weaponBonusCondition) {
        m_weaponSet.Weapon_Set_On_Weapon_Bonus_Change(this);
    }
}

float Object::Get_Shroud_Clearing_Range() const
{
    float range = m_shroudClearingRange;

    if (Get_Status_Bits().Test(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
        range = Get_Geometry_Info().Get_Bounding_Circle_Radius();
    }

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_debugVisibility) {
        Vector3 v(range, 0.0f, 0.0f);

        for (int i = 0; i < g_theWriteableGlobalData->m_debugVisibilityTileCount; i++) {
            float f1 = i * 1.0f / g_theWriteableGlobalData->m_debugVisibilityTileCount;
            float angle = (f1 + f1) * GAMEMATH_PI;
            v.Rotate_Z(angle);
            Coord3D pos;
            pos.x = v.X + Get_Position()->x;
            pos.y = v.Y + Get_Position()->y;
            pos.z = v.Z + Get_Position()->z;
            Add_Icon(&pos,
                g_theWriteableGlobalData->m_debugVisibilityTileWidth,
                g_theWriteableGlobalData->m_debugVisibilityTileDuration,
                g_theWriteableGlobalData->m_debugVisibilityTileTargettableColor);
        }
    }
#endif
    return range;
}

bool Object::Has_Any_Special_Power() const
{
    return m_specialPowers.Any();
}

bool Object::Has_Special_Power(SpecialPowerType spt) const
{
    return m_specialPowers.Test(spt);
}

SpecialPowerModuleInterface *Object::Find_Special_Power_Module_Interface(SpecialPowerType type) const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        SpecialPowerModuleInterface *power = (*module)->Get_Special_Power();

        if (power != nullptr) {
            const SpecialPowerTemplate *tmplate = power->Get_Special_Power_Template();

            if (tmplate != nullptr) {
                if (tmplate->Get_Type() == type) {
                    return power;
                }

                if (type == SPECIAL_INVALID) {
                    return power;
                }
            }
        }
    }

    return nullptr;
}

SpecialPowerModuleInterface *Object::Find_Any_Shortcut_Special_Power_Module_Interface() const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        SpecialPowerModuleInterface *power = (*module)->Get_Special_Power();

        if (power != nullptr) {
            const SpecialPowerTemplate *tmplate = power->Get_Special_Power_Template();

            if (tmplate != nullptr) {
                if (tmplate->Get_Shortcut_Power()) {
                    return power;
                }
            }
        }
    }

    return nullptr;
}

bool Object::Has_Any_Weapon() const
{
    return m_weaponSet.Has_Any_Weapon();
}

unsigned int Object::Get_Most_Percent_Ready_To_Fire_Any_Weapon() const
{
    return m_weaponSet.Get_Most_Percent_Ready_To_Fire_Any_Weapon();
}

void Object::Leave_Group()
{
    if (m_aiGroup != nullptr) {
        AIGroup *group = m_aiGroup;
        m_aiGroup = nullptr;
        group->Remove(this);
    }
}

bool Object::Set_Script_Status(ObjectScriptStatusBit bit, bool set)
{
#ifdef GAME_DLL
    return Call_Method<bool, Object, ObjectScriptStatusBit, bool>(PICK_ADDRESS(0x00547100, 0x007D0360), this, bit, set);
#else
    return false;
#endif
}

bool Object::Is_Faction_Structure() const
{
    return Is_Any_KindOf(KINDOFMASK_FS);
}

ProductionUpdateInterface *Object::Get_Production_Update_Interface()
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        ProductionUpdateInterface *update = (*module)->Get_Production_Update_Interface();

        if (update != nullptr) {
            return update;
        }
    }

    return nullptr;
}

void Object::Set_Vision_Spied(bool vision_spied, int player_index)
{
    bool changed = false;

    if (vision_spied) {
        if (++m_spiedOnByPlayer[player_index] == 1) {
            changed = true;
        }
    } else if (--m_spiedOnByPlayer[player_index] == 0) {
        changed = true;
    }

    if (changed) {
        unsigned short flags = 0;

        for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
            if (m_spiedOnByPlayer[i] <= 0) {
                flags &= ~(1 << i);
            } else {
                flags |= 1 << i;
            }
        }

        m_spiedOnPlayers = flags;
        Handle_Partition_Cell_Maintenance();
    }
}

void Object::Set_ID(ObjectID id)
{
    captainslog_dbgassert(id != OBJECT_UNK, "Object::Set_ID - Invalid id");

    if (m_id != id) {
        g_theGameLogic->Remove_Object_From_Lookup_Table(this);
        m_id = id;
        g_theGameLogic->Add_Object_To_Lookup_Table(this);
    }
}

void Object::Friend_Set_Undetected_Defector(bool set)
{
    if (set) {
        m_privateStatus |= STATUS_UNDETECTED_DEFECTOR;
    } else {
        m_privateStatus &= ~STATUS_UNDETECTED_DEFECTOR;
    }
}

void Object::Clear_Special_Model_Condition_States()
{
    BitFlags<MODELCONDITION_COUNT> flags(BitFlags<MODELCONDITION_COUNT>::kInit, MODELCONDITION_SPECIAL_CHEERING);
    Object::Clear_Model_Condition_Flags(flags);
    m_specialModelConditionSleepFrame = 0;
}

void Object::Clear_Model_Condition_Flags(const BitFlags<MODELCONDITION_COUNT> &clr)
{
    if (m_drawable != nullptr) {
        m_drawable->Clear_Model_Condition_Flags(clr);
    }
}

void Object::Adjust_Model_Condition_For_Weapon_Status()
{
    unsigned int frame = g_theGameLogic->Get_Frame();

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        Weapon *weapon = m_weaponSet.Get_Weapon_In_Weapon_Slot(static_cast<WeaponSlotType>(i));

        if (weapon != nullptr) {
            int condition = -1;

            if (i == m_weaponSet.Get_Cur_Weapon_Slot()) {
                if (weapon->Get_Last_Fire_Frame() == frame) {
                    condition = WSF_FIRING;
                } else if (Get_Status(OBJECT_STATUS_IS_ATTACKING)) {
                    static WeaponSetConditionType s_wsfLookup[WSF_COUNT] = {
                        WSF_NONE, WSF_NONE, WSF_BETWEEN, WSF_RELOADING, WSF_PREATTACK
                    };
                    Weapon::WeaponStatus status = weapon->Get_Status();
                    condition = s_wsfLookup[status];

                    if (status == Weapon::READY_TO_FIRE && condition == WSF_NONE && Get_Status(OBJECT_STATUS_IS_ATTACKING)
                        && (Get_Status(OBJECT_STATUS_IS_AIMING_WEAPON) || Get_Status(OBJECT_STATUS_IS_FIRING_WEAPON))) {
                        condition = WSF_BETWEEN;
                    }
                } else {
                    condition = WSF_NONE;
                }
            } else {
                condition = WSF_NONE;
            }

            if (m_drawable != nullptr) {
                m_drawable->Update_Drawable_Clip_Status(
                    weapon->Get_Ammo_In_Clip(), weapon->Get_Clip_Size(), weapon->Get_Weapon_Slot());

                if (condition != -1 && condition != m_weaponSetConditions[i]) {
                    m_weaponSetConditions[i] = condition;
                    BitFlags<MODELCONDITION_COUNT> flags = WeaponSet::Get_Model_Condition_For_Weapon_Slot(
                        static_cast<WeaponSlotType>(i), static_cast<WeaponSetConditionType>(condition));
                    m_drawable->Clear_And_Set_Model_Condition_Flags(s_allWeaponFireFlags[i], flags);

                    if (condition == WSF_PREATTACK) {
                        unsigned int finished = weapon->Get_When_Pre_Attack_Finished();

                        if (finished > frame) {
                            m_drawable->Set_Animation_Loop_Duration(finished - frame);
                        }
                    }
                }
            }
        } else {
            m_weaponSetConditions[i] = WSF_NONE;
        }
    }
}

void Object::Clear_And_Set_Model_Condition_Flags(
    const BitFlags<MODELCONDITION_COUNT> &clr, const BitFlags<MODELCONDITION_COUNT> &set)
{
    if (m_drawable != nullptr) {
        m_drawable->Clear_And_Set_Model_Condition_Flags(clr, set);
    }
}

void Object::Reload_All_Ammo(bool now)
{
    m_weaponSet.Reload_All_Ammo(this, now);
}

void Object::Friend_Bind_To_Drawable(Drawable *drawable)
{
    m_drawable = drawable;

    if (m_drawable != nullptr) {
        BitFlags<MODELCONDITION_COUNT> must_be_set;
        BitFlags<MODELCONDITION_COUNT> must_be_clear;

        for (int i = 0; i < WEAPONSET_COUNT; i++) {
            ModelConditionFlagType flag = s_theWeaponSetTypeToModelConditionTypeMap[i];

            if (flag != MODELCONDITION_INVALID) {
                if (m_curWeaponSetFlags.Test(i)) {
                    must_be_set.Set(flag, true);
                } else {
                    must_be_clear.Set(flag, true);
                }
            }
        }

        if (g_theWriteableGlobalData != nullptr) {
            if (g_theWriteableGlobalData->m_forceModelsFollowTimeOfDay) {
                must_be_set.Set(MODELCONDITION_NIGHT, g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT);
            }

            if (g_theWriteableGlobalData->m_forceModelsFollowWeather) {
                must_be_set.Set(MODELCONDITION_SNOW, g_theWriteableGlobalData->m_weather == WEATHER_SNOWY);
            }
        }

        m_drawable->Clear_And_Set_Model_Condition_Flags(must_be_clear, must_be_set);

        for (BehaviorModule **m = m_allModules; *m != nullptr; m++) {
            (*m)->On_Drawable_Bound_To_Object();
        }
    }
}

void Object::Set_Trigger_Area_Flags_For_Change_In_Position()
{
    if (!Is_KindOf(KINDOF_PROJECTILE) && !Is_KindOf(KINDOF_INERT)) {
        const Coord3D *pos = Get_Position();
        ICoord3D ipos;
        ipos.x = GameMath::Fast_To_Int_Truncate(pos->x);
        ipos.y = GameMath::Fast_To_Int_Truncate(pos->y);
        ipos.z = 0;

        if (m_iPos.x != ipos.x || m_iPos.y != ipos.y) {
            if (!Is_KindOf(KINDOF_IMMOBILE) && (Is_KindOf(KINDOF_INFANTRY) || Is_KindOf(KINDOF_VEHICLE))) {
                g_theGameClient->Notify_Terrain_Object_Moved(this);
            }

            if (Get_AI_Update_Interface() != nullptr) {
                g_theAI->Get_Pathfinder()->Update_Pos(this, Get_Position());
            }

            unsigned int frame = g_theGameLogic->Get_Frame();

            if (m_enteredOrExited != 0 && m_enteredOrExited != frame) {
                Update_Trigger_Area_Flags();
            }

            for (int i = 0; i < m_numTriggerAreasActive; i++) {
                if (!m_triggerInfo[i].polygon_trigger->Point_In_Trigger(m_iPos)) {
                    m_triggerInfo[i].inside = false;
                    m_triggerInfo[i].exited = true;
                    m_enteredOrExited = frame;

                    if (m_team != nullptr) {
                        m_team->Set_Entered_Exited();
                    }

                    g_theGameLogic->Save_Frame();
                }
            }

            m_iPos = ipos;

            for (PolygonTrigger *t = PolygonTrigger::Get_First_Polygon_Trigger(); t != nullptr; t = t->Get_Next()) {
                bool trigger_found = false;

                for (int i = 0; i < m_numTriggerAreasActive; i++) {
                    if (m_triggerInfo[i].polygon_trigger == t) {
                        trigger_found = true;
                        break;
                    }
                }

                if (!trigger_found && t->Point_In_Trigger(m_iPos)) {
                    if (m_numTriggerAreasActive >= 5) {
                        static bool didWarn;

                        if (!didWarn) {
                            didWarn = true;
                            g_theScriptEngine->Append_Debug_Message("***WARNING - Too many nested trigger areas. ***", true);
                        }
                    } else {
                        m_triggerInfo[m_numTriggerAreasActive].inside = true;
                        m_triggerInfo[m_numTriggerAreasActive].entered = true;
                        m_triggerInfo[m_numTriggerAreasActive].exited = false;
                        m_triggerInfo[m_numTriggerAreasActive].polygon_trigger = t;
                        m_enteredOrExited = frame;

                        if (m_team != nullptr) {
                            m_team->Set_Entered_Exited();
                        }

                        g_theGameLogic->Save_Frame();
                        m_numTriggerAreasActive++;
                    }
                }
            }
        }
    }
}

void Object::Update_Trigger_Area_Flags()
{
    int count = 0;

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        if (m_triggerInfo[count].inside) {
            m_triggerInfo[count].entered = false;
            m_triggerInfo[count].exited = false;
            m_triggerInfo[count].inside = m_triggerInfo[i].inside;
            m_triggerInfo[count].polygon_trigger = m_triggerInfo[i].polygon_trigger;
            count++;
        }
    }

    m_numTriggerAreasActive = count;
}

void Object::Set_Or_Restore_Team(Team *team, bool b)
{
    if (m_team != team) {
        Team *old_team = m_team;

        if (m_team != nullptr && m_team->Is_In_List_Team_Member_List(this)) {
            m_team->Remove_From_Team_Member_List(this);
            m_team->Get_Controlling_Player()->Becoming_Team_Member(this, false);
        }

        m_team = team;

        if (m_team != nullptr) {
            if (!m_team->Is_In_List_Team_Member_List(this)) {
                m_team->Prepend_To_Team_Member_List(this);
                m_team->Get_Controlling_Player()->Becoming_Team_Member(this, true);
            }

            const TeamPrototype *proto = m_team->Get_Prototype();

            if (proto != nullptr) {
                if (proto->Get_Template_Info() != nullptr) {
                    AIUpdateInterface *update = Get_AI_Update_Interface();

                    if (update != nullptr) {
                        update->Set_Attitude(proto->Get_Template_Info()->m_initialTeamAttitude);

                        if (m_team->Get_Prototype()->Get_Attack_Priority_Name().Is_Not_Empty()) {
                            const AttackPriorityInfo *attack_info =
                                g_theScriptEngine->Get_Attack_Info(m_team->Get_Prototype()->Get_Attack_Priority_Name());

                            if (attack_info != nullptr) {
                                if (attack_info->Get_Name().Is_Not_Empty()) {
                                    update->Set_Attack_Priority_Info(attack_info);
                                }
                            }
                        }
                    }
                }
            }

            Drawable *drawable = Get_Drawable();

            if (drawable != nullptr) {
                drawable->Changed_Team();
            }
        }

        if (old_team != nullptr && team != nullptr && !b) {
            On_Capture(old_team->Get_Controlling_Player(), team->Get_Controlling_Player());
        }

        if (m_radarData != nullptr) {
            g_theRadar->Remove_Object(this);
            g_theRadar->Add_Object(this);
        }

        int old_index;

        if (old_team != nullptr) {
            old_index = old_team->Get_Controlling_Player()->Get_Player_Index();
        } else {
            old_index = -1;
        }

        int new_index;

        if (m_team != nullptr) {
            new_index = m_team->Get_Controlling_Player()->Get_Player_Index();
        } else {
            new_index = -1;
        }

        if (old_index != new_index) {
            g_theInGameUI->Object_Changed_Team(this, old_index, new_index);
        }
    }
}

void Object::On_Capture(Player *o, Player *n)
{
    if (Get_AI_Update_Interface() != nullptr && o != n) {
        Get_AI_Update_Interface()->AI_Idle(COMMANDSOURCE_AI);
    }

    n->Get_Score_Keeper()->Add_Object_Captured(this);

    for (BehaviorModule **m = m_allModules; *m != nullptr; m++) {
        (*m)->On_Capture(o, n);
    }

    Handle_Partition_Cell_Maintenance();
    Clear_Script_Status(STATUS_UNSELLABLE);
    g_theControlBar->Mark_UI_Dirty();

    if (o != n && n->Is_Skirmish_AI_Player()) {
        if (Is_Faction_Structure()) {
            g_theBuildAssistant->Sell_Object(this);
        }
    }
}

void Object::Do_Status_Damage(ObjectStatusTypes status, float damage)
{
    if (m_statusDamageHelper != nullptr) {
        m_statusDamageHelper->Do_Status_Damage(status, damage);
    }
}

void Object::Notify_Subdual_Damage(float damage)
{
    if (m_subdualDamageHelper != nullptr) {
        m_subdualDamageHelper->Notify_Subdual_Damage(damage);
    }

    if (Get_Drawable() != nullptr) {
        if (damage <= 0.0f) {
            Get_Drawable()->Clear_Draw_Bit(Drawable::DRAW_BIT_DAMAGED);
        } else {
            Get_Drawable()->Set_Draw_Bit(Drawable::DRAW_BIT_DAMAGED);
        }
    }
}

void Object::Do_Temp_Weapon_Bonus(WeaponBonusConditionType type, unsigned int frame)
{
    if (m_tempWeaponBonusHelper != nullptr) {
        m_tempWeaponBonusHelper->Do_Temp_Weapon_Bonus(type, frame);
    }
}

void Object::Fire_Current_Weapon(const Coord3D *pos)
{
    if (pos != nullptr) {
        Weapon *weapon = m_weaponSet.Get_Cur_Weapon();

        if (weapon != nullptr) {
            if (weapon->Get_Status() == Weapon::READY_TO_FIRE) {
                bool fired = weapon->Fire_Weapon(this, pos, nullptr);
                captainslog_dbgassert(
                    m_firingTracker != nullptr, "hey, we are firing but have no firing tracker. this is wrong.");

                if (m_firingTracker != nullptr) {
                    m_firingTracker->Shot_Fired(weapon, OBJECT_UNK);
                }

                if (fired) {
                    Release_Weapon_Lock(LOCKED_LEVEL_1);
                }

                Friend_Set_Undetected_Defector(false);
            }
        }
    }
}

void Object::Fire_Current_Weapon(Object *target)
{
    if (target != nullptr) {
        Weapon *weapon = m_weaponSet.Get_Cur_Weapon();

        if (weapon != nullptr) {
            if (weapon->Get_Status() == Weapon::READY_TO_FIRE) {
                bool fired = weapon->Fire_Weapon(this, target, nullptr);
                captainslog_dbgassert(
                    m_firingTracker != nullptr, "hey, we are firing but have no firing tracker. this is wrong.");

                if (m_firingTracker != nullptr) {
                    m_firingTracker->Shot_Fired(weapon, target->Get_ID());
                }

                if (fired) {
                    Release_Weapon_Lock(LOCKED_LEVEL_1);
                }

                Friend_Set_Undetected_Defector(false);
            }
        }
    }
}

void Object::Notify_Firing_Tracker_Shot_Fired(const Weapon *weapon, ObjectID id)
{
    if (m_firingTracker != nullptr) {
        m_firingTracker->Shot_Fired(weapon, id);
    }
}
