/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Model Draw Module
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dmodeldraw.h"
#include "aiupdate.h"
#include "drawable.h"
#include "fpusetting.h"
#include "fxlist.h"
#include "gamelod.h"
#include "gamelogic.h"
#include "gamestate.h"
#include "globaldata.h"
#include "hanim.h"
#include "hlod.h"
#include "htree.h"
#include "mesh.h"
#include "meshmdl.h"
#include "obbox.h"
#include "particlesys.h"
#include "particlesystemplate.h"
#include "physicsupdate.h"
#include "randomvalue.h"
#include "rendobj.h"
#include "w3dassetmanager.h"
#include "w3ddisplay.h"
#include "w3dprojectedshadow.h"
#include "w3dscene.h"
#include "w3dshadow.h"
#include "w3dterraintracks.h"
#include <cctype>
#include <cstring>
#include <stdio.h>
using std::strcpy;
using std::strstr;
using std::tolower;
#define MSEC_PER_LOGICFRAME_REAL (1000.0f / 30.0f)

#ifdef GAME_DEBUG_STRUCTS
#ifdef GAME_DLL
extern Utf8String &s_theThingTemplateBeingParsedName;
float &s_skateDistanceOverride = Make_Global<float>(0x00E260C8);
unsigned int &s_theObjectIDToDebug = Make_Global<unsigned int>(0x00E25EF0);
#else
extern Utf8String s_theThingTemplateBeingParsedName;
float s_skateDistanceOverride;
unsigned int s_theObjectIDToDebug;
#endif
#endif

HAnimClass *W3DAnimationInfo::Get_Anim_Handle() const
{
    HAnimClass *anim = W3DDisplay::s_assetManager->Get_HAnim(m_name.Str());

    if (anim != nullptr) {
        if (m_framesPerSecond < 0.0f) {
            m_framesPerSecond = (anim->Get_Num_Frames() * 1000.0f) / anim->Get_Frame_Rate();
        }
    } else {
        // Thyme specific: Original assert has been demoted to log message because it is a data issue.
        captainslog_error("*** ASSET ERROR: animation %s not found", m_name.Str());
    }

    return anim;
}

void ModelConditionInfo::WeaponBarrelInfo::Set_Muzzle_Flash_Hidden(RenderObjClass *robj, bool hidden) const
{
    if (robj != nullptr) {
        RenderObjClass *child_object = robj->Get_Sub_Object_On_Bone(0, m_weaponMuzzleFlashBone);

        if (child_object != nullptr) {
            child_object->Set_Hidden(hidden);
            child_object->Release_Ref();
        } else {
#ifdef GAME_DEBUG_STRUCTS
            captainslog_debug("*** ASSET ERROR: child_object %s not found in Set_Muzzle_Flash_Hidden()",
                m_weaponMuzzleFlashBoneName.Str());
#endif
        }
    }
}

void ModelConditionInfo::Preload_Assets(TimeOfDayType time_of_day, float scale) const
{
    if (!m_modelName.Is_Empty()) {
        g_theDisplay->Preload_Model_Assets(m_modelName);
    }
}

void ModelConditionInfo::Add_Public_Bone(Utf8String const &bone) const
{
    if (!bone.Is_Empty() && !bone.Is_None()) {
        Utf8String str(bone);
        str.To_Lower();

        if (std::find(m_publicBones.begin(), m_publicBones.end(), str) == m_publicBones.end()) {
            m_publicBones.push_back(str);
        }
    }
}

bool ModelConditionInfo::Matches_Mode(bool night, bool snow) const
{
    for (auto &flags : m_conditionsYesVec) {
        if (flags.Test(MODELCONDITION_NIGHT) == night && flags.Test(MODELCONDITION_SNOW) == snow) {
            return true;
        }
    }

    return false;
}

bool Should_Validate()
{
    return (g_theGameLogic && g_theGameLogic->Get_In_Game_Logic_Update())
        || (g_theGameState && g_theGameState->Is_Loading());
}

void ModelConditionInfo::Validate_Stuff(RenderObjClass *robj, float scale, std::vector<Utf8String> const &bones) const
{
    Load_Animations();

    if ((m_validStuff & PUBLIC_BONES_VALID) == 0 && Should_Validate()) {
        for (auto &bone : bones) {
            Add_Public_Bone(bone);
        }

        m_validStuff |= PUBLIC_BONES_VALID;
    }

    Validate_Cached_Bones(robj, scale);
    Validate_Turret_Info();
    Validate_Weapon_Barrel_Info();
}

bool Test_Animation_Flag(int flags, AnimationStateFlag flag)
{
    return ((1 << flag) & flags) != 0;
}

bool Find_Single_Bone(RenderObjClass *r, Utf8String const &bone, Matrix3D &transform, int &index)
{
    if (bone.Is_None() || bone.Is_Empty()) {
        return false;
    }

    index = r->Get_Bone_Index(bone.Str());

    if (index == 0) {
        return false;
    }

    transform = r->Get_Bone_Transform(index);
    return true;
}

bool Find_Single_Sub_Obj(RenderObjClass *r, Utf8String const &sub_obj_name, Matrix3D &transform, int &index)
{
    if (sub_obj_name.Is_None() || sub_obj_name.Is_Empty()) {
        return false;
    }

    RenderObjClass *subobj = r->Get_Sub_Object_By_Name(sub_obj_name.Str(), nullptr);

    if (subobj == nullptr) {
        return false;
    }

    transform = r->Get_Bone_Transform(index);

    for (int i = 0; i < r->Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = r->Get_Sub_Object(i);

        if (robj == subobj) {
            index = r->Get_Sub_Object_Bone_Index(0, i);
            robj->Release_Ref();
            robj = r->Get_Sub_Object_On_Bone(0, index);
            captainslog_dbgassert(robj && robj == subobj, "*** ASSET ERROR: Hmm, bone problem");
        }

        if (robj != nullptr) {
            robj->Release_Ref();
        }
    }

    subobj->Release_Ref();
    return true;
}

bool Do_Single_Bone_Name(RenderObjClass *robj, Utf8String const &bone, std::map<NameKeyType, PristineBoneInfo> &map)
{
    bool bone_found = false;
    bool sub_obj_found = false;
    PristineBoneInfo info;
    Utf8String bone_id;
    Utf8String bone_lower(bone);
    bone_lower.To_Lower();
    Set_FP_Mode();

    if (Find_Single_Bone(robj, bone_lower, info.transform, info.index)) {
        map[g_theNameKeyGenerator->Name_To_Key(bone_lower.Str())] = info;
        bone_found = true;
    }

    for (int i = 1; i <= 99; i++) {
        bone_id.Format("%s%02d", bone_lower.Str(), i);

        if (!Find_Single_Bone(robj, bone_id, info.transform, info.index)) {
            break;
        }

        map[g_theNameKeyGenerator->Name_To_Key(bone_id.Str())] = info;
        bone_found = true;
    }

    if (!bone_found) {
        if (Find_Single_Sub_Obj(robj, bone_lower, info.transform, info.index)) {
            map[g_theNameKeyGenerator->Name_To_Key(bone_lower.Str())] = info;
            sub_obj_found = true;
        }

        for (int i = 1; i <= 99; i++) {
            bone_id.Format("%s%02d", bone_lower.Str(), i);

            if (!Find_Single_Sub_Obj(robj, bone_id, info.transform, info.index)) {
                break;
            }

            map[g_theNameKeyGenerator->Name_To_Key(bone_id.Str())] = info;
            sub_obj_found = true;
        }
    }

    return bone_found || sub_obj_found;
}

void ModelConditionInfo::Validate_Cached_Bones(RenderObjClass *robj, float scale) const
{
    if ((m_validStuff & PRISTINE_BONES_VALID) != 0) {
        return;
    }

    if (!Should_Validate()) {
        m_boneMap.clear();
        m_validStuff &= ~PRISTINE_BONES_VALID;
        return;
    }

    Set_FP_Mode();
    m_boneMap.clear();
    m_validStuff |= PRISTINE_BONES_VALID;
    bool ref = false;

    if (robj == nullptr && !m_modelName.Is_Empty()) {
        robj = W3DDisplay::s_assetManager->Create_Render_Obj(m_modelName.Str(), scale, 0, nullptr, nullptr);

        if (robj != nullptr) {
            ref = true;
        } else {
            // Thyme specific: Original assert has been demoted to log message because it is a data issue.
            captainslog_error("*** ASSET ERROR: Model %s not found!", m_modelName.Str());
        }
    }

    if (robj != nullptr) {
        Matrix3D m(robj->Get_Transform());
        HLodClass *hlod = nullptr;
        HAnimClass *anim = nullptr;
        int frames = 0;
        float frame = 0.0f;
        int mode = 0;
        float multiplier = 1.0f;
        HAnimClass *first_anim;

        if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            hlod = static_cast<HLodClass *>(robj);
            anim = hlod->Peek_Animation_And_Info(frame, frames, mode, multiplier);
        }

        if (m_animations.size()) {
            first_anim = m_animations.front().Get_Anim_Handle();
        } else {
            first_anim = anim;

            if (first_anim != nullptr) {
                first_anim->Add_Ref();
            }
        }

        if (first_anim != nullptr) {
            int fr;

            if (Test_Animation_Flag(m_flags, PRISTINE_BONE_POS_IN_FINAL_FRAME)) {
                fr = first_anim->Get_Num_Frames() - 1;
            } else {
                fr = 0;
            }

            robj->Set_Animation(first_anim, fr);
            Ref_Ptr_Release(first_anim);
        }

        Matrix3D m2(true);
        m2.Scale(scale);
        robj->Set_Transform(m2);

        if (g_theWriteableGlobalData != nullptr) {
            for (auto &bone_name : g_theWriteableGlobalData->m_standardPublicBones) {
                Do_Single_Bone_Name(robj, bone_name, m_boneMap);
            }
        }

        for (auto &bone_name : m_publicBones) {
            if (!Do_Single_Bone_Name(robj, bone_name, m_boneMap)) {
                captainslog_debug("*** ASSET ERROR: public bone '%s' (and variations thereof) not found in model %s!",
                    bone_name.Str(),
                    m_modelName.Str());
            }
        }

        robj->Set_Transform(m);

        if (anim != nullptr) {
            robj->Set_Animation(anim, frame, mode);
            captainslog_assert(hlod != nullptr);
            hlod->Set_Animation_Frame_Rate_Multiplier(multiplier);
        }

        if (ref) {
            robj->Release_Ref();
        }
    }
}

void ModelConditionInfo::Validate_Weapon_Barrel_Info() const
{
    if ((m_validStuff & WEAPON_BARREL_INFO_VALID) == 0 && Should_Validate()) {
        Set_FP_Mode();

        for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
            m_weaponBarrelInfoVec[i].clear();
            m_hasWeaponBone[i] = false;
            const Utf8String &weaponfirefxbonename = m_weaponFireFXBoneName[i];
            const Utf8String &weaponrecoilbonename = m_weaponRecoilBoneName[i];
            const Utf8String &weaponmuzzleflashbonename = m_weaponMuzzleFlashName[i];
            const Utf8String &weaponlaunchbonename = m_weaponLaunchBoneName[i];

            if (weaponfirefxbonename.Is_Not_Empty() || weaponrecoilbonename.Is_Not_Empty()
                || weaponmuzzleflashbonename.Is_Not_Empty() || weaponlaunchbonename.Is_Not_Empty()) {
                int weaponfirefxbone = 0;

                for (int j = 1; j <= 99; j++) {
                    ModelConditionInfo::WeaponBarrelInfo info;
                    info.m_weaponLaunchBoneTransform.Make_Identity();

                    if (!weaponrecoilbonename.Is_Empty()) {
                        char bone_id[256];
                        sprintf(bone_id, "%s%02d", weaponrecoilbonename.Str(), j);
                        Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(bone_id), &info.m_weaponRecoilBone);
                    }

                    if (!weaponmuzzleflashbonename.Is_Empty()) {
                        char bone_id[256];
                        sprintf(bone_id, "%s%02d", weaponmuzzleflashbonename.Str(), j);
                        Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(bone_id), &info.m_weaponMuzzleFlashBone);

#ifdef GAME_DEBUG_STRUCTS
                        if (info.m_weaponMuzzleFlashBone) {
                            info.m_weaponMuzzleFlashBoneName = bone_id;
                        }
#endif
                    }

                    if (!weaponfirefxbonename.Is_Empty()) {
                        char bone_id[256];
                        sprintf(bone_id, "%s%02d", weaponfirefxbonename.Str(), j);
                        Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(bone_id), &info.m_weaponFireFXBone);

                        if (!info.m_weaponFireFXBone) {
                            if (info.m_weaponMuzzleFlashBone) {
                                info.m_weaponFireFXBone = weaponfirefxbone;
                            }
                        }
                    }

                    int weaponlaunchbone = 0;

                    if (!weaponlaunchbonename.Is_Empty()) {
                        char bone_id[256];
                        sprintf(bone_id, "%s%02d", weaponlaunchbonename.Str(), j);
                        const Matrix3D *m =
                            Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(bone_id), &weaponlaunchbone);

                        if (m != nullptr) {
                            info.m_weaponLaunchBoneTransform = *m;
                        }
                    }

                    if (!info.m_weaponFireFXBone && !info.m_weaponRecoilBone && !info.m_weaponMuzzleFlashBone
                        && !weaponlaunchbone) {
                        break;
                    }

                    m_weaponBarrelInfoVec[i].push_back(info);

                    if (info.m_weaponRecoilBone || info.m_weaponMuzzleFlashBone) {
                        m_hasWeaponBone[i] = true;
                    }

                    weaponfirefxbone = info.m_weaponFireFXBone;
                }

                if (m_weaponBarrelInfoVec[i].empty()) {
                    ModelConditionInfo::WeaponBarrelInfo info;

                    if (!weaponrecoilbonename.Is_Empty()) {
                        Find_Pristine_Bone(
                            g_theNameKeyGenerator->Name_To_Key(weaponrecoilbonename.Str()), &info.m_weaponRecoilBone);
                    }

                    if (!weaponmuzzleflashbonename.Is_Empty()) {
                        Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(weaponmuzzleflashbonename.Str()),
                            &info.m_weaponMuzzleFlashBone);
                    }

