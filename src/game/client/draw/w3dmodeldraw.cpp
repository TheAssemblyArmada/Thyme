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
#include "assetmgr.h"
#include "drawable.h"
#include "fpusetting.h"
#include "gamelod.h"
#include "gamelogic.h"
#include "gamestate.h"
#include "globaldata.h"
#include "hanim.h"
#include "hlod.h"
#include "particlesystemplate.h"
#include "rendobj.h"
#include "w3ddisplay.h"
#include <stdio.h>

#ifdef GAME_DEBUG_STRUCTS
#ifdef GAME_DLL
extern Utf8String &s_theThingTemplateBeingParsedName;
#else
extern Utf8String s_theThingTemplateBeingParsedName;
#endif
#endif

HAnimClass *W3DAnimationInfo::Get_Anim_Handle() const
{
    HAnimClass *anim = W3DDisplay::s_assetManager->Get_HAnim(m_name);
// assert disabled, the game tries to load cbnretal04_dg.cbnretal04_dg which doesn't exist and therefore triggers the assert
#if 0
    captainslog_dbgassert(anim, "*** ASSET ERROR: animation %s not found", m_name.Str());
#endif

    if (anim && m_framesPerSecond < 0.0f) {
        m_framesPerSecond = (anim->Get_Num_Frames() * 1000.0f) / anim->Get_Frame_Rate();
    }

    return anim;
}

void ModelConditionInfo::WeaponBarrelInfo::Set_Muzzle_Flash_Hidden(RenderObjClass *robj, bool hidden) const
{
    if (robj) {
        RenderObjClass *child_object = robj->Get_Sub_Object_On_Bone(0, m_weaponMuzzleFlashBone);

        if (child_object) {
            child_object->Set_Hidden(hidden);
            child_object->Release_Ref();
        } else {
#ifdef GAME_DEBUG_STRUCTS
            captainslog_debug(
                "*** ASSET ERROR: child_object %s not found in Set_Muzzle_Flash_Hidden()", m_weaponMuzzleFlashBoneName);
#endif
        }
    }
}

void ModelConditionInfo::Preload_Assets(TimeOfDayType time_of_day, float scale)
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

bool Test_Animation_Flag(int flags, char flag)
{
    return ((1 << flags) & flag) != 0;
}

