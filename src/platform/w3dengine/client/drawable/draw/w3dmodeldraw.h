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
#pragma once
#include "always.h"
#include "drawmodule.h"
#include "particlesysmanager.h"

class RenderObjClass;
class Shadow;
class TerrainTracksRenderObjClass;
class ParticleSystemTemplate;
class HAnimClass;

class W3DAnimationInfo
{
public:
    W3DAnimationInfo(const Utf8String &name, bool idle, float distance_covered) :
        m_name(name), m_distanceCovered(distance_covered), m_framesPerSecond(-1.0f), m_isIdleAnim(idle)
    {
    }

    HAnimClass *Get_Anim_Handle() const;
    float Get_Distance_Covered() const { return m_distanceCovered; }
    float Get_Frames_Per_Second() const { return m_framesPerSecond; }
    bool Is_Idle_Anim() const { return m_isIdleAnim; }

private:
    Utf8String m_name;
    float m_distanceCovered;
    mutable float m_framesPerSecond;
    bool m_isIdleAnim;
};

struct PristineBoneInfo
{
    Matrix3D transform;
    int index;
};

enum ParseCondStateType
{
    PARSE_NORMAL,
    PARSE_DEFAULT,
    PARSE_TRANSITION,
    PARSE_ALIAS,
};

enum AnimationStateFlag
{
    RANDOMSTART,
    START_FRAME_FIRST,
    START_FRAME_LAST,
    ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT,
    PRISTINE_BONE_POS_IN_FINAL_FRAME,
    MAINTAIN_FRAME_ACROSS_STATES,
    RESTART_ANIM_WHEN_COMPLETE,
    MAINTAIN_FRAME_ACROSS_STATES2,
    MAINTAIN_FRAME_ACROSS_STATES3,
    MAINTAIN_FRAME_ACROSS_STATES4,
};

enum IniReadFlags
{
    INIREAD_CONDITION_STATE = 1,
    INIREAD_ANIMS = 2,
    INIREAD_IDLE_ANIMS = 4,
};

enum ValidateFlags
{
    PRISTINE_BONES_VALID = 1,
    TURRET_VALID = 2,
    LAUNCH_BONES_VALID = 4,
    WEAPON_BARREL_INFO_VALID = 8,
    PUBLIC_BONES_VALID = 0x10,
};

struct ParticleSysBoneInfo
{
    Utf8String bone_name;
    ParticleSystemTemplate *particle_system_template;
};

struct ModelConditionInfo
{
    struct HideShowSubObjInfo
    {
        Utf8String sub_obj_name;
        bool hide;
    };

    struct TurretInfo
    {
        void Clear()
        {
            m_turretAngleName = NAMEKEY_INVALID;
            m_turretPitchName = NAMEKEY_INVALID;
            m_turretArtAngle = 0.0f;
            m_turretArtPitch = 0.0f;
            m_turretAngleBone = 0;
            m_turretPitchBone = 0;
        }

        NameKeyType m_turretAngleName;
        NameKeyType m_turretPitchName;
        float m_turretArtAngle;
        float m_turretArtPitch;
        int m_turretAngleBone;
        int m_turretPitchBone;
    };

    struct WeaponBarrelInfo
    {
        WeaponBarrelInfo() { Clear(); }

        void Set_Muzzle_Flash_Hidden(RenderObjClass *robj, bool hidden) const;

        void Clear()
        {
            m_weaponRecoilBone = 0;
            m_weaponFireFXBone = 0;
            m_weaponMuzzleFlashBone = 0;
            m_weaponLaunchBoneTransform.Make_Identity();
#ifdef GAME_DEBUG_STRUCTS
            m_weaponMuzzleFlashBoneName.Clear();
#endif
        }

        int m_weaponRecoilBone;
        int m_weaponFireFXBone;
        int m_weaponMuzzleFlashBone;
        Matrix3D m_weaponLaunchBoneTransform;
#ifdef GAME_DEBUG_STRUCTS
        Utf8String m_weaponMuzzleFlashBoneName;
#endif
    };

    ModelConditionInfo() { Clear(); }
    void Add_Public_Bone(Utf8String const &bone) const;
    void Clear();
    Matrix3D const *Find_Pristine_Bone(NameKeyType key, int *index) const;
    bool Find_Pristine_Bone_Pos(NameKeyType key, Coord3D &pos) const;
    void Load_Animations() const {}
    bool Matches_Mode(bool night, bool snow) const;
    void Preload_Assets(TimeOfDayType time_of_day, float scale) const;
    void Validate_Cached_Bones(RenderObjClass *robj, float scale) const;
    void Validate_Stuff(RenderObjClass *robj, float scale, std::vector<Utf8String> const &bones) const;
    void Validate_Turret_Info() const;
    void Validate_Weapon_Barrel_Info() const;