#ifdef GAME_DEBUG_STRUCTS
                    if (info.m_weaponMuzzleFlashBone) {
                        info.m_weaponMuzzleFlashBoneName = weaponmuzzleflashbonename;
                    }
#endif

                    const Matrix3D *m;

                    if (weaponlaunchbonename.Is_Empty()) {
                        m = nullptr;
                    } else {
                        m = Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(weaponlaunchbonename.Str()), nullptr);
                    }

                    if (m != nullptr) {
                        info.m_weaponLaunchBoneTransform = *m;
                    } else {
                        info.m_weaponLaunchBoneTransform.Make_Identity();
                    }

                    if (!weaponfirefxbonename.Is_Empty()) {
                        Find_Pristine_Bone(
                            g_theNameKeyGenerator->Name_To_Key(weaponfirefxbonename.Str()), &info.m_weaponFireFXBone);
                    }

                    if (info.m_weaponFireFXBone || info.m_weaponRecoilBone || info.m_weaponMuzzleFlashBone || m != nullptr) {
                        m_weaponBarrelInfoVec[i].push_back(info);

                        if (info.m_weaponRecoilBone || info.m_weaponMuzzleFlashBone) {
                            m_hasWeaponBone[i] = true;
                        }
                    }
                }

                // Thyme specific: Original assert has been demoted to log message because it is a data issue.
                if (m_modelName.Is_Not_Empty() && m_weaponBarrelInfoVec[i].empty()) {
                    captainslog_error("*** ASSET ERROR: No fx bone named '%s' found in model %s!",
                        weaponfirefxbonename.Str(),
                        m_modelName.Str());
                }
            }
        }

        m_validStuff |= WEAPON_BARREL_INFO_VALID;
    }
}

void ModelConditionInfo::Validate_Turret_Info() const
{
    if ((m_validStuff & TURRET_VALID) == 0) {
        Set_FP_Mode();

        for (int i = 0; i < MAX_TURRETS; i++) {
            TurretInfo *info = &m_turretInfo[i];

            if (!Should_Validate() || m_modelName.Is_Empty()) {
                info->m_turretAngleBone = 0;
                info->m_turretPitchBone = 0;
            } else {
                if (info->m_turretAngleName) {
                    if (!Find_Pristine_Bone(info->m_turretAngleName, &info->m_turretAngleBone)) {
                        captainslog_debug("*** ASSET ERROR: TurretBone %s not found! (%s)",
                            g_theNameKeyGenerator->Key_To_Name(info->m_turretAngleName).Str(),
                            m_modelName.Str());
                        info->m_turretAngleBone = 0;
                    }
                } else {
                    info->m_turretAngleBone = 0;
                }

                if (info->m_turretPitchName) {
                    if (!Find_Pristine_Bone(info->m_turretPitchName, &info->m_turretPitchBone)) {
                        captainslog_debug("*** ASSET ERROR: TurretBone %s not found! (%s)",
                            g_theNameKeyGenerator->Key_To_Name(info->m_turretPitchName).Str(),
                            m_modelName.Str());
                        info->m_turretPitchBone = 0;
                    }
                } else {
                    info->m_turretPitchBone = 0;
                }
            }
        }

        if (Should_Validate()) {
            m_validStuff |= TURRET_VALID;
        }
    }
}

Matrix3D const *ModelConditionInfo::Find_Pristine_Bone(NameKeyType key, int *index) const
{
    captainslog_dbgassert((m_validStuff & PRISTINE_BONES_VALID) != 0, "*** ASSET ERROR: bones are not valid");

    if ((m_validStuff & PRISTINE_BONES_VALID) != 0) {
        if (key != NAMEKEY_INVALID) {
            auto i = m_boneMap.find(key);

            if (i != m_boneMap.end()) {
                if (index != nullptr) {
                    *index = i->second.index;
                }

                return &i->second.transform;
            }
        }
    }

    if (index != nullptr) {
        *index = 0;
    }

    return nullptr;
}

bool ModelConditionInfo::Find_Pristine_Bone_Pos(NameKeyType key, Coord3D &pos) const
{
    const Matrix3D *bone = Find_Pristine_Bone(key, nullptr);

    if (bone != nullptr) {
        Vector3 v = bone->Get_Translation();
        pos.x = v.X;
        pos.y = v.Y;
        pos.z = v.Z;
        return true;
    } else {
        pos.Zero();
        return false;
    }
}

void ModelConditionInfo::Clear()
{
#ifdef GAME_DEBUG_STRUCTS
    m_description.Clear();
#endif
    m_conditionsYesVec.clear();
    m_modelName.Clear();

    for (int i = 0; i < MAX_TURRETS; i++) {
        m_turretInfo[i].Clear();
    }

    m_hideShowVec.clear();

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        m_weaponFireFXBoneName[i].Clear();
        m_weaponRecoilBoneName[i].Clear();
        m_weaponMuzzleFlashName[i].Clear();
        m_weaponLaunchBoneName[i].Clear();
        m_weaponBarrelInfoVec[i].clear();
        m_hasWeaponBone[i] = false;
    }

    m_ParticleSysBones.clear();
    m_animations.clear();

    m_flags = 0;
    m_transitionKey = NAMEKEY_INVALID;
    m_allowToFinishKey = NAMEKEY_INVALID;
    m_iniReadFlags = 0;
    m_mode = RenderObjClass::ANIM_MODE_ONCE;
    m_transition = 0;
    m_animationSpeedFactorMin = 1.0f;
    m_animationSpeedFactorMax = 1.0f;
    m_boneMap.clear();
    m_validStuff = 0;
}

W3DModelDrawModuleData::W3DModelDrawModuleData() :
    m_stateCount(-1),
    m_projectileBoneFeedbackEnabledSlots(false),
    m_initialRecoil(2.0f),
    m_maxRecoil(3.0f),
    m_recoilDamping(0.4f),
    m_recoilSettle(0.065f),
    m_minLodRequired(STATLOD_LOW),
    m_okToChangeModelColor(false),
    m_animationsRequirePower(true),
    m_attachToDrawableBoneOffsetSet(false),
    m_timeAndWeatherFlags(false),
    m_particlesAttachedToAnimatedBones(false),
    m_recievesDynamicLights(true)
{
}

W3DModelDrawModuleData::~W3DModelDrawModuleData()
{
    m_conditionStateMap.Clear();
}

unsigned int Get_Old_State_From_Transition(uint64_t transition)
{
    return (unsigned int)(transition >> 32);
}

unsigned int Get_New_State_From_Transition(uint64_t transition)
{
    return (unsigned int)transition;
}

void W3DModelDrawModuleData::Validate_Stuff_For_Time_And_Weather(Drawable const *drawable, bool night, bool snow) const
{
    if (Should_Validate()) {
        int flags = night ? (snow ? TIMEANDWEATHER_NIGHTSNOW : TIMEANDWEATHER_NIGHT) :
                            (snow ? TIMEANDWEATHER_SNOW : TIMEANDWEATHER_DAY);

        if ((flags & m_timeAndWeatherFlags) == 0) {
            m_timeAndWeatherFlags |= flags;

            for (auto &model_condition : m_conditionStates) {
                if (model_condition.Matches_Mode(false, false) || model_condition.Matches_Mode(night, snow)) {
                    model_condition.Validate_Stuff(nullptr, drawable->Get_Scale(), m_extraPublicBone);
                }
            }

            for (auto &transition_pair : m_transitionMap) {
                int oldstate = Get_Old_State_From_Transition(transition_pair.first);
                int newstate = Get_New_State_From_Transition(transition_pair.first);
                bool foundoldstate = false;
                bool foundnewstate = false;

                for (auto &model_condition : m_conditionStates) {
                    if (!foundoldstate && model_condition.m_transitionKey == oldstate
                        && model_condition.Matches_Mode(night, snow)) {
                        foundoldstate = true;
                    }

                    if (!foundnewstate && model_condition.m_transitionKey == newstate
                        && model_condition.Matches_Mode(night, snow)) {
                        foundnewstate = true;
                    }
                }

                if (foundoldstate && foundnewstate) {
                    transition_pair.second.Load_Animations();
                    transition_pair.second.Validate_Stuff(nullptr, drawable->Get_Scale(), m_extraPublicBone);
                }
            }
        }
    }
}

void W3DModelDrawModuleData::Preload_Assets(TimeOfDayType time_of_day, float scale) const
{
    for (auto &model_condition : m_conditionStates) {
        model_condition.Preload_Assets(time_of_day, scale);
    }
}

Utf8String W3DModelDrawModuleData::Get_Best_Model_Name_For_WB(BitFlags<MODELCONDITION_COUNT> const &flags) const
{
    const ModelConditionInfo *info = Find_Best_Info(flags);

    if (info != nullptr) {
        return info->m_modelName;
    } else {
        return Utf8String::s_emptyString;
    }
}

Vector3 *W3DModelDrawModuleData::Get_Attach_To_Drawable_Bone_Offset(Drawable const *drawable) const
{
    if (m_attachToBoneInAnotherModule.Is_Empty()) {
        return nullptr;
    }

    if (!m_attachToDrawableBoneOffsetSet) {
        Matrix3D m;

        if (drawable->Get_Pristine_Bone_Positions(m_attachToBoneInAnotherModule.Str(), 0, nullptr, &m, 1) == 1) {
            m_attachToDrawableBoneOffset = m.Get_Translation();
        } else {
            m_attachToDrawableBoneOffset.X = 0.0f;
            m_attachToDrawableBoneOffset.Y = 0.0f;
            m_attachToDrawableBoneOffset.Z = 0.0f;
        }

        m_attachToDrawableBoneOffsetSet = true;
    }

    return &m_attachToDrawableBoneOffset;
}

const ModelConditionInfo *W3DModelDrawModuleData::Find_Best_Info(BitFlags<MODELCONDITION_COUNT> const &flags) const
{
    BitFlags<MODELCONDITION_COUNT> set_flags = flags;
    set_flags.Clear(m_ignoreConditionStates);
    return m_conditionStateMap.Find_Best_Info(m_conditionStates, set_flags);
}

static const char *s_theWeaponSlotTypeNames[] = { "PRIMARY", "SECONDARY", "TERTIARY", nullptr };

void Parse_Ascii_String_LC(INI *ini, void *formal, void *store, void const *user_data)
{
    Utf8String *store_str = static_cast<Utf8String *>(store);
    *store_str = ini->Get_Next_Ascii_String();
    store_str->To_Lower();
}

