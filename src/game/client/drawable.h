/**
 * @file
 *
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
#pragma once

#include "always.h"
#include "anim2d.h"
#include "module.h"
#include "object.h"
#include "thing.h"

class DisplayString;
class TintEnvelope;
class DynamicAudioEventInfo;
class DynamicAudioEventRTS;
class DrawableIconInfo;
class Drawable;
class GhostObject;
class Locomotor;
class View;
class ClientUpdateModule;
class DrawModule;
class Anim2D;

struct TWheelInfo
{
    float m_frontLeftHeightOffset;
    float m_frontRightHeightOffset;
    float m_rearLeftHeightOffset;
    float m_rearRightHeightOffset;
    float m_wheelAngle;
    int m_framesAirborneCounter;
    int m_framesAirborne;
};

class DrawableLocoInfo : public MemoryPoolObject
{
    IMPLEMENT_POOL(DrawableLocoInfo)

public:
    DrawableLocoInfo() :
        m_pitch(0.0f),
        m_pitchRate(0.0f),
        m_roll(0.0f),
        m_rollRate(0.0f),
        m_thrustWobble(0.0f),
        m_accelerationPitch(0.0f),
        m_accelerationPitchRate(0.0f),
        m_accelerationRoll(0.0f),
        m_accelerationRollRate(0.0f),
        m_overlapZVel(0.0f),
        m_overlapZ(0.0f),
        m_thrust(1.0f),
        m_rudder(0.0f),
        m_elevator(0.0f)
    {
        // #BUGFIX Initialize all members
        m_wheelInfo = TWheelInfo{};
    }
    virtual ~DrawableLocoInfo() override {}
    float m_pitch;
    float m_pitchRate;
    float m_roll;
    float m_rollRate;
    float m_thrustWobble; // not 100% identified yet
    float m_accelerationPitch;
    float m_accelerationPitchRate;
    float m_accelerationRoll;
    float m_accelerationRollRate;
    float m_overlapZVel;
    float m_overlapZ;
    float m_thrust; // not 100% identified yet
    float m_rudder; // not 100% identified yet
    float m_elevator; // not 100% identified yet
    TWheelInfo m_wheelInfo;
};

enum DrawableIconType
{
    ICON_FIRST = 0,
    ICON_HEAL = 0,
    ICON_HEAL_STRUCTURE,
    ICON_HEAL_VEHICLE,
    ICON_DEMORALIZED_OBSOLETE,
    ICON_BOMB_TIMED,
    ICON_BOMB_REMOTE,
    ICON_DISABLED,
    ICON_BATTLEPLAN_BOMBARDMENT,
    ICON_BATTLEPLAN_HOLDTHELINE,
    ICON_BATTLEPLAN_SEARCHANDDESTROY,
    ICON_EMOTICON,
    ICON_ENTHUSIASTIC,
    ICON_SUBLIMINAL,
    ICON_CARBOMB,
    MAX_ICONS,
    ICON_INVALID = -1,
};

class DrawableIconInfo : public MemoryPoolObject
{
    IMPLEMENT_POOL(DrawableIconInfo)

public:
    DrawableIconInfo()
    {
        for (int i = 0; i < MAX_ICONS; i++) {
            anims[i] = nullptr;
            timings[i] = 0;
        }
    }

    void Kill_Icon(DrawableIconType icon)
    {
        if (anims[icon] != nullptr) {
            anims[icon]->Delete_Instance();
            anims[icon] = nullptr;
            timings[icon] = 0;
        }
    }

    void Reset()
    {
        for (int i = 0; i < MAX_ICONS; i++) {
            if (anims[i] != nullptr) {
                anims[i]->Delete_Instance();
                anims[i] = nullptr;
            }

            timings[i] = 0;
        }
    }
    virtual ~DrawableIconInfo() override { Reset(); }

private:
    Anim2D *anims[MAX_ICONS];
    unsigned int timings[MAX_ICONS];
    friend class Drawable;
};

struct DrawableInfo
{
    ObjectID object_id;
    Drawable *drawable;
    GhostObject *ghost_object;
    int flags;
};

class Drawable : public Thing, public SnapShot
{
    IMPLEMENT_POOL(Drawable);

public:
    struct PhysicsXformInfo
    {
        float m_totalPitch;
        float m_totalRoll;
        float m_yaw;
        float m_totalZ;
        PhysicsXformInfo() : m_totalPitch(0.0f), m_totalRoll(0.0f), m_yaw(0.0f), m_totalZ(0.0f) {}
    };

    enum DrawBits
    {
        DRAW_BIT_DISABLED = 1,
        DRAW_BIT_DEAD = 2,
        DRAW_BIT_POISONED = 4,
        DRAW_BIT_DAMAGED = 8,
        DRAW_BIT_WEAPON_BONUS = 16,
    };

    enum FadingMode
    {
        FADING_MODE_OFF,
        FADING_MODE_OUT,
        FADING_MODE_IN,
    };

    Drawable(ThingTemplate const *thing_template, DrawableStatus status);
    virtual ~Drawable() override;
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual Drawable *As_Drawable_Meth() override { return this; }
    virtual const Drawable *As_Drawable_Meth() const override { return this; }
    virtual void React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle) override;
    virtual void Set_Animation_Frame(int frame);

    Object *Get_Object() { return m_object; }
    const Object *Get_Object() const { return m_object; }
    bool Is_Hidden() const { return m_hidden || m_stealthInvisible; }
    StealthLookType Get_Stealth_Look() const { return m_stealthLook; }
    void Set_Remain_Visible_Frames(unsigned int frames) { m_remainVisibleFrames = frames; }
    unsigned int Get_Remain_Visible_Frames() const { return m_remainVisibleFrames; }
    bool Is_Fully_Obscured_By_Shroud() const { return m_fullyObscuredByShroud; }
    bool Get_Ambient_Sound_From_Script_Enabled() const { return m_ambientSoundFromScriptEnabled; }

    bool Get_Draws_In_Mirror() const
    {
        return m_status & DRAWABLE_STATUS_DRAWS_IN_MIRROR || Is_KindOf(KINDOF_CAN_CAST_REFLECTIONS);
    }

    const TWheelInfo *Get_Wheel_Info() const
    {
        if (m_drawableLocoInfo) {
            return &m_drawableLocoInfo->m_wheelInfo;
        } else {
            return nullptr;
        }
    }

    float Get_Alpha_Override() const { return m_opacity * m_effectiveOpacity2; }
    float Get_Stealth_Emissive_Scale() const { return m_stealthEmissiveScale; }
    bool Recieves_Dynamic_Lights() const { return m_receivesDynamicLights; }
    DrawableInfo *Get_Drawable_Info() { return &m_drawableInfo; }
    bool Check_Status_Bit(DrawableStatus status) const { return (status & m_status) != 0; }
    float Get_Instance_Scale() const { return m_instanceScale; }
    const BitFlags<MODELCONDITION_COUNT> &Get_Condition_State() const { return m_conditionState; }
    Module **Get_Modules(ModuleType type)
    {
        captainslog_assert(type != MODULE_DEFAULT);
        return m_modules[type - 1];
    }
    Module **Get_Modules(ModuleType type) const
    {
        captainslog_assert(type != MODULE_DEFAULT);
        return m_modules[type - 1];
    }

    ClientUpdateModule **Get_Client_Update_Modules()
    {
        return reinterpret_cast<ClientUpdateModule **>(Get_Modules(MODULE_CLIENT_UPDATE));
    }

    DrawModule **Get_Draw_Modules_Non_Dirty() { return reinterpret_cast<DrawModule **>(Get_Modules(MODULE_DRAW)); }

    void Set_Model_Condition_State(ModelConditionFlagType set)
    {
        Clear_And_Set_Model_Condition_State(MODELCONDITION_INVALID, set);
    }

    void Clear_Model_Condition_State(ModelConditionFlagType clr)
    {
        Clear_And_Set_Model_Condition_State(clr, MODELCONDITION_INVALID);
    }

    void Set_Model_Condition_Flags(const BitFlags<MODELCONDITION_COUNT> &set)
    {
        BitFlags<MODELCONDITION_COUNT> b;
        Clear_And_Set_Model_Condition_Flags(b, set);
    }

    void Clear_Model_Condition_Flags(const BitFlags<MODELCONDITION_COUNT> &clr)
    {
        BitFlags<MODELCONDITION_COUNT> b;
        Clear_And_Set_Model_Condition_Flags(clr, b);
    }

    TerrainDecalType Get_Terrain_Decal() const { return m_terrainDecal; }
    void Set_Recieves_Dynamic_Lights(bool enable) { m_receivesDynamicLights = enable; }
    TintEnvelope *Get_Tint_Color_Envelope() const { return m_tintColorEnvelope; }
    void Set_Tint_Color_Envelope(const TintEnvelope *envelope);
    Drawable *Get_Next() { return m_nextDrawable; }
    void Set_Status_Bit(DrawableStatus status) { m_status |= status; }
    void Clear_Status_Bit(DrawableStatus status) { m_status &= ~status; }
    int Is_Selected() { return m_selected != false; }
    void Set_Opacity(float opacity) { m_opacity = opacity; }
    float Get_Effective_Opacity1() { return m_effectiveOpacity1; }
    float Get_Opacity() { return m_opacity; }
    float Get_Effective_Opacity2() { return m_effectiveOpacity2; }
    void Clear_Draw_Bit(unsigned int bit) { m_drawBits &= ~bit; }
    bool Check_Draw_Bit(unsigned int bit) { return (bit & m_drawBits) != 0; }
    const Matrix3D &Get_Instance_Matrix() const { return m_instance; }
    bool Is_Instance_Identity() { return m_instanceIsIdentity; }

    void Kill_Icon(DrawableIconType icon)
    {
        if (m_drawableIconInfo) {
            m_drawableIconInfo->Kill_Icon(icon);
        }
    }

    bool Has_Drawable_Icon_Info() { return m_drawableIconInfo != nullptr; }
    void Set_Draw_Bit(unsigned int bit) { m_drawBits |= bit; }
    Drawable *Get_Previous() { return m_prevDrawable; }
    void Set_Flash_Time(int time) { m_flashTime = time; }
    void Set_Flash_Color(int color) { m_flashColor = color; }
    DrawableLocoInfo *Get_Loco_Info() { return m_drawableLocoInfo; }
    unsigned int Get_Expiration_Date() { return m_expirationDate; }

    void Allocate_Shadows();

    void Apply_Physics_Xform(Matrix3D *mtx);
    bool Calc_Physics_Xform(PhysicsXformInfo &xform);
    void Calc_Physics_Xform_Hover_Or_Wings(Locomotor const *locomotor, PhysicsXformInfo &xform);
    void Calc_Physics_Xform_Motorcycle(Locomotor const *locomotor, PhysicsXformInfo &xform);
    void Calc_Physics_Xform_Thrust(Locomotor const *locomotor, PhysicsXformInfo &xform);
    void Calc_Physics_Xform_Treads(Locomotor const *locomotor, PhysicsXformInfo &xform);
    void Calc_Physics_Xform_Wheels(Locomotor const *locomotor, PhysicsXformInfo &xform);

    void Changed_Team();
    void Clear_And_Set_Model_Condition_Flags(
        BitFlags<MODELCONDITION_COUNT> const &clr, BitFlags<MODELCONDITION_COUNT> const &set);
    void Clear_And_Set_Model_Condition_State(ModelConditionFlagType clr, ModelConditionFlagType set);
    void Clear_Caption_Text();
    void Clear_Custom_Sound_Ambient(bool restart);
    void Clear_Emoticon();

    bool Client_Only_Get_First_Render_Obj_Info(Coord3D *position, float *radius, Matrix3D *transform);
    void Color_Flash(RGBColor const *color, unsigned int attack_frames, unsigned int decay_frames, unsigned int peak_frames);
    void Color_Tint(RGBColor const *color);

    void Draw(View *view);
    void Draw_Ammo(IRegion2D const *region);
    void Draw_Bombed(IRegion2D const *region);
    void Draw_Caption(IRegion2D const *region);
    void Draw_Construct_Percent(IRegion2D const *region);
    void Draw_Contained(IRegion2D const *region);
    void Draw_Disabled(IRegion2D const *region);
    void Draw_Emoticon(IRegion2D const *region);
    void Draw_Enthusiastic(IRegion2D const *region);
    void Draw_Healing(IRegion2D const *region);
    void Draw_Health_Bar(IRegion2D const *region);
    void Draw_Icon_UI();
    void Draw_UI_Text();
    void Draw_Veterancy(IRegion2D const *region);
    bool Draws_Any_UI_Text();

    void Enable_Ambient_Sound(bool enable);
    void Enable_Ambient_Sound_From_Script(bool enable);

    void Fade_In(unsigned int time);
    void Fade_Out(unsigned int time);

    ClientUpdateModule *Find_Client_Update_Module(NameKeyType key);
    void Flash_As_Selected(RGBColor const *color);

    void Friend_Bind_To_Object(Object *obj);
    void Friend_Clear_Selected();
    void Friend_Set_Selected();

    const AudioEventRTS *Get_Ambient_Sound_By_Damage(BodyDamageType damage);
    int Get_Barrel_Count(WeaponSlotType slot) const;
    const AudioEventInfo *Get_Base_Sound_Ambient_Info() const;
    Utf16String Get_Caption_Text();
    int Get_Current_Client_Bone_Positions(
        char const *bone_name_prefix, int start_index, Coord3D *positions, Matrix3D *transforms, int max_bones) const;
    bool Get_Current_Worldspace_Client_Bone_Positions(char const *bone_name_prefix, Matrix3D &transform) const;
    DrawModule **Get_Draw_Modules();
    DrawModule const **Get_Draw_Modules() const;
    GeometryInfo const &Get_Drawable_Geometry_Info() const;
    DrawableID Get_ID() const
    {
        captainslog_dbgassert(m_id != INVALID_DRAWABLE_ID, "Drawable::getID - Using ID before it was assigned!!!!");
        return m_id;
    }
    DrawableIconInfo *Get_Icon_Info();
    int Get_Pristine_Bone_Positions(
        char const *bone_name_prefix, int start_index, Coord3D *positions, Matrix3D *transforms, int max_bones) const;
    bool Get_Projectile_Launch_Offset(WeaponSlotType wslot,
        int ammo_index,
        Matrix3D *launch_pos,
        WhichTurretType tur,
        Coord3D *turret_rot_pos,
        Coord3D *turret_pitch_pos) const;
    float const Get_Scale() const { return m_instanceScale; }
    Vector3 const *Get_Selection_Color() const;
    bool Get_Should_Animate(bool when_powered) const;
    Vector3 const *Get_Tint_Color() const;
    Matrix3D const *Get_Transform_Matrix() const;

    bool Handle_Weapon_Fire_FX(WeaponSlotType wslot,
        int specific_barrel_to_use,
        FXList const *fxl,
        float weapon_speed,
        float recoil_amount,
        float recoil_angle,
        Coord3D const *victim_pos,
        float radius);
    void Imitate_Stealth_Look(Drawable &imitate);

    bool Is_Mass_Selectable() const;
    bool Is_Selectable() const;
    bool Is_Visible() const;

    void Mangle_Custom_Audio_Name(DynamicAudioEventInfo *info) const;
    void Notify_Drawable_Dependency_Cleared();

    void On_Destroy();
    void On_Level_Start();
    void On_Selected();
    void On_Unselected() {}

    void Preload_Assets(TimeOfDayType time_of_day);
    void Prepend_To_List(Drawable **list);
    void React_To_Body_Damage_State_Change(BodyDamageType damage);
    void React_To_Geometry_Change();
    void Release_Shadows();
    void Remove_From_List(Drawable **list);
    void Replace_Model_Condition_Flags(BitFlags<MODELCONDITION_COUNT> const &flags, bool dirty);
    void Saturate_RGB(RGBColor &color, float factor);

    void Set_Animation_Completion_Time(unsigned int time);
    void Set_Animation_Loop_Duration(unsigned int num_frames);
    void Set_Caption_Text(Utf16String const &caption);
    void Set_Custom_Sound_Ambient_Info(DynamicAudioEventInfo *info);
    void Set_Custom_Sound_Ambient_Off();
    void Set_Drawable_Hidden(bool hidden);
    void Set_Effective_Opacity(float opacity1, float opacity2);
    void Set_Emoticon(Utf8String const &emoticon, int frames);
    void Set_Fully_Obscured_By_Shroud(bool fully_obscured);
    void Set_ID(DrawableID id);
    void Set_Indicator_Color(int color);
    void Set_Instance_Matrix(Matrix3D const *matrix);
    void Set_Position(Coord3D const *pos);
    void Set_Selectable(bool selectable);
    void Set_Shadows_Enabled(bool enable);
    void Set_Stealth_Look(StealthLookType look);
    void Set_Terrain_Decal(TerrainDecalType decal);
    void Set_Terrain_Decal_Fade_Target(float target1, float target2);
    void Set_Terrain_Decal_Size(float width, float height);
    void Set_Time_Of_Day(TimeOfDayType tod);

    void Show_Sub_Object(Utf8String const &sub_object, bool visible);
    void Start_Ambient_Sound(BodyDamageType damage, TimeOfDayType tod, bool unk);
    void Start_Ambient_Sound(bool unk);
    void Stop_Ambient_Sound();

    void Update_Drawable();
    void Update_Drawable_Clip_Status(unsigned int show, unsigned int count, WeaponSlotType slot);
    void Update_Drawable_Supply_Status(int max, int current);
    void Update_Hidden_Status();
    void Update_Sub_Objects();
    void Xfer_Drawable_Modules(Xfer *xfer);

    static void Friend_Lock_Dirty_Stuff_For_Iteration();
    static void Friend_Unlock_Dirty_Stuff_For_Iteration();
    static void Init_Static_Images();
    static void Kill_Static_Images();

private:
    TintEnvelope *m_selectionColorEnvelope;
    TintEnvelope *m_tintColorEnvelope;

    TerrainDecalType m_terrainDecal;

    float m_opacity;
    float m_effectiveOpacity1;
    float m_effectiveOpacity2;
    float m_terrainDecalFadeTarget1;
    float m_terrainDecalFadeTarget2;
    float m_terrainDecalOpacity;

    Object *m_object;

    DrawableID m_id;
    Drawable *m_nextDrawable;
    Drawable *m_prevDrawable;

    DynamicAudioEventInfo *m_customSoundAmbientInfo;

    unsigned int m_status;

    unsigned int m_drawBits;
    unsigned int m_previousDrawBits;

    FadingMode m_fadingMode;
    unsigned int m_curFadeFrame;
    unsigned int m_timeToFade;

    unsigned int m_remainVisibleFrames;

    DrawableLocoInfo *m_drawableLocoInfo;

    DynamicAudioEventRTS *m_ambientSound;

    Module **m_modules[NUM_DRAWABLE_MODULE_TYPES];

    StealthLookType m_stealthLook;

    int m_flashColor;
    int m_flashTime;

    Matrix3D m_instance;
    float m_instanceScale;

    DrawableInfo m_drawableInfo;

    BitFlags<MODELCONDITION_COUNT> m_conditionState;

    float m_lastConstructDisplayed;

    DisplayString *m_constructDisplayString;
    DisplayString *m_captionText;
    DisplayString *m_groupString;

    unsigned int m_expirationDate;

    DrawableIconInfo *m_drawableIconInfo;

    float m_stealthEmissiveScale;

    bool m_selected;
    bool m_hidden;
    bool m_stealthInvisible;
    bool m_instanceIsIdentity;
    bool m_fullyObscuredByShroud;
    bool m_ambientSoundEnabled;
    bool m_ambientSoundFromScriptEnabled;
    bool m_receivesDynamicLights;
    mutable bool m_isModelDirty;

    static bool s_staticImagesInited;
    static Image *s_veterancyImage[4];
    static Image *s_fullAmmo;
    static Image *s_emptyAmmo;
    static Image *s_fullContainer;
    static Image *s_emptyContainer;
    static Anim2DTemplate **s_animationTemplates;
#ifdef GAME_DLL
    static int &s_modelLockCount;
#else
    static int s_modelLockCount;
#endif
};