    const BitFlags<MODELCONDITION_COUNT> &Get_Conditions_Yes(int condition_idx) const
    {
        return m_conditionsYesVec[condition_idx];
    }

    int Get_Conditions_Count() const { return m_conditionsYesVec.size(); }
#ifdef GAME_DEBUG_STRUCTS
    Utf8String Get_Definition() const { return m_description; }
#else
    Utf8String Get_Definition() const { return "ModelConditionInfo"; }
#endif

#ifdef GAME_DEBUG_STRUCTS
    Utf8String m_description;
#endif
    std::vector<BitFlags<MODELCONDITION_COUNT>> m_conditionsYesVec;
    Utf8String m_modelName;
    std::vector<HideShowSubObjInfo> m_hideShowVec;
    mutable std::vector<Utf8String> m_publicBones;
    Utf8String m_weaponFireFXBoneName[WEAPONSLOT_COUNT];
    Utf8String m_weaponRecoilBoneName[WEAPONSLOT_COUNT];
    Utf8String m_weaponMuzzleFlashName[WEAPONSLOT_COUNT];
    Utf8String m_weaponLaunchBoneName[WEAPONSLOT_COUNT];
    Utf8String m_weaponHideShowBoneName[WEAPONSLOT_COUNT];
    std::vector<W3DAnimationInfo> m_animations;
    NameKeyType m_transitionKey;
    NameKeyType m_allowToFinishKey;
    int m_flags;
    int m_iniReadFlags;
    int m_mode;
    std::vector<ParticleSysBoneInfo> m_ParticleSysBones;
    uint64_t m_transition;
    float m_animationSpeedFactorMin;
    float m_animationSpeedFactorMax;
    mutable std::map<NameKeyType, PristineBoneInfo> m_boneMap;
    mutable TurretInfo m_turretInfo[MAX_TURRETS];
    mutable std::vector<WeaponBarrelInfo> m_weaponBarrelInfoVec[WEAPONSLOT_COUNT];
    mutable bool m_hasWeaponBone[WEAPONSLOT_COUNT];
    mutable int8_t m_validStuff;
};

class W3DModelDrawModuleData : public ModuleData
{
public:
    enum
    {
        TIMEANDWEATHER_DAY = 1,
        TIMEANDWEATHER_NIGHT = 2,
        TIMEANDWEATHER_SNOW = 4,
        TIMEANDWEATHER_NIGHTSNOW = 8,
    };

#ifdef GAME_DLL
    W3DModelDrawModuleData *Hook_Ctor() { return new (this) W3DModelDrawModuleData(); }
    void Hook_Dtor() { W3DModelDrawModuleData::~W3DModelDrawModuleData(); }
#endif
    W3DModelDrawModuleData();
    virtual ~W3DModelDrawModuleData() override;
    virtual void CRC_Snapshot(Xfer *xfer) override { Xfer_Snapshot(xfer); }
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
    virtual StaticGameLODLevel Get_Minimum_Required_Game_LOD() const override { return m_minLodRequired; }
    virtual const W3DModelDrawModuleData *Get_As_W3D_Model_Draw_Module_Data() const override { return this; }

    static void Build_Field_Parse(MultiIniFieldParse &p);
    const ModelConditionInfo *Find_Best_Info(BitFlags<MODELCONDITION_COUNT> const &flags) const;
    Vector3 *Get_Attach_To_Drawable_Bone_Offset(Drawable const *drawable) const;
    Utf8String Get_Best_Model_Name_For_WB(BitFlags<MODELCONDITION_COUNT> const &flags) const;
    static void Parse_Condition_State(INI *ini, void *formal, void *store, void const *user_data);
    void Preload_Assets(TimeOfDayType time_of_day, float scale) const;
    void Validate_Stuff_For_Time_And_Weather(Drawable const *drawable, bool night, bool snow) const;

private:
    std::vector<ModelConditionInfo> m_conditionStates;
    SparseMatchFinder<ModelConditionInfo, BitFlags<MODELCONDITION_COUNT>> m_conditionStateMap;
    std::map<uint64_t, ModelConditionInfo> m_transitionMap;
    std::vector<Utf8String> m_extraPublicBone;
    Utf8String m_trackFile;
    Utf8String m_attachToBoneInAnotherModule;
    mutable Vector3 m_attachToDrawableBoneOffset;
    int m_stateCount;
    int m_projectileBoneFeedbackEnabledSlots;
    float m_initialRecoil;
    float m_maxRecoil;
    float m_recoilDamping;
    float m_recoilSettle;
    StaticGameLODLevel m_minLodRequired;
    BitFlags<MODELCONDITION_COUNT> m_ignoreConditionStates;
    bool m_okToChangeModelColor;
    bool m_animationsRequirePower;
    mutable bool m_attachToDrawableBoneOffsetSet;
    mutable char m_timeAndWeatherFlags;
    bool m_particlesAttachedToAnimatedBones;
    bool m_recievesDynamicLights;
    friend class W3DModelDraw;
};