void W3DModelDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static const FieldParse dataFieldParse[] = {
        { "InitialRecoilSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(W3DModelDrawModuleData, m_initialRecoil) },
        { "MaxRecoilDistance", &INI::Parse_Real, nullptr, offsetof(W3DModelDrawModuleData, m_maxRecoil) },
        { "RecoilDamping", &INI::Parse_Real, nullptr, offsetof(W3DModelDrawModuleData, m_recoilDamping) },
        { "RecoilSettleSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(W3DModelDrawModuleData, m_recoilSettle) },
        { "OkToChangeModelColor", &INI::Parse_Bool, nullptr, offsetof(W3DModelDrawModuleData, m_okToChangeModelColor) },
        { "AnimationsRequirePower", &INI::Parse_Bool, nullptr, offsetof(W3DModelDrawModuleData, m_animationsRequirePower) },
        { "ParticlesAttachedToAnimatedBones", &INI::Parse_Bool, nullptr, offsetof(W3DModelDrawModuleData, m_particlesAttachedToAnimatedBones) },
        { "MinLODRequired", &GameLODManager::Parse_Static_Game_LOD_Level, nullptr, offsetof(W3DModelDrawModuleData, m_minLodRequired) },
        { "ProjectileBoneFeedbackEnabledSlots", &INI::Parse_Bitstring32, s_theWeaponSlotTypeNames, offsetof(W3DModelDrawModuleData, m_projectileBoneFeedbackEnabledSlots) },
        { "DefaultConditionState", &W3DModelDrawModuleData::Parse_Condition_State, reinterpret_cast<const void *>(PARSE_DEFAULT), 0 },
        { "ConditionState", &W3DModelDrawModuleData::Parse_Condition_State, reinterpret_cast<const void *>(PARSE_NORMAL), 0 },
        { "AliasConditionState", &W3DModelDrawModuleData::Parse_Condition_State, reinterpret_cast<const void *>(PARSE_ALIAS), 0 },
        { "TransitionState", &W3DModelDrawModuleData::Parse_Condition_State, reinterpret_cast<const void *>(PARSE_TRANSITION), 0 },
        { "TrackMarks", &Parse_Ascii_String_LC, nullptr, offsetof(W3DModelDrawModuleData, m_trackFile) },
        { "ExtraPublicBone", &INI::Parse_AsciiString_Vector_Append, nullptr, offsetof(W3DModelDrawModuleData, m_extraPublicBone) },
        { "AttachToBoneInAnotherModule", &Parse_Ascii_String_LC, nullptr, offsetof(W3DModelDrawModuleData, m_attachToBoneInAnotherModule) },
        { "IgnoreConditionStates", &BitFlags<117ul>::Parse_From_INI, nullptr, offsetof(W3DModelDrawModuleData, m_ignoreConditionStates) },
        { "ReceivesDynamicLights", &INI::Parse_Bool, nullptr, offsetof(W3DModelDrawModuleData, m_recievesDynamicLights) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ModuleData::Build_Field_Parse(p);
    p.Add(dataFieldParse, 0);
}

void W3DModelDrawModuleData::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 1;
    xfer->xferVersion(&ver, 1);

    for (auto &model_condition : m_conditionStates) {
        xfer->xferByte(&model_condition.m_validStuff);
#ifdef GAME_DEBUG_STRUCTS
        xfer->xferAsciiString(&model_condition.m_description);
#endif

        if (model_condition.m_validStuff) {
            for (auto &bone_pair : model_condition.m_boneMap) {
                xfer->xferInt(&bone_pair.second.index);
                xfer->xferUser(static_cast<void *>(&bone_pair.second.transform), sizeof(bone_pair.second.transform));
            }

            for (int j = 0; j < MAX_TURRETS; j++) {
                xfer->xferInt(&model_condition.m_turretInfo[j].m_turretAngleBone);
                xfer->xferInt(&model_condition.m_turretInfo[j].m_turretPitchBone);
            }

            for (int j = 0; j < WEAPONSLOT_COUNT; j++) {
                for (auto &weapon_barrel : model_condition.m_weaponBarrelInfoVec[j]) {
                    xfer->xferUser(static_cast<void *>(&weapon_barrel.m_weaponLaunchBoneTransform),
                        sizeof(weapon_barrel.m_weaponLaunchBoneTransform));
                }
            }
        }
    }
}

bool Does_State_Exist(std::vector<ModelConditionInfo> const &v, BitFlags<MODELCONDITION_COUNT> const &flags)
{
    for (auto &model_condition : v) {
        for (int count = model_condition.Get_Conditions_Count() - 1; count >= 0; count--) {
            if (flags == model_condition.Get_Conditions_Yes(count)) {
                return true;
            }
        }
    }

    return false;
}

uint64_t Make_Transition(int old_state, int new_state)
{
    return new_state | (uint64_t)((int64_t)old_state << 32);
}

void Parse_Bone_Name_Key(INI *ini, void *instance, void *store, void const *user_data)
{
    Utf8String str(ini->Get_Next_Token());
    str.To_Lower();

    if (instance != nullptr) {
        static_cast<ModelConditionInfo *>(instance)->Add_Public_Bone(str);
    }

    if (str.Is_Empty() || str.Is_None()) {
        *static_cast<NameKeyType *>(store) = NAMEKEY_INVALID;
    } else {
        *static_cast<NameKeyType *>(store) = g_theNameKeyGenerator->Name_To_Key(str.Str());
    }
}

void Parse_Show_Hide_Sub_Object(INI *ini, void *instance, void *store, void const *user_data)
{
    auto *v = static_cast<std::vector<ModelConditionInfo::HideShowSubObjInfo> *>(store);
    Utf8String str = ini->Get_Next_Ascii_String();
    str.To_Lower();

    if (str.Is_None()) {
        v->clear();
    } else {
        while (str.Is_Not_Empty()) {
            bool found = false;

            for (auto &i : *v) {
                if (!strcasecmp(i.sub_obj_name.Str(), str.Str())) {
                    i.hide = (uintptr_t)user_data != 0;
                    found = true;
                }
            }

            if (!found) {
                ModelConditionInfo::HideShowSubObjInfo info;
                info.sub_obj_name = str;
                info.hide = (uintptr_t)user_data != 0;
                v->push_back(info);
            }

            str = ini->Get_Next_Ascii_String();
            str.To_Lower();
        }
    }
}

void Parse_Weapon_Bone_Name(INI *ini, void *instance, void *store, void const *user_data)
{
    Utf8String *str = static_cast<Utf8String *>(store);
    int index = ini->Scan_IndexList(ini->Get_Next_Token(), s_theWeaponSlotTypeNames);
    str[index] = ini->Get_Next_Ascii_String();
    str[index].To_Lower();

    if (str[index].Is_None()) {
        str[index].Clear();
    }

    if (instance != nullptr) {
        static_cast<ModelConditionInfo *>(instance)->Add_Public_Bone(str[index]);
    }
}

void Parse_Animation(INI *ini, void *instance, void *store, void const *user_data)
{
    ModelConditionInfo *minfo = static_cast<ModelConditionInfo *>(instance);
    Utf8String name = ini->Get_Next_Ascii_String();
    name.To_Lower();
    const char *str = ini->Get_Next_Token_Or_Null();
    float distance;

    if (str != nullptr) {
        distance = ini->Scan_Real(str);
    } else {
        distance = 0.0f;
    }

    captainslog_dbgassert((uintptr_t)user_data != 1 || distance == 0.0f,
        "You should not specify nonzero DistanceCovered values for Idle Anims");

    str = ini->Get_Next_Token_Or_Null();
    int count = std::min(1, str ? ini->Scan_Int(str) : 1);
    W3DAnimationInfo info(name, (uintptr_t)user_data == 1, distance);

    if ((minfo->m_iniReadFlags & INIREAD_CONDITION_STATE) != 0) {
        minfo->m_iniReadFlags &= ~(INIREAD_CONDITION_STATE | INIREAD_ANIMS | INIREAD_IDLE_ANIMS);
        minfo->m_animations.clear();
    }

    if (info.Is_Idle_Anim()) {
        minfo->m_iniReadFlags |= INIREAD_IDLE_ANIMS;
    } else {
        minfo->m_iniReadFlags |= INIREAD_ANIMS;
    }

    if (!name.Is_Empty() && !name.Is_None()) {
        while (count-- != 0) {
            minfo->m_animations.push_back(info);
        }
    }
}

void Parse_Lowercase_Name_Key(INI *ini, void *formal, void *store, void const *user_data)
{
    Utf8String str(ini->Get_Next_Token());
    str.To_Lower();
    *static_cast<NameKeyType *>(store) = g_theNameKeyGenerator->Name_To_Key(str.Str());
}

void Parse_Particle_Sys_Bone(INI *ini, void *instance, void *store, void const *user_data)
{
    ParticleSysBoneInfo info;
    info.bone_name = ini->Get_Next_Ascii_String();
    info.bone_name.To_Lower();
    ParticleSystemTemplate::Parse(ini, instance, &info.particle_system_template, user_data);
    static_cast<ModelConditionInfo *>(instance)->m_ParticleSysBones.push_back(info);
}

void Parse_Real_Range(INI *ini, void *instance, void *store, void const *user_data)
{
    ModelConditionInfo *info = static_cast<ModelConditionInfo *>(instance);
    info->m_animationSpeedFactorMin = ini->Scan_Real(ini->Get_Next_Token());
    info->m_animationSpeedFactorMax = ini->Scan_Real(ini->Get_Next_Token());
}

static const char *s_theAnimModeNames[] = {
    "MANUAL", "LOOP", "ONCE", "LOOP_PINGPONG", "LOOP_BACKWARDS", "ONCE_BACKWARDS", nullptr
};

static const char *s_ACBitsNames[] = { "RANDOMSTART",
    "START_FRAME_FIRST",
    "START_FRAME_LAST",
    "ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT",
    "PRISTINE_BONE_POS_IN_FINAL_FRAME",
    "MAINTAIN_FRAME_ACROSS_STATES",
    "RESTART_ANIM_WHEN_COMPLETE",
    "MAINTAIN_FRAME_ACROSS_STATES2",
    "MAINTAIN_FRAME_ACROSS_STATES3",
    "MAINTAIN_FRAME_ACROSS_STATES4",
    nullptr };

void W3DModelDrawModuleData::Parse_Condition_State(INI *ini, void *instance, void *store, void const *user_data)
{
    ModelConditionInfo info;
    W3DModelDrawModuleData *data = static_cast<W3DModelDrawModuleData *>(instance);

    switch ((uintptr_t)user_data) {
        case PARSE_NORMAL: {
            if (data->m_stateCount >= 0 && (uintptr_t)user_data != PARSE_ALIAS) {
                info = data->m_conditionStates.at(data->m_stateCount);
                info.m_iniReadFlags |= INIREAD_CONDITION_STATE;
                info.m_conditionsYesVec.clear();
            }

            BitFlags<MODELCONDITION_COUNT> flags;
            Utf8String str;
            flags.Parse(ini, &str);

#ifdef GAME_DEBUG_STRUCTS
            info.m_description.Clear();
            info.m_description += s_theThingTemplateBeingParsedName;
            info.m_description.Concat("\n         ");
            info.m_description += str;
#endif
            if (flags.Any_Intersection_With(data->m_ignoreConditionStates)) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug("You should not specify bits in a state once they are used in IgnoreConditionStates (%s)",
                    s_theThingTemplateBeingParsedName.Str());
#endif
                throw CODE_06;
            }

            if (data->m_stateCount < 0 && data->m_conditionStates.empty() && flags.Any()) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug("*** ASSET ERROR: when not using DefaultConditionState, the first ConditionState must be "
                                  "for NONE (%s)",
                    s_theThingTemplateBeingParsedName.Str());
#endif
                throw CODE_06;
            }

            captainslog_relassert(flags.Any() || data->m_stateCount < 0,
                CODE_06,
                "*** ASSET ERROR: you may not specify both a Default state and a Conditions=None state");

            if (Does_State_Exist(data->m_conditionStates, flags)) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug(
                    "*** ASSET ERROR: duplicate condition states are not currently allowed (%s)", info.m_description.Str());
#endif
                throw CODE_06;
            }

            captainslog_relassert(
                !info.m_conditionsYesVec.size(), CODE_06, "*** ASSET ERROR: nonempty m_conditionsYesVec.size()");

            info.m_conditionsYesVec.clear();
            info.m_conditionsYesVec.push_back(flags);
            break;
        }
        case PARSE_DEFAULT: {
            captainslog_relassert(data->m_stateCount < 0, CODE_06, "*** ASSET ERROR: you may have only one default state!");
            captainslog_relassert(!ini->Get_Next_Token_Or_Null(), CODE_06, "*** ASSET ERROR: unknown keyword");

            if (!data->m_conditionStates.empty()) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug(
                    "*** ASSET ERROR: when using DefaultConditionState, it must be the first state listed (%s)",
                    s_theThingTemplateBeingParsedName.Str());
#endif
                throw CODE_06;
            }

            data->m_stateCount = data->m_conditionStates.size();
            BitFlags<MODELCONDITION_COUNT> flags;
            info.m_conditionsYesVec.clear();
            info.m_conditionsYesVec.push_back(flags);
#ifdef GAME_DEBUG_STRUCTS
            info.m_description.Clear();
            info.m_description += s_theThingTemplateBeingParsedName;
            info.m_description.Concat(" DEFAULT");
#endif
            break;
        }
        case PARSE_TRANSITION: {
            Utf8String str(ini->Get_Next_Token());
            str.To_Lower();
            Utf8String str2(ini->Get_Next_Token());
            str2.To_Lower();
            NameKeyType key = g_theNameKeyGenerator->Name_To_Key(str.Str());
            NameKeyType key2 = g_theNameKeyGenerator->Name_To_Key(str2.Str());
            captainslog_relassert(
                key != key2, CODE_06, "*** ASSET ERROR: You may not declare a transition between two identical states");

            if (data->m_stateCount >= 0) {
                info = data->m_conditionStates.at(data->m_stateCount);
                info.m_iniReadFlags |= INIREAD_CONDITION_STATE;
                info.m_transitionKey = NAMEKEY_INVALID;
                info.m_allowToFinishKey = NAMEKEY_INVALID;
            }

            info.m_transition = Make_Transition(key, key2);
#ifdef GAME_DEBUG_STRUCTS
            info.m_description.Clear();
            info.m_description += s_theThingTemplateBeingParsedName;
            info.m_description.Concat(" TRANSITION: ");
            info.m_description += str;
            info.m_description.Concat(" ");
            info.m_description += str2;
#endif
            break;
        }
        case PARSE_ALIAS: {
            captainslog_relassert(!data->m_conditionStates.empty(),
                CODE_06,
                "*** ASSET ERROR: AliasConditionState must refer to the previous state!");
            ModelConditionInfo &info2 = data->m_conditionStates.at(data->m_conditionStates.size() - 1);
            BitFlags<MODELCONDITION_COUNT> flags;
            Utf8String str;
            flags.Parse(ini, &str);
#ifdef GAME_DEBUG_STRUCTS
            info2.m_description.Concat("\nAKA: ");
            info2.m_description += str;
#endif

            if (flags.Any_Intersection_With(data->m_ignoreConditionStates)) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug("You should not specify bits in a state once they are used in IgnoreConditionStates (%s)",
                    s_theThingTemplateBeingParsedName.Str());
#endif
                throw CODE_06;
            }

            captainslog_relassert(!Does_State_Exist(data->m_conditionStates, flags),
                CODE_06,
                "*** ASSET ERROR: duplicate condition states are not currently allowed");

            captainslog_relassert(flags.Any() || data->m_stateCount < 0,
                CODE_06,
                "*** ASSET ERROR: you may not specify both a Default state and a Conditions=None state");

            info2.m_conditionsYesVec.push_back(flags);
            return;
        }
    }

    // clang-format off
    static const FieldParse myFieldParse[] = {
        { "Model", &Parse_Ascii_String_LC, nullptr, offsetof(ModelConditionInfo, m_modelName) },
        { "Turret", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretAngleName) },
        { "TurretArtAngle", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretArtAngle) },
        { "TurretPitch", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretPitchName) },
        { "TurretArtPitch", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretArtPitch) },
        { "AltTurret", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretAngleName) },
        { "AltTurretArtAngle", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretArtAngle) },
        { "AltTurretPitch", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretPitchName) },
        { "AltTurretArtPitch", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretArtPitch) },
        { "ShowSubObject", &Parse_Show_Hide_Sub_Object, reinterpret_cast<const void *>(0), offsetof(ModelConditionInfo, m_hideShowVec) },
        { "HideSubObject", &Parse_Show_Hide_Sub_Object, reinterpret_cast<const void *>(1), offsetof(ModelConditionInfo, m_hideShowVec) },
        { "WeaponFireFXBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponFireFXBoneName) },
        { "WeaponRecoilBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponRecoilBoneName) },
        { "WeaponMuzzleFlash", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponMuzzleFlashName) },
        { "WeaponLaunchBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponLaunchBoneName) },
        { "WeaponHideShowBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponHideShowBoneName) },
        { "Animation", &Parse_Animation, reinterpret_cast<const void *>(0), offsetof(ModelConditionInfo, m_animations) },
        { "IdleAnimation", &Parse_Animation, reinterpret_cast<const void *>(1), offsetof(ModelConditionInfo, m_animations) },
        { "AnimationMode", &INI::Parse_Index_List, s_theAnimModeNames, offsetof(ModelConditionInfo, m_mode) },
        { "TransitionKey", &Parse_Lowercase_Name_Key, nullptr, offsetof(ModelConditionInfo, m_transitionKey) },
        { "WaitForStateToFinishIfPossible", &Parse_Lowercase_Name_Key, nullptr, offsetof(ModelConditionInfo, m_allowToFinishKey) },
        { "Flags", &INI::Parse_Bitstring32, &s_ACBitsNames, offsetof(ModelConditionInfo, m_flags) },
        { "ParticleSysBone", &Parse_Particle_Sys_Bone, nullptr, 0 },
        { "AnimationSpeedFactorRange", &Parse_Real_Range, nullptr, 0 },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ini->Init_From_INI(&info, myFieldParse);

    captainslog_relassert(!info.m_modelName.Is_Empty(), CODE_06, "*** ASSET ERROR: you must specify a model name");

    if (info.m_modelName.Is_None()) {
        info.m_modelName.Clear();
    }

    captainslog_relassert((info.m_iniReadFlags & INIREAD_IDLE_ANIMS) == 0 || (info.m_iniReadFlags & INIREAD_ANIMS) == 0,
        CODE_06,
        "*** ASSET ERROR: you should not specify both Animations and IdleAnimations for the same state");

    if ((info.m_iniReadFlags & INIREAD_IDLE_ANIMS) != 0 && info.m_mode != RenderObjClass::ANIM_MODE_ONCE
        && info.m_mode != RenderObjClass::ANIM_MODE_ONCE_BACKWARDS) {
#ifdef GAME_DEBUG_STRUCTS
        captainslog_debug("*** ASSET ERROR: Idle Anims should always use ONCE or ONCE_BACKWARDS (%s)",
            s_theThingTemplateBeingParsedName.Str());
#endif
        throw CODE_06;
    }

    info.m_validStuff &= ~LAUNCH_BONES_VALID;

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (info.m_weaponLaunchBoneName[i].Is_Not_Empty()) {
            info.m_validStuff |= LAUNCH_BONES_VALID;
            break;
        }
    }

    if ((uintptr_t)user_data == PARSE_TRANSITION) {
        captainslog_relassert((info.m_iniReadFlags & INIREAD_IDLE_ANIMS) == 0,
            CODE_06,
            "*** ASSET ERROR: Transition States should not specify Idle anims");

        captainslog_relassert(
            info.m_mode == RenderObjClass::ANIM_MODE_ONCE || info.m_mode == RenderObjClass::ANIM_MODE_ONCE_BACKWARDS,
            CODE_06,
            "*** ASSET ERROR: Transition States should always use ONCE or ONCE_BACKWARDS");

        captainslog_relassert(!info.m_transitionKey && !info.m_allowToFinishKey,
            CODE_06,
            "*** ASSET ERROR: Transition States must not have transition keys or m_allowToFinishKey");

        data->m_transitionMap[info.m_transition] = info;
    } else {
        data->m_conditionStates.push_back(info);
    }
}

W3DModelDraw::W3DModelDraw(Thing *thing, ModuleData const *module_data) :
    DrawModule(thing, module_data),
    m_curState(nullptr),
    m_nextState(nullptr),
    m_loopDuration(-1),
    m_hexColor(0),
    m_whichAnimInCurState(-1),
    m_recalcBones(false),
    m_fullyObscuredByShroud(false),
    m_hidden(true),
    m_renderObject(nullptr),
    m_shadow(nullptr),
    m_decalShadow(nullptr),
    m_trackRenderObject(nullptr),
    m_isDaytime(true),
    m_pauseAnimation(false),
    m_animMode(RenderObjClass::ANIM_MODE_LOOP)
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        m_weaponRecoilInfoVec[i].clear();
    }

    Get_W3D_Model_Draw_Module_Data()->Validate_Stuff_For_Time_And_Weather(Get_Drawable(),
        g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT,
        g_theWriteableGlobalData->m_weather == WEATHER_SNOWY);

    BitFlags<MODELCONDITION_COUNT> flags;
    const ModelConditionInfo *idle = Find_Best_Info(flags);
    captainslog_relassert(idle, CODE_06, "*** ASSET ERROR: all draw modules must have an IDLE state");

    Drawable *drawable = Get_Drawable();

    if (drawable != nullptr) {
        Object *object = drawable->Get_Object();

        if (object != nullptr) {
            if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
                m_hexColor = object->Get_Night_Indicator_Color();
            } else {
                m_hexColor = object->Get_Indicator_Color();
            }
        }

        if (!Get_W3D_Model_Draw_Module_Data()->m_recievesDynamicLights) {
            drawable->Set_Recieves_Dynamic_Lights(false);
            captainslog_debug("Set_Recieves_Dynamic_Lights = FALSE: %s", drawable->Get_Template()->Get_Name().Str());
        }
    }

    Set_Model_State(idle);
}