bool Find_Single_Bone(RenderObjClass *r, Utf8String const &bone, Matrix3D &transform, int &index)
{
    if (bone.Is_None() || bone.Is_Empty()) {
        return false;
    }

    index = r->Get_Bone_Index(bone);

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

    RenderObjClass *subobj = r->Get_Sub_Object_By_Name(sub_obj_name, nullptr);

    if (!subobj) {
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

        if (robj) {
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
        map[g_theNameKeyGenerator->Name_To_Key(bone_lower)] = info;
        bone_found = true;
    }

    for (int i = 1; i <= 99; i++) {
        bone_id.Format("%s%02d", bone_lower.Str(), i);

        if (!Find_Single_Bone(robj, bone_id, info.transform, info.index)) {
            break;
        }

        map[g_theNameKeyGenerator->Name_To_Key(bone_id)] = info;
        bone_found = true;
    }

    if (!bone_found) {
        if (Find_Single_Sub_Obj(robj, bone_lower, info.transform, info.index)) {
            map[g_theNameKeyGenerator->Name_To_Key(bone_lower)] = info;
            sub_obj_found = true;
        }

        for (int i = 1; i <= 99; i++) {
            bone_id.Format("%s%02d", bone_lower.Str(), i);

            if (!Find_Single_Sub_Obj(robj, bone_id, info.transform, info.index)) {
                break;
            }

            map[g_theNameKeyGenerator->Name_To_Key(bone_id)] = info;
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
        robj = W3DDisplay::s_assetManager->Create_Render_Obj(m_modelName, scale, 0, nullptr, nullptr);
        captainslog_dbgassert(robj, "*** ASSET ERROR: Model %s not found!", m_modelName.Str());

        if (robj) {
            ref = true;
        }
    }

    if (robj) {
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

            if (first_anim) {
                first_anim->Add_Ref();
            }
        }

        if (first_anim) {
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

        if (g_theWriteableGlobalData) {
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

        if (anim) {
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

        for (int i = 0; i < BONE_COUNT; i++) {
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
                            info.m_weaponMuzzleFlashBoneName = buf;
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

                        if (m) {
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
                            g_theNameKeyGenerator->Name_To_Key(weaponrecoilbonename), &info.m_weaponRecoilBone);
                    }

                    if (!weaponmuzzleflashbonename.Is_Empty()) {
                        Find_Pristine_Bone(
                            g_theNameKeyGenerator->Name_To_Key(weaponmuzzleflashbonename), &info.m_weaponMuzzleFlashBone);
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
                        m = Find_Pristine_Bone(g_theNameKeyGenerator->Name_To_Key(weaponlaunchbonename), nullptr);
                    }

                    if (m) {
                        info.m_weaponLaunchBoneTransform = *m;
                    } else {
                        info.m_weaponLaunchBoneTransform.Make_Identity();
                    }

                    if (!weaponfirefxbonename.Is_Empty()) {
                        Find_Pristine_Bone(
                            g_theNameKeyGenerator->Name_To_Key(weaponfirefxbonename), &info.m_weaponFireFXBone);
                    }

                    if (info.m_weaponFireFXBone || info.m_weaponRecoilBone || info.m_weaponMuzzleFlashBone || m) {
                        m_weaponBarrelInfoVec[i].push_back(info);

                        if (info.m_weaponRecoilBone || info.m_weaponMuzzleFlashBone) {
                            m_hasWeaponBone[i] = true;
                        }
                    }
                }

                // assert disabled, the game references bones on the avlazertnk_d1 model that the model does't have triggers
                // the assert
#if 0
                captainslog_dbgassert(!m_modelName.Is_Not_Empty() || !m_weaponBarrelInfoVec[i].empty(),
                    "*** ASSET ERROR: No fx bone named '%s' found in model %s!",
                    weaponfirefxbonename.Str(),
                    m_modelName.Str());
#endif
            }
        }

        m_validStuff |= WEAPON_BARREL_INFO_VALID;
    }
}

void ModelConditionInfo::Validate_Turret_Info() const
{
    if ((m_validStuff & TURRET_VALID) == 0) {
        Set_FP_Mode();

        for (int i = 0; i < ARRAY_SIZE(m_turretInfo); i++) {
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
                if (index) {
                    *index = i->second.index;
                }

                return &i->second.transform;
            }
        }
    }

    if (index) {
        *index = 0;
    }

    return nullptr;
}