class W3DModelDraw : public DrawModule, public ObjectDrawInterface
{
    IMPLEMENT_POOL(W3DModelDraw);

public:
    struct RecoilInfo
    {
        enum RecoilState
        {
            IDLE,
            RECOIL_START,
            RECOIL,
            SETTLE,
        };

        RecoilInfo() { Clear(); }

        void Clear()
        {
            m_state = IDLE;
            m_shift = 0.0f;
            m_recoilRate = 0.0f;
        }

        RecoilState m_state;
        float m_shift;
        float m_recoilRate;
    };

    struct ParticleSysTrackerType
    {
        ParticleSystemID id;
        int index;
    };

#ifdef GAME_DLL
    W3DModelDraw *Hook_Ctor(Thing *thing, ModuleData const *module_data)
    {
        return new (this) W3DModelDraw(thing, module_data);
    }
    void Hook_Dtor() { W3DModelDraw::~W3DModelDraw(); }
#endif
    W3DModelDraw(Thing *thing, ModuleData const *module_data);

    virtual ~W3DModelDraw() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Drawable_Bound_To_Object() override;
    virtual void Preload_Assets(TimeOfDayType time_of_day) override;
    virtual void Do_Draw_Module(const Matrix3D *transform) override;
    virtual void Set_Shadows_Enabled(bool enable) override;
    virtual void Release_Shadows() override;
    virtual void Allocate_Shadows() override;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Gather_Draw_Stats(DebugDrawStats *stats) override;
#endif
    virtual void Set_Terrain_Decal(TerrainDecalType decal) override;
    virtual void Set_Terrain_Decal_Size(float width, float height) override;
    virtual void Set_Terrain_Decal_Opacity(float opacity) override;
    virtual void Set_Fully_Obscured_By_Shroud(bool obscured) override;
    virtual bool Is_Visible() const override;
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override;
    virtual void React_To_Geometry_Change() override;
    virtual ObjectDrawInterface *Get_Object_Draw_Interface() override { return this; }
    virtual const ObjectDrawInterface *Get_Object_Draw_Interface() const override { return this; }
    virtual void On_Render_Obj_Recreated();
    virtual void Adjust_Transform_Mtx(Matrix3D &transform) const;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    virtual bool Client_Only_Get_Render_Obj_Info(Coord3D *pos, float *radius, Matrix3D *transform) const override;
    virtual bool Client_Only_Get_Render_Obj_Bound_Box(OBBoxClass *box) const override;
    virtual bool Client_Only_Get_Render_Obj_Bone_Transform(Utf8String const &bone, Matrix3D *transform) const override;
    virtual int Get_Pristine_Bone_Positions_For_Condition_State(BitFlags<MODELCONDITION_COUNT> const &flags,
        char const *bone_name,
        int start_index,
        Coord3D *positions,
        Matrix3D *transforms,
        int max_bones) const override;
    virtual int Get_Current_Bone_Positions(char const *bone_name_prefix,
        int start_index,
        Coord3D *positions,
        Matrix3D *transforms,
        int max_bones) const override;
    virtual bool Get_Current_Worldspace_Client_Bone_Positions(
        char const *bone_name_prefix, Matrix3D &transform) const override;
    virtual bool Get_Projectile_Launch_Offset(BitFlags<MODELCONDITION_COUNT> const &flags,
        WeaponSlotType wslot,
        int specific_barrel_to_use,
        Matrix3D *launch_pos,
        WhichTurretType tur,
        Coord3D *turret_rot_pos,
        Coord3D *turret_pitch_pos) const override;
    virtual void Update_Projectile_Clip_Status(unsigned int show, unsigned int count, WeaponSlotType wslot) override;
    virtual void Update_Draw_Module_Supply_Status(int max, int current) override;
    virtual void Notify_Draw_Module_Dependency_Cleared() override;
    virtual void Set_Hidden(bool hidden) override;
    virtual void Replace_Model_Condition_State(BitFlags<MODELCONDITION_COUNT> const &flags) override;
    virtual void Replace_Indicator_Color(int color) override;
    virtual bool Handle_Weapon_Fire_FX(WeaponSlotType wslot,
        int specific_barrel_to_use,
        FXList const *fxl,
        float weapon_speed,
        Coord3D const *victim_pos,
        float radius) override;
    virtual int Get_Barrel_Count(WeaponSlotType wslot) const override;
    virtual void Set_Selectable(bool selectable) override;
    virtual void Set_Animation_Loop_Duration(unsigned int num_frames) override;
    virtual void Set_Animation_Completion_Time(unsigned int num_frames) override;
    virtual bool Update_Bones_For_Client_Particle_Systems() override;
    virtual void Set_Animation_Frame(int frame) override;
    virtual void Set_Pause_Animation(bool pause) override;
    virtual void Update_Sub_Objects() override;
    virtual void Show_Sub_Object(Utf8String const &sub_obj_name, bool visible) override;