W3DModelDraw::~W3DModelDraw()
{
    if (m_trackRenderObject != nullptr && g_theTerrainTracksRenderObjClassSystem != nullptr) {
        g_theTerrainTracksRenderObjClassSystem->Unbind_Track(m_trackRenderObject);
        m_trackRenderObject = nullptr;
    }

    Nuke_Current_Render(nullptr);
}

void W3DModelDraw::On_Drawable_Bound_To_Object()
{
    Get_W3D_Model_Draw_Module_Data()->Validate_Stuff_For_Time_And_Weather(Get_Drawable(),
        g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT,
        g_theWriteableGlobalData->m_weather == WEATHER_SNOWY);
}

void W3DModelDraw::Do_Start_Or_Stop_Particle_Sys()
{
    Drawable *drawable = Get_Drawable();
    bool hidden = drawable->Is_Hidden() || m_fullyObscuredByShroud;

    for (auto &tracker : m_particleSystemIDs) {
        ParticleSystem *psys = g_theParticleSystemManager->Find_Particle_System(tracker.id);

        if (psys != nullptr) {
            if (hidden) {
                psys->Stop();
            } else {
                psys->Start();
            }
        }
    }
}

void W3DModelDraw::Set_Hidden(bool hidden)
{
    if (m_renderObject != nullptr) {
        m_renderObject->Set_Hidden(hidden);
    }

    if (m_shadow != nullptr) {
        m_shadow->Enable_Shadow_Render(!hidden);
    }

    m_hidden = hidden;

    if (m_decalShadow != nullptr) {
        m_decalShadow->Enable_Shadow_Render(!hidden);
    }

    if (m_trackRenderObject != nullptr && hidden) {
        const Coord3D *pos = Get_Drawable()->Get_Position();
        m_trackRenderObject->Add_Cap_Edge_To_Track(pos->x, pos->y);
    }

    Do_Start_Or_Stop_Particle_Sys();
}

void W3DModelDraw::Release_Shadows()
{
    if (m_shadow != nullptr) {
        m_shadow->Release();
        m_shadow = nullptr;
    }
}

void W3DModelDraw::Allocate_Shadows()
{
    Shadow::ShadowTypeInfo info;
    const ThingTemplate *thing = Get_Drawable()->Get_Template();

    if (m_shadow == nullptr && m_renderObject != nullptr && g_theW3DShadowManager != nullptr
        && thing->Get_Shadow_Type() != SHADOW_NONE) {

        strcpy(info.m_shadowName, thing->Get_Shadow_Texture_Name().Str());
        captainslog_dbgassert(info.m_shadowName[0], "this should be validated in ThingTemplate now");

        info.m_allowUpdates = false;
        info.m_allowWorldAlign = true;
        info.m_type = (ShadowType)thing->Get_Shadow_Type();
        info.m_sizeX = thing->Get_Shadow_Size_X();
        info.m_sizeY = thing->Get_Shadow_Size_Y();
        info.m_offsetX = thing->Get_Shadow_Offset_X();
        info.m_offsetY = thing->Get_Shadow_Offset_Y();

        m_shadow = g_theW3DShadowManager->Add_Shadow(m_renderObject, &info, nullptr);

        if (m_shadow != nullptr) {
            m_shadow->Enable_Shadow_Invisible(m_fullyObscuredByShroud);

            if (m_renderObject->Is_Hidden() || !m_hidden) {
                m_shadow->Enable_Shadow_Render(false);
            }
        }
    }
}

void W3DModelDraw::Set_Shadows_Enabled(bool enable)
{
    if (m_shadow != nullptr) {
        m_shadow->Enable_Shadow_Render(enable);
    }

    m_hidden = enable;
}

#ifdef GAME_DEBUG_STRUCTS
void W3DModelDraw::Gather_Draw_Stats(DebugDrawStats *stats)
{
    Gather_Draw_Stats_For_Render_Object(stats, m_renderObject);

    if (m_shadow != nullptr) {
        m_shadow->Gather_Draw_Stats(stats);
    }
}
#endif

void W3DModelDraw::Set_Fully_Obscured_By_Shroud(bool obscured)
{
    if (m_fullyObscuredByShroud != obscured) {
        m_fullyObscuredByShroud = obscured;

        if (m_shadow != nullptr) {
            m_shadow->Enable_Shadow_Invisible(m_fullyObscuredByShroud);
        }

        if (m_decalShadow != nullptr) {
            m_decalShadow->Enable_Shadow_Invisible(m_fullyObscuredByShroud);
        }

        Do_Start_Or_Stop_Particle_Sys();
    }
}

void W3DModelDraw::Adjust_Transform_Mtx(Matrix3D &transform) const
{
    Vector3 *offset = Get_W3D_Model_Draw_Module_Data()->Get_Attach_To_Drawable_Bone_Offset(Get_Drawable());

    if (offset != nullptr) {
        Vector3 v = transform.Rotate_Vector(*offset);
        transform.Adjust_X_Translation(v.X);
        transform.Adjust_Y_Translation(v.Y);
        transform.Adjust_Z_Translation(v.Z);
    }

    if (Test_Animation_Flag(m_curState->m_flags, ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT)) {
        const Object *object = Get_Drawable()->Get_Object();

        if (object != nullptr) {
            float percent = object->Get_Construction_Percent();

            if (percent >= 0.0f) {
                float height = object->Get_Geometry_Info().Get_Max_Height_Above_Position();
                transform.Translate_Z(height * percent / 100.0f - height);
            }
        }
    }
}

bool Is_Animation_Complete(RenderObjClass *r)
{
    if (r != nullptr && r->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        return static_cast<HLodClass *>(r)->Is_Animation_Complete();
    } else {
        return true;
    }
}

void W3DModelDraw::Do_Draw_Module(const Matrix3D *transform)
{
    Set_Pause_Animation(!Get_Drawable()->Get_Should_Animate(Get_W3D_Model_Draw_Module_Data()->m_animationsRequirePower));

    if (Get_Drawable()->Get_Instance_Scale() != 1.0f) {
        Matrix3D m;
        m = *transform;
        m.Scale(Get_Drawable()->Get_Instance_Scale());
        transform = &m;

        if (m_renderObject != nullptr) {
            m_renderObject->Set_ObjectScale(Get_Drawable()->Get_Instance_Scale());
        }
    }

    if (Is_Animation_Complete(m_renderObject)) {
        if (m_curState != nullptr) {
            if (m_nextState != nullptr) {
                const ModelConditionInfo *info = m_nextState;
                int loop = m_loopDuration;
                m_nextState = nullptr;
                m_loopDuration = -1;
                Set_Model_State(info);

                if (loop != -1) {
                    Set_Animation_Loop_Duration(loop);
                }
            }
        }

        if (m_renderObject != nullptr) {
            if (m_curState != nullptr) {
                if (m_whichAnimInCurState != -1) {
                    if (m_curState->m_animations[m_whichAnimInCurState].Is_Idle_Anim()
                        || Test_Animation_Flag(m_curState->m_flags, RESTART_ANIM_WHEN_COMPLETE)) {
                        Adjust_Animation(m_curState, -1.0f);
                    }
                }
            }
        }
    }

    Adjust_Anim_Speed_To_Movement_Speed();

    if (m_renderObject != nullptr) {
        Matrix3D m = *transform;
        Adjust_Transform_Mtx(m);
        m_renderObject->Set_Transform(m);
    }

    Handle_Client_Turret_Positioning();
    Recalc_Bones_For_Client_Particle_Systems();

    if (Get_W3D_Model_Draw_Module_Data()->m_particlesAttachedToAnimatedBones) {
        Update_Bones_For_Client_Particle_Systems();
    }

    Handle_Client_Recoil();
}

const ModelConditionInfo *W3DModelDraw::Find_Transition_For_Sig(uint64_t sig) const
{
    auto trans = Get_W3D_Model_Draw_Module_Data()->m_transitionMap.find(sig);

    if (trans != Get_W3D_Model_Draw_Module_Data()->m_transitionMap.end()) {
        return &trans->second;
    } else {
        return nullptr;
    }
}

namespace
{
constexpr int g_maintain_frame_across_states_flags = (1 << MAINTAIN_FRAME_ACROSS_STATES)
    | (1 << MAINTAIN_FRAME_ACROSS_STATES2) | (1 << MAINTAIN_FRAME_ACROSS_STATES3)
    | (1 << MAINTAIN_FRAME_ACROSS_STATES4); // 0x3A0
}

bool Maintain_Frame_Across_States(int flags)
{
    return (flags & g_maintain_frame_across_states_flags) != 0;
}

bool Maintain_Frame_Across_States_2(int flags, int flags2)
{
    return (flags2 & flags & g_maintain_frame_across_states_flags) != 0;
}