bool ModelConditionInfo::Find_Pristine_Bone_Pos(NameKeyType key, Coord3D &pos) const
{
    const Matrix3D *bone = Find_Pristine_Bone(key, nullptr);

    if (bone) {
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

    for (int i = 0; i < ARRAY_SIZE(m_turretInfo); i++) {
        m_turretInfo[i].Clear();
    }

    m_hideShowVec.clear();

    for (int i = 0; i < BONE_COUNT; i++) {
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
    m_minLodRequired(STATLOD_LOW),
    m_okToChangeModelColor(false),
    m_animationsRequirePower(true),
    m_attachToDrawableBoneOffsetSet(false),
    m_timeAndWeatherFlags(false),
    m_particlesAttachedToAnimatedBones(false),
    m_projectileBoneFeedbackEnabledSlots(false),
    m_initialRecoil(2.0f),
    m_maxRecoil(3.0f),
    m_recoilDamping(0.4f),
    m_recoilSettle(0.065f),
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

void W3DModelDrawModuleData::Preload_Assets(TimeOfDayType time_of_day, float scale)
{
    for (auto &model_condition : m_conditionStates) {
        model_condition.Preload_Assets(time_of_day, scale);
    }
}

Utf8String W3DModelDrawModuleData::Get_Best_Model_Name_For_WB(BitFlags<MODELCONDITION_COUNT> const &c) const
{
    const ModelConditionInfo *info = Find_Best_Info(c);

    if (info) {
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

        if (drawable->Get_Pristine_Bone_Positions(m_attachToBoneInAnotherModule, 0, nullptr, &m, 1) == 1) {
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

const ModelConditionInfo *W3DModelDrawModuleData::Find_Best_Info(BitFlags<MODELCONDITION_COUNT> const &c) const
{
    BitFlags<MODELCONDITION_COUNT> flags = c;
    flags.Clear(m_ignoreConditionStates);
    return m_conditionStateMap.Find_Best_Info(m_conditionStates, flags);
}

static const char *s_theWeaponSlotTypeNames[] = { "PRIMARY", "SECONDARY", "TERTIARY", nullptr };

void Parse_Ascii_String_LC(INI *ini, void *formal, void *store, void const *user_data)
{
    *((Utf8String *)store) = ini->Get_Next_Ascii_String();
    (*((Utf8String *)store)).To_Lower();
}

void W3DModelDrawModuleData::Build_Field_Parse(MultiIniFieldParse &p)
{
    // clang-format off
    static FieldParse dataFieldParse[] = {
        { "InitialRecoilSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(W3DModelDrawModuleData, m_initialRecoil) },
        { "MaxRecoilDistance", &INI::Parse_Real, nullptr, offsetof(W3DModelDrawModuleData, m_maxRecoil) },
        { "RecoilDamping", &INI::Parse_Real, nullptr, offsetof(W3DModelDrawModuleData, m_recoilDamping) },
        { "RecoilSettleSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(W3DModelDrawModuleData, m_recoilSettle) },
        { "OkToChangeModelColor", &INI::Parse_Bool, nullptr, offsetof(W3DModelDrawModuleData, m_okToChangeModelColor) },
        { "AnimationsRequirePower", &INI::Parse_Bool, nullptr, offsetof(W3DModelDrawModuleData, m_animationsRequirePower) },
        { "ParticlesAttachedToAnimatedBones", &INI::Parse_Bool, nullptr, offsetof(W3DModelDrawModuleData, m_particlesAttachedToAnimatedBones) },
        { "MinLODRequired", &GameLODManager::Parse_Static_Game_LOD_Level, nullptr, offsetof(W3DModelDrawModuleData, m_minLodRequired) },
        { "ProjectileBoneFeedbackEnabledSlots", &INI::Parse_Bitstring32, s_theWeaponSlotTypeNames, offsetof(W3DModelDrawModuleData, m_projectileBoneFeedbackEnabledSlots) },
        { "DefaultConditionState", &W3DModelDrawModuleData::Parse_Condition_State, (void *)PARSE_DEFAULT, 0 },
        { "ConditionState", &W3DModelDrawModuleData::Parse_Condition_State, (void *)PARSE_NORMAL, 0 },
        { "AliasConditionState", &W3DModelDrawModuleData::Parse_Condition_State, (void *)PARSE_ALIAS, 0 },
        { "TransitionState", &W3DModelDrawModuleData::Parse_Condition_State, (void *)PARSE_TRANSITION, 0 },
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
        xfer->xferByte((int8_t *)&model_condition.m_validStuff);
#ifdef GAME_DEBUG_STRUCTS
        xfer->xferAsciiString(&model_condition.m_description);
#endif

        if (model_condition.m_validStuff) {
            for (auto &bone_pair : model_condition.m_boneMap) {
                xfer->xferInt(&bone_pair.second.index);
                xfer->xferUser((void *)&bone_pair.second.transform, sizeof(bone_pair.second.transform));
            }

            for (int j = 0; j < ARRAY_SIZE(model_condition.m_turretInfo); j++) {
                xfer->xferInt(&model_condition.m_turretInfo[j].m_turretAngleBone);
                xfer->xferInt(&model_condition.m_turretInfo[j].m_turretPitchBone);
            }

            for (int j = 0; j < BONE_COUNT; j++) {
                for (auto &weapon_barrel : model_condition.m_weaponBarrelInfoVec[j]) {
                    xfer->xferUser((void *)&weapon_barrel.m_weaponLaunchBoneTransform,
                        sizeof(weapon_barrel.m_weaponLaunchBoneTransform));
                }
            }
        }
    }
}

bool Does_State_Exist(std::vector<ModelConditionInfo> const &v, BitFlags<MODELCONDITION_COUNT> const &f)
{
    for (auto &model_condition : v) {
        for (int count = model_condition.Get_Conditions_Count() - 1; count >= 0; count--) {
            if (f == model_condition.Get_Conditions_Yes(count)) {
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

    if (instance) {
        ((ModelConditionInfo *)instance)->Add_Public_Bone(str);
    }

    if (str.Is_Empty() || str.Is_None()) {
        *(NameKeyType *)store = NAMEKEY_INVALID;
    } else {
        *(NameKeyType *)store = g_theNameKeyGenerator->Name_To_Key(str);
    }
}

void Parse_Show_Hide_Sub_Object(INI *ini, void *instance, void *store, void const *user_data)
{
    std::vector<ModelConditionInfo::HideShowSubObjInfo> *v = (std::vector<ModelConditionInfo::HideShowSubObjInfo> *)store;
    Utf8String str = ini->Get_Next_Ascii_String();
    str.To_Lower();

    if (str.Is_None()) {
        v->clear();
    } else {
        while (str.Is_Not_Empty()) {
            bool found = false;

            for (auto &i : *v) {
                if (!strcasecmp(i.name, str)) {
                    i.visible = (uintptr_t)user_data != 0;
                    found = true;
                }
            }

            if (!found) {
                ModelConditionInfo::HideShowSubObjInfo info;
                info.name = str;
                info.visible = (uintptr_t)user_data != 0;
                v->push_back(info);
            }

            str = ini->Get_Next_Ascii_String();
            str.To_Lower();
        }
    }
}

void Parse_Weapon_Bone_Name(INI *ini, void *instance, void *store, void const *user_data)
{
    Utf8String *str = (Utf8String *)store;
    int index = ini->Scan_IndexList(ini->Get_Next_Token(), s_theWeaponSlotTypeNames);
    str[index] = ini->Get_Next_Ascii_String();
    str[index].To_Lower();

    if (str[index].Is_None()) {
        str[index].Clear();
    }

    if (instance) {
        ((ModelConditionInfo *)instance)->Add_Public_Bone(str[index]);
    }
}

void Parse_Animation(INI *ini, void *instance, void *store, void const *user_data)
{
    ModelConditionInfo *minfo = (ModelConditionInfo *)instance;
    Utf8String name = ini->Get_Next_Ascii_String();
    name.To_Lower();
    const char *str = ini->Get_Next_Token_Or_Null();
    float distance;

    if (str) {
        distance = ini->Scan_Real(str);
    } else {
        distance = 0.0f;
    }

    captainslog_dbgassert((uintptr_t)user_data != 1 || distance == 0.0f,
        "You should not specify nonzero DistanceCovered values for Idle Anims");

    str = ini->Get_Next_Token_Or_Null();
    int count = std::min(str ? ini->Scan_Int(str) : 1, 1);
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
    *(NameKeyType *)store = g_theNameKeyGenerator->Name_To_Key(str);
}

void Parse_Particle_Sys_Bone(INI *ini, void *instance, void *store, void const *user_data)
{
    ParticleSysBoneInfo info;
    info.bone_name = ini->Get_Next_Ascii_String();
    info.bone_name.To_Lower();
    ParticleSystemTemplate::Parse(ini, instance, &info.particle_system_template, user_data);
    ((ModelConditionInfo *)instance)->m_ParticleSysBones.push_back(info);
}

void Parse_Real_Range(INI *ini, void *instance, void *store, void const *user_data)
{
    ModelConditionInfo *info = (ModelConditionInfo *)instance;
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
    W3DModelDrawModuleData *data = (W3DModelDrawModuleData *)instance;

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
                    s_theThingTemplateBeingParsedName);
#endif
                throw CODE_06;
            }

            if (data->m_stateCount < 0 && data->m_conditionStates.empty() && flags.Any()) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug("*** ASSET ERROR: when not using DefaultConditionState, the first ConditionState must be "
                                  "for NONE (%s)",
                    s_theThingTemplateBeingParsedName);
#endif
                throw CODE_06;
            }

            captainslog_relassert(flags.Any() || data->m_stateCount < 0,
                CODE_06,
                "*** ASSET ERROR: you may not specify both a Default state and a Conditions=None state");

            if (Does_State_Exist(data->m_conditionStates, flags)) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug(
                    "*** ASSET ERROR: duplicate condition states are not currently allowed (%s)", info.m_description);
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
                    s_theThingTemplateBeingParsedName);
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
            NameKeyType key = g_theNameKeyGenerator->Name_To_Key(str);
            NameKeyType key2 = g_theNameKeyGenerator->Name_To_Key(str2);
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
            info2.m_descrption.Concat("\nAKA: ");
            info2.m_descrption += str;
#endif

            if (flags.Any_Intersection_With(data->m_ignoreConditionStates)) {
#ifdef GAME_DEBUG_STRUCTS
                captainslog_debug("You should not specify bits in a state once they are used in IgnoreConditionStates (%s)",
                    s_theThingTemplateBeingParsedName);
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
    static FieldParse myFieldParse[] = {
        { "Model", &Parse_Ascii_String_LC, nullptr, offsetof(ModelConditionInfo, m_modelName) },
        { "Turret", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretAngleName) },
        { "TurretArtAngle", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretArtAngle) },
        { "TurretPitch", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretPitchName) },
        { "TurretArtPitch", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[0].m_turretArtPitch) },
        { "AltTurret", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretAngleName) },
        { "AltTurretArtAngle", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretArtAngle) },
        { "AltTurretPitch", &Parse_Bone_Name_Key, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretPitchName) },
        { "AltTurretArtPitch", &INI::Parse_Angle_Real, nullptr, offsetof(ModelConditionInfo, m_turretInfo[1].m_turretArtPitch) },
        { "ShowSubObject", &Parse_Show_Hide_Sub_Object, (void *)0, offsetof(ModelConditionInfo, m_hideShowVec) },
        { "HideSubObject", &Parse_Show_Hide_Sub_Object, (void *)1, offsetof(ModelConditionInfo, m_hideShowVec) },
        { "WeaponFireFXBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponFireFXBoneName) },
        { "WeaponRecoilBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponRecoilBoneName) },
        { "WeaponMuzzleFlash", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponMuzzleFlashName) },
        { "WeaponLaunchBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponLaunchBoneName) },
        { "WeaponHideShowBone", &Parse_Weapon_Bone_Name, nullptr, offsetof(ModelConditionInfo, m_weaponHideShowBoneName) },
        { "Animation", &Parse_Animation, (void *)0, offsetof(ModelConditionInfo, m_animations) },
        { "IdleAnimation", &Parse_Animation, (void *)1, offsetof(ModelConditionInfo, m_animations) },
        { "AnimationMode", &INI::Parse_Index_List, s_theAnimModeNames, offsetof(ModelConditionInfo, m_mode) },
        { "TransitionKey", &Parse_Lowercase_Name_Key, nullptr, offsetof(ModelConditionInfo, m_transitionKey) },
        { "WaitForStateToFinishIfPossible", &Parse_Lowercase_Name_Key, nullptr, offsetof(ModelConditionInfo, m_allowToFinishKey) },
        { "Flags", &INI::Parse_Bitstring32, &s_ACBitsNames, offsetof(ModelConditionInfo, m_flags) },
        { "ParticleSysBone", &Parse_Particle_Sys_Bone, nullptr, 0 },
        { "AnimationSpeedFactorRange", &Parse_Real_Range, nullptr, 0 },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ini->Init_From_INI((void *)&info, myFieldParse);

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
        captainslog_debug(
            "*** ASSET ERROR: Idle Anims should always use ONCE or ONCE_BACKWARDS (%s)", s_theThingTemplateBeingParsedName);
#endif
        throw CODE_06;
    }

    info.m_validStuff &= ~LAUNCH_BONES_VALID;

    for (int i = 0; i < BONE_COUNT; i++) {
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