    void Adjust_Anim_Speed_To_Movement_Speed();
#ifdef GAME_DEBUG_STRUCTS
    void Gather_Draw_Stats_For_Render_Object(DebugDrawStats *stats, RenderObjClass *robj);
#endif
    void Adjust_Animation(ModelConditionInfo const *prev_state, float f);
    void Do_Hide_Show_Projectile_Objects(unsigned int show, unsigned int count, WeaponSlotType wslot);
    void Do_Hide_Show_Sub_Objs(std::vector<ModelConditionInfo::HideShowSubObjInfo> const *vec);
    void Do_Start_Or_Stop_Particle_Sys();
    const ModelConditionInfo *Find_Best_Info(BitFlags<MODELCONDITION_COUNT> const &flags) const;
    const ModelConditionInfo *Find_Transition_For_Sig(uint64_t sig) const;
    static ModuleData *Friend_New_Module_Data(INI *ini);
    static Module *Friend_New_Module_Instance(Thing *thing, ModuleData const *module_data);
    float Get_Cur_Anim_Distance_Covered() const;
    float Get_Current_Anim_Fraction() const;
    void Handle_Client_Recoil();
    void Handle_Client_Turret_Positioning();
    void Hide_All_Garrison_Flags(bool hide);
    void Hide_All_Headlights(bool hide);
    void Hide_All_Muzzle_Flashes(ModelConditionInfo const *condition, RenderObjClass *robj);
    void Nuke_Current_Render(Matrix3D *xform);
    void Rebuild_Weapon_RecoilInfo(ModelConditionInfo const *state);
    void Recalc_Bones_For_Client_Particle_Systems();
    bool Set_Cur_Anim_Duration_In_Msec(float desired_duration_in_msec);
    void Set_Model_State(ModelConditionInfo const *new_state);
    void Stop_Client_Particle_Systems();

    RenderObjClass *Get_Render_Object() const { return m_renderObject; }
    bool Get_Fully_Obscured_By_Shroud() const { return m_fullyObscuredByShroud; }
    const W3DModelDrawModuleData *Get_W3D_Model_Draw_Module_Data() const
    {
        return static_cast<const W3DModelDrawModuleData *>(Module::Get_Module_Data());
    }

private:
    const ModelConditionInfo *m_curState;
    const ModelConditionInfo *m_nextState;
    int m_loopDuration;
    int m_hexColor;
    int m_whichAnimInCurState;
    std::vector<RecoilInfo> m_weaponRecoilInfoVec[WEAPONSLOT_COUNT];
    bool m_recalcBones;
    bool m_fullyObscuredByShroud;
    bool m_hidden;
    RenderObjClass *m_renderObject;
    Shadow *m_shadow;
    Shadow *m_decalShadow;
    TerrainTracksRenderObjClass *m_trackRenderObject;
    std::vector<ParticleSysTrackerType> m_particleSystemIDs;
    std::vector<ModelConditionInfo::HideShowSubObjInfo> m_subObjects;
    bool m_isDaytime;
    bool m_pauseAnimation;
    int m_animMode;
};