float W3DModelDraw::Get_Current_Anim_Fraction() const
{
    if (m_curState == nullptr || !Maintain_Frame_Across_States(m_curState->m_flags) || !m_renderObject
        || m_renderObject->Class_ID() != RenderObjClass::CLASSID_HLOD) {
        return -1.0f;
    }

    HLodClass *hlod = static_cast<HLodClass *>(m_renderObject);

    float frame;
    int frames;
    int mode;
    float multiplier;
    hlod->Peek_Animation_And_Info(frame, frames, mode, multiplier);

    if (frame < 0.0f) {
        return 0.0f;
    }

    if (frames > frame) {
        return frame / (frames - 1.0f);
    }

    return 1.0f;
}

void W3DModelDraw::Adjust_Animation(ModelConditionInfo const *prev_state, float f)
{
    if (m_curState != nullptr) {
        int count = m_curState->m_animations.size();

        if (count > 0) {
            if (count == 1) {
                m_whichAnimInCurState = 0;
            } else if (prev_state == m_curState) {
                int which = m_whichAnimInCurState;

                while (m_whichAnimInCurState == which) {
                    m_whichAnimInCurState = Get_Client_Random_Value(0, count - 1);
                }
            } else {
                m_whichAnimInCurState = Get_Client_Random_Value(0, count - 1);
            }

            // TODO possible leak here
            HAnimClass *anim = m_curState->m_animations[m_whichAnimInCurState].Get_Anim_Handle();

            if (m_renderObject != nullptr && anim != nullptr) {
                int frame = 0;

                if (m_curState->m_mode == RenderObjClass::ANIM_MODE_ONCE_BACKWARDS
                    || m_curState->m_mode == RenderObjClass::ANIM_MODE_LOOP_BACKWARDS) {
                    frame = anim->Get_Num_Frames() - 1;
                }

                if (Test_Animation_Flag(m_curState->m_flags, RANDOMSTART)) {
                    frame = Get_Client_Random_Value(0, anim->Get_Num_Frames() - 1);
                } else if (Test_Animation_Flag(m_curState->m_flags, START_FRAME_FIRST)) {
                    frame = 0;
                } else if (Test_Animation_Flag(m_curState->m_flags, START_FRAME_LAST)) {
                    frame = anim->Get_Num_Frames() - 1;
                } else if (Maintain_Frame_Across_States(m_curState->m_flags) && prev_state != nullptr
                    && prev_state != m_curState && Maintain_Frame_Across_States(prev_state->m_flags)
                    && Maintain_Frame_Across_States_2(m_curState->m_flags, prev_state->m_flags) && f >= 0.0f) {
                    frame = GameMath::Fast_To_Int_Truncate(anim->Get_Num_Frames() * f - 1.0f);
                }

                m_renderObject->Set_Animation(anim, frame, m_curState->m_mode);
                anim->Release_Ref();

                if (m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD) {
                    HLodClass *hlod = static_cast<HLodClass *>(m_renderObject);
                    hlod->Set_Animation_Frame_Rate_Multiplier(Get_Client_Random_Value_Real(
                        m_curState->m_animationSpeedFactorMin, m_curState->m_animationSpeedFactorMax));
                }
            }
        } else {
            m_whichAnimInCurState = -1;
        }
    }
}

bool W3DModelDraw::Set_Cur_Anim_Duration_In_Msec(float desired_duration_in_msec)
{
    if (m_renderObject == nullptr || m_renderObject->Class_ID() != RenderObjClass::CLASSID_HLOD) {
        return false;
    }

    HLodClass *hlod = static_cast<HLodClass *>(m_renderObject);
    HAnimClass *anim = hlod->Peek_Animation();

    if (anim == nullptr) {
        return false;
    }

    float f = (anim->Get_Num_Frames() * 1000.0f) / anim->Get_Frame_Rate();

    if (f > 0.0f && desired_duration_in_msec > 0.0f) {
        hlod->Set_Animation_Frame_Rate_Multiplier(f / desired_duration_in_msec);
        return true;
    }

    return false;
}

float W3DModelDraw::Get_Cur_Anim_Distance_Covered() const
{
    if (m_curState == nullptr || m_whichAnimInCurState < 0) {
        return 0.0f;
    }

#ifdef GAME_DEBUG_STRUCTS
    if (s_skateDistanceOverride != 0.0f) {
        return s_skateDistanceOverride;
    } else {
        return m_curState->m_animations[m_whichAnimInCurState].Get_Distance_Covered();
    }
#else
    return m_curState->m_animations[m_whichAnimInCurState].Get_Distance_Covered();
#endif
}

void Do_Hide_Show_Bone_Sub_Objs(
    bool state, int num_sub_objects, int bone_idx, RenderObjClass *full_object, HTreeClass const *htree)
{
    for (int i = 0; i < num_sub_objects; i++) {
        bool found = false;
        int index = full_object->Get_Sub_Object_Bone_Index(0, i);

        while (index != 0) {
            index = htree->Get_Parent_Index(index);

            if (index == bone_idx) {
                found = true;
                break;
            }
        }

        if (found) {
            RenderObjClass *robj = full_object->Get_Sub_Object(i);
            robj->Set_Hidden(state);
            robj->Release_Ref();
        }
    }
}

void W3DModelDraw::Do_Hide_Show_Sub_Objs(std::vector<ModelConditionInfo::HideShowSubObjInfo> const *vec)
{
    if (m_renderObject != nullptr) {
        if (!vec->empty()) {
            for (auto &info : *vec) {
                int index;
                RenderObjClass *robj = m_renderObject->Get_Sub_Object_By_Name(info.sub_obj_name.Str(), &index);

                if (robj != nullptr) {
                    robj->Set_Hidden(info.hide);
                    const HTreeClass *tree = m_renderObject->Get_HTree();

                    if (tree != nullptr) {
                        Do_Hide_Show_Bone_Sub_Objs(info.hide,
                            m_renderObject->Get_Num_Sub_Objects(),
                            m_renderObject->Get_Sub_Object_Bone_Index(0, index),
                            m_renderObject,
                            tree);
                    }

                    robj->Release_Ref();
                } else {
                    captainslog_dbgassert(robj == nullptr,
                        "*** ASSET ERROR: SubObject %s not found (%s)!",
                        info.sub_obj_name.Str(),
                        Get_Drawable()->Get_Template()->Get_Name().Str());
                }
            }
        }

        if (!m_subObjects.empty()) {
            Update_Sub_Objects();
        }
    }
}

void W3DModelDraw::Stop_Client_Particle_Systems()
{
    for (auto &tracker : m_particleSystemIDs) {
        ParticleSystem *psys = g_theParticleSystemManager->Find_Particle_System(tracker.id);

        if (psys != nullptr) {
            psys->Destroy();
        }
    }

    m_particleSystemIDs.clear();
}

void W3DModelDraw::Handle_Client_Turret_Positioning()
{
    if (m_curState != nullptr && (m_curState->m_validStuff & TURRET_VALID) != 0) {
        for (int i = 0; i < MAX_TURRETS; i++) {
            ModelConditionInfo::TurretInfo *info = &m_curState->m_turretInfo[i];
            float turret_angle = 0.0f;
            float turret_pitch = 0.0f;

            if (info->m_turretAngleBone != 0 || info->m_turretPitchBone != 0) {
                Object *object = Get_Drawable()->Get_Object();

                if (object != nullptr) {
                    AIUpdateInterface *ai = object->Get_AI_Update_Interface();

                    if (ai != nullptr) {
                        ai->Get_Turret_Rot_And_Pitch((WhichTurretType)i, &turret_angle, &turret_pitch);
                    }
                }

                if (info->m_turretAngleBone != 0) {
                    if (m_curState != nullptr) {
                        turret_angle += info->m_turretArtAngle;
                    }

                    Matrix3D m(true);
                    m.Rotate_Z(turret_angle);

                    if (m_renderObject != nullptr) {
                        m_renderObject->Capture_Bone(info->m_turretAngleBone);
                        m_renderObject->Control_Bone(info->m_turretAngleBone, m);
                    }
                }

                if (info->m_turretPitchBone != 0) {
                    if (m_curState != nullptr) {
                        turret_pitch += info->m_turretArtPitch;
                    }

                    Matrix3D m(true);
                    m.Rotate_Y(-turret_pitch);

                    if (m_renderObject != nullptr) {
                        m_renderObject->Capture_Bone(info->m_turretPitchBone);
                        m_renderObject->Control_Bone(info->m_turretPitchBone, m);
                    }
                }
            }
        }
    }
}

void W3DModelDraw::Handle_Client_Recoil()
{
    const W3DModelDrawModuleData *data = Get_W3D_Model_Draw_Module_Data();

    if ((m_curState->m_validStuff & WEAPON_BARREL_INFO_VALID) != 0) {
        for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
            if (m_curState->m_hasWeaponBone[i]) {
                std::vector<ModelConditionInfo::WeaponBarrelInfo> &barrel_vec = m_curState->m_weaponBarrelInfoVec[i];
                std::vector<RecoilInfo> &recoil_vec = m_weaponRecoilInfoVec[i];
                int barrel_count = barrel_vec.size();
                int recoil_count = recoil_vec.size();
                captainslog_dbgassert(barrel_count == recoil_count, "Barrel count != recoil count!");

                if (barrel_count > recoil_count) {
                    barrel_count = recoil_count;
                }

                for (int j = 0; j < barrel_count; j++) {
                    if (barrel_vec[j].m_weaponMuzzleFlashBone) {
                        barrel_vec[j].Set_Muzzle_Flash_Hidden(
                            m_renderObject, recoil_vec[j].m_state != RecoilInfo::RECOIL_START);
                    }

                    float f = 0.01f;

                    if (barrel_vec[j].m_weaponRecoilBone) {
                        int state = recoil_vec[j].m_state;
                        if (state > RecoilInfo::IDLE) {
                            if (state <= RecoilInfo::RECOIL) {
                                recoil_vec[j].m_shift += recoil_vec[j].m_recoilRate;
                                recoil_vec[j].m_recoilRate *= data->m_recoilDamping;

                                if (recoil_vec[j].m_shift < data->m_maxRecoil) {
                                    if (f > GameMath::Fabs(recoil_vec[j].m_recoilRate)) {
                                        recoil_vec[j].m_state = RecoilInfo::SETTLE;
                                    }
                                } else {
                                    recoil_vec[j].m_shift = data->m_maxRecoil;
                                    recoil_vec[j].m_state = RecoilInfo::SETTLE;
                                }
                            } else if (state == RecoilInfo::SETTLE) {
                                recoil_vec[j].m_shift -= data->m_recoilSettle;
                                if (recoil_vec[j].m_shift <= 0.0f) {
                                    recoil_vec[j].m_shift = 0.0f;
                                    recoil_vec[j].m_state = RecoilInfo::IDLE;
                                }
                            }
                        }

                        Matrix3D m;
                        m.Make_Identity();
                        m.Translate_X(-recoil_vec[j].m_shift);

                        if (m_renderObject != nullptr) {
                            m_renderObject->Capture_Bone(barrel_vec[j].m_weaponRecoilBone);
                            m_renderObject->Control_Bone(barrel_vec[j].m_weaponRecoilBone, m);
                        }
                    } else {
                        recoil_vec[j].m_state = RecoilInfo::IDLE;
                    }
                }
            }
        }
    }
}

void W3DModelDraw::Recalc_Bones_For_Client_Particle_Systems()
{
    if (m_recalcBones) {
        Drawable *drawable = Get_Drawable();

        if (drawable != nullptr && m_curState != nullptr && !drawable->Check_Status_Bit(DRAWABLE_STATUS_8)) {
            for (auto &bone : m_curState->m_ParticleSysBones) {
                ParticleSystem *psys =
                    g_theParticleSystemManager->Create_Particle_System(bone.particle_system_template, true);

                if (psys != nullptr) {
                    Coord3D pos;
                    pos.Zero();
                    float rot = 0.0f;
                    int index;

                    if (m_renderObject != nullptr) {
                        index = m_renderObject->Get_Bone_Index(bone.bone_name.Str());
                    } else {
                        index = 0;
                    }

                    if (index != 0) {
                        Matrix3D m(m_renderObject->Get_Transform());
                        Matrix3D m2(true);
                        m2.Scale(Get_Drawable()->Get_Scale());
                        m_renderObject->Set_Transform(m2);
                        Matrix3D m3(m_renderObject->Get_Bone_Transform(index));
                        Vector3 v = m3.Get_Translation();
                        rot = m3.Get_Z_Rotation();
                        m_renderObject->Set_Transform(m);
                        pos.x = v.X;
                        pos.y = v.Y;
                        pos.z = v.Z;
                    }

                    psys->Set_Position(pos);
                    psys->Rotate_Local_Transform_Z(rot);
                    psys->Attach_To_Drawable(drawable);
                    psys->Set_Saveable(false);

                    if (drawable->Is_Hidden() || m_fullyObscuredByShroud) {
                        psys->Stop();
                    }

                    ParticleSysTrackerType tracker;
                    tracker.id = psys->Get_System_ID();
                    tracker.index = index;
                    m_particleSystemIDs.push_back(tracker);
                }
            }
        }

        m_recalcBones = false;
    }
}

bool W3DModelDraw::Update_Bones_For_Client_Particle_Systems()
{
    if (Get_Drawable() && m_curState != nullptr && m_renderObject != nullptr) {
        for (auto &tracker : m_particleSystemIDs) {
            ParticleSystem *psys = g_theParticleSystemManager->Find_Particle_System(tracker.id);
            int index = tracker.index;

            if (psys != nullptr) {
                if (index != 0) {
                    Matrix3D m(m_renderObject->Get_Bone_Transform(index));
                    Vector3 v = m.Get_Translation();
                    Coord3D pos;
                    pos.x = v.X;
                    pos.y = v.Y;
                    pos.z = v.Z;
                    psys->Set_Position(pos);
                    psys->Rotate_Local_Transform_Z(m.Get_Z_Rotation());
                    psys->Set_Local_Transform(m);
                    psys->Set_Unk(true);
                }
            }
        }
    }

    return true;
}

const char *const s_terrainDecalTextureName[] = { "TERRAIN_DECAL_DEMORALIZED_OBSOLETE",
    "EXHorde",
    "EXHorde_UP",
    "EXHordeB",
    "EXHordeB_UP",
    "EXJunkCrate",
    "EXHordeC_UP",
    "EXChemSuit",
    "",
    "" };

void W3DModelDraw::Set_Terrain_Decal(TerrainDecalType decal)
{
    if (m_decalShadow != nullptr) {
        m_decalShadow->Release();
        m_decalShadow = nullptr;
    }

    if (decal != TERRAIN_DECAL_8 && decal < TERRAIN_DECAL_COUNT) {
        const ThingTemplate *tmplate = Get_Drawable()->Get_Template();
        Shadow::ShadowTypeInfo info;
        info.m_allowUpdates = false;
        info.m_allowWorldAlign = true;
        info.m_type = SHADOW_ALPHA_DECAL;

        if (decal == TERRAIN_DECAL_9) {
            strcpy(info.m_shadowName, tmplate->Get_Shadow_Texture_Name().Str());
        } else {
            strcpy(info.m_shadowName, s_terrainDecalTextureName[decal]);
        }

        info.m_sizeX = tmplate->Get_Shadow_Size_X();
        info.m_sizeY = tmplate->Get_Shadow_Size_Y();
        info.m_offsetX = tmplate->Get_Shadow_Offset_X();
        info.m_offsetY = tmplate->Get_Shadow_Offset_Y();

        if (g_theProjectedShadowManager != nullptr) {
            m_decalShadow = g_theProjectedShadowManager->Add_Decal(m_renderObject, &info);
        }

        if (m_decalShadow != nullptr) {
            m_decalShadow->Enable_Shadow_Invisible(m_fullyObscuredByShroud);
            m_decalShadow->Enable_Shadow_Render(m_hidden);
        }
    }
}

void W3DModelDraw::Set_Terrain_Decal_Size(float width, float height)
{
    if (m_decalShadow != nullptr) {
        m_decalShadow->Set_Size(width, height);
    }
}

void W3DModelDraw::Set_Terrain_Decal_Opacity(float opacity)
{
    if (m_decalShadow != nullptr) {
        m_decalShadow->Set_Opacity((int)(255.0f * opacity));
    }
}

void W3DModelDraw::Nuke_Current_Render(Matrix3D *xform)
{
    m_pauseAnimation = false;

    if (m_shadow != nullptr) {
        m_shadow->Release();
        m_shadow = nullptr;
    }

    if (m_decalShadow != nullptr) {
        m_decalShadow->Release();
        m_decalShadow = nullptr;
    }

    if (m_renderObject != nullptr) {
        if (xform != nullptr) {
            *xform = m_renderObject->Get_Transform();
        }

        W3DDisplay::s_3DScene->Remove_Render_Object(m_renderObject);
        Ref_Ptr_Release(m_renderObject);
    } else if (xform != nullptr) {
        *xform = *Get_Drawable()->Get_Transform_Matrix();
    }
}

void W3DModelDraw::Hide_All_Garrison_Flags(bool hide)
{
    if (m_renderObject != nullptr) {
        int index;
        RenderObjClass *robj = m_renderObject->Get_Sub_Object_By_Name("POLE", &index);

        if (robj != nullptr) {
            robj->Set_Hidden(hide);
            const HTreeClass *tree = m_renderObject->Get_HTree();

            if (tree != nullptr) {
                Do_Hide_Show_Bone_Sub_Objs(hide,
                    m_renderObject->Get_Num_Sub_Objects(),
                    m_renderObject->Get_Sub_Object_Bone_Index(0, index),
                    m_renderObject,
                    tree);
            }

            robj->Release_Ref();
        }
    }
}

void W3DModelDraw::Hide_All_Headlights(bool hide)
{
    if (m_renderObject != nullptr) {
        for (int i = 0; i < m_renderObject->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *robj = m_renderObject->Get_Sub_Object(i);

            if (strstr(robj->Get_Name(), "HEADLIGHT") != nullptr) {
                robj->Set_Hidden(hide);
            }

            robj->Release_Ref();
        }
    }
}

void W3DModelDraw::Hide_All_Muzzle_Flashes(ModelConditionInfo const *condition, RenderObjClass *robj)
{
    if (condition != nullptr && robj != nullptr && (condition->m_validStuff & WEAPON_BARREL_INFO_VALID) != 0) {
        for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
            if (condition->m_hasWeaponBone[i]) {
                for (auto &barrel : condition->m_weaponBarrelInfoVec[i]) {
                    if (barrel.m_weaponMuzzleFlashBone != 0) {
                        barrel.Set_Muzzle_Flash_Hidden(robj, true);
                    }
                }
            }
        }
    }
}

bool Turret_Names_Differ(ModelConditionInfo const *state1, ModelConditionInfo const *state2)
{
    if ((state1->m_validStuff & TURRET_VALID) == 0 || (state2->m_validStuff & TURRET_VALID) == 0) {
        return true;
    }

    for (int i = 0; i < MAX_TURRETS; i++) {
        if (state1->m_turretInfo[i].m_turretAngleName != state2->m_turretInfo[i].m_turretAngleName
            || state1->m_turretInfo[i].m_turretPitchName != state2->m_turretInfo[i].m_turretPitchName) {
            return true;
        }
    }

    return false;
}

void W3DModelDraw::Set_Model_State(ModelConditionInfo const *new_state)
{
    captainslog_dbgassert(new_state, "invalid state in W3DModelDraw::setModelState");

#ifdef GAME_DEBUG_STRUCTS
    if (Get_Drawable() != nullptr) {
        if (Get_Drawable()->Get_Object() != nullptr) {
            if (Get_Drawable()->Get_Object()->Get_ID() == s_theObjectIDToDebug) {
                captainslog_debug("REQUEST switching to state %s for obj %s %d",
                    new_state->m_description.Str(),
                    Get_Drawable()->Get_Object()->Get_Template()->Get_Name().Str(),
                    Get_Drawable()->Get_Object()->Get_ID());
            }
        }
    }
#endif

    const ModelConditionInfo *next = nullptr;

    if (m_curState != nullptr && new_state != nullptr) {
        if (m_curState == new_state && m_nextState == nullptr || m_curState != nullptr && m_nextState == new_state) {
#ifdef GAME_DEBUG_STRUCTS
            if (Get_Drawable() != nullptr) {
                if (Get_Drawable()->Get_Object() != nullptr) {
                    if (Get_Drawable()->Get_Object()->Get_ID() == s_theObjectIDToDebug) {
                        captainslog_debug("IGNORE duplicate state %s for obj %s %d",
                            new_state->m_description.Str(),
                            Get_Drawable()->Get_Object()->Get_Template()->Get_Name().Str(),
                            Get_Drawable()->Get_Object()->Get_ID());
                    }
                }
            }
#endif

            return;
        }

        if (new_state != m_curState && new_state->m_allowToFinishKey != NAMEKEY_INVALID
            && new_state->m_allowToFinishKey == m_curState->m_transitionKey && m_renderObject != nullptr
            && !Is_Animation_Complete(m_renderObject)) {
#ifdef GAME_DEBUG_STRUCTS
            if (Get_Drawable() != nullptr) {
                if (Get_Drawable()->Get_Object() != nullptr) {
                    if (Get_Drawable()->Get_Object()->Get_ID() == s_theObjectIDToDebug) {
                        captainslog_debug("ALLOW_TO_FINISH state %s for obj %s %d",
                            new_state->m_description.Str(),
                            Get_Drawable()->Get_Object()->Get_Template()->Get_Name().Str(),
                            Get_Drawable()->Get_Object()->Get_ID());
                    }
                }
            }
#endif

            m_nextState = new_state;
            m_loopDuration = -1;
            return;
        }

        if (new_state != m_curState) {
            if (m_curState->m_transitionKey != NAMEKEY_INVALID) {
                if (new_state->m_transitionKey != NAMEKEY_INVALID) {
                    const ModelConditionInfo *state =
                        Find_Transition_For_Sig(Make_Transition(m_curState->m_transitionKey, new_state->m_transitionKey));

                    if (state) {
#ifdef GAME_DEBUG_STRUCTS
                        if (Get_Drawable() != nullptr) {
                            if (Get_Drawable()->Get_Object() != nullptr) {
                                if (Get_Drawable()->Get_Object()->Get_ID() == s_theObjectIDToDebug) {
                                    captainslog_debug("using TRANSITION state %s before requested state %s for obj %s %d",
                                        state->m_description.Str(),
                                        new_state->m_description.Str(),
                                        Get_Drawable()->Get_Object()->Get_Template()->Get_Name().Str(),
                                        Get_Drawable()->Get_Object()->Get_ID());
                                }
                            }
                        }
#endif
                        next = new_state;
                        new_state = state;
                    }
                }
            }
        }
    }

    float fraction = Get_Current_Anim_Fraction();

    if (!Get_Drawable()->Check_Status_Bit(DRAWABLE_STATUS_8)) {
        m_recalcBones = true;
    }

    Stop_Client_Particle_Systems();
    Hide_All_Muzzle_Flashes(new_state, m_renderObject);

    if (m_curState == nullptr || new_state->m_modelName != m_curState->m_modelName
        || Turret_Names_Differ(new_state, m_curState)) {
        Matrix3D tm;
        Nuke_Current_Render(&tm);
        Drawable *drawable = Get_Drawable();

        if (new_state->m_modelName.Is_Empty()) {
            m_renderObject = nullptr;
        } else {
            m_renderObject = W3DDisplay::s_assetManager->Create_Render_Obj(
                new_state->m_modelName.Str(), drawable->Get_Scale(), m_hexColor, nullptr, nullptr);

            // Thyme specific: Original assert has been demoted to log message because it is a data issue.
            if (m_renderObject == nullptr) {
                captainslog_error("*** ASSET ERROR: Model %s not found!", new_state->m_modelName.Str());
            }
        }

        new_state->Validate_Stuff(
            m_renderObject, drawable->Get_Scale(), Get_W3D_Model_Draw_Module_Data()->m_extraPublicBone);
        Rebuild_Weapon_RecoilInfo(new_state);
        Do_Hide_Show_Sub_Objs(&new_state->m_hideShowVec);

#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData->m_hideGarrisonFlags && drawable->Is_KindOf(KINDOF_STRUCTURE)) {
            Hide_All_Garrison_Flags(true);
        }
#endif

        Shadow::ShadowTypeInfo info;
        const ThingTemplate *thing = Get_Drawable()->Get_Template();

        if (m_renderObject != nullptr) {
            if (g_theWriteableGlobalData->m_makeTrackMarks && m_trackRenderObject == nullptr) {
                if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
                    if (!Get_W3D_Model_Draw_Module_Data()->m_trackFile.Is_Empty()) {
                        m_trackRenderObject = g_theTerrainTracksRenderObjClassSystem->Bind_Track(
                            m_renderObject, 10.0f, Get_W3D_Model_Draw_Module_Data()->m_trackFile.Str());

                        if (drawable != nullptr) {
                            if (m_trackRenderObject != nullptr) {
                                m_trackRenderObject->Set_Bound_Drawable(drawable);
                            }
                        }
                    }
                }
            }
        }

        if (m_renderObject != nullptr && g_theW3DShadowManager != nullptr && thing->Get_Shadow_Type()) {
            strcpy(info.m_shadowName, thing->Get_Shadow_Texture_Name().Str());
            captainslog_dbgassert(info.m_shadowName[0], "this should be validated in ThingTemplate now");
            info.m_allowUpdates = false;
            info.m_allowWorldAlign = true;
            info.m_type = (ShadowType)thing->Get_Shadow_Type();
            info.m_sizeX = thing->Get_Shadow_Size_X();
            info.m_sizeY = thing->Get_Shadow_Size_Y();
            info.m_offsetX = thing->Get_Shadow_Offset_X();
            info.m_offsetY = thing->Get_Shadow_Offset_Y();
            m_shadow = g_theW3DShadowManager->Add_Shadow(m_renderObject, &info, nullptr);

            if (m_shadow != nullptr) {
                m_shadow->Enable_Shadow_Invisible(m_fullyObscuredByShroud);
                m_shadow->Enable_Shadow_Render(m_hidden);
            }
        }

        if (m_renderObject != nullptr) {
            if (thing->Is_KindOf(KINDOF_SELECTABLE)) {
                m_renderObject->Set_Collision_Type(COLLISION_TYPE_1);
            }

            if (thing->Is_KindOf(KINDOF_SHRUBBERY)) {
                m_renderObject->Set_Collision_Type(COLLISION_TYPE_2);
            }

            if (thing->Is_KindOf(KINDOF_MINE)) {
                m_renderObject->Set_Collision_Type(COLLISION_TYPE_3);
            }

            if (thing->Is_KindOf(KINDOF_FORCEATTACKABLE)) {
                m_renderObject->Set_Collision_Type(COLLISION_TYPE_4);
            }

            if (thing->Is_KindOf(KINDOF_CLICK_THROUGH)) {
                m_renderObject->Set_Collision_Type(0);
            }

            Object *object = drawable->Get_Object();

            if (object != nullptr) {
                if (!object->Is_KindOf(KINDOF_BRIDGE) && !object->Is_KindOf(KINDOF_BRIDGE_TOWER)) {
                    if (object->Is_KindOf(KINDOF_STRUCTURE)
                        && (drawable->Get_Condition_State().Test(MODELCONDITION_RUBBLE))) {
                        m_renderObject->Set_Collision_Type(0);
                    } else if (object->Is_Effectively_Dead()) {
                        m_renderObject->Set_Collision_Type(0);
                    }
                }
            }

            W3DDisplay::s_3DScene->Add_Render_Object(m_renderObject);
            m_renderObject->Set_User_Data(drawable->Get_Drawable_Info());
            Set_Terrain_Decal(drawable->Get_Terrain_Decal());

            if (drawable->Is_Hidden()) {
                m_renderObject->Set_Hidden(true);

                if (m_shadow != nullptr) {
                    m_shadow->Enable_Shadow_Render(false);
                }

                m_hidden = false;
            }

            m_renderObject->Set_Transform(tm);
            On_Render_Obj_Recreated();
        }
    } else {
        new_state->Validate_Stuff(
            m_renderObject, Get_Drawable()->Get_Scale(), Get_W3D_Model_Draw_Module_Data()->m_extraPublicBone);
        Rebuild_Weapon_RecoilInfo(new_state);
        Do_Hide_Show_Sub_Objs(&new_state->m_hideShowVec);
    }

    Hide_All_Headlights(m_isDaytime);
    const ModelConditionInfo *old_state = m_curState;
    m_curState = new_state;
    m_nextState = next;
    m_loopDuration = -1;
    Adjust_Animation(old_state, fraction);
}

void W3DModelDraw::Replace_Model_Condition_State(BitFlags<MODELCONDITION_COUNT> const &flags)
{
    m_isDaytime = !flags.Test(MODELCONDITION_NIGHT);
    const ModelConditionInfo *info = Find_Best_Info(flags);

    if (info != nullptr) {
        Set_Model_State(info);
    }

    Hide_All_Headlights(m_isDaytime);
}

void W3DModelDraw::Set_Selectable(bool selectable)
{
    if (m_renderObject != nullptr) {
        if (selectable) {
            m_renderObject->Set_Collision_Type(m_renderObject->Get_Collision_Type() | COLLISION_TYPE_1);
        } else {
            m_renderObject->Set_Collision_Type(m_renderObject->Get_Collision_Type() & ~COLLISION_TYPE_1);
        }
    }
}

void W3DModelDraw::Replace_Indicator_Color(int color)
{
    if (Get_W3D_Model_Draw_Module_Data()->m_okToChangeModelColor && Get_Render_Object()) {
        int new_color;

        if (color) {
            new_color = color | 0xFF000000;
        } else {
            new_color = 0;
        }

        if (new_color != m_hexColor) {
            m_hexColor = new_color;
            const ModelConditionInfo *state = m_curState;
            m_curState = nullptr;
            m_nextState = nullptr;
            m_loopDuration = -1;
            Set_Model_State(state);
        }
    }
}

bool W3DModelDraw::Client_Only_Get_Render_Obj_Info(Coord3D *pos, float *radius, Matrix3D *transform) const
{
    if (m_renderObject == nullptr) {
        return false;
    }

    Vector3 v = m_renderObject->Get_Position();
    pos->x = v.X;
    pos->y = v.Y;
    pos->z = v.Z;
    *transform = m_renderObject->Get_Transform();
    *radius = m_renderObject->Get_Bounding_Sphere().Radius;
    return true;
}

bool W3DModelDraw::Get_Projectile_Launch_Offset(BitFlags<MODELCONDITION_COUNT> const &flags,
    WeaponSlotType wslot,
    int specific_barrel_to_use,
    Matrix3D *launch_pos,
    WhichTurretType tur,
    Coord3D *turret_rot_pos,
    Coord3D *turret_pitch_pos) const
{
    const ModelConditionInfo *info = Find_Best_Info(flags);

    if (info) {
        const W3DModelDrawModuleData *data = Get_W3D_Model_Draw_Module_Data();
        info->Validate_Stuff(nullptr, Get_Drawable()->Get_Scale(), data->m_extraPublicBone);
        captainslog_dbgassert(info->m_transition == 0,
            "It is never legal to Get_Projectile_Launch_Offset from a Transition state (they vary on a per-client basis)... "
            "however, we can fix this");
        captainslog_dbgassert(specific_barrel_to_use >= 0, "specific_barrel_to_use should now always be explicit");
        std::vector<ModelConditionInfo::WeaponBarrelInfo> &barrel_vec = info->m_weaponBarrelInfoVec[wslot];

        if (barrel_vec.empty()) {
            launch_pos = nullptr;
        } else {
            if (specific_barrel_to_use < 0 || specific_barrel_to_use >= (int)barrel_vec.size()) {
                specific_barrel_to_use = 0;
            }

            if (launch_pos) {
                *launch_pos = barrel_vec[specific_barrel_to_use].m_weaponLaunchBoneTransform;

                if (tur != TURRET_INVALID) {
                    launch_pos->Pre_Rotate_Z(info->m_turretInfo[tur].m_turretArtAngle);
                    launch_pos->Pre_Rotate_Y(-info->m_turretInfo[tur].m_turretArtPitch);
                }
            }
        }

        if (turret_rot_pos) {
            turret_rot_pos->Zero();
        }

        if (turret_pitch_pos) {
            turret_pitch_pos->Zero();
        }

        if (tur != TURRET_INVALID) {
            Vector3 *offset = data->Get_Attach_To_Drawable_Bone_Offset(Get_Drawable());
            ModelConditionInfo::TurretInfo &tinfo = info->m_turretInfo[tur];

            if (turret_rot_pos) {
                bool b = tinfo.m_turretAngleName && !info->Find_Pristine_Bone_Pos(tinfo.m_turretAngleName, *turret_rot_pos);
                captainslog_dbgassert(!b,
                    "*** ASSET ERROR: TurretBone %s not found!",
                    g_theNameKeyGenerator->Key_To_Name(tinfo.m_turretAngleName).Str());

                if (offset) {
                    turret_rot_pos->x += offset->X;
                    turret_rot_pos->y += offset->Y;
                    turret_rot_pos->z += offset->Z;
                }
            }

            if (turret_pitch_pos) {
                bool b =
                    tinfo.m_turretPitchName && !info->Find_Pristine_Bone_Pos(tinfo.m_turretPitchName, *turret_pitch_pos);
                captainslog_dbgassert(!b,
                    "*** ASSET ERROR: TurretBone %s not found!",
                    g_theNameKeyGenerator->Key_To_Name(tinfo.m_turretPitchName).Str());

                if (offset) {
                    turret_pitch_pos->x += offset->X;
                    turret_pitch_pos->y += offset->Y;
                    turret_pitch_pos->z += offset->Z;
                }
            }
        }

        return launch_pos != nullptr;
    } else {
        return false;
    }
}

int W3DModelDraw::Get_Pristine_Bone_Positions_For_Condition_State(BitFlags<MODELCONDITION_COUNT> const &flags,
    char const *bone_name,
    int start_index,
    Coord3D *positions,
    Matrix3D *transforms,
    int max_bones) const
{
    const ModelConditionInfo *info = Find_Best_Info(flags);

    if (info == nullptr) {
        return 0;
    }

    RenderObjClass *robj;

    if (info == m_curState) {
        robj = m_renderObject;
    } else {
        robj = nullptr;
    }

    info->Validate_Stuff(robj, Get_Drawable()->Get_Scale(), Get_W3D_Model_Draw_Module_Data()->m_extraPublicBone);
    Matrix3D t[64];

    if (max_bones > 64) {
        max_bones = 64;
    }

    if (!transforms) {
        transforms = t;
    }

    int current = 0;
    int count = start_index != 0 ? 99 : 0;

    for (int i = start_index; i <= count; i++) {
        char bone_id[256];

        if (i) {
            sprintf(bone_id, "%s%02d", bone_name, i);
        } else {
            strcpy(bone_id, bone_name);
        }

        for (char *j = bone_id; j && *j; j++) {
            *j = tolower(*j);
        }

        const Matrix3D *bone = info->Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(bone_id), nullptr);

        if (bone == nullptr) {
            const Object *object = Get_Drawable()->Get_Object();

            if (object) {
                transforms[current] = *object->Get_Transform_Matrix();
            } else {
                transforms[current].Make_Identity();
            }

            break;
        }

        transforms[current] = *bone;
        current++;

        if (current >= max_bones) {
            break;
        }
    }

    if (positions != nullptr && transforms != nullptr) {
        for (int i = 0; i < current; i++) {
            Vector3 v = transforms[i].Get_Translation();
            positions[i].x = v.X;
            positions[i].y = v.Y;
            positions[i].z = v.Z;
        }
    }

    return current;
}

bool W3DModelDraw::Client_Only_Get_Render_Obj_Bound_Box(OBBoxClass *box) const
{
    if (m_renderObject == nullptr) {
        return false;
    }

    AABoxClass aabox;
    m_renderObject->Get_Obj_Space_Bounding_Box(aabox);
    Matrix3D m = m_renderObject->Get_Transform();
    OBBoxClass obbox(aabox.m_center, aabox.m_extent);
    OBBoxClass::Transform(m, obbox, box);
    return true;
}

bool W3DModelDraw::Client_Only_Get_Render_Obj_Bone_Transform(Utf8String const &bone, Matrix3D *transform) const
{
    if (m_renderObject == nullptr) {
        return false;
    }

    int index = m_renderObject->Get_Bone_Index(bone.Str());

    if (index == 0) {
        transform->Make_Identity();
        return false;
    } else {
        *transform = m_renderObject->Get_Bone_Transform(index);
        return true;
    }
}

bool W3DModelDraw::Get_Current_Worldspace_Client_Bone_Positions(char const *bone_name_prefix, Matrix3D &transform) const
{
    if (m_renderObject == nullptr) {
        return false;
    }

    int index = m_renderObject->Get_Bone_Index(bone_name_prefix);

    if (!index) {
        return false;
    }

    transform = m_renderObject->Get_Bone_Transform(index);
    return true;
}

int W3DModelDraw::Get_Current_Bone_Positions(
    char const *bone_name_prefix, int start_index, Coord3D *positions, Matrix3D *transforms, int max_bones) const
{
    Matrix3D t[64];

    if (max_bones > 64) {
        max_bones = 64;
    }

    if (!transforms) {
        transforms = t;
    }

    if (!m_renderObject) {
        return 0;
    }

    Matrix3D m(m_renderObject->Get_Transform());
    Matrix3D m2;
    m.Get_Orthogonal_Inverse(m2);
    m2.Scale(Get_Drawable()->Get_Scale());

    int current = 0;
    int count = start_index != 0 ? 99 : 0;

    for (int i = start_index; i <= count; i++) {
        char bone_id[256];

        if (i) {
            sprintf(bone_id, "%s%02d", bone_name_prefix, i);
        } else {
            strcpy(bone_id, bone_name_prefix);
        }

        int index = m_renderObject->Get_Bone_Index(bone_id);

        if (!index) {
            break;
        }

        transforms[current] = m_renderObject->Get_Bone_Transform(index);
        transforms[current].Mul(m2);
        current++;

        if (current >= max_bones) {
            break;
        }
    }

    if (positions && transforms) {
        for (int i = 0; i < current; i++) {
            Vector3 v = transforms[i].Get_Translation();
            positions[i].x = v.X;
            positions[i].y = v.Y;
            positions[i].z = v.Z;
        }
    }

    return current;
}

void W3DModelDraw::React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle)
{
    if (m_renderObject) {
        const Matrix3D *dtm = Get_Drawable()->Get_Transform_Matrix();
        Matrix3D rtm(*dtm);
        Adjust_Transform_Mtx(rtm);
        m_renderObject->Set_Transform(rtm);
    }

    if (m_trackRenderObject) {
        const Object *object = Get_Drawable()->Get_Object();
        const Coord3D *position = Get_Drawable()->Get_Position();

        if (m_fullyObscuredByShroud || object->Get_Status(OBJECT_STATUS_STEALTHED)) {
            m_trackRenderObject->Add_Cap_Edge_To_Track(position->x, position->y);
        } else {
            if (object) {
                if (object->Is_Significantly_Above_Terrain()) {
                    m_trackRenderObject->Set_Above_Terrain();
                }
            }

            m_trackRenderObject->Add_Edge_To_Track(position->x, position->y);
        }
    }
}

const ModelConditionInfo *W3DModelDraw::Find_Best_Info(BitFlags<MODELCONDITION_COUNT> const &flags) const
{
    return Get_W3D_Model_Draw_Module_Data()->Find_Best_Info(flags);
}

int W3DModelDraw::Get_Barrel_Count(WeaponSlotType wslot) const
{
    if (m_curState != nullptr && (m_curState->m_validStuff & WEAPON_BARREL_INFO_VALID) != 0) {
        return m_curState->m_weaponBarrelInfoVec[wslot].size();
    } else {
        return 0;
    }
}

void Do_FX_Pos(FXList const *list,
    const Coord3D *primary,
    const Matrix3D *primary_mtx,
    float primary_speed,
    const Coord3D *secondary,
    float radius)
{
    if (list) {
        list->Do_FX_Pos(primary, primary_mtx, primary_speed, secondary, radius);
    }
}

bool W3DModelDraw::Handle_Weapon_Fire_FX(WeaponSlotType wslot,
    int specific_barrel_to_use,
    FXList const *fxl,
    float weapon_speed,
    Coord3D const *victim_pos,
    float radius)
{
    captainslog_dbgassert(specific_barrel_to_use >= 0, "specific_barrel_to_use should now always be explicit");

    if (m_curState == nullptr || (m_curState->m_validStuff & WEAPON_BARREL_INFO_VALID) == 0) {
        return false;
    }

    std::vector<ModelConditionInfo::WeaponBarrelInfo> &barrel_vec = m_curState->m_weaponBarrelInfoVec[wslot];

    if (barrel_vec.empty()) {
        return false;
    }

    bool ret = false;

    if (specific_barrel_to_use < 0 || specific_barrel_to_use > (int)barrel_vec.size()) {
        specific_barrel_to_use = 0;
    }

    ModelConditionInfo::WeaponBarrelInfo &info = barrel_vec[specific_barrel_to_use];

    if (fxl != nullptr) {
        if (info.m_weaponFireFXBone && m_renderObject != nullptr) {
            const Object *object = Get_Drawable()->Get_Object();

            if (!m_renderObject->Is_Hidden() || object == nullptr) {
                Matrix3D m(m_renderObject->Get_Bone_Transform(info.m_weaponFireFXBone));
                Coord3D c;
                c.x = m.Get_X_Translation();
                c.y = m.Get_Y_Translation();
                c.z = m.Get_Z_Translation();
                Do_FX_Pos(fxl, &c, &m, weapon_speed, victim_pos, radius);
            } else {
                Do_FX_Pos(fxl, object->Get_Position(), object->Get_Transform_Matrix(), weapon_speed, victim_pos, radius);
            }

            ret = true;
        } else {
            captainslog_debug("*** no FXBone found for a non-null FXL");
        }
    }

    if (info.m_weaponRecoilBone || info.m_weaponMuzzleFlashBone) {
        RecoilInfo &recoil = m_weaponRecoilInfoVec[wslot][specific_barrel_to_use];
        recoil.m_state = RecoilInfo::RECOIL_START;
        recoil.m_recoilRate = Get_W3D_Model_Draw_Module_Data()->m_initialRecoil;

        if (info.m_weaponMuzzleFlashBone) {
            info.Set_Muzzle_Flash_Hidden(m_renderObject, false);
        }
    }

    return ret;
}

void W3DModelDraw::Set_Animation_Loop_Duration(unsigned int num_frames)
{
    m_loopDuration = -1;
    Set_Cur_Anim_Duration_In_Msec(GameMath::Ceil(num_frames * MSEC_PER_LOGICFRAME_REAL));
}

void W3DModelDraw::Set_Animation_Completion_Time(unsigned int num_frames)
{
    if (m_curState != nullptr && m_curState->m_transition && m_curState->m_animations.size() && m_nextState != nullptr
        && m_nextState->m_transition && m_nextState->m_animations.size()) {
        float cur_fps = m_curState->m_animations.front().Get_Frames_Per_Second();
        float next_fps = m_nextState->m_animations.front().Get_Frames_Per_Second();
        int duration = GameMath::Fast_To_Int_Floor(num_frames * cur_fps / (cur_fps + next_fps));
        Set_Animation_Loop_Duration(duration);
        m_loopDuration = num_frames - duration;
    } else {
        Set_Animation_Loop_Duration(num_frames);
    }
}

void W3DModelDraw::Set_Animation_Frame(int frame)
{
    if (m_renderObject != nullptr && m_whichAnimInCurState >= 0) {
        HAnimClass *anim = m_curState->m_animations[m_whichAnimInCurState].Get_Anim_Handle();
        m_renderObject->Set_Animation(anim, frame);

        if (anim) {
            anim->Release_Ref();
        }
    }
}

void W3DModelDraw::Set_Pause_Animation(bool pause)
{
    if (m_pauseAnimation != pause) {
        m_pauseAnimation = pause;

        if (m_renderObject != nullptr) {
            if (m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD) {
                HLodClass *hlod = static_cast<HLodClass *>(m_renderObject);
                float frame;
                int frames;
                int mode;
                float multiplier;
                HAnimClass *anim = hlod->Peek_Animation_And_Info(frame, frames, mode, multiplier);

                if (anim) {
                    if (m_pauseAnimation) {
                        m_animMode = mode;
                        hlod->Set_Animation(anim, frame, RenderObjClass::ANIM_MODE_MANUAL);
                    } else {
                        hlod->Set_Animation(anim, frame, m_animMode);
                    }
                }
            }
        }
    }
}

void W3DModelDraw::Rebuild_Weapon_RecoilInfo(ModelConditionInfo const *state)
{
    if (state != nullptr) {
        for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
            unsigned int barrel_size = state->m_weaponBarrelInfoVec[i].size();
            unsigned int recoil_size = m_weaponRecoilInfoVec[i].size();

            if (recoil_size != barrel_size) {
                RecoilInfo info;
                m_weaponRecoilInfoVec[i].resize(barrel_size, info);
            }

            for (auto &recoil : m_weaponRecoilInfoVec[i]) {
                recoil.Clear();
            }
        }
    } else {
        for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
            m_weaponRecoilInfoVec[i].clear();
        }
    }
}

void W3DModelDraw::Preload_Assets(TimeOfDayType time_of_day)
{
    const W3DModelDrawModuleData *data = Get_W3D_Model_Draw_Module_Data();

    if (data) {
        data->Preload_Assets(time_of_day, Get_Drawable()->Get_Scale());
    }
}

bool W3DModelDraw::Is_Visible() const
{
    return m_renderObject != nullptr && m_renderObject->Is_Really_Visible();
}

void W3DModelDraw::Update_Projectile_Clip_Status(unsigned int show, unsigned int count, WeaponSlotType wslot)
{
    if (((1 << wslot) & Get_W3D_Model_Draw_Module_Data()->m_projectileBoneFeedbackEnabledSlots) != 0) {
        Do_Hide_Show_Projectile_Objects(show, count, wslot);
    }
}

void W3DModelDraw::Update_Draw_Module_Supply_Status(int max, int current)
{
    if (current > 0) {
        Get_Drawable()->Set_Model_Condition_State(MODELCONDITION_CARRYING);
    } else {
        Get_Drawable()->Clear_Model_Condition_State(MODELCONDITION_CARRYING);
    }
}

void W3DModelDraw::Do_Hide_Show_Projectile_Objects(unsigned int show, unsigned int count, WeaponSlotType wslot)
{
    if (count < show) {
        captainslog_dbgassert(count >= show, "Someone is trying to show more projectiles than they have.");
    } else {
        int val = count - show;
        std::vector<ModelConditionInfo::HideShowSubObjInfo> vector;
        ModelConditionInfo::HideShowSubObjInfo info;

        if (m_curState->m_weaponHideShowBoneName[wslot].Is_Empty()) {
            for (unsigned int i = 0; i < count; i++) {
                info.sub_obj_name.Format("%s%02d", m_curState->m_weaponLaunchBoneName[wslot].Str(), i + 1);
                info.hide = (int)(i + 1) <= val;
                vector.push_back(info);
            }
        } else {
            info.sub_obj_name = m_curState->m_weaponHideShowBoneName[wslot];
            info.hide = val > 0;
            vector.push_back(info);
        }

        Do_Hide_Show_Sub_Objs(&vector);
    }
}

void W3DModelDraw::Update_Sub_Objects()
{
    if (m_renderObject != nullptr && !m_subObjects.empty()) {
        for (auto &info : m_subObjects) {
            int index;
            RenderObjClass *robj = m_renderObject->Get_Sub_Object_By_Name(info.sub_obj_name.Str(), &index);

            if (robj) {
                robj->Set_Hidden(info.hide);

                const HTreeClass *tree = m_renderObject->Get_HTree();

                if (tree) {
                    Do_Hide_Show_Bone_Sub_Objs(info.hide,
                        m_renderObject->Get_Num_Sub_Objects(),
                        m_renderObject->Get_Sub_Object_Bone_Index(0, index),
                        m_renderObject,
                        tree);
                }

                robj->Release_Ref();
            } else {
                captainslog_dbgassert(robj,
                    "*** ASSET ERROR: SubObject %s not found (%s)!",
                    info.sub_obj_name.Str(),
                    Get_Drawable()->Get_Template()->Get_Name().Str());
            }
        }
    }
}

void W3DModelDraw::CRC_Snapshot(Xfer *xfer)
{
    DrawModule::CRC_Snapshot(xfer);
}

void W3DModelDraw::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t ver = 2;
    xfer->xferVersion(&ver, 2);
    DrawModule::Xfer_Snapshot(xfer);
    RecoilInfo recoilinfo;
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        unsigned char size = (unsigned char)m_weaponRecoilInfoVec[i].size();
        xfer->xferUnsignedByte(&size);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (auto &info : m_weaponRecoilInfoVec[i]) {
                recoilinfo.m_state = info.m_state;
                xfer->xferUser(&recoilinfo.m_state, sizeof(recoilinfo.m_state));
                recoilinfo.m_shift = info.m_shift;
                xfer->xferReal(&recoilinfo.m_shift);
                recoilinfo.m_recoilRate = info.m_recoilRate;
                xfer->xferReal(&recoilinfo.m_recoilRate);
            }
        } else {
            m_weaponRecoilInfoVec[i].clear();
            for (int j = 0; j < size; j++) {
                xfer->xferUser(&recoilinfo.m_state, sizeof(recoilinfo.m_state));
                xfer->xferReal(&recoilinfo.m_shift);
                xfer->xferReal(&recoilinfo.m_recoilRate);
                m_weaponRecoilInfoVec[i].push_back(recoilinfo);
            }
        }
    }

    unsigned char size = (unsigned char)m_subObjects.size();
    xfer->xferUnsignedByte(&size);
    ModelConditionInfo::HideShowSubObjInfo subobj;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto &info : m_subObjects) {
            subobj.sub_obj_name = info.sub_obj_name;
            xfer->xferAsciiString(&subobj.sub_obj_name);
            subobj.hide = info.hide;
            xfer->xferBool(&subobj.hide);
        }
    } else {
        m_subObjects.clear();
        for (int j = 0; j < size; j++) {
            xfer->xferAsciiString(&subobj.sub_obj_name);
            xfer->xferBool(&subobj.hide);
            m_subObjects.push_back(subobj);
        }
    }

    if (ver >= 2) {
        if (xfer->Get_Mode() == XFER_SAVE) {
            if (m_renderObject != nullptr && m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD
                && m_curState != nullptr && m_curState->m_transition) {
                HLodClass *hlod = static_cast<HLodClass *>(m_renderObject);
                float frame;
                int frames;
                int mode;
                float multiplier;
                HAnimClass *anim = hlod->Peek_Animation_And_Info(frame, frames, mode, multiplier);
                bool b = anim != nullptr;
                xfer->xferBool(&b);

                if (anim) {
                    xfer->xferInt(&mode);
                    float f = frame / (anim->Get_Num_Frames() - 1);
                    xfer->xferReal(&f);
                }
            } else {
                bool b = false;
                xfer->xferBool(&b);
            }
        } else {
            bool b;
            xfer->xferBool(&b);

            if (b) {
                int mode;
                float newframe;
                xfer->xferInt(&mode);
                xfer->xferReal(&newframe);

                if (m_renderObject != nullptr) {
                    if (m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD) {
                        HLodClass *hlod = static_cast<HLodClass *>(m_renderObject);
                        HAnimClass *anim = hlod->Peek_Animation();

                        if (anim) {
                            float frame;
                            int frames;
                            int anim_mode;
                            float multiplier;
                            hlod->Peek_Animation_And_Info(frame, frames, anim_mode, multiplier);
                            hlod->Set_Animation(anim, (anim->Get_Num_Frames() - 1) / newframe, anim_mode);
                        }
                    }
                }
            }
        }
    }

    if (xfer->Get_Mode() == XFER_LOAD && !m_subObjects.empty()) {
        Update_Sub_Objects();
    }
}

void W3DModelDraw::Load_Post_Process()
{
    DrawModule::Load_Post_Process();
}

NameKeyType W3DModelDraw::Get_Module_Name_Key() const
{
    static const NameKeyType _key = g_theNameKeyGenerator->Name_To_Key("W3DModelDraw");
    return _key;
}

void W3DModelDraw::Show_Sub_Object(Utf8String const &sub_obj_name, bool visible)
{
    if (sub_obj_name.Is_Not_Empty()) {
        bool found = false;

        for (auto &info : m_subObjects) {
            if (!strcasecmp(sub_obj_name.Str(), info.sub_obj_name.Str())) {
                info.hide = visible == 0;
                found = true;
            }
        }

        if (!found) {
            ModelConditionInfo::HideShowSubObjInfo info;
            info.sub_obj_name = sub_obj_name;
            info.hide = visible == 0;
            m_subObjects.push_back(info);
        }
    }
}

void W3DModelDraw::Adjust_Anim_Speed_To_Movement_Speed()
{
    float distance = Get_Cur_Anim_Distance_Covered();

    if (distance > 0.0f) {
        Object *object = Get_Drawable()->Get_Object();

        if (object) {
            PhysicsBehavior *phys = object->Get_Physics();

            if (phys) {
                float vel = phys->Get_Velocity_Magnitude();

                if (vel > 0.0f) {
                    Set_Cur_Anim_Duration_In_Msec(distance / vel * MSEC_PER_LOGICFRAME_REAL);
                }
            }
        }
    }
}

void W3DModelDraw::React_To_Geometry_Change() {}

void W3DModelDraw::Notify_Draw_Module_Dependency_Cleared() {}

void W3DModelDraw::On_Render_Obj_Recreated() {}

#ifdef GAME_DEBUG_STRUCTS
void W3DModelDraw::Gather_Draw_Stats_For_Render_Object(DebugDrawStats *stats, RenderObjClass *robj)
{
    if (robj != nullptr) {
        for (int i = 0; i < robj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *subobj = robj->Get_Sub_Object(i);
            Gather_Draw_Stats_For_Render_Object(stats, subobj);

            if (subobj) {
                subobj->Release_Ref();
            }
        }

        if (robj->Is_Not_Hidden_At_All()) {
            if (robj->Class_ID() == RenderObjClass::CLASSID_MESH) {
                MeshClass *mesh = static_cast<MeshClass *>(robj);
                MeshModelClass *model = mesh->Peek_Model();

                if (model) {
                    if (model->Get_Flag(MeshGeometryClass::SORT)) {
                        stats->Add_Sort_Meshes(1);
                    }

                    if (model->Get_Flag(MeshGeometryClass::SKIN)) {
                        stats->Add_Skins(1);
                    }
                }

                stats->Add_Draw_Calls(mesh->Get_Draw_Call_Count());
            }
        }

        const HTreeClass *tree = robj->Get_HTree();

        if (tree) {
            stats->Add_Bones(tree->Num_Pivots());
        }
    }
}
#endif

ModuleData *W3DModelDraw::Friend_New_Module_Data(INI *ini)
{
    W3DModelDrawModuleData *data = new W3DModelDrawModuleData();

    if (ini) {
        ini->Init_From_INI_Multi_Proc(data, W3DModelDrawModuleData::Build_Field_Parse);
    }

    return data;
}

Module *W3DModelDraw::Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data)
{
    return NEW_POOL_OBJ(W3DModelDraw, thing, module_data);
}
