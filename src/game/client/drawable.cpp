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
#include "drawable.h"
#include "aiupdate.h"
#include "audiomanager.h"
#include "bodymodule.h"
#include "clientupdatemodule.h"
#include "colorspace.h"
#include "displaystring.h"
#include "displaystringmanager.h"
#include "drawgroupinfo.h"
#include "drawmodule.h"
#include "gameclient.h"
#include "gamefont.h"
#include "gamelogic.h"
#include "gamestate.h"
#include "gametext.h"
#include "globaldata.h"
#include "globallanguage.h"
#include "ingameui.h"
#include "languagefilter.h"
#include "locomotor.h"
#include "module.h"
#include "modulefactory.h"
#include "moduleinfo.h"
#include "object.h"
#include "opencontain.h"
#include "physicsupdate.h"
#include "player.h"
#include "playerlist.h"
#include "scriptengine.h"
#include "stealthupdate.h"
#include "stickybombupdate.h"
#include "terrainlogic.h"
#include "tintenvelope.h"
#include "w3ddisplay.h"
#include "w3dview.h"

bool Drawable::s_staticImagesInited;
Image *Drawable::s_veterancyImage[4];
Image *Drawable::s_fullAmmo;
Image *Drawable::s_emptyAmmo;
Image *Drawable::s_fullContainer;
Image *Drawable::s_emptyContainer;
Anim2DTemplate **Drawable::s_animationTemplates;

#ifndef GAME_DLL
int Drawable::s_modelLockCount;
#endif

static const char *s_theDrawableIconNames[] = { "DefaultHeal",
    "StructureHeal",
    "VehicleHeal",
    "Demoralized_OBSOLETE",
    "BombTimed",
    "BombRemote",
    "Disabled",
    "BattlePlanIcon_Bombard",
    "BattlePlanIcon_HoldTheLine",
    "BattlePlanIcon_SeekAndDestroy",
    "Emoticon",
    "Enthusiastic",
    "Subliminal",
    "CarBomb",
    nullptr };

const RGBColor s_darkGreyDisabledColor = RGBColor{ -0.5f, -0.5f, -0.5f };
const RGBColor s_subdualDamageColor = RGBColor{ -0.2f, -0.2f, 0.80000001f };
const RGBColor s_frenzyColor = RGBColor{ 0.2f, -0.2f, -0.2f };
const RGBColor s_frenzyColorInfantry = RGBColor{ 0.0f, -0.69999999f, -0.69999999f };

const Matrix3D *Drawable::Get_Transform_Matrix() const
{
    const Object *object = Get_Object();

    if (object != nullptr) {
        return object->Get_Transform_Matrix();
    }

    return Thing::Get_Transform_Matrix();
}

// zh: 0x0046EB10 wb: 0x007C0CBA
bool Drawable::Is_Visible() const
{
    for (const DrawModule **draw_modules = Get_Draw_Modules(); *draw_modules != nullptr; ++draw_modules) {
        if ((*draw_modules)->Is_Visible()) {
            return true;
        }
    }
    return false;
}

void Drawable::Friend_Lock_Dirty_Stuff_For_Iteration()
{
    if (s_modelLockCount != 0 && g_theGameClient != nullptr) {
        for (Drawable *draw = g_theGameClient->First_Drawable(); draw != nullptr; draw = draw->Get_Next()) {
            draw->Get_Draw_Modules();
        }
    }

    s_modelLockCount++;
}

void Drawable::Friend_Unlock_Dirty_Stuff_For_Iteration()
{
    if (s_modelLockCount > 0) {
        s_modelLockCount--;
    }
}

const Vector3 *Drawable::Get_Tint_Color() const
{
    if (m_tintColorEnvelope != nullptr && m_tintColorEnvelope->Is_Tinted()) {
        return m_tintColorEnvelope->Get_Tint_Color();
    } else {
        return nullptr;
    }
}

const Vector3 *Drawable::Get_Selection_Color() const
{
    if (m_selectionColorEnvelope != nullptr && m_selectionColorEnvelope->Is_Tinted()) {
        return m_selectionColorEnvelope->Get_Tint_Color();
    } else {
        return nullptr;
    }
}

DrawModule **Drawable::Get_Draw_Modules()
{
    DrawModule **modules = reinterpret_cast<DrawModule **>(Get_Modules(MODULE_DRAW));

    if (m_isModelDirty) {
        if (s_modelLockCount > 0) {
#if 0
            // Thyme specific: Disabled, because it is printed each frame.
            captainslog_debug("Should not need to update dirty stuff while locked-for-iteration. Ignoring.");
#endif
            return modules;
        }

        for (DrawModule **i = modules; *i != nullptr; i++) {
            ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

            if (draw) {
                draw->Replace_Model_Condition_State(m_conditionState);
            }
        }

        m_isModelDirty = false;
    }

    return modules;
}

const DrawModule **Drawable::Get_Draw_Modules() const
{
    DrawModule **modules = reinterpret_cast<DrawModule **>(Get_Modules(MODULE_DRAW));

    if (m_isModelDirty) {
        if (s_modelLockCount > 0) {
            captainslog_debug("Should not need to update dirty stuff while locked-for-iteration. Ignoring.");
            return const_cast<const DrawModule **>(modules);
        }

        for (DrawModule **i = modules; *i != nullptr; i++) {
            ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

            if (draw) {
                draw->Replace_Model_Condition_State(m_conditionState);
            }
        }

        m_isModelDirty = false;
    }

    return const_cast<const DrawModule **>(modules);
}

bool Drawable::Get_Current_Worldspace_Client_Bone_Positions(char const *bone_name_prefix, Matrix3D &transform) const
{
    for (const DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        const ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw && draw->Get_Current_Worldspace_Client_Bone_Positions(bone_name_prefix, transform)) {
            return true;
        }
    }

    return false;
}

void Drawable::Set_Instance_Matrix(Matrix3D const *matrix)
{
    if (matrix != nullptr) {
        m_instance = *matrix;
        m_instanceIsIdentity = false;
    } else {
        m_instance.Make_Identity();
        m_instanceIsIdentity = true;
    }
}

void Drawable::Set_Position(Coord3D const *pos)
{
    Thing::Set_Position(pos);
}

ClientUpdateModule *Drawable::Find_Client_Update_Module(NameKeyType key)
{
    ClientUpdateModule **modules = Get_Client_Update_Modules();

    if (modules != nullptr) {
        while (*modules != nullptr) {
            if ((*modules)->Get_Module_Name_Key() == key) {
                return *modules;
            }

            modules++; // BUGFIX taken from BFME2
        }
    }

    return nullptr;
}

int Drawable::Get_Pristine_Bone_Positions(
    char const *bone_name_prefix, int start_index, Coord3D *positions, Matrix3D *transforms, int max_bones) const
{
    int ret = 0;
    for (const DrawModule **i = Get_Draw_Modules(); *i != nullptr && max_bones > 0; i++) {
        const ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            int val = draw->Get_Pristine_Bone_Positions_For_Condition_State(
                m_conditionState, bone_name_prefix, start_index, positions, transforms, max_bones);

            if (val > 0) {
                ret += val;

                if (positions) {
                    positions += val;
                }
                if (transforms) {
                    transforms += val;
                }

                max_bones -= val;
            }
        }
    }

    return ret;
}

bool Drawable::Get_Should_Animate(bool when_powered) const
{
    const Object *object = Get_Object();

    if (object == nullptr) {
        return true;
    }

    if (when_powered && object->Get_Script_Status(STATUS_POWERED)) {
        return false;
    }

    if (!object->Is_Disabled()) {
        return true;
    }

    if (!object->Is_KindOf(KINDOF_PRODUCED_AT_HELIPAD)
        && (object->Get_Disabled_State(DISABLED_TYPE_DISABLED_HACKED)
            || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_PARALYZED)
            || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_EMP)
            || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_SUBDUED)
            || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_UNMANNED))) {
        return false;
    }

    if (when_powered && object->Get_Disabled_State(DISABLED_TYPE_DISABLED_UNDERPOWERED)) {
        return false;
    }

    return true;
}

void Drawable::Clear_And_Set_Model_Condition_State(ModelConditionFlagType clr, ModelConditionFlagType set)
{
    BitFlags<MODELCONDITION_COUNT> c;
    BitFlags<MODELCONDITION_COUNT> s;

    if (clr != -1) {
        c.Set(clr, 1);
    }

    if (set != -1) {
        s.Set(set, 1);
    }

    Clear_And_Set_Model_Condition_Flags(c, s);
}

void Drawable::Clear_And_Set_Model_Condition_Flags(
    BitFlags<MODELCONDITION_COUNT> const &clr, BitFlags<MODELCONDITION_COUNT> const &set)
{
    BitFlags<MODELCONDITION_COUNT> state = m_conditionState;
    m_conditionState.Clear_And_Set(clr, set);

    if (!(m_conditionState == state)) {
        m_isModelDirty = true;
    }
}

void Drawable::Init_Static_Images()
{
    if (!s_staticImagesInited) {
        s_veterancyImage[0] = nullptr;
        s_veterancyImage[1] = g_theMappedImageCollection->Find_Image_By_Name("SCVeter1");
        s_veterancyImage[2] = g_theMappedImageCollection->Find_Image_By_Name("SCVeter2");
        s_veterancyImage[3] = g_theMappedImageCollection->Find_Image_By_Name("SCVeter3");
        s_fullAmmo = g_theMappedImageCollection->Find_Image_By_Name("SCPAmmoFull");
        s_emptyAmmo = g_theMappedImageCollection->Find_Image_By_Name("SCPAmmoEmpty");
        s_fullContainer = g_theMappedImageCollection->Find_Image_By_Name("SCPPipFull");
        s_emptyContainer = g_theMappedImageCollection->Find_Image_By_Name("SCPPipEmpty");
        s_animationTemplates = new Anim2DTemplate *[MAX_ICONS];
        s_animationTemplates[ICON_HEAL] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_HEAL]);
        s_animationTemplates[ICON_HEAL_STRUCTURE] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_HEAL_STRUCTURE]);
        s_animationTemplates[ICON_HEAL_VEHICLE] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_HEAL_VEHICLE]);
        s_animationTemplates[ICON_BOMB_TIMED] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_BOMB_TIMED]);
        s_animationTemplates[ICON_BOMB_REMOTE] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_BOMB_REMOTE]);
        s_animationTemplates[ICON_DISABLED] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_DISABLED]);
        s_animationTemplates[ICON_BATTLEPLAN_BOMBARDMENT] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_BATTLEPLAN_BOMBARDMENT]);
        s_animationTemplates[ICON_BATTLEPLAN_HOLDTHELINE] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_BATTLEPLAN_HOLDTHELINE]);
        s_animationTemplates[ICON_BATTLEPLAN_SEARCHANDDESTROY] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_BATTLEPLAN_SEARCHANDDESTROY]);
        s_animationTemplates[ICON_EMOTICON] = nullptr;
        s_animationTemplates[ICON_ENTHUSIASTIC] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_ENTHUSIASTIC]);
        s_animationTemplates[ICON_SUBLIMINAL] =
            g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_SUBLIMINAL]);
        s_animationTemplates[ICON_CARBOMB] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[ICON_CARBOMB]);
        s_staticImagesInited = true;
    }
}

void Drawable::Kill_Static_Images()
{
    // static image instances are destroyed by the ImageCollection destructor and don't need to be freed here.
    if (s_animationTemplates != nullptr) {
        delete[] s_animationTemplates;
        s_animationTemplates = nullptr;
    }
}

bool Drawable::Draws_Any_UI_Text()
{
    if (!Is_Selected()) {
        return false;
    }

    Object *object = Get_Object();

    if (object == nullptr || !object->Is_Locally_Controlled()) {
        return false;
    }

    Player *player = object->Get_Controlling_Player();
    int squad = player->Get_Squad_Number_For_Object(object);

    if (squad > -1 && squad < 10) {
        return true;
    }

    m_groupString = nullptr;
    return object->Get_Formation_ID() != INVALID_FORMATION_ID;
}

bool Compute_Health_Region(Drawable const *drawable, IRegion2D &region)
{
    if (drawable == nullptr) {
        return false;
    }

    const Object *object = drawable->Get_Object();

    if (object == nullptr) {
        return false;
    }

    Coord3D pos;
    object->Get_Health_Box_Position(pos);
    ICoord2D screen;

    if (!g_theTacticalView->World_To_Screen_Tri(&pos, &screen)) {
        return false;
    }

    float width;
    float height;

    if (!object->Get_Health_Box_Dimensions(width, height)) {
        return false;
    }

    float zoom = 1.0f / g_theTacticalView->Get_Zoom();
    height = height * zoom;
    width = 3.0f;
    region.lo.x = screen.x - height * 0.45f;
    region.lo.y = screen.y - width * 0.5f;
    region.hi.x = region.lo.x + height;
    region.hi.y = region.lo.y + width;
    return true;
}

void Drawable::Draw_Icon_UI()
{
    if (g_theGameLogic->Get_Draw_Icon_UI() && g_theScriptEngine->Get_Fade() == ScriptEngine::FADE_NONE) {
        IRegion2D healthregion;
        IRegion2D *region = nullptr;

        if (Compute_Health_Region(this, healthregion)) {
            region = &healthregion;
        }

        Object *object = Get_Object();

        if (object) {
            Draw_Health_Bar(region);
            Draw_Emoticon(region);
            Draw_Caption(region);
            Draw_Construct_Percent(region);

            if (!object->Is_Effectively_Dead() && !object->Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
                Draw_Healing(region);
                Draw_Bombed(region);

                if (Draws_Any_UI_Text()) {
                    g_theGameClient->Add_Text_Bearing_Drawable(this);
                }

                Draw_Enthusiastic(region);
                Draw_Disabled(region);
                Draw_Ammo(region);
                Draw_Contained(region);
                Draw_Veterancy(region);
            }
        }
    }
}

void Drawable::Draw_Emoticon(IRegion2D const *region)
{
    if (Has_Drawable_Icon_Info() && Get_Icon_Info()->anims[ICON_EMOTICON] != nullptr) {
        unsigned int frame = g_theGameLogic->Get_Frame();

        if (region && Get_Icon_Info()->timings[ICON_EMOTICON] >= frame) {
            int width = Get_Icon_Info()->anims[ICON_EMOTICON]->Get_Current_Frame_Width();
            int height = Get_Icon_Info()->anims[ICON_EMOTICON]->Get_Current_Frame_Height();
            Get_Icon_Info()->anims[ICON_EMOTICON]->Draw(
                region->lo.x + (region->hi.x - region->lo.x) * 0.5f - width * 0.5f, region->hi.y - height, width, height);
        } else {
            Clear_Emoticon();
        }
    }
}

void Drawable::Clear_Emoticon()
{
    if (Has_Drawable_Icon_Info()) {
        Kill_Icon(ICON_EMOTICON);
    }
}

DrawableIconInfo *Drawable::Get_Icon_Info()
{
    if (!m_drawableIconInfo) {
        m_drawableIconInfo = new DrawableIconInfo();
    }

    return m_drawableIconInfo;
}

void Drawable::Draw_Ammo(IRegion2D const *region)
{
    Object *object = Get_Object();

    if (g_theWriteableGlobalData->m_showObjectHealth) {
        if (Is_Selected() || (g_theInGameUI && g_theInGameUI->Get_Moused_Over_Drawable_ID() == Get_ID())) {
            Player *player = object->Get_Controlling_Player();

            if (player == g_thePlayerList->Get_Local_Player()) {
                int clip_size;
                int ammo_in_clip;

                if (object->Get_Ammo_Pip_Showing_Info(clip_size, ammo_in_clip)) {
                    if (s_fullAmmo != nullptr && s_emptyAmmo != nullptr) {
                        float scale = 1.0f;
                        int width = GameMath::Fast_To_Int_Truncate(s_emptyAmmo->Get_Image_Width() * scale);
                        int height = GameMath::Fast_To_Int_Truncate(s_emptyAmmo->Get_Image_Height() * scale);
                        Coord3D pos = *object->Get_Position();
                        pos.x += g_theWriteableGlobalData->m_ammoPipWorldOffset.x;
                        pos.y += g_theWriteableGlobalData->m_ammoPipWorldOffset.y;
                        pos.z += object->Get_Geometry_Info().Get_Max_Height_Above_Position()
                            + g_theWriteableGlobalData->m_ammoPipWorldOffset.z;
                        ICoord2D screen;

                        if (g_theTacticalView->World_To_Screen_Tri(&pos, &screen)) {
                            float radius = object->Get_Geometry_Info().Get_Bounding_Sphere_Radius() * scale;
                            int left = region->lo.x;
                            int top =
                                GameMath::Fast_To_Int_Truncate(radius * g_theWriteableGlobalData->m_ammoPipScreenOffset.y)
                                + screen.y;

                            for (int i = 0; i < clip_size; i++) {
                                if (i < ammo_in_clip) {
                                    g_theDisplay->Draw_Image(s_fullAmmo,
                                        left,
                                        top + 1,
                                        width + left,
                                        top + height + 1,
                                        0xFFFFFFFF,
                                        Display::DRAWIMAGE_ADDITIVE);
                                } else {
                                    g_theDisplay->Draw_Image(s_emptyAmmo,
                                        left,
                                        top + 1,
                                        width + left,
                                        top + height + 1,
                                        0xFFFFFFFF,
                                        Display::DRAWIMAGE_ADDITIVE);
                                }

                                left += width + 1;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Drawable::Draw_Contained(IRegion2D const *region)
{
    Object *object = Get_Object();
    ContainModuleInterface *contain = object->Get_Contain();

    if (contain != nullptr) {
        if (g_theWriteableGlobalData->m_showObjectHealth) {
            if (Is_Selected() || (g_theInGameUI && g_theInGameUI->Get_Moused_Over_Drawable_ID() == Get_ID())) {
                Player *player = object->Get_Controlling_Player();
                int max;
                int count;

                if (player == g_thePlayerList->Get_Local_Player() && contain->Get_Container_Pips_To_Show(max, count)
                    && count != 0) {
                    int infantry_count = 0;
                    const std::list<Object *> *list = contain->Get_Contained_Items_List();

                    if (list) {
                        for (Object *obj : (*list)) {
                            if (obj->Is_KindOf(KINDOF_INFANTRY)) {
                                infantry_count++;
                            }
                        }
                    }

                    float scale = 1.0f;
                    int width = GameMath::Fast_To_Int_Truncate(s_emptyAmmo->Get_Image_Width() * scale);
                    int height = GameMath::Fast_To_Int_Truncate(s_emptyAmmo->Get_Image_Height() * scale);
                    Coord3D pos = *object->Get_Position();
                    pos.x += g_theWriteableGlobalData->m_containerPipWorldOffset.x;
                    pos.y += g_theWriteableGlobalData->m_containerPipWorldOffset.y;
                    pos.z += object->Get_Geometry_Info().Get_Max_Height_Above_Position()
                        + g_theWriteableGlobalData->m_containerPipWorldOffset.z;
                    ICoord2D screen;

                    if (g_theTacticalView->World_To_Screen_Tri(&pos, &screen)) {
                        float radius = object->Get_Geometry_Info().Get_Bounding_Sphere_Radius() * scale;
                        int left = region->lo.x;
                        int top =
                            GameMath::Fast_To_Int_Truncate(radius * g_theWriteableGlobalData->m_containerPipScreenOffset.y)
                            + screen.y;

                        for (int i = 0; i < max; i++) {
                            constexpr uint32_t infantry = Make_Color(0, 255, 0, 255);
                            constexpr uint32_t vehicle = Make_Color(0, 0, 255, 255);

                            if (i < count) {
                                if (i < infantry_count) {
                                    g_theDisplay->Draw_Image(s_fullContainer,
                                        left,
                                        top + 1,
                                        width + left,
                                        top + height + 1,
                                        infantry,
                                        Display::DRAWIMAGE_ADDITIVE);
                                } else {
                                    g_theDisplay->Draw_Image(s_fullContainer,
                                        left,
                                        top + 1,
                                        width + left,
                                        top + height + 1,
                                        vehicle,
                                        Display::DRAWIMAGE_ADDITIVE);
                                }
                            } else {
                                g_theDisplay->Draw_Image(s_emptyContainer,
                                    left,
                                    top + 1,
                                    width + left,
                                    top + height + 1,
                                    0xFFFFFFFF,
                                    Display::DRAWIMAGE_ADDITIVE);
                            }

                            left += width + 1;
                        }
                    }
                }
            }
        }
    }
}

void Drawable::Draw_Healing(IRegion2D const *region)
{
    Object *object = Get_Object();

    if (!object->Is_KindOf(KINDOF_NO_HEAL_ICON)) {
        const BitFlags<OBJECT_STATUS_COUNT> &bits = object->Get_Status_Bits();

        if (!bits.Test(OBJECT_STATUS_SOLD)) {
            bool draw = false;
            BodyModuleInterface *body = object->Get_Body_Module();

            if (body->Get_Max_Health() != body->Get_Health() && g_theGameLogic->Get_Frame() > 90) {
                if (g_theGameLogic->Get_Frame() - body->Get_Last_Healing_Timestamp() <= 90) {
                    draw = true;
                }
            }

            DrawableIconType icon;

            if (Is_KindOf(KINDOF_STRUCTURE)) {
                icon = ICON_HEAL_STRUCTURE;
            } else if (Is_KindOf(KINDOF_VEHICLE)) {
                icon = ICON_HEAL_VEHICLE;
            } else {
                icon = ICON_HEAL;
            }

            if (draw) {
                if (region) {
                    if (Get_Icon_Info()->anims[icon] == nullptr) {
                        Get_Icon_Info()->anims[icon] = new Anim2D(s_animationTemplates[icon], g_theAnim2DCollection);
                    }

                    if (Get_Icon_Info()->anims[icon] != nullptr) {
                        int width = Get_Icon_Info()->anims[icon]->Get_Current_Frame_Width();
                        int height = Get_Icon_Info()->anims[icon]->Get_Current_Frame_Height();
                        int x = GameMath::Fast_To_Int_Truncate(
                            region->lo.x + (region->hi.x - region->lo.x) * 0.75f - width * 0.5f);
                        int y = GameMath::Fast_To_Int_Truncate(region->lo.y - height);
                        Get_Icon_Info()->anims[icon]->Draw(x, y, width, height);
                    }
                }
            } else {
                Kill_Icon(icon);
            }
        }
    }
}

void Drawable::Draw_Enthusiastic(IRegion2D const *region)
{
    Object *object = Get_Object();

    if (object->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_ENTHUSIASTIC) && region != nullptr) {
        DrawableIconType icon = ICON_ENTHUSIASTIC;

        if (object->Test_Weapon_Bonus_Condition(WEAPONBONUSCONDITION_SUBLIMINAL)) {
            icon = ICON_SUBLIMINAL;
        }

        if (Get_Icon_Info()->anims[icon] == nullptr) {
            Get_Icon_Info()->anims[icon] = new Anim2D(s_animationTemplates[icon], g_theAnim2DCollection);
        }

        if (Get_Icon_Info()->anims[icon] != nullptr) {
            float scale;

            if (Is_KindOf(KINDOF_STRUCTURE) || Is_KindOf(KINDOF_HUGE_VEHICLE)) {
                scale = 1.0f;
            } else if (Is_KindOf(KINDOF_VEHICLE)) {
                scale = 0.75f;
            } else {
                scale = 0.5f;
            }

            int width = Get_Icon_Info()->anims[icon]->Get_Current_Frame_Width() * scale;
            int height = Get_Icon_Info()->anims[icon]->Get_Current_Frame_Height() * scale;
            int x = GameMath::Fast_To_Int_Truncate(region->lo.x + (region->hi.x - region->lo.x) * 0.25f - width * 0.5f);
            int y = region->hi.y + height * 0.25f;
            Get_Icon_Info()->anims[icon]->Draw(x, y, width, height);
        }
    } else {
        Kill_Icon(ICON_ENTHUSIASTIC);
        Kill_Icon(ICON_SUBLIMINAL);
    }
}

void Drawable::Draw_Bombed(IRegion2D const *region)
{
    Object *object = Get_Object();
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (object->Test_Weapon_Set_Flag(WEAPONSET_CARBOMB)
        && object->Get_Controlling_Player() == g_thePlayerList->Get_Local_Player()) {
        if (Get_Icon_Info()->anims[ICON_CARBOMB] == nullptr) {
            Get_Icon_Info()->anims[ICON_CARBOMB] = new Anim2D(s_animationTemplates[ICON_CARBOMB], g_theAnim2DCollection);
        }

        if (Get_Icon_Info()->anims[ICON_CARBOMB] != nullptr && region != nullptr) {
            int fwidth = Get_Icon_Info()->anims[ICON_CARBOMB]->Get_Current_Frame_Width();
            int fheight = Get_Icon_Info()->anims[ICON_CARBOMB]->Get_Current_Frame_Height();
            int width = GameMath::Fast_To_Int_Truncate((region->hi.x - region->lo.x) * 0.5f);
            int height = GameMath::Fast_To_Int_Truncate((float)width / (float)(fwidth) * (float(fheight)));
            int x = GameMath::Fast_To_Int_Truncate(region->lo.x + (region->hi.x - region->lo.x) * 0.5f - width * 0.5f);
            int y = GameMath::Fast_To_Int_Truncate(region->lo.y + (region->hi.y - region->lo.y) * 0.5f) + 5;
            Get_Icon_Info()->anims[ICON_CARBOMB]->Draw(x, y, width, height);
            Get_Icon_Info()->timings[ICON_CARBOMB] = 0x3FFFFFFF;
        }
    } else {
        Kill_Icon(ICON_CARBOMB);
    }

    static const NameKeyType _stickyBombUpdateKey = g_theNameKeyGenerator->Name_To_Key("StickyBombUpdate");
    StickyBombUpdate *update = static_cast<StickyBombUpdate *>(object->Find_Update_Module(_stickyBombUpdateKey));

    if (update != nullptr && update->Get_Target_Object() != nullptr) {
        if (update->Has_Die_Frame()) {
            if (Get_Icon_Info()->anims[ICON_BOMB_TIMED] == nullptr) {
                captainslog_dbgassert(
                    Get_Icon_Info()->anims[ICON_BOMB_REMOTE] == nullptr, "Must be null, otherwise would leak here");

                Get_Icon_Info()->anims[ICON_BOMB_REMOTE] =
                    new Anim2D(s_animationTemplates[ICON_BOMB_REMOTE], g_theAnim2DCollection);

                Get_Icon_Info()->anims[ICON_BOMB_TIMED] =
                    new Anim2D(s_animationTemplates[ICON_BOMB_TIMED], g_theAnim2DCollection);

                int i1 = GameMath::Fast_To_Int_Ceil((update->Get_Die_Frame() - frame) * (1.0f / 30.0f));
                unsigned short frames = Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Get_Template()->Get_Num_Frames();

                if (i1 > frames - 1) {
                    i1 = frames - 1;
                }

                Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Set_First_Frame(frames - i1 - 1);
                Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Reset();
            }

            if (Get_Icon_Info()->anims[ICON_BOMB_TIMED] != nullptr && region != nullptr) {
                int fwidth = Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Get_Current_Frame_Width();
                int fheight = Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Get_Current_Frame_Height();
                int width = GameMath::Fast_To_Int_Truncate((region->hi.x - region->lo.x) * 0.65f);
                int height = GameMath::Fast_To_Int_Truncate((float)width / (float)fwidth * (float)fheight);
                int x = GameMath::Fast_To_Int_Truncate(region->lo.x + (region->hi.x - region->lo.x) * 0.5f - width * 0.5f);
                int y = GameMath::Fast_To_Int_Truncate(region->lo.y + (region->hi.y - region->lo.y) * 0.5f) + 5;
                Get_Icon_Info()->anims[ICON_BOMB_REMOTE]->Draw(x, y, width, height);
                Get_Icon_Info()->timings[ICON_BOMB_REMOTE] = frame + 1;
                Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Draw(x, y, width, height);
                Get_Icon_Info()->timings[ICON_BOMB_TIMED] = frame + 1;
            }
        } else {
            if (Get_Icon_Info()->anims[ICON_BOMB_REMOTE] == nullptr) {
                Get_Icon_Info()->anims[ICON_BOMB_REMOTE] =
                    new Anim2D(s_animationTemplates[ICON_BOMB_REMOTE], g_theAnim2DCollection);
            }

            if (Get_Icon_Info()->anims[ICON_BOMB_REMOTE] != nullptr && region != nullptr) {
                int fwidth = Get_Icon_Info()->anims[ICON_BOMB_REMOTE]->Get_Current_Frame_Width();
                int fheight = Get_Icon_Info()->anims[ICON_BOMB_REMOTE]->Get_Current_Frame_Height();
                int width = GameMath::Fast_To_Int_Truncate((region->hi.x - region->lo.x) * 0.65f);
                int height = GameMath::Fast_To_Int_Truncate((float)width / (float)fwidth * (float)fheight);
                int x = GameMath::Fast_To_Int_Truncate(region->lo.x + (region->hi.x - region->lo.x) * 0.5f - width * 0.5f);
                int y = GameMath::Fast_To_Int_Truncate(region->lo.y + (region->hi.y - region->lo.y) * 0.5f) + 5;
                Get_Icon_Info()->anims[ICON_BOMB_REMOTE]->Draw(x, y, width, height);
                Get_Icon_Info()->timings[ICON_BOMB_REMOTE] = frame + 1;
            }
        }
    }

    if (Has_Drawable_Icon_Info()) {
        if (Get_Icon_Info()->timings[ICON_BOMB_TIMED] <= frame) {
            Kill_Icon(ICON_BOMB_TIMED);
        }
        if (Get_Icon_Info()->timings[ICON_BOMB_REMOTE] <= frame) {
            Kill_Icon(ICON_BOMB_REMOTE);
        }
    }
}

void Drawable::Draw_Disabled(IRegion2D const *region)
{
    Object *object = Get_Object();

    if (object->Get_Disabled_State(DISABLED_TYPE_DISABLED_HACKED)
        || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_PARALYZED)
        || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_EMP)
        || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_SUBDUED)
        || object->Get_Disabled_State(DISABLED_TYPE_DISABLED_UNDERPOWERED)) {
        if (Get_Icon_Info()->anims[ICON_DISABLED] == nullptr) {
            Get_Icon_Info()->anims[ICON_DISABLED] = new Anim2D(s_animationTemplates[ICON_DISABLED], g_theAnim2DCollection);
        }

        if (region != nullptr) {
            int width = Get_Icon_Info()->anims[ICON_DISABLED]->Get_Current_Frame_Width();
            int height = Get_Icon_Info()->anims[ICON_DISABLED]->Get_Current_Frame_Height();
            int y = region->hi.y - ((region->hi.y - region->lo.y) + height);
            int x = region->lo.x;
            Get_Icon_Info()->anims[ICON_DISABLED]->Draw(x, y, width, height);
        }
    } else {
        Kill_Icon(ICON_DISABLED);
    }
}

void Drawable::Draw_Construct_Percent(IRegion2D const *region)
{
    Object *object = Get_Object();

    if (object != nullptr && object->Get_Status_Bits().Test(OBJECT_STATUS_UNDER_CONSTRUCTION)
        && !object->Get_Status_Bits().Test(OBJECT_STATUS_SOLD)) {
        if (m_constructDisplayString == nullptr) {
            m_constructDisplayString = g_theDisplayStringManager->New_Display_String();
        }

        if (object->Get_Construction_Percent() != m_lastConstructDisplayed) {
            Utf16String str;
            str.Format(g_theGameText->Fetch("CONTROLBAR:UnderConstructionDesc"), object->Get_Construction_Percent());
            m_constructDisplayString->Set_Text(str);
            m_lastConstructDisplayed = object->Get_Construction_Percent();
        }

        const Coord3D *pos = Get_Position();
        Coord3D center;
        Get_Drawable_Geometry_Info().Get_Center_Position(*pos, center);
        ICoord2D screen;
        g_theTacticalView->World_To_Screen_Tri(&center, &screen);

        if (screen.x >= 1) {
            m_constructDisplayString->Draw(screen.x - m_constructDisplayString->Get_Width(-1) / 2,
                screen.y,
                Make_Color(255, 255, 255, 255),
                Make_Color(0, 0, 0, 255));
        }
    } else if (m_constructDisplayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_constructDisplayString);
        m_constructDisplayString = nullptr;
    }
}

GeometryInfo const &Drawable::Get_Drawable_Geometry_Info() const
{
    if (Get_Object() != nullptr) {
        return Get_Object()->Get_Geometry_Info();
    } else {
        return Get_Template()->Get_Template_Geometry_Info();
    }
}

void Drawable::Draw_Caption(IRegion2D const *region)
{
    if (m_captionText != nullptr) {
        const Coord3D *pos = Get_Position();
        Coord3D center;
        Get_Drawable_Geometry_Info().Get_Center_Position(*pos, center);
        ICoord2D screen;
        g_theTacticalView->World_To_Screen_Tri(&center, &screen);
        screen.x -= m_captionText->Get_Width(-1) / 2;

        int x;
        int y;
        m_captionText->Get_Size(&x, &y);
        g_theDisplay->Draw_Fill_Rect(screen.x - 1, screen.y - 1, x + 2, y + 2, Make_Color(0, 0, 0, 125));
        g_theDisplay->Draw_Open_Rect(screen.x - 1, screen.y - 1, x + 2, y + 2, 1.0f, Make_Color(20, 20, 20, 255));
        m_captionText->Draw(screen.x, screen.y, g_theInGameUI->Get_Drawable_Caption_Color(), Make_Color(0, 0, 0, 255));
    }
}

void Drawable::Draw_Veterancy(IRegion2D const *region)
{
    Object *object = Get_Object();

    if (object->Get_Experience_Tracker() != nullptr) {
        VeterancyLevel level = object->Get_Veterancy_Level();
        Image *image = s_veterancyImage[level];

        if (image != nullptr) {
            float scale = 1.0f;
            float iwidth = image->Get_Image_Width() * scale;
            float iheight = image->Get_Image_Height() * scale;
            Coord3D pos;
            object->Get_Health_Box_Position(pos);
            ICoord2D screen;

            if (g_theTacticalView->World_To_Screen_Tri(&pos, &screen)) {
                float width;
                float height;

                if (object->Get_Health_Box_Dimensions(width, height)) {
                    screen.x = screen.x + height * (1.3f / g_theTacticalView->Get_Zoom()) * 0.5f;
                    g_theDisplay->Draw_Image(image,
                        screen.x + 1,
                        screen.y + 1,
                        screen.x + 1 + iwidth,
                        screen.y + 1 + iheight,
                        0xFFFFFFFF,
                        Display::DRAWIMAGE_ADDITIVE);
                }
            }
        }
    }
}

void Drawable::Draw_Health_Bar(IRegion2D const *region)
{
    if (region != nullptr) {
        if (g_theWriteableGlobalData->m_showObjectHealth) {
            if (Is_Selected() || (g_theInGameUI && g_theInGameUI->Get_Moused_Over_Drawable_ID() == Get_ID())) {
                Object *object = Get_Object();

                if (object != nullptr) {
                    if (!object->Is_KindOf(KINDOF_FORCEATTACKABLE)) {
                        BodyModuleInterface *body = object->Get_Body_Module();
                        float health = body->Get_Health();
                        float max_health = body->Get_Max_Health();

                        if (max_health != 0.0f && health != 0.0f) {
                            float health_percent = health / max_health;
                            int bar_color;
                            int outline_color;

                            if (object->Get_Status_Bits().Test(OBJECT_STATUS_UNDER_CONSTRUCTION)
                                || (object->Is_Disabled() && !object->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD))) {
                                bar_color = Make_Color(0, health_percent * 255.0f, 255, 255);
                                outline_color = Make_Color(0, health_percent * 128.0f, 128, 255);
                            } else {
                                float bar_color_b = 0.0f;
                                float outline_color_b = 0.0f;
                                float bar_color_r;
                                float bar_color_g;

                                if (health_percent < 0.5f) {
                                    bar_color_r = 1.0f;
                                    bar_color_g = 1.0f - (0.5f - health_percent) / 0.5f;
                                } else {
                                    bar_color_r = 1.0f - (health_percent - 0.5f) / 0.5f;
                                    bar_color_g = 1.0f;
                                }

                                float outline_color_r = bar_color_r * 0.5f;
                                float outline_color_g = bar_color_g * 0.5f;

                                if (m_conditionState.Test(MODELCONDITION_REALLYDAMAGED)) {
                                    bar_color_r = (bar_color_r + 1.0f) * 0.5f;
                                    bar_color_g = bar_color_g * 0.5f;
                                } else if (m_conditionState.Test(MODELCONDITION_DAMAGED)) {
                                    bar_color_g = (bar_color_g + 1.0f) * 0.5f;
                                    bar_color_r = bar_color_r * 0.5f;
                                }

                                bar_color =
                                    Make_Color(bar_color_r * 255.0f, bar_color_g * 255.0f, bar_color_b * 255.0f, 255);
                                outline_color = Make_Color(
                                    outline_color_r * 255.0f, outline_color_g * 255.0f, outline_color_b * 255.0f, 255);
                            }

                            int width = region->hi.x - region->lo.x;
                            int height = std::max(3, region->hi.y - region->lo.y);
                            g_theDisplay->Draw_Open_Rect(region->lo.x, region->lo.y, width, height, 1.0f, outline_color);
                            g_theDisplay->Draw_Fill_Rect(region->lo.x + 1,
                                region->lo.y + 1,
                                (width - 2.0f) * health_percent,
                                height - 2.0f,
                                bar_color);
                        }
                    }
                }
            }
        }
    }
}

void Drawable::Set_Emoticon(Utf8String const &emoticon, int frames)
{
    Clear_Emoticon();
    Anim2DTemplate *tmplate = g_theAnim2DCollection->Find_Template(emoticon);

    if (tmplate != nullptr) {
        captainslog_dbgassert(Get_Icon_Info()->anims[ICON_EMOTICON] == nullptr,
            "Drawable::Set_Emoticon - Emoticon isn't empty, need to refuse to set or destroy the old one in favor of the "
            "new "
            "one");
        if (Get_Icon_Info()->anims[ICON_EMOTICON] == nullptr) {
            Get_Icon_Info()->anims[ICON_EMOTICON] = new Anim2D(tmplate, g_theAnim2DCollection);

            if (frames < 0) {
                Get_Icon_Info()->timings[ICON_EMOTICON] = 0x3FFFFFFF;
            } else {
                Get_Icon_Info()->timings[ICON_EMOTICON] = frames + g_theGameLogic->Get_Frame();
            }
        }
    }
}

void Drawable::Draw_UI_Text()
{
    IRegion2D healthregion;
    IRegion2D *region = nullptr;

    if (Compute_Health_Region(this, healthregion)) {
        region = &healthregion;
    }

    if (region) {
        Object *object = Get_Object();
        Player *player = object->Get_Controlling_Player();
        int squad = player->Get_Squad_Number_For_Object(object);
        int color;

        if (g_theDrawGroupInfo->m_usePlayerColor) {
            color = player->Get_Color();
        } else {
            color = g_theDrawGroupInfo->m_colorForText;
        }

        if (squad > -1 && squad < 10) {
            int x = region->lo.x;
            int y = region->lo.y;

            if (g_theDrawGroupInfo->m_drawPositionXAbsolute) {
                x += g_theDrawGroupInfo->m_drawPositionX.absolute;
            } else {
                x = (region->hi.x - region->lo.x) * g_theDrawGroupInfo->m_drawPositionX.relative + x;
            }

            if (g_theDrawGroupInfo->m_drawPositionYAbsolute) {
                y += g_theDrawGroupInfo->m_drawPositionY.absolute;
            } else {
                y = (region->hi.y - region->lo.y) * g_theDrawGroupInfo->m_drawPositionY.relative + y;
            }

            m_groupString = g_theDisplayStringManager->Get_Group_Numeral_String(squad);
            m_groupString->Draw(x,
                y,
                color,
                g_theDrawGroupInfo->m_colorForTextDropShadow,
                g_theDrawGroupInfo->m_dropShadowOffsetX,
                g_theDrawGroupInfo->m_dropShadowOffsetY);
        }

        if (object->Get_Formation_ID()) {
            Coord3D pos;
            object->Get_Health_Box_Position(pos);
            ICoord2D screen;

            if (g_theTacticalView->World_To_Screen_Tri(&pos, &screen)) {
                float width;
                float height;

                if (object->Get_Health_Box_Dimensions(width, height)) {
                    screen.x += height * (1.3f / g_theTacticalView->Get_Zoom()) * 0.5f + 10.0f;
                    DisplayString *string = g_theDisplayStringManager->Get_Formation_Letter_String();

                    if (string) {
                        string->Draw(screen.x,
                            screen.y,
                            color,
                            g_theDrawGroupInfo->m_colorForTextDropShadow,
                            g_theDrawGroupInfo->m_dropShadowOffsetX,
                            g_theDrawGroupInfo->m_dropShadowOffsetY);
                    }
                }
            }
        }
    }
}

Drawable::Drawable(ThingTemplate const *thing_template, DrawableStatus status) :
    Thing(thing_template),
    m_terrainDecal(TERRAIN_DECAL_8),
    m_opacity(1.0f),
    m_effectiveOpacity1(1.0f),
    m_effectiveOpacity2(1.0f),
    m_terrainDecalFadeTarget1(0.0f),
    m_terrainDecalFadeTarget2(0.0f),
    m_terrainDecalOpacity(0.0f),
    m_nextDrawable(nullptr),
    m_prevDrawable(nullptr),
    m_customSoundAmbientInfo(nullptr),
    m_status(status),
    m_fadingMode(FADING_MODE_OFF),
    m_curFadeFrame(0),
    m_timeToFade(0),
    m_remainVisibleFrames(0),
    m_drawableLocoInfo(nullptr),
    m_ambientSound(nullptr),
    m_stealthLook(STEALTHLOOK_NONE),
    m_flashColor(0),
    m_flashTime(0),
    m_lastConstructDisplayed(-1.0f),
    m_expirationDate(0),
    m_selected(false),
    m_ambientSoundEnabled(true),
    m_ambientSoundFromScriptEnabled(true)
{
    g_theGameClient->Register_Drawable(this);
    m_constructDisplayString = g_theDisplayStringManager->New_Display_String();
    m_constructDisplayString->Set_Font(g_theFontLibrary->Get_Font(g_theInGameUI->Get_Drawable_Caption_Font(),
        g_theGlobalLanguage->Adjust_Font_Size(g_theInGameUI->Get_Drawable_Caption_Size()),
        g_theInGameUI->Get_Drawable_Caption_Bold()));

    for (int i = 0; i < NUM_DRAWABLE_MODULE_TYPES; i++) {
        m_modules[i] = nullptr;
    }

    if (g_theGameClient != nullptr && thing_template != nullptr) {
        m_instance.Make_Identity();
        m_instanceIsIdentity = true;
        m_instanceScale = thing_template->Get_Asset_Scale();
        m_object = nullptr;
        m_drawBits = 0;
        m_previousDrawBits = 0;
        m_isModelDirty = true;
        m_hidden = false;
        m_stealthInvisible = false;
        m_stealthEmissiveScale = 0.0f;
        m_fullyObscuredByShroud = false;
        m_receivesDynamicLights = true;
        const ModuleInfo *draw = thing_template->Get_Draw_Modules();
        m_modules[0] = new Module *[draw->Get_Count() + 1];

        for (unsigned int i = 0; i < draw->Get_Count(); i++) {
            const ModuleData *data = draw->Get_Nth_Data(i);

            if (g_theWriteableGlobalData->m_extraAnimationsDisabled) {
                if (data->Get_Minimum_Required_Game_LOD() > g_theGameLODManager->Get_Static_LOD_Level()) {
                    continue;
                }
            }

            m_modules[0][i] = g_theModuleFactory->New_Module(this, draw->Get_Nth_Name(i), data, MODULE_DRAW);
        }

        m_modules[0][draw->Get_Count()] = nullptr;

        const ModuleInfo *update = thing_template->Get_Client_Update_Modules();

        if (update->Get_Count() != 0) {
            m_modules[1] = new Module *[update->Get_Count() + 1];

            for (unsigned int i = 0; i < update->Get_Count(); i++) {
                const ModuleData *data = update->Get_Nth_Data(i);

                if (thing_template->Is_KindOf(KINDOF_SHRUBBERY) && !g_theWriteableGlobalData->m_useTreeSway) {
                    if (update->Get_Nth_Name(i) == "SwayClientUpdate") {
                        continue;
                    }
                }

                m_modules[1][i] = g_theModuleFactory->New_Module(this, update->Get_Nth_Name(i), data, MODULE_CLIENT_UPDATE);
            }

            m_modules[1][update->Get_Count()] = nullptr;
        }

        for (int i = 0; i < NUM_DRAWABLE_MODULE_TYPES; i++) {
            for (Module **j = m_modules[i]; j != nullptr && *j != nullptr; j++) {
                (*j)->On_Object_Created();
            }
        }

        m_groupString = nullptr;
        m_captionText = nullptr;
        m_drawableInfo.drawable = this;
        m_drawableInfo.ghost_object = nullptr;
        m_drawableIconInfo = nullptr;
        m_selectionColorEnvelope = nullptr;
        m_tintColorEnvelope = nullptr;
        Init_Static_Images();

        if (g_theGameLogic && !g_theGameLogic->Get_Prepare_New_Game() && g_theGameState && !g_theGameState->Is_Loading()) {
            Start_Ambient_Sound(false);
        }
    } else {
        // #BUGFIX Initialize all members always
        m_captionText = nullptr;
        m_drawBits = 0;
        m_drawableIconInfo = nullptr;
        m_drawableInfo = DrawableInfo{};
        m_fullyObscuredByShroud = false;
        m_groupString = nullptr;
        m_hidden = false;
        m_id = INVALID_DRAWABLE_ID;
        m_instanceIsIdentity = false;
        m_instanceScale = 1.0f;
        m_isModelDirty = false;
        m_object = nullptr;
        m_previousDrawBits = 0;
        m_receivesDynamicLights = false;
        m_selectionColorEnvelope = nullptr;
        m_stealthEmissiveScale = 1.0f;
        m_stealthInvisible = false;
        m_tintColorEnvelope = nullptr;
    }
}

void Drawable::Start_Ambient_Sound(bool unk)
{
    if (m_ambientSoundEnabled && m_ambientSoundFromScriptEnabled) {
        Stop_Ambient_Sound();
        BodyDamageType damage = BODY_PRISTINE;
        Object *object = Get_Object();

        if (object) {
            damage = object->Get_Body_Module()->Get_Damage_State();
        }

        Start_Ambient_Sound(damage, g_theWriteableGlobalData->m_timeOfDay, unk);
    }
}

void Drawable::Stop_Ambient_Sound()
{
    if (m_ambientSound) {
        g_theAudio->Remove_Audio_Event(m_ambientSound->m_event.Get_Playing_Handle());
    }
}

DynamicAudioEventInfo *Get_No_Sound_Marker()
{
    static DynamicAudioEventInfo *marker = nullptr;

    if (!marker) {
        marker = new DynamicAudioEventInfo();
    }

    return marker;
}

void Drawable::Start_Ambient_Sound(BodyDamageType damage, TimeOfDayType tod, bool unk)
{
    Stop_Ambient_Sound();
    bool play = false;

    if (damage != BODY_RUBBLE && m_customSoundAmbientInfo != nullptr) {
        if (m_customSoundAmbientInfo != Get_No_Sound_Marker()) {
            if (!m_ambientSound) {
                m_ambientSound = new DynamicAudioEventRTS();
            }

            m_ambientSound->m_event.Set_Event_Name(m_customSoundAmbientInfo->m_eventName);
            m_ambientSound->m_event.Set_Event_Info(m_customSoundAmbientInfo);
            play = true;
        }
    } else {
        const AudioEventRTS *sound = Get_Ambient_Sound_By_Damage(damage);

        if (sound->Get_Event_Name().Is_Not_Empty()) {
            if (!m_ambientSound) {
                m_ambientSound = new DynamicAudioEventRTS();
            }

            m_ambientSound->m_event = *sound;
            play = true;
        } else if (damage != BODY_PRISTINE) {
            if (damage != BODY_RUBBLE) {
                const AudioEventRTS *sound2 = Get_Ambient_Sound_By_Damage(BODY_PRISTINE);
                if (sound2->Get_Event_Name().Is_Not_Empty()) {
                    if (!m_ambientSound) {
                        m_ambientSound = new DynamicAudioEventRTS();
                    }

                    m_ambientSound->m_event = *sound2;
                    play = true;
                }
            }
        }
    }

    if (play && m_ambientSound != nullptr) {
        const AudioEventInfo *info = m_ambientSound->m_event.Get_Event_Info();

        if (info != nullptr) {
            if (!unk || info->Is_Looping()) {
                if ((info->m_visibility & VISIBILITY_GLOBAL) != 0 || info->m_priority == 4) {
                    m_ambientSound->m_event.Set_Drawable_ID(Get_ID());
                    m_ambientSound->m_event.Set_Time_Of_Day(tod);
                    m_ambientSound->m_event.Set_Playing_Handle(g_theAudio->Add_Audio_Event(&m_ambientSound->m_event));
                } else {
                    Coord3D pos = *Get_Position();
                    pos.Sub(g_theAudio->Get_Listener_Position());

                    if (GameMath::Square(info->m_maxRange) > pos.Length2()) {
                        m_ambientSound->m_event.Set_Drawable_ID(Get_ID());
                        m_ambientSound->m_event.Set_Time_Of_Day(tod);
                        m_ambientSound->m_event.Set_Playing_Handle(g_theAudio->Add_Audio_Event(&m_ambientSound->m_event));
                    }
                }
            }
        } else {
            captainslog_debug("Ambient sound %s missing! Skipping...", m_ambientSound->m_event.Get_Event_Name().Str());
            m_ambientSound->Delete_Instance();
            m_ambientSound = nullptr;
        }
    }
}

const AudioEventRTS *Drawable::Get_Ambient_Sound_By_Damage(BodyDamageType damage)
{
    if (damage == BODY_DAMAGED) {
        return Get_Template()->Get_Sound_Ambient_Damaged();
    } else if (damage == BODY_REALLYDAMAGED) {
        return Get_Template()->Get_Sound_Ambient_Really_Damaged();
    } else if (damage == BODY_RUBBLE) {
        return Get_Template()->Get_Sound_Ambient_Rubble();
    } else {
        return Get_Template()->Get_Sound_Ambient();
    }
}

void Drawable::Set_Custom_Sound_Ambient_Info(DynamicAudioEventInfo *info)
{
    Clear_Custom_Sound_Ambient(false);
    captainslog_dbgassert(info != Get_No_Sound_Marker(), "No sound marker passed as custom ambient");
    m_customSoundAmbientInfo = info;
    Start_Ambient_Sound(false);
}

void Drawable::Set_Custom_Sound_Ambient_Off()
{
    Clear_Custom_Sound_Ambient(false);
    m_customSoundAmbientInfo = Get_No_Sound_Marker();
}

void Drawable::Enable_Ambient_Sound(bool enable)
{
    if (m_ambientSoundEnabled != enable) {
        m_ambientSoundEnabled = enable;

        if (enable) {
            if (m_ambientSoundFromScriptEnabled) {
                Start_Ambient_Sound(false);
            }
        } else {
            Stop_Ambient_Sound();
        }
    }
}

void Drawable::Enable_Ambient_Sound_From_Script(bool enable)
{
    m_ambientSoundFromScriptEnabled = enable;

    if (enable) {
        if (m_ambientSoundEnabled) {
            Start_Ambient_Sound(false);
        }
    } else {
        Stop_Ambient_Sound();
    }
}

void Drawable::Clear_Custom_Sound_Ambient(bool restart)
{
    if (m_ambientSound != nullptr) {
        m_ambientSound->m_event.Set_Event_Info(nullptr);
    }

    Stop_Ambient_Sound();
    m_customSoundAmbientInfo = nullptr;

    if (restart) {
        Start_Ambient_Sound(false);
    }
}

const AudioEventInfo *Drawable::Get_Base_Sound_Ambient_Info() const
{
    const AudioEventRTS *audio = Get_Template()->Get_Sound_Ambient();

    if (audio != nullptr) {
        return audio->Get_Event_Info();
    } else {
        return nullptr;
    }
}

void Drawable::Mangle_Custom_Audio_Name(DynamicAudioEventInfo *info) const
{
    Utf8String str;
    str.Format(" CUSTOM %d ", Get_ID());
    str += info->m_eventName;
    info->Override_Audio_Name(str);
}

Drawable::~Drawable()
{
    if (m_constructDisplayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_constructDisplayString);
        m_constructDisplayString = nullptr;
    }

    if (m_captionText != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_captionText);
        m_captionText = nullptr;
    }

    m_groupString = nullptr;

    for (int i = 0; i < NUM_DRAWABLE_MODULE_TYPES; i++) {
        for (Module **j = m_modules[i]; j != nullptr && *j != nullptr; j++) {
            (*j)->Delete_Instance();
            *j = nullptr;
        }

        delete[] m_modules[i];
        m_modules[i] = nullptr;
    }

    Stop_Ambient_Sound();

    if (m_ambientSound != nullptr) {
        m_ambientSound->Delete_Instance();
        m_ambientSound = nullptr;
    }

    Clear_Custom_Sound_Ambient(false);
    g_theGameClient->Remove_From_Ray_Effects(this);
    m_object = nullptr;

    if (m_drawableIconInfo != nullptr) {
        m_drawableIconInfo->Delete_Instance();
    }

    if (m_selectionColorEnvelope != nullptr) {
        m_selectionColorEnvelope->Delete_Instance();
    }

    if (m_tintColorEnvelope != nullptr) {
        m_tintColorEnvelope->Delete_Instance();
    }

    if (m_drawableLocoInfo != nullptr) {
        m_drawableLocoInfo->Delete_Instance();
        m_drawableLocoInfo = nullptr;
    }
}

const char *Drawable_Icon_Index_To_Name(DrawableIconType index)
{
    captainslog_dbgassert(
        index >= ICON_FIRST && index < MAX_ICONS, "Drawable_Icon_Index_To_Name - Illegal index '%d'", index);
    return s_theDrawableIconNames[index];
}

DrawableIconType Drawable_Icon_Name_To_Index(const char *name)
{
    captainslog_dbgassert(name != nullptr, "drawableIconNameToIndex - Illegal name");

    for (int i = ICON_FIRST; i < MAX_ICONS; i++) {
        if (!strcasecmp(s_theDrawableIconNames[i], name)) {
            return (DrawableIconType)i;
        }
    }

    return ICON_INVALID;
}

void Drawable::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 7;
    xfer->xferVersion(&version, 7);

    if (xfer->Get_Mode() == XFER_LOAD && m_ambientSound != nullptr) {
        g_theAudio->Kill_Event_Immediately(m_ambientSound->m_event.Get_Playing_Handle());
        m_ambientSound->Delete_Instance();
        m_ambientSound = nullptr;
    }

    DrawableID id = Get_ID();
    xfer->xferDrawableID(&id);
    Set_ID(id);

    if (version >= 2) {
        m_conditionState.Xfer(xfer);

        if (xfer->Get_Mode() == XFER_LOAD) {
            Replace_Model_Condition_Flags(m_conditionState, true);
        }
    }

    if (version >= 3) {
        if (version >= 5) {
            Matrix3D m = *Get_Transform_Matrix();
            xfer->xferMatrix3D(&m);
            Set_Transform_Matrix(&m);
        } else {
            Coord3D pos = *Get_Position();
            xfer->xferCoord3D(&pos);
            Set_Position(&pos);
            float angle = Get_Orientation();
            xfer->xferReal(&angle);
            Set_Orientation(angle);
        }
    }

    bool has_selection_color = m_selectionColorEnvelope != nullptr;
    xfer->xferBool(&has_selection_color);

    if (has_selection_color) {
        if (m_selectionColorEnvelope == nullptr) {
            m_selectionColorEnvelope = new TintEnvelope();
        }

        xfer->xferSnapshot(m_selectionColorEnvelope);
    }

    bool has_tint_color = m_tintColorEnvelope != nullptr;
    xfer->xferBool(&has_tint_color);

    if (has_tint_color) {
        if (m_tintColorEnvelope == nullptr) {
            m_tintColorEnvelope = new TintEnvelope();
        }

        xfer->xferSnapshot(m_tintColorEnvelope);
    }

    TerrainDecalType decal = Get_Terrain_Decal();
    xfer->xferUser(&decal, sizeof(decal));

    if (xfer->Get_Mode() == XFER_LOAD) {
        Set_Terrain_Decal(decal);
    }

    xfer->xferReal(&m_opacity);
    xfer->xferReal(&m_effectiveOpacity1);
    xfer->xferReal(&m_effectiveOpacity2);
    xfer->xferReal(&m_terrainDecalFadeTarget1);
    xfer->xferReal(&m_terrainDecalFadeTarget2);
    xfer->xferReal(&m_terrainDecalOpacity);

    ObjectID object_id;
    if (m_object != nullptr) {
        object_id = m_object->Get_ID();
    } else {
        object_id = INVALID_OBJECT_ID;
    }

    xfer->xferObjectID(&object_id);

    if (xfer->Get_Mode() == XFER_LOAD) {
        if (m_object != nullptr) {
            captainslog_relassert(object_id == m_object->Get_ID(),
                6,
                "Drawable::Xfer_Snapshot - Drawable '%s' is attached to wrong object '%s'",
                Get_Template()->Get_Name().Str(),
                m_object->Get_Template()->Get_Name().Str());
        } else if (object_id != INVALID_OBJECT_ID) {
            Object *object = g_theGameLogic->Find_Object_By_ID(object_id);
            captainslog_relassert(0,
                6,
                "Drawable::Xfer_Snapshot - Drawable '%s' is not attached to an object but should be attached to object '%s' "
                "with id '%d'",
                Get_Template()->Get_Name().Str(),
                object != nullptr ? object->Get_Template()->Get_Name().Str() : "Unknown",
                object_id);
        }
    }

    xfer->xferUnsignedInt(&m_status);
    xfer->xferUnsignedInt(&m_drawBits);
    xfer->xferUnsignedInt(&m_previousDrawBits);
    xfer->xferUser(&m_fadingMode, sizeof(m_fadingMode));
    xfer->xferUnsignedInt(&m_curFadeFrame);
    xfer->xferUnsignedInt(&m_timeToFade);

    bool has_loco_info = m_drawableLocoInfo != nullptr;
    xfer->xferBool(&has_loco_info);

    if (has_loco_info) {
        if (xfer->Get_Mode() == XFER_LOAD && m_drawableLocoInfo == nullptr) {
            m_drawableLocoInfo = new DrawableLocoInfo();
        }

        xfer->xferReal(&m_drawableLocoInfo->m_pitch);
        xfer->xferReal(&m_drawableLocoInfo->m_pitchRate);
        xfer->xferReal(&m_drawableLocoInfo->m_roll);
        xfer->xferReal(&m_drawableLocoInfo->m_rollRate);
        xfer->xferReal(&m_drawableLocoInfo->m_thrustWobble);
        xfer->xferReal(&m_drawableLocoInfo->m_accelerationPitch);
        xfer->xferReal(&m_drawableLocoInfo->m_accelerationPitchRate);
        xfer->xferReal(&m_drawableLocoInfo->m_accelerationRoll);
        xfer->xferReal(&m_drawableLocoInfo->m_accelerationRollRate);
        xfer->xferReal(&m_drawableLocoInfo->m_overlapZVel);
        xfer->xferReal(&m_drawableLocoInfo->m_overlapZ);
        xfer->xferReal(&m_drawableLocoInfo->m_thrust);
        xfer->xferReal(&m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset);
        xfer->xferReal(&m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset);
        xfer->xferReal(&m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset);
        xfer->xferReal(&m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset);
        xfer->xferReal(&m_drawableLocoInfo->m_wheelInfo.m_wheelAngle);
        xfer->xferInt(&m_drawableLocoInfo->m_wheelInfo.m_framesAirborneCounter);
        xfer->xferInt(&m_drawableLocoInfo->m_wheelInfo.m_framesAirborne);
    }

    Xfer_Drawable_Modules(xfer);
    xfer->xferUser(&m_stealthLook, sizeof(m_stealthLook));
    xfer->xferInt(&m_flashTime);
    xfer->xferColor(&m_flashColor);
    xfer->xferBool(&m_hidden);
    xfer->xferBool(&m_stealthInvisible);
    xfer->xferReal(&m_stealthEmissiveScale);
    xfer->xferBool(&m_instanceIsIdentity);
    xfer->xferUser(&m_instance, sizeof(m_instance));
    xfer->xferReal(&m_instanceScale);
    xfer->xferObjectID(&m_drawableInfo.object_id);

    if (version < 2) {
        captainslog_dbgassert(xfer->Get_Mode() == XFER_LOAD, "Drawable::Xfer_Snapshot - Writing an old format!!!");
        m_conditionState.Xfer(xfer);

        if (xfer->Get_Mode() == XFER_LOAD) {
            Replace_Model_Condition_Flags(m_conditionState, true);
        }
    }

    xfer->xferUnsignedInt(&m_expirationDate);

    unsigned char count = 0;

    if (Has_Drawable_Icon_Info()) {
        for (int i = 0; i < MAX_ICONS; i++) {
            if (Get_Icon_Info()->anims[i] != nullptr) {
                count++;
            }
        }
    }

    xfer->xferUnsignedByte(&count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (int i = 0; i < MAX_ICONS; i++) {
            if (Has_Drawable_Icon_Info()) {
                if (Get_Icon_Info()->anims[i] != nullptr) {
                    Utf8String str = Drawable_Icon_Index_To_Name((DrawableIconType)i);
                    xfer->xferAsciiString(&str);
                    unsigned int timing = Get_Icon_Info()->timings[i];
                    xfer->xferUnsignedInt(&timing);
                    Utf8String name = Get_Icon_Info()->anims[i]->Get_Template()->Get_Name();
                    xfer->xferAsciiString(&name);
                    xfer->xferSnapshot(Get_Icon_Info()->anims[i]);
                }
            }
        }
    } else {
        if (Has_Drawable_Icon_Info()) {
            Get_Icon_Info()->Reset();
        }

        for (int j = 0; j < count; j++) {
            Utf8String str;
            xfer->xferAsciiString(&str);
            DrawableIconType index = Drawable_Icon_Name_To_Index(str.Str());
            unsigned int timing;
            xfer->xferUnsignedInt(&timing);
            Get_Icon_Info()->timings[index] = timing;
            Utf8String name;
            xfer->xferAsciiString(&name);
            Anim2DTemplate *tmplate = g_theAnim2DCollection->Find_Template(name);
            captainslog_relassert(tmplate, 6, "Drawable::Xfer_Snapshot - Unknown icon template '%s'", name.Str());
            Get_Icon_Info()->anims[index] = new Anim2D(tmplate, g_theAnim2DCollection);
            xfer->xferSnapshot(Get_Icon_Info()->anims[index]);
        }
    }

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_stealthLook = STEALTHLOOK_NONE;

        if (m_hidden || m_stealthInvisible) {
            Update_Hidden_Status();
        }
    }

    if (xfer->Get_Mode() == XFER_SAVE) {
        captainslog_dbgassert(!m_isModelDirty, "Drawble::Xfer_Snapshot - m_isModelDirty is not FALSE!");
    } else {
        m_isModelDirty = true;
    }

    if (xfer->Get_Mode() == XFER_LOAD) {
        Stop_Ambient_Sound();
    }

    if (version >= 4) {
        xfer->xferBool(&m_ambientSoundEnabled);
    }

    if (version >= 6) {
        xfer->xferBool(&m_ambientSoundFromScriptEnabled);
    }

    if (version >= 7) {
        bool sound = m_customSoundAmbientInfo != nullptr;
        xfer->xferBool(&sound);

        if (sound) {
            bool nosound = m_customSoundAmbientInfo == Get_No_Sound_Marker();
            xfer->xferBool(&nosound);

            if (xfer->Get_Mode() == XFER_LOAD) {
                if (nosound) {
                    Set_Custom_Sound_Ambient_Off();
                } else {
                    Utf8String str;
                    xfer->xferAsciiString(&str);
                    AudioEventInfo *info = g_theAudio->Find_Audio_Event_Info(str);
                    DynamicAudioEventInfo *event;

                    bool add = true;

                    if (info == nullptr) {
                        captainslog_debug(
                            "Load failed to load customized ambient sound because sound '%s' no longer exists", str.Str());
                        event = new DynamicAudioEventInfo();
                        add = false;
                    } else {
                        event = new DynamicAudioEventInfo(*info);
                    }

                    Mangle_Custom_Audio_Name(event);
                    event->Xfer_No_Name(xfer);

                    if (add) {
                        g_theAudio->Add_Audio_Event_Info(event);
                        Clear_Custom_Sound_Ambient(false);
                        m_customSoundAmbientInfo = event;
                    } else {
                        event->Delete_Instance();
                    }
                }
            } else if (!nosound) {
                Utf8String str = m_customSoundAmbientInfo->Get_Original_Name();
                xfer->xferAsciiString(&str);
                m_customSoundAmbientInfo->Xfer_No_Name(xfer);
            }
        }
    }
}

void Drawable::Replace_Model_Condition_Flags(BitFlags<MODELCONDITION_COUNT> const &flags, bool dirty)
{
    if (dirty || !(m_conditionState == flags)) {
        m_conditionState = flags;

        if (dirty == true) {
            for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
                ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

                if (draw != nullptr) {
                    draw->Replace_Model_Condition_State(m_conditionState);
                }
            }

            m_isModelDirty = false;
        } else {
            m_isModelDirty = true;
        }
    }
}

void Drawable::Set_ID(DrawableID id)
{
    if (m_id != id) {
        if (m_id != INVALID_DRAWABLE_ID) {
            g_theGameClient->Remove_Drawable_From_Lookup_Table(this);
        }

        m_id = id;

        if (m_id != INVALID_DRAWABLE_ID) {
            g_theGameClient->Add_Drawable_To_Lookup_Table(this);

            if (m_ambientSound != nullptr) {
                m_ambientSound->m_event.Set_Drawable_ID(m_id);
            }
        }
    }
}

void Drawable::Set_Terrain_Decal(TerrainDecalType decal)
{
    if (m_terrainDecal != decal) {
        m_terrainDecal = decal;

        DrawModule **modules = Get_Draw_Modules();

        if (*modules != nullptr) {
            (*modules)->Set_Terrain_Decal(decal);
        }
    }
}

void Drawable::Load_Post_Process()
{
    if (m_object != nullptr) {
        Set_Transform_Matrix(m_object->Get_Transform_Matrix());
    }

    if (m_ambientSoundEnabled && m_ambientSoundFromScriptEnabled) {
        Start_Ambient_Sound(true);
    } else {
        Stop_Ambient_Sound();
    }
}

void Drawable::React_To_Transform_Change(const Matrix3D *matrix, const Coord3D *pos, float angle)
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        (*i)->React_To_Transform_Change(matrix, pos, angle);
    }
}

void Drawable::Update_Hidden_Status()
{
    bool hidden = m_hidden || m_stealthInvisible;

    if (hidden) {
        g_theInGameUI->Deselect_Drawable(this);
    }

    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            draw->Set_Hidden(hidden);
        }
    }
}

void Drawable::Xfer_Drawable_Modules(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);

    if (xfer->Get_Mode() == XFER_SAVE) {
        Get_Draw_Modules();
    }

    unsigned short count = 2;
    xfer->xferUnsignedShort(&count);
    Utf8String str;

    for (int i = 0; i < count; i++) {
        unsigned short module_count = 0;

        for (Module **j = m_modules[i]; j && *j; j++) {
            module_count++;
        }

        xfer->xferUnsignedShort(&module_count);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (Module **j = m_modules[i]; j && *j; j++) {
                str = g_theNameKeyGenerator->Key_To_Name((*j)->Get_Tag_Key());
                captainslog_dbgassert(str != Utf8String::s_emptyString,
                    "Drawable::Xfer_Drawable_Modules - module name key does not translate to a string!");
                xfer->xferAsciiString(&str);
                xfer->Begin_Block();
                xfer->xferSnapshot(*j);
                xfer->End_Block();
            }
        } else {
            for (int k = 0; k < module_count; k++) {
                xfer->xferAsciiString(&str);
                NameKeyType key = g_theNameKeyGenerator->Name_To_Key(str.Str());
                Module *module = nullptr;

                for (Module **j = m_modules[i]; j && *j; j++) {
                    if ((*j)->Get_Tag_Key() == key) {
                        module = *j;
                    }
                }

                int size = xfer->Begin_Block();

                if (module != nullptr) {
                    xfer->xferSnapshot(module);
                } else {
                    captainslog_dbgassert(0,
                        "Drawable::xferDrawableModules - Module '%s' was indicated in file, but not found on Drawable %s "
                        "%d",
                        str.Str(),
                        Get_Template()->Get_Name().Str(),
                        Get_ID());
                    xfer->Skip(size);
                }

                xfer->End_Block();
            }
        }
    }
}

void Drawable::Set_Tint_Color_Envelope(const TintEnvelope *envelope)
{
    if (m_tintColorEnvelope != nullptr) {
        *m_tintColorEnvelope = *envelope;
    }
}

void Drawable::Set_Animation_Frame(int frame)
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            draw->Set_Animation_Frame(frame);
        }
    }
}

void Drawable::Allocate_Shadows()
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        (*i)->Allocate_Shadows();
    }
}

bool Drawable::Client_Only_Get_First_Render_Obj_Info(Coord3D *position, float *radius, Matrix3D *transform)
{
    DrawModule **modules = Get_Draw_Modules();
    ObjectDrawInterface *draw;

    if (modules != nullptr && *modules != nullptr) {
        draw = (*modules)->Get_Object_Draw_Interface();
    } else {
        draw = nullptr;
    }

    return draw && draw->Client_Only_Get_Render_Obj_Info(position, radius, transform);
}

void Drawable::Set_Animation_Loop_Duration(unsigned int num_frames)
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            draw->Set_Animation_Loop_Duration(num_frames);
        }
    }
}

void Drawable::Set_Animation_Completion_Time(unsigned int time)
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            draw->Set_Animation_Completion_Time(time);
        }
    }
}

void Drawable::Update_Sub_Objects()
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            draw->Update_Sub_Objects();
        }
    }
}

void Drawable::Show_Sub_Object(Utf8String const &sub_object, bool visible)
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            draw->Show_Sub_Object(sub_object, visible);
        }
    }
}

void Drawable::Set_Terrain_Decal_Size(float width, float height)
{
    DrawModule **modules = Get_Draw_Modules();

    if (*modules != nullptr) {
        (*modules)->Set_Terrain_Decal_Size(width, height);
    }
}

void Drawable::Set_Shadows_Enabled(bool enable)
{
    if (enable) {
        Set_Status_Bit(DRAWABLE_STATUS_SHADOWS_ENABLED);
    } else {
        Clear_Status_Bit(DRAWABLE_STATUS_SHADOWS_ENABLED);
    }

    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        (*i)->Set_Shadows_Enabled(enable);
    }
}

void Drawable::Release_Shadows()
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        (*i)->Release_Shadows();
    }
}

int Drawable::Get_Current_Client_Bone_Positions(
    char const *bone_name_prefix, int start_index, Coord3D *positions, Matrix3D *transforms, int max_bones) const
{
    int ret = 0;
    for (const DrawModule **i = Get_Draw_Modules(); *i != nullptr && max_bones > 0; i++) {
        const ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            int val = draw->Get_Current_Bone_Positions(bone_name_prefix, start_index, positions, transforms, max_bones);

            if (val > 0) {
                ret += val;

                if (positions) {
                    positions += val;
                }
                if (transforms) {
                    transforms += val;
                }

                max_bones -= val;
            }
        }
    }

    return ret;
}

void Drawable::Set_Terrain_Decal_Fade_Target(float target1, float target2)
{
    if (m_terrainDecalFadeTarget1 != target1) {
        m_terrainDecalFadeTarget1 = target1;
        m_terrainDecalFadeTarget2 = target2;
    }
}

void Drawable::Imitate_Stealth_Look(Drawable &imitate)
{
    m_effectiveOpacity1 = imitate.Get_Effective_Opacity1();
    m_opacity = imitate.Get_Opacity();
    m_effectiveOpacity2 = imitate.Get_Effective_Opacity2();
    m_hidden = imitate.Is_Hidden();
    m_stealthInvisible = imitate.Is_Hidden();
    m_stealthLook = imitate.Get_Stealth_Look();
    m_stealthEmissiveScale = imitate.Get_Stealth_Emissive_Scale();
}

void Drawable::Set_Stealth_Look(StealthLookType look)
{
    if (look != m_stealthLook) {
        m_effectiveOpacity1 = 1.0f;

        switch (look) {
            case STEALTHLOOK_NONE:
                m_stealthInvisible = false;
                m_stealthEmissiveScale = 0.0f;
                break;
            case STEALTHLOOK_VISIBLE_FRIENDLY:
            case STEALTHLOOK_VISIBLE_DETECTED_FRIENDLY: {
                float opacity = g_theWriteableGlobalData->m_stealthFriendlyOpacity;
                Object *object = Get_Object();

                if (object == nullptr) {
                    m_effectiveOpacity1 = opacity;
                    m_stealthInvisible = false;

                    if (look == STEALTHLOOK_VISIBLE_DETECTED_FRIENDLY && !Is_KindOf(KINDOF_MINE)) {
                        m_stealthEmissiveScale = 1.0f;
                    } else {
                        m_stealthEmissiveScale = 0.0f;
                    }

                    break;
                }

                StealthUpdate *update = object->Get_Stealth_Update();

                if (update == nullptr) {
                    m_effectiveOpacity1 = opacity;
                    m_stealthInvisible = false;

                    if (look == STEALTHLOOK_VISIBLE_DETECTED_FRIENDLY && !Is_KindOf(KINDOF_MINE)) {
                        m_stealthEmissiveScale = 1.0f;
                    } else {
                        m_stealthEmissiveScale = 0.0f;
                    }
                } else if (update->Has_Disguised_Template()) {
                    m_stealthInvisible = false;
                } else {
                    if (update->Get_Friendly_Opacity() != -1.0f) {
                        opacity = update->Get_Friendly_Opacity();
                    }

                    m_effectiveOpacity1 = opacity;
                    m_stealthInvisible = false;

                    if (look == STEALTHLOOK_VISIBLE_DETECTED_FRIENDLY && !Is_KindOf(KINDOF_MINE)) {
                        m_stealthEmissiveScale = 1.0f;
                    } else {
                        m_stealthEmissiveScale = 0.0f;
                    }
                }
            } break;
            case STEALTHLOOK_DISGUISED:
                m_stealthInvisible = false;
                m_stealthEmissiveScale = 0.0f;
                break;
            case STEALTHLOOK_VISIBLE_DETECTED:
                m_stealthInvisible = false;
                if (Is_KindOf(KINDOF_MINE)) {
                    m_stealthEmissiveScale = 0.0f;
                } else {
                    m_stealthEmissiveScale = 1.0f;
                }
                break;
            case STEALTHLOOK_INVISIBLE:
                m_stealthInvisible = true;
                m_stealthEmissiveScale = 0.0f;
                break;
            default:
                break;
        }

        m_stealthLook = look;
        Update_Hidden_Status();
    }
}

void Drawable::Calc_Physics_Xform_Wheels(Locomotor const *locomotor, PhysicsXformInfo &xform)
{
    if (m_drawableLocoInfo == nullptr) {
        m_drawableLocoInfo = new DrawableLocoInfo();
    }

    float accel_pitch_limit = locomotor->Get_Accel_Pitch_Limit();
    float deacel_pitch_limit = locomotor->Get_Deaccel_Pitch_Limit();
    float bounce_kick = locomotor->Get_Bounce_Kick();
    float pitch_stiffness = locomotor->Get_Pitch_Stiffness();
    float roll_stiffness = locomotor->Get_Roll_Stiffness();
    float pitch_damping = locomotor->Get_Pitch_Damping();
    float roll_damping = locomotor->Get_Roll_Damping();
    float forward_accel_coef = locomotor->Get_Forward_Accel_Coef();
    float lateral_accel_coef = locomotor->Get_Lateral_Accel_Coef();
    float uniform_axial_damping = locomotor->Get_Uniform_Axial_Damping();
    float max_wheel_extension = locomotor->Get_Max_Wheel_Extension();
    float wheel_turn_angle = locomotor->Get_Wheel_Turn_Angle();
    bool has_suspension = locomotor->Has_Suspension();
    Object *object = Get_Object();

    if (object != nullptr) {
        AIUpdateInterface *update = object->Get_AI_Update_Interface();

        if (update != nullptr) {
            PhysicsBehavior *physics = object->Get_Physics();

            if (physics != nullptr) {
                const Coord3D *position = Get_Position();
                const Coord3D *direction = Get_Unit_Dir_Vector2D();
                const Coord3D &prev_accel = physics->Get_Prev_Accel();
                float diry = -direction->y;
                float dirx = direction->x;
                PathfindLayerEnum layer = object->Get_Layer();

                Coord3D n;
                float height = g_theTerrainLogic->Get_Layer_Height(position->x, position->y, layer, &n, true);
                float pitch = (n.x * direction->x + n.y * direction->y) * DEG_TO_RADF(90.0f);
                float roll = (n.x * diry + n.y * dirx) * DEG_TO_RADF(90.0f);
                bool is_significantly_above_terrain = object->Is_Significantly_Above_Terrain();

                if (is_significantly_above_terrain) {
                    if (has_suspension) {
                        m_drawableLocoInfo->m_wheelInfo.m_framesAirborne = 0;
                        m_drawableLocoInfo->m_wheelInfo.m_framesAirborneCounter++;

                        if (-max_wheel_extension < position->z - height) {
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset =
                                (max_wheel_extension - m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset) / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                            m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset =
                                (max_wheel_extension - m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset) / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset;
                        } else {
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset =
                                (0.0f - m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset) / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                            m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset =
                                (0.0f - m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset) / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset;
                        }
                    }

                    float major_radius = object->Get_Geometry_Info().Get_Major_Radius();
                    float minor_radius = object->Get_Geometry_Info().Get_Minor_Radius();
                    float pitch2 = m_drawableLocoInfo->m_pitch + m_drawableLocoInfo->m_accelerationPitch - pitch;
                    float pitch3 = GameMath::Sin(pitch2) * major_radius;
                    float roll2 = m_drawableLocoInfo->m_roll + m_drawableLocoInfo->m_accelerationRoll - roll;
                    float roll3 = GameMath::Sin(roll2) * minor_radius;
                    xform.m_totalZ = GameMath::Fabs(roll3) / 4.0f + GameMath::Fabs(pitch3) / 4.0f;
                } else {
                    float velocity_magnitude = physics->Get_Velocity_Magnitude();
                    float cur_locomotor_speed = update->Get_Cur_Locomotor_Speed();

                    if (!is_significantly_above_terrain && cur_locomotor_speed / 10.0f < velocity_magnitude) {
                        float ratio = velocity_magnitude / cur_locomotor_speed;

                        if (ratio * bounce_kick / 4.0f > GameMath::Fabs(m_drawableLocoInfo->m_pitchRate)) {
                            if (ratio * bounce_kick / 8.0f > GameMath::Fabs(m_drawableLocoInfo->m_rollRate)) {
                                switch (Get_Client_Random_Value(0, 3)) {
                                    case 0:
                                        m_drawableLocoInfo->m_pitchRate =
                                            m_drawableLocoInfo->m_pitchRate - bounce_kick * ratio;
                                        m_drawableLocoInfo->m_rollRate =
                                            m_drawableLocoInfo->m_rollRate - bounce_kick * ratio / 2.0f;
                                        break;
                                    case 1:
                                        m_drawableLocoInfo->m_pitchRate =
                                            bounce_kick * ratio + m_drawableLocoInfo->m_pitchRate;
                                        m_drawableLocoInfo->m_rollRate =
                                            m_drawableLocoInfo->m_rollRate - bounce_kick * ratio / 2.0f;
                                        break;
                                    case 2:
                                        m_drawableLocoInfo->m_pitchRate =
                                            m_drawableLocoInfo->m_pitchRate - bounce_kick * ratio;
                                        m_drawableLocoInfo->m_rollRate =
                                            bounce_kick * ratio / 2.0f + m_drawableLocoInfo->m_rollRate;
                                        break;
                                    case 3:
                                        m_drawableLocoInfo->m_pitchRate =
                                            bounce_kick * ratio + m_drawableLocoInfo->m_pitchRate;
                                        m_drawableLocoInfo->m_rollRate =
                                            bounce_kick * ratio / 2.0f + m_drawableLocoInfo->m_rollRate;
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }

                    if (!is_significantly_above_terrain) {
                        m_drawableLocoInfo->m_pitchRate = -pitch_stiffness * (m_drawableLocoInfo->m_pitch - pitch)
                            + -pitch_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitchRate;

                        if (m_drawableLocoInfo->m_pitchRate > 0.0f) {
                            m_drawableLocoInfo->m_pitchRate = m_drawableLocoInfo->m_pitchRate * 0.5f;
                        }

                        m_drawableLocoInfo->m_rollRate = -roll_stiffness * (m_drawableLocoInfo->m_roll - roll)
                            + -roll_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_rollRate;
                    }

                    m_drawableLocoInfo->m_pitch =
                        uniform_axial_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitch;
                    m_drawableLocoInfo->m_roll =
                        uniform_axial_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_roll;
                    m_drawableLocoInfo->m_accelerationPitchRate = -pitch_stiffness * m_drawableLocoInfo->m_accelerationPitch
                        + -pitch_damping * m_drawableLocoInfo->m_accelerationPitchRate
                        + m_drawableLocoInfo->m_accelerationPitchRate;
                    m_drawableLocoInfo->m_accelerationPitch =
                        m_drawableLocoInfo->m_accelerationPitch + m_drawableLocoInfo->m_accelerationPitchRate;
                    m_drawableLocoInfo->m_accelerationRollRate = -roll_stiffness * m_drawableLocoInfo->m_accelerationRoll
                        + -roll_damping * m_drawableLocoInfo->m_accelerationRollRate
                        + m_drawableLocoInfo->m_accelerationRollRate;
                    m_drawableLocoInfo->m_accelerationRoll =
                        m_drawableLocoInfo->m_accelerationRoll + m_drawableLocoInfo->m_accelerationRollRate;
                    xform.m_totalPitch = m_drawableLocoInfo->m_pitch + m_drawableLocoInfo->m_accelerationPitch;
                    xform.m_totalRoll = m_drawableLocoInfo->m_roll + m_drawableLocoInfo->m_accelerationRoll;

                    if (physics->Is_Motive()) {
                        m_drawableLocoInfo->m_accelerationPitchRate = m_drawableLocoInfo->m_accelerationPitchRate
                            - forward_accel_coef * (direction->x * prev_accel.x + direction->y * prev_accel.y);
                        m_drawableLocoInfo->m_accelerationRollRate = m_drawableLocoInfo->m_accelerationRollRate
                            - lateral_accel_coef * (-direction->y * prev_accel.x + direction->x * prev_accel.y);
                    }

                    if (m_drawableLocoInfo->m_accelerationPitch > deacel_pitch_limit) {
                        m_drawableLocoInfo->m_accelerationPitch = deacel_pitch_limit;
                    } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationPitch) {
                        m_drawableLocoInfo->m_accelerationPitch = -accel_pitch_limit;
                    }

                    if (m_drawableLocoInfo->m_accelerationRoll > deacel_pitch_limit) {
                        m_drawableLocoInfo->m_accelerationRoll = deacel_pitch_limit;
                    } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationRoll) {
                        m_drawableLocoInfo->m_accelerationRoll = -accel_pitch_limit;
                    }

                    xform.m_totalZ = 0.0f;
                    float major_radius = object->Get_Geometry_Info().Get_Major_Radius();
                    float minor_radius = object->Get_Geometry_Info().Get_Minor_Radius();
                    float pitch_offset = GameMath::Sin(xform.m_totalPitch - pitch) * major_radius;
                    float roll_offset = GameMath::Sin(xform.m_totalRoll - roll) * minor_radius;

                    if (has_suspension) {
                        m_drawableLocoInfo->m_wheelInfo.m_framesAirborne =
                            m_drawableLocoInfo->m_wheelInfo.m_framesAirborneCounter;
                        m_drawableLocoInfo->m_wheelInfo.m_framesAirborneCounter = 0;
                        TWheelInfo wheel_info = m_drawableLocoInfo->m_wheelInfo;
                        PhysicsTurningType turning = physics->Get_Turning();

                        if (turning == TURN_NEGATIVE) {
                            wheel_info.m_wheelAngle = -wheel_turn_angle;
                        } else if (turning == TURN_POSITIVE) {
                            wheel_info.m_wheelAngle = wheel_turn_angle;
                        } else {
                            wheel_info.m_wheelAngle = 0.0f;
                        }

                        if (physics->Get_Forward_Speed_2D() < 0.0f) {
                            wheel_info.m_wheelAngle = -wheel_info.m_wheelAngle;
                        }

                        m_drawableLocoInfo->m_wheelInfo.m_wheelAngle =
                            (wheel_info.m_wheelAngle - m_drawableLocoInfo->m_wheelInfo.m_wheelAngle) / 10.0f
                            + m_drawableLocoInfo->m_wheelInfo.m_wheelAngle;
                        float offset_mult = 0.9f;

                        if (pitch_offset < 0.0f) {
                            wheel_info.m_frontLeftHeightOffset = (pitch_offset / 3.0f + pitch_offset / 2.0f) * offset_mult;
                            wheel_info.m_frontRightHeightOffset = wheel_info.m_frontLeftHeightOffset;
                            wheel_info.m_rearLeftHeightOffset = -pitch_offset / 2.0f + pitch_offset / 4.0f;
                            wheel_info.m_rearRightHeightOffset = wheel_info.m_rearLeftHeightOffset;
                        } else {
                            wheel_info.m_frontLeftHeightOffset = -pitch_offset / 4.0f + pitch_offset / 2.0f;
                            wheel_info.m_frontRightHeightOffset = wheel_info.m_frontLeftHeightOffset;
                            wheel_info.m_rearLeftHeightOffset = (-pitch_offset / 2.0f + -pitch_offset / 3.0f) * offset_mult;
                            wheel_info.m_rearRightHeightOffset = wheel_info.m_rearLeftHeightOffset;
                        }

                        if (roll_offset > 0.0f) {
                            wheel_info.m_frontRightHeightOffset = -offset_mult * (roll_offset / 3.0f + roll_offset / 2.0f)
                                + wheel_info.m_frontRightHeightOffset;
                            wheel_info.m_rearRightHeightOffset = -offset_mult * (roll_offset / 3.0f + roll_offset / 2.0f)
                                + wheel_info.m_rearRightHeightOffset;
                            wheel_info.m_rearLeftHeightOffset =
                                roll_offset / 2.0f - roll_offset / 4.0f + wheel_info.m_rearLeftHeightOffset;
                            wheel_info.m_frontLeftHeightOffset =
                                roll_offset / 2.0f - roll_offset / 4.0f + wheel_info.m_frontLeftHeightOffset;
                        } else {
                            wheel_info.m_frontRightHeightOffset =
                                -roll_offset / 2.0f + roll_offset / 4.0f + wheel_info.m_frontRightHeightOffset;
                            wheel_info.m_rearRightHeightOffset =
                                -roll_offset / 2.0f + roll_offset / 4.0f + wheel_info.m_rearRightHeightOffset;
                            wheel_info.m_rearLeftHeightOffset =
                                (roll_offset / 3.0f + roll_offset / 2.0f) * offset_mult + wheel_info.m_rearLeftHeightOffset;
                            wheel_info.m_frontLeftHeightOffset =
                                (roll_offset / 3.0f + roll_offset / 2.0f) * offset_mult + wheel_info.m_frontLeftHeightOffset;
                        }

                        if (wheel_info.m_frontLeftHeightOffset < m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset) {
                            m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset =
                                (wheel_info.m_frontLeftHeightOffset
                                    - m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset)
                                    / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset;
                        } else {
                            m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset = wheel_info.m_frontLeftHeightOffset;
                        }

                        if (wheel_info.m_frontRightHeightOffset < m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset) {
                            m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset =
                                (wheel_info.m_frontRightHeightOffset
                                    - m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset)
                                    / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset;
                        } else {
                            m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset = wheel_info.m_frontRightHeightOffset;
                        }

                        if (wheel_info.m_rearLeftHeightOffset < m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset) {
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset =
                                (wheel_info.m_rearLeftHeightOffset - m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset)
                                    / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                        } else {
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset = wheel_info.m_rearLeftHeightOffset;
                        }

                        if (wheel_info.m_rearRightHeightOffset < m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset) {
                            m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset =
                                (wheel_info.m_rearRightHeightOffset
                                    - m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset)
                                    / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset;
                        } else {
                            m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset = wheel_info.m_rearRightHeightOffset;
                        }

                        if (m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset < max_wheel_extension) {
                            m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset = max_wheel_extension;
                        }

                        if (m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset < max_wheel_extension) {
                            m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset = max_wheel_extension;
                        }

                        if (m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset < max_wheel_extension) {
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset = max_wheel_extension;
                        }

                        if (m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset < max_wheel_extension) {
                            m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset = max_wheel_extension;
                        }
                    }

                    float z_mult = 4.0f;
                    float pitch3 = GameMath::Fabs(xform.m_totalPitch - pitch);

                    if (pitch3 > DEG_TO_RADF(22.5f)) {
                        z_mult = ((pitch3 - DEG_TO_RADF(22.5f)) * 1.0f + DEG_TO_RADF(90.0f)) / pitch3;
                    }

                    xform.m_totalZ = GameMath::Fabs(pitch_offset) / z_mult + xform.m_totalZ;
                    xform.m_totalZ = GameMath::Fabs(roll_offset) / z_mult + xform.m_totalZ;
                }
            }
        }
    }
}

void Drawable::Calc_Physics_Xform_Treads(Locomotor const *locomotor, PhysicsXformInfo &xform)
{
    if (m_drawableLocoInfo == nullptr) {
        m_drawableLocoInfo = new DrawableLocoInfo();
    }

    float f1 = 0.8f;
    float f2 = 0.5f;
    float f3 = 0.024543693f;
    float f4 = 5.0f;
    float f5 = 0.5f;

    float accel_pitch_limit = locomotor->Get_Accel_Pitch_Limit();
    float deacel_pitch_limit = locomotor->Get_Deaccel_Pitch_Limit();
    float pitch_stiffness = locomotor->Get_Pitch_Stiffness();
    float roll_stiffness = locomotor->Get_Roll_Stiffness();
    float pitch_damping = locomotor->Get_Pitch_Damping();
    float roll_damping = locomotor->Get_Roll_Damping();
    float forward_accel_coef = locomotor->Get_Forward_Accel_Coef();
    float lateral_accel_coef = locomotor->Get_Lateral_Accel_Coef();
    float uniform_axial_damping = locomotor->Get_Uniform_Axial_Damping();
    Object *object = Get_Object();

    if (object != nullptr) {
        if (object->Get_AI_Update_Interface()) {
            PhysicsBehavior *physics = object->Get_Physics();

            if (physics != nullptr) {
                const Coord3D *position = Get_Position();
                const Coord3D *direction = Get_Unit_Dir_Vector2D();
                const Coord3D &prev_accel = physics->Get_Prev_Accel();
                const Coord3D &velocity = physics->Get_Velocity();
                float diry = -direction->y;
                float dirx = direction->x;
                PathfindLayerEnum layer = object->Get_Layer();

                Coord3D n;
                g_theTerrainLogic->Get_Layer_Height(position->x, position->y, layer, &n, true);
                float overlap_z = 0.0f;
                Object *overlap = g_theGameLogic->Find_Object_By_ID(physics->Get_Current_Overlap());

                if (overlap != nullptr && overlap->Is_KindOf(KINDOF_SHRUBBERY)) {
                    overlap = nullptr;
                }

                if (overlap != nullptr) {
                    const Coord3D *overlap_pos = overlap->Get_Position();
                    float overlap_offset_x = overlap_pos->x - position->x;
                    float overlap_offset_y = overlap_pos->y - position->y;
                    float overlap_dist_sqr = GameMath::Square(overlap_offset_y) + GameMath::Square(overlap_offset_x);
                    float radius = Get_Drawable_Geometry_Info().Get_Bounding_Circle_Radius();
                    float radius2 = overlap->Get_Geometry_Info().Get_Bounding_Circle_Radius();
                    float dist_threshold = (radius2 + radius) * f1;

                    if (GameMath::Square(dist_threshold) > overlap_dist_sqr) {
                        float overlap_dist = GameMath::Sqrt(overlap_dist_sqr);
                        float overlap_speed = 1.0f - overlap_dist / dist_threshold;

                        if (overlap_speed < 0.0f) {
                            overlap_speed = 0.0f;
                        } else if (overlap_speed > 1.0f) {
                            overlap_speed = 1.0f;
                        }

                        float max_velocity = (velocity.x * velocity.x + velocity.y * velocity.y) * f4;

                        if (max_velocity > f5) {
                            max_velocity = f5;
                        }

                        overlap_z = overlap->Get_Geometry_Info().Get_Max_Height_Above_Position();

                        bool low = false;

                        if (overlap->Is_KindOf(KINDOF_LOW_OVERLAPPABLE) || overlap->Is_KindOf(KINDOF_INFANTRY)
                            || (overlap->Get_Body_Module()->Get_Front_Crushed()
                                && overlap->Get_Body_Module()->Get_Back_Crushed())) {
                            low = true;
                            overlap_z = f2;
                        }

                        if (overlap_speed < f2 && !low) {
                            overlap_z = (overlap_z + overlap_z) * overlap_speed;
                            Coord3D a;
                            a.x = overlap_offset_x / overlap_dist;
                            a.y = overlap_offset_y / overlap_dist;
                            a.z = 0.2f;

                            Coord3D a2;
                            a2.x = Get_Client_Random_Value_Real(-max_velocity, max_velocity);
                            a2.y = Get_Client_Random_Value_Real(-max_velocity, max_velocity);
                            a2.z = 1.0f;
                            a2.Normalize();

                            Coord3D a3;
                            Coord3D::Cross_Product(&a, &a2, &a3);
                            Coord3D::Cross_Product(&a3, &a, &n);
                            n.Normalize();
                        } else {
                            n.x = Get_Client_Random_Value_Real(-max_velocity, max_velocity);
                            n.y = Get_Client_Random_Value_Real(-max_velocity, max_velocity);
                            n.z = 1.0f;
                            n.Normalize();
                        }
                    }
                } else if (physics->Get_Previous_Overlap() != INVALID_OBJECT_ID && m_drawableLocoInfo->m_overlapZ > 0.0f) {
                    m_drawableLocoInfo->m_pitchRate += f3;
                }

                float pitch = (n.x * direction->x + n.y * direction->y) * DEG_TO_RADF(90.0f);
                float roll = (n.x * diry + n.y * dirx) * DEG_TO_RADF(90.0f);

                if (overlap != nullptr || m_drawableLocoInfo->m_overlapZ <= 0.0f) {
                    m_drawableLocoInfo->m_pitchRate = -pitch_stiffness * (m_drawableLocoInfo->m_pitch - pitch)
                        + -pitch_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitchRate;

                    if (m_drawableLocoInfo->m_pitchRate > 0.0f) {
                        m_drawableLocoInfo->m_pitchRate = m_drawableLocoInfo->m_pitchRate * 0.5f;
                    }

                    m_drawableLocoInfo->m_rollRate = -roll_stiffness * (m_drawableLocoInfo->m_roll - roll)
                        + -roll_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_rollRate;
                }

                m_drawableLocoInfo->m_pitch =
                    uniform_axial_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitch;
                m_drawableLocoInfo->m_roll =
                    uniform_axial_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_roll;

                m_drawableLocoInfo->m_accelerationPitchRate = -pitch_stiffness * m_drawableLocoInfo->m_accelerationPitch
                    + -pitch_damping * m_drawableLocoInfo->m_accelerationPitchRate
                    + m_drawableLocoInfo->m_accelerationPitchRate;
                m_drawableLocoInfo->m_accelerationPitch =
                    m_drawableLocoInfo->m_accelerationPitch + m_drawableLocoInfo->m_accelerationPitchRate;

                m_drawableLocoInfo->m_accelerationRollRate = -roll_stiffness * m_drawableLocoInfo->m_accelerationRoll
                    + -roll_damping * m_drawableLocoInfo->m_accelerationRollRate
                    + m_drawableLocoInfo->m_accelerationRollRate;
                m_drawableLocoInfo->m_accelerationRoll =
                    m_drawableLocoInfo->m_accelerationRoll + m_drawableLocoInfo->m_accelerationRollRate;

                xform.m_totalPitch = m_drawableLocoInfo->m_pitch + m_drawableLocoInfo->m_accelerationPitch;
                xform.m_totalRoll = m_drawableLocoInfo->m_roll + m_drawableLocoInfo->m_accelerationRoll;

                if (physics->Is_Motive()) {
                    m_drawableLocoInfo->m_accelerationPitchRate = m_drawableLocoInfo->m_accelerationPitchRate
                        - forward_accel_coef * (direction->x * prev_accel.x + direction->y * prev_accel.y);
                    m_drawableLocoInfo->m_accelerationRollRate = m_drawableLocoInfo->m_accelerationRollRate
                        - lateral_accel_coef * (-direction->y * prev_accel.x + direction->x * prev_accel.y);
                }

                if (m_drawableLocoInfo->m_accelerationPitch > deacel_pitch_limit) {
                    m_drawableLocoInfo->m_accelerationPitch = deacel_pitch_limit;
                } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationPitch) {
                    m_drawableLocoInfo->m_accelerationPitch = -accel_pitch_limit;
                }

                if (m_drawableLocoInfo->m_accelerationRoll > deacel_pitch_limit) {
                    m_drawableLocoInfo->m_accelerationRoll = deacel_pitch_limit;
                } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationRoll) {
                    m_drawableLocoInfo->m_accelerationRoll = -accel_pitch_limit;
                }

                if (overlap_z > m_drawableLocoInfo->m_overlapZ) {
                    m_drawableLocoInfo->m_overlapZ = overlap_z;
                    m_drawableLocoInfo->m_overlapZVel = 0.0f;
                }

                float total_z = m_drawableLocoInfo->m_overlapZ / 2.0f;

                if (m_drawableLocoInfo->m_overlapZ > 0.0f) {
                    m_drawableLocoInfo->m_overlapZVel = m_drawableLocoInfo->m_overlapZVel - 0.2f;
                    m_drawableLocoInfo->m_overlapZ = m_drawableLocoInfo->m_overlapZ + m_drawableLocoInfo->m_overlapZVel;
                }

                if (m_drawableLocoInfo->m_overlapZ <= 0.0f) {
                    m_drawableLocoInfo->m_overlapZ = 0.0f;
                    m_drawableLocoInfo->m_overlapZVel = 0.0f;
                }

                xform.m_totalZ = total_z;
            }
        }
    }
}

void Drawable::Calc_Physics_Xform_Motorcycle(Locomotor const *locomotor, PhysicsXformInfo &xform)
{
    if (m_drawableLocoInfo == nullptr) {
        m_drawableLocoInfo = new DrawableLocoInfo();
    }

    float accel_pitch_limit = locomotor->Get_Accel_Pitch_Limit();
    float deacel_pitch_limit = locomotor->Get_Deaccel_Pitch_Limit();
    float bounce_kick = locomotor->Get_Bounce_Kick();
    float pitch_stiffness = locomotor->Get_Pitch_Stiffness();
    float roll_stiffness = locomotor->Get_Roll_Stiffness();
    float pitch_damping = locomotor->Get_Pitch_Damping();
    float roll_damping = locomotor->Get_Roll_Damping();
    float forward_accel_coef = locomotor->Get_Forward_Accel_Coef();
    float lateral_accel_coef = locomotor->Get_Lateral_Accel_Coef();
    float uniform_axial_damping = locomotor->Get_Uniform_Axial_Damping();
    float max_wheel_extension = locomotor->Get_Max_Wheel_Extension();
    float wheel_turn_angle = locomotor->Get_Wheel_Turn_Angle();
    bool has_suspension = locomotor->Has_Suspension();
    Object *object = Get_Object();

    if (object != nullptr) {
        AIUpdateInterface *update = object->Get_AI_Update_Interface();

        if (update != nullptr) {
            PhysicsBehavior *physics = object->Get_Physics();

            if (physics != nullptr) {
                const Coord3D *position = Get_Position();
                const Coord3D *direction = Get_Unit_Dir_Vector2D();
                const Coord3D &prev_accel = physics->Get_Prev_Accel();
                float diry = -direction->y;
                float dirx = direction->x;
                PathfindLayerEnum layer = object->Get_Layer();

                Coord3D n;
                float height = g_theTerrainLogic->Get_Layer_Height(position->x, position->y, layer, &n, true);
                float pitch = (n.x * direction->x + n.y * direction->y) * DEG_TO_RADF(90.0f);
                float roll = (n.x * diry + n.y * dirx) * DEG_TO_RADF(90.0f);
                bool is_significantly_above_terrain = object->Is_Significantly_Above_Terrain();

                if (is_significantly_above_terrain) {
                    if (has_suspension) {
                        m_drawableLocoInfo->m_wheelInfo.m_framesAirborne = 0;
                        m_drawableLocoInfo->m_wheelInfo.m_framesAirborneCounter++;

                        if (-max_wheel_extension < position->z - height) {
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset =
                                (max_wheel_extension - m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset) / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                        } else {
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset =
                                (0.0f - m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset) / 2.0f
                                + m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                        }

                        m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset =
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                    }

                    float major_radius = object->Get_Geometry_Info().Get_Major_Radius();
                    float pitch2 = m_drawableLocoInfo->m_pitch + m_drawableLocoInfo->m_accelerationPitch - pitch;
                    float pitch3 = GameMath::Sin(pitch2) * major_radius;
                    xform.m_totalZ = GameMath::Fabs(pitch3) / 4.0f;
                }

                float velocity_magnitude = physics->Get_Velocity_Magnitude();
                float cur_locomotor_speed = update->Get_Cur_Locomotor_Speed();

                if (!is_significantly_above_terrain && cur_locomotor_speed / 10.0f < velocity_magnitude) {
                    float ratio = velocity_magnitude / cur_locomotor_speed;

                    if (ratio * bounce_kick / 4.0f > GameMath::Fabs(m_drawableLocoInfo->m_pitchRate)) {
                        if (ratio * bounce_kick / 8.0f > GameMath::Fabs(m_drawableLocoInfo->m_rollRate)) {
                            switch (Get_Client_Random_Value(0, 3)) {
                                case 0:
                                    m_drawableLocoInfo->m_pitchRate = m_drawableLocoInfo->m_pitchRate - bounce_kick * ratio;
                                    m_drawableLocoInfo->m_rollRate =
                                        m_drawableLocoInfo->m_rollRate - bounce_kick * ratio / 2.0f;
                                    break;
                                case 1:
                                    m_drawableLocoInfo->m_pitchRate = bounce_kick * ratio + m_drawableLocoInfo->m_pitchRate;
                                    m_drawableLocoInfo->m_rollRate =
                                        m_drawableLocoInfo->m_rollRate - bounce_kick * ratio / 2.0f;
                                    break;
                                case 2:
                                    m_drawableLocoInfo->m_pitchRate = m_drawableLocoInfo->m_pitchRate - bounce_kick * ratio;
                                    m_drawableLocoInfo->m_rollRate =
                                        bounce_kick * ratio / 2.0f + m_drawableLocoInfo->m_rollRate;
                                    break;
                                case 3:
                                    m_drawableLocoInfo->m_pitchRate = bounce_kick * ratio + m_drawableLocoInfo->m_pitchRate;
                                    m_drawableLocoInfo->m_rollRate =
                                        bounce_kick * ratio / 2.0f + m_drawableLocoInfo->m_rollRate;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }

                if (!is_significantly_above_terrain) {
                    m_drawableLocoInfo->m_pitchRate = -pitch_stiffness * (m_drawableLocoInfo->m_pitch - pitch)
                        + -pitch_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitchRate;

                    m_drawableLocoInfo->m_rollRate = -roll_stiffness * (m_drawableLocoInfo->m_roll - roll)
                        + -roll_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_rollRate;
                } else {
                    m_drawableLocoInfo->m_pitchRate = -pitch_stiffness * m_drawableLocoInfo->m_pitch
                        + -pitch_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitchRate;

                    m_drawableLocoInfo->m_rollRate = -roll_stiffness * m_drawableLocoInfo->m_roll
                        + -roll_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_rollRate;
                }

                m_drawableLocoInfo->m_pitch =
                    uniform_axial_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitch;
                m_drawableLocoInfo->m_roll =
                    uniform_axial_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_roll;
                m_drawableLocoInfo->m_accelerationPitchRate = -pitch_stiffness * m_drawableLocoInfo->m_accelerationPitch
                    + -pitch_damping * m_drawableLocoInfo->m_accelerationPitchRate
                    + m_drawableLocoInfo->m_accelerationPitchRate;
                m_drawableLocoInfo->m_accelerationPitch =
                    m_drawableLocoInfo->m_accelerationPitch + m_drawableLocoInfo->m_accelerationPitchRate;
                m_drawableLocoInfo->m_accelerationRollRate = -roll_stiffness * m_drawableLocoInfo->m_accelerationRoll
                    + -roll_damping * m_drawableLocoInfo->m_accelerationRollRate
                    + m_drawableLocoInfo->m_accelerationRollRate;
                m_drawableLocoInfo->m_accelerationRoll =
                    m_drawableLocoInfo->m_accelerationRoll + m_drawableLocoInfo->m_accelerationRollRate;
                xform.m_totalPitch = m_drawableLocoInfo->m_pitch + m_drawableLocoInfo->m_accelerationPitch;

                float roll2 = m_drawableLocoInfo->m_roll + m_drawableLocoInfo->m_accelerationRoll;

                if (roll2 > 0.5f && roll2 < -0.5f) {
                    xform.m_totalRoll = roll2;
                } else {
                    xform.m_totalRoll = 0.0f;
                }

                if (physics->Is_Motive()) {
                    m_drawableLocoInfo->m_accelerationPitchRate = m_drawableLocoInfo->m_accelerationPitchRate
                        - forward_accel_coef * (direction->x * prev_accel.x + direction->y * prev_accel.y);
                    m_drawableLocoInfo->m_accelerationRollRate = m_drawableLocoInfo->m_accelerationRollRate
                        - lateral_accel_coef * (-direction->y * prev_accel.x + direction->x * prev_accel.y);
                }

                if (m_drawableLocoInfo->m_accelerationPitch > deacel_pitch_limit) {
                    m_drawableLocoInfo->m_accelerationPitch = deacel_pitch_limit;
                } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationPitch) {
                    m_drawableLocoInfo->m_accelerationPitch = -accel_pitch_limit;
                }

                if (m_drawableLocoInfo->m_accelerationRoll > deacel_pitch_limit) {
                    m_drawableLocoInfo->m_accelerationRoll = deacel_pitch_limit;
                } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationRoll) {
                    m_drawableLocoInfo->m_accelerationRoll = -accel_pitch_limit;
                }

                xform.m_totalZ = 0.0f;
                float major_radius = object->Get_Geometry_Info().Get_Major_Radius();
                float minor_radius = object->Get_Geometry_Info().Get_Minor_Radius();
                float pitch_offset = GameMath::Sin(xform.m_totalPitch - pitch) * major_radius;
                float roll_offset = GameMath::Sin(xform.m_totalRoll - roll) * minor_radius;

                if (has_suspension) {
                    m_drawableLocoInfo->m_wheelInfo.m_framesAirborne =
                        m_drawableLocoInfo->m_wheelInfo.m_framesAirborneCounter;
                    m_drawableLocoInfo->m_wheelInfo.m_framesAirborneCounter = 0;
                    TWheelInfo wheel_info = m_drawableLocoInfo->m_wheelInfo;
                    PhysicsTurningType turning = physics->Get_Turning();

                    if (turning == TURN_NEGATIVE) {
                        wheel_info.m_wheelAngle = -wheel_turn_angle;
                    } else if (turning == TURN_POSITIVE) {
                        wheel_info.m_wheelAngle = wheel_turn_angle;
                    } else {
                        wheel_info.m_wheelAngle = 0.0f;
                    }

                    if (physics->Get_Forward_Speed_2D() < 0.0f) {
                        wheel_info.m_wheelAngle = -wheel_info.m_wheelAngle;
                    }

                    m_drawableLocoInfo->m_wheelInfo.m_wheelAngle =
                        (wheel_info.m_wheelAngle - m_drawableLocoInfo->m_wheelInfo.m_wheelAngle) / 10.0f
                        + m_drawableLocoInfo->m_wheelInfo.m_wheelAngle;
                    float offset_mult = 0.9f;

                    if (pitch_offset < 0.0f) {
                        wheel_info.m_frontLeftHeightOffset = (pitch_offset / 3.0f + pitch_offset / 2.0f) * offset_mult;
                        wheel_info.m_rearLeftHeightOffset = -pitch_offset / 2.0f + pitch_offset / 4.0f;
                    } else {
                        wheel_info.m_frontLeftHeightOffset = -pitch_offset / 4.0f + pitch_offset / 2.0f;
                        wheel_info.m_rearLeftHeightOffset = (-pitch_offset / 2.0f + -pitch_offset / 3.0f) * offset_mult;
                    }

                    wheel_info.m_frontRightHeightOffset = wheel_info.m_frontLeftHeightOffset;
                    wheel_info.m_rearRightHeightOffset = wheel_info.m_rearLeftHeightOffset;

                    if (wheel_info.m_frontLeftHeightOffset < m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset) {
                        m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset =
                            (wheel_info.m_frontLeftHeightOffset - m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset)
                                / 2.0f
                            + m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset;
                        m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset =
                            m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset;
                    } else {
                        m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset = wheel_info.m_frontLeftHeightOffset;
                        m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset = wheel_info.m_frontLeftHeightOffset;
                    }

                    if (wheel_info.m_rearLeftHeightOffset < m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset) {
                        m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset =
                            (wheel_info.m_rearLeftHeightOffset - m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset)
                                / 2.0f
                            + m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                        m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset =
                            m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset;
                    } else {
                        m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset = wheel_info.m_rearLeftHeightOffset;
                        m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset = wheel_info.m_rearLeftHeightOffset;
                    }

                    if (m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset < max_wheel_extension) {
                        m_drawableLocoInfo->m_wheelInfo.m_frontLeftHeightOffset = max_wheel_extension;
                        m_drawableLocoInfo->m_wheelInfo.m_frontRightHeightOffset = max_wheel_extension;
                    }

                    if (m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset < max_wheel_extension) {
                        m_drawableLocoInfo->m_wheelInfo.m_rearLeftHeightOffset = max_wheel_extension;
                        m_drawableLocoInfo->m_wheelInfo.m_rearRightHeightOffset = max_wheel_extension;
                    }
                }

                float z_mult = 4.0f;
                float pitch4 = GameMath::Fabs(xform.m_totalPitch - pitch);

                if (pitch4 > DEG_TO_RADF(22.5f)) {
                    z_mult = ((pitch4 - DEG_TO_RADF(22.5f)) * 1.0f + DEG_TO_RADF(90.0f)) / pitch4;
                }

                if (!is_significantly_above_terrain) {
                    xform.m_totalZ = GameMath::Fabs(pitch_offset) / z_mult + xform.m_totalZ;
                    xform.m_totalZ = GameMath::Fabs(roll_offset) / z_mult + xform.m_totalZ;
                }
            }
        }
    }
}

void Drawable::Calc_Physics_Xform_Hover_Or_Wings(Locomotor const *locomotor, PhysicsXformInfo &xform)
{
    if (m_drawableLocoInfo == nullptr) {
        m_drawableLocoInfo = new DrawableLocoInfo();
    }

    float accel_pitch_limit = locomotor->Get_Accel_Pitch_Limit();
    float deacel_pitch_limit = locomotor->Get_Deaccel_Pitch_Limit();
    float pitch_stiffness = locomotor->Get_Pitch_Stiffness();
    float roll_stiffness = locomotor->Get_Roll_Stiffness();
    float pitch_damping = locomotor->Get_Pitch_Damping();
    float roll_damping = locomotor->Get_Roll_Damping();
    float pitch_in_direction_of_z_vel_factor = locomotor->Get_Pitch_In_Direction_Of_Z_Vel_Factor();
    float forward_vel_coef = locomotor->Get_Forward_Vel_Coef();
    float lateral_vel_coef = locomotor->Get_Lateral_Vel_Coef();
    float forward_accel_coef = locomotor->Get_Forward_Accel_Coef();
    float lateral_accel_coef = locomotor->Get_Lateral_Accel_Coef();
    float uniform_axial_damping = locomotor->Get_Uniform_Axial_Damping();

    Object *object = Get_Object();

    if (object != nullptr) {
        if (object->Get_AI_Update_Interface()) {
            PhysicsBehavior *physics = object->Get_Physics();

            if (physics != nullptr) {
                const Coord3D *direction = Get_Unit_Dir_Vector2D();
                const Coord3D &prev_accel = physics->Get_Prev_Accel();
                const Coord3D &velocity = physics->Get_Velocity();

                m_drawableLocoInfo->m_pitchRate = -pitch_stiffness * m_drawableLocoInfo->m_pitch
                    + -pitch_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitchRate;
                m_drawableLocoInfo->m_rollRate = -roll_stiffness * m_drawableLocoInfo->m_roll
                    + -roll_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_rollRate;

                m_drawableLocoInfo->m_pitch =
                    uniform_axial_damping * m_drawableLocoInfo->m_pitchRate + m_drawableLocoInfo->m_pitch;
                m_drawableLocoInfo->m_roll =
                    uniform_axial_damping * m_drawableLocoInfo->m_rollRate + m_drawableLocoInfo->m_roll;

                m_drawableLocoInfo->m_accelerationPitchRate = -pitch_stiffness * m_drawableLocoInfo->m_accelerationPitch
                    + -pitch_damping * m_drawableLocoInfo->m_accelerationPitchRate
                    + m_drawableLocoInfo->m_accelerationPitchRate;
                m_drawableLocoInfo->m_accelerationPitch =
                    m_drawableLocoInfo->m_accelerationPitch + m_drawableLocoInfo->m_accelerationPitchRate;

                m_drawableLocoInfo->m_accelerationRollRate = -roll_stiffness * m_drawableLocoInfo->m_accelerationRoll
                    + -roll_damping * m_drawableLocoInfo->m_accelerationRollRate
                    + m_drawableLocoInfo->m_accelerationRollRate;
                m_drawableLocoInfo->m_accelerationRoll =
                    m_drawableLocoInfo->m_accelerationRoll + m_drawableLocoInfo->m_accelerationRollRate;

                xform.m_totalPitch = m_drawableLocoInfo->m_pitch + m_drawableLocoInfo->m_accelerationPitch;
                xform.m_totalRoll = m_drawableLocoInfo->m_roll + m_drawableLocoInfo->m_accelerationRoll;

                if (physics->Is_Motive()) {
                    if (pitch_in_direction_of_z_vel_factor != 0.0f && GameMath::Fabs(velocity.z) > 0.001f) {
                        m_drawableLocoInfo->m_pitch = m_drawableLocoInfo->m_pitch
                            - pitch_in_direction_of_z_vel_factor
                                * GameMath::Atan2(
                                    velocity.z, GameMath::Sqrt(GameMath::Square(velocity.y) + GameMath::Square(velocity.x)));
                    }

                    m_drawableLocoInfo->m_pitch = m_drawableLocoInfo->m_pitch
                        - forward_vel_coef * (direction->x * velocity.x + direction->y * velocity.y);
                    m_drawableLocoInfo->m_roll = m_drawableLocoInfo->m_roll
                        - lateral_vel_coef * (-direction->y * velocity.x + direction->x * velocity.y);

                    m_drawableLocoInfo->m_accelerationPitchRate = m_drawableLocoInfo->m_accelerationPitchRate
                        - forward_accel_coef * (direction->x * prev_accel.x + direction->y * prev_accel.y);
                    m_drawableLocoInfo->m_accelerationRollRate = m_drawableLocoInfo->m_accelerationRollRate
                        - lateral_accel_coef * (-direction->y * prev_accel.x + direction->x * prev_accel.y);
                }

                if (m_drawableLocoInfo->m_accelerationPitch > deacel_pitch_limit) {
                    m_drawableLocoInfo->m_accelerationPitch = deacel_pitch_limit;
                } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationPitch) {
                    m_drawableLocoInfo->m_accelerationPitch = -accel_pitch_limit;
                }

                if (m_drawableLocoInfo->m_accelerationRoll > deacel_pitch_limit) {
                    m_drawableLocoInfo->m_accelerationRoll = deacel_pitch_limit;
                } else if (-accel_pitch_limit > m_drawableLocoInfo->m_accelerationRoll) {
                    m_drawableLocoInfo->m_accelerationRoll = -accel_pitch_limit;
                }

                float rudder_correction_degree = locomotor->Get_Rudder_Correction_Degree();
                float rudder_correction_rate = locomotor->Get_Rudder_Correction_Rate();
                float elevator_correction_degree = locomotor->Get_Elevator_Correction_Degree();
                float elevator_correction_rate = locomotor->Get_Elevator_Correction_Rate();

                m_drawableLocoInfo->m_rudder = rudder_correction_rate + m_drawableLocoInfo->m_rudder;
                xform.m_yaw = GameMath::Sin(m_drawableLocoInfo->m_rudder) * rudder_correction_degree;

                m_drawableLocoInfo->m_elevator = elevator_correction_rate + m_drawableLocoInfo->m_elevator;
                xform.m_totalPitch =
                    GameMath::Cos(m_drawableLocoInfo->m_elevator) * elevator_correction_degree + xform.m_totalPitch;
                xform.m_totalZ = 0.0f;
            }
        }
    }
}

void Drawable::Calc_Physics_Xform_Thrust(Locomotor const *locomotor, PhysicsXformInfo &xform)
{
    if (m_drawableLocoInfo == nullptr) {
        m_drawableLocoInfo = new DrawableLocoInfo();
    }

    float thrust_roll = locomotor->Get_Thrust_Roll();
    float thrust_wobble_rate = locomotor->Get_Thrust_Wobble_Rate();
    float thrust_max_wobble = locomotor->Get_Thrust_Max_Wobble();
    float thrust_min_wobble = locomotor->Get_Thrust_Min_Wobble();

    if (thrust_wobble_rate != 0.0f) {
        if (m_drawableLocoInfo->m_thrust >= 1.0f) {
            if (thrust_max_wobble - (thrust_wobble_rate + thrust_wobble_rate) > m_drawableLocoInfo->m_pitch) {
                m_drawableLocoInfo->m_pitch = thrust_wobble_rate + m_drawableLocoInfo->m_pitch;
                m_drawableLocoInfo->m_thrustWobble = thrust_wobble_rate + m_drawableLocoInfo->m_thrustWobble;
            } else {
                m_drawableLocoInfo->m_pitch = thrust_wobble_rate / 2.0f + m_drawableLocoInfo->m_pitch;
                m_drawableLocoInfo->m_thrustWobble = thrust_wobble_rate / 2.0f + m_drawableLocoInfo->m_thrustWobble;
            }

            if (m_drawableLocoInfo->m_pitch >= thrust_max_wobble) {
                m_drawableLocoInfo->m_thrust = -1.0f;
            }
        } else {
            if (thrust_wobble_rate + thrust_wobble_rate + thrust_min_wobble <= m_drawableLocoInfo->m_pitch) {
                m_drawableLocoInfo->m_pitch = m_drawableLocoInfo->m_pitch - thrust_wobble_rate;
                m_drawableLocoInfo->m_thrustWobble = m_drawableLocoInfo->m_thrustWobble - thrust_wobble_rate;
            } else {
                m_drawableLocoInfo->m_pitch = m_drawableLocoInfo->m_pitch - thrust_wobble_rate / 2.0f;
                m_drawableLocoInfo->m_thrustWobble = m_drawableLocoInfo->m_thrustWobble - thrust_wobble_rate / 2.0f;
            }

            if (m_drawableLocoInfo->m_pitch <= thrust_min_wobble) {
                m_drawableLocoInfo->m_thrust = 1.0f;
            }
        }

        xform.m_totalPitch = m_drawableLocoInfo->m_pitch;
        xform.m_yaw = m_drawableLocoInfo->m_thrustWobble;
    }

    if (thrust_roll != 0.0f) {
        m_drawableLocoInfo->m_roll = thrust_roll + m_drawableLocoInfo->m_roll;
        xform.m_totalRoll = m_drawableLocoInfo->m_roll;
    }
}

bool Drawable::Calc_Physics_Xform(PhysicsXformInfo &xform)
{
    Object *object = Get_Object();
    AIUpdateInterface *update;

    if (object) {
        update = object->Get_AI_Update_Interface();
    } else {
        update = nullptr;
    }

    bool did_calculate = false;

    if (update) {
        const Locomotor *locomotor = update->Get_Cur_Locomotor();

        if (locomotor) {
            switch (locomotor->Get_Appearance()) {
                case LOCO_WHEELS_FOUR:
                    Calc_Physics_Xform_Wheels(locomotor, xform);
                    did_calculate = true;
                    break;
                case LOCO_TREADS:
                    Calc_Physics_Xform_Treads(locomotor, xform);
                    did_calculate = true;
                    break;
                case LOCO_HOVER:
                case LOCO_WINGS:
                    Calc_Physics_Xform_Hover_Or_Wings(locomotor, xform);
                    did_calculate = true;
                    break;
                case LOCO_THRUST:
                    Calc_Physics_Xform_Thrust(locomotor, xform);
                    did_calculate = true;
                    break;
                case LOCO_MOTORCYCLE:
                    Calc_Physics_Xform_Motorcycle(locomotor, xform);
                    did_calculate = true;
                    break;
                default:
                    break;
            }
        }
    }

    if (did_calculate) {
        const float epsilon = 0.00000000000000000001f;

        if (xform.m_totalPitch > -epsilon && xform.m_totalPitch < epsilon) {
            xform.m_totalPitch = 0.0f;
        }

        if (xform.m_totalRoll > -epsilon && xform.m_totalRoll < epsilon) {
            xform.m_totalRoll = 0.0f;
        }

        if (xform.m_yaw > -epsilon && xform.m_yaw < epsilon) {
            xform.m_yaw = 0.0f;
        }

        if (xform.m_totalZ > -epsilon && xform.m_totalZ < epsilon) {
            xform.m_totalZ = 0.0f;
        }
    }

    return did_calculate;
}

void Drawable::Apply_Physics_Xform(Matrix3D *mtx)
{
    Object *object = Get_Object();

    if (object && !object->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)
        && g_theWriteableGlobalData->m_showClientPhysics) {

        bool frozen = g_theTacticalView->Is_Time_Frozen() && !g_theTacticalView->Is_Camera_Movement_Finished();

        if (!frozen && !g_theScriptEngine->Is_Time_Frozen_Debug() && !g_theScriptEngine->Is_Time_Frozen_Script()) {
            PhysicsXformInfo xform;

            if (Calc_Physics_Xform(xform)) {
                mtx->Translate(0.0f, 0.0f, xform.m_totalZ);
                mtx->Rotate_Y(xform.m_totalPitch);
                mtx->Rotate_X(-xform.m_totalRoll);
                mtx->Rotate_Z(xform.m_yaw);
            }
        }
    }
}

void Drawable::Set_Fully_Obscured_By_Shroud(bool fully_obscured)
{
    if (m_fullyObscuredByShroud != fully_obscured) {
        for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
            (*i)->Set_Fully_Obscured_By_Shroud(fully_obscured);
        }

        m_fullyObscuredByShroud = fully_obscured;
    }
}

void Drawable::Update_Drawable()
{
    unsigned int frame = g_theGameLogic->Get_Frame();
    Object *object = Get_Object();

    for (ClientUpdateModule **i = Get_Client_Update_Modules(); i != nullptr && *i != nullptr; i++) {
        (*i)->Client_Update();
    }

    if (m_fadingMode != FADING_MODE_OFF) {
        Set_Opacity(
            (float)(m_fadingMode == FADING_MODE_OUT ? m_curFadeFrame : m_timeToFade - m_curFadeFrame) / (float)m_timeToFade);
        m_curFadeFrame++;

        if (m_curFadeFrame > m_timeToFade) {
            m_fadingMode = FADING_MODE_OFF;
        }
    }

    if (Get_Terrain_Decal() == TERRAIN_DECAL_8) {
        m_terrainDecalOpacity = 0.0f;
    } else {
        DrawModule **modules = Get_Draw_Modules();

        if (*modules != nullptr) {
            if (m_terrainDecalFadeTarget2 != 0.0f) {
                (*modules)->Set_Terrain_Decal_Opacity(m_terrainDecalOpacity);
            }

            if (m_terrainDecalFadeTarget2 < 0.0f || m_terrainDecalOpacity <= 0.0f) {
                m_terrainDecalFadeTarget2 = 0.0f;
                m_terrainDecalOpacity = 0.0f;
                Set_Terrain_Decal(TERRAIN_DECAL_8);
            } else if (m_terrainDecalFadeTarget2 > 0.0f && m_terrainDecalOpacity >= 1.0f) {
                m_terrainDecalOpacity = 1.0f;
                m_terrainDecalFadeTarget2 = 0.0f;
                (*modules)->Set_Terrain_Decal_Opacity(m_terrainDecalOpacity);
            }
        }
    }

    if (m_expirationDate && frame >= m_expirationDate) {
        captainslog_dbgassert(object == nullptr, "Drawables with Objects should not have expiration dates!");
        g_theGameClient->Destroy_Drawable(this);
        return;
    }

    if (m_flashTime > 0 && (g_theGameClient->Get_Frame() % 15) == 0) {
        RGBColor color;
        color.Set_From_Int(m_flashColor);
        Color_Flash(&color, 4, 0, 0);
        m_flashTime--;
    }

    if (m_previousDrawBits != m_drawBits) {
        if (Check_Draw_Bit(DRAW_BIT_DISABLED)) {
            if (m_tintColorEnvelope == nullptr) {
                m_tintColorEnvelope = new TintEnvelope();
            }

            m_tintColorEnvelope->Play(&s_darkGreyDisabledColor, 30, 30, 0xFFFFFFFE);
        } else if (Check_Draw_Bit(DRAW_BIT_DAMAGED)) {
            if (m_tintColorEnvelope == nullptr) {
                m_tintColorEnvelope = new TintEnvelope();
            }

            m_tintColorEnvelope->Play(&s_subdualDamageColor, 150, 150, 0xFFFFFFFE);
        } else if (Check_Draw_Bit(DRAW_BIT_WEAPON_BONUS)) {
            if (m_tintColorEnvelope == nullptr) {
                m_tintColorEnvelope = new TintEnvelope();
            }

            if (Is_KindOf(KINDOF_INFANTRY)) {
                m_tintColorEnvelope->Play(&s_frenzyColorInfantry, 30, 30, 0xFFFFFFFE);
            } else {
                m_tintColorEnvelope->Play(&s_frenzyColor, 30, 30, 0xFFFFFFFE);
            }
        } else {
            if (m_tintColorEnvelope == nullptr) {
                m_tintColorEnvelope = new TintEnvelope();
            }

            m_tintColorEnvelope->Set_Decay_State();
        }
    }

    m_previousDrawBits = m_drawBits;

    if (object != nullptr && !object->Is_Effectively_Dead()) {
        Clear_Draw_Bit(DRAW_BIT_DEAD);
    }

    if (m_tintColorEnvelope != nullptr) {
        m_tintColorEnvelope->Update();
    }

    if (m_selectionColorEnvelope != nullptr) {
        m_selectionColorEnvelope->Update();
    }

    if (m_ambientSound != nullptr && m_ambientSoundEnabled && m_ambientSoundFromScriptEnabled) {
        if (!m_ambientSound->m_event.Get_Event_Name().Is_Empty() && !m_ambientSound->m_event.Is_Currently_Playing()) {
            const AudioEventInfo *info = m_ambientSound->m_event.Get_Event_Info();

            if (info == nullptr && g_theAudio != nullptr) {
                g_theAudio->Get_Info_For_Audio_Event(&m_ambientSound->m_event);
                info = m_ambientSound->m_event.Get_Event_Info();
            }

            if (info == nullptr || info->Is_Looping()) {
                Start_Ambient_Sound(false);
            }
        }
    }
}

void Drawable::Color_Flash(
    RGBColor const *color, unsigned int attack_frames, unsigned int decay_frames, unsigned int peak_frames)
{
    if (m_tintColorEnvelope == nullptr) {
        m_tintColorEnvelope = new TintEnvelope();
    }

    if (color) {
        m_tintColorEnvelope->Play(color, attack_frames, decay_frames, peak_frames);
    } else {
        RGBColor c;
        c.Set_From_Int(0xFFFFFFFF);
        m_tintColorEnvelope->Play(&c, 1, 4, 1);
    }

    Clear_Status_Bit(DRAWABLE_STATUS_AMBIENT_LIGHT_LOCKED);
}

void Drawable::Draw(View *view)
{
    if (!Check_Draw_Bit(DRAW_BIT_WEAPON_BONUS)) {
        if (Get_Object() != nullptr && Get_Object()->Is_Effectively_Dead()) {
            m_stealthEmissiveScale = 0.0f;
        } else if (m_stealthEmissiveScale > 0.001f) {
            m_stealthEmissiveScale = m_stealthEmissiveScale * 0.80000001f;
        } else {
            m_stealthEmissiveScale = 0.0f;
        }
    }

    if (!m_hidden && !m_stealthInvisible && !Is_Fully_Obscured_By_Shroud()) {
        if (Get_Object() != nullptr) {
            if (!Get_Object()->Is_Effectively_Dead()) {
                Set_Shadows_Enabled(m_stealthLook != STEALTHLOOK_VISIBLE_DETECTED);
            }
        }

        Matrix3D m = *Get_Transform_Matrix();

        if (!Is_Instance_Identity()) {
            m.Post_Mul(Get_Instance_Matrix());
        }

        Apply_Physics_Xform(&m);

        for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
            (*i)->Do_Draw_Module(&m);
        }
    }
}

void Drawable::Set_Indicator_Color(int color)
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw) {
            draw->Replace_Indicator_Color(color);
        }
    }
}

void Drawable::Color_Tint(RGBColor const *color)
{
    if (color) {
        Color_Flash(color, 0, 0, 1);
        Set_Status_Bit(DRAWABLE_STATUS_AMBIENT_LIGHT_LOCKED);
    } else {
        if (m_tintColorEnvelope == nullptr) {
            m_tintColorEnvelope = new TintEnvelope();
        }

        m_tintColorEnvelope->Set_Idle_State();
        Clear_Status_Bit(DRAWABLE_STATUS_AMBIENT_LIGHT_LOCKED);
    }
}

void Drawable::Changed_Team()
{
    Object *object = Get_Object();

    if (object != nullptr) {
        Set_Indicator_Color(g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT ?
                object->Get_Night_Indicator_Color() :
                object->Get_Indicator_Color());

        if (object->Is_KindOf(KINDOF_FS_FAKE)) {
            Relationship relationship = g_thePlayerList->Get_Local_Player()->Get_Relationship(object->Get_Team());

            if (relationship == ALLIES || relationship == NEUTRAL) {
                Set_Terrain_Decal(TERRAIN_DECAL_9);
            } else {
                Set_Terrain_Decal(TERRAIN_DECAL_8);
            }
        }
    }
}

void Drawable::Clear_Caption_Text()
{
    if (m_captionText) {
        g_theDisplayStringManager->Free_Display_String(m_captionText);
        m_captionText = nullptr;
    }
}

void Drawable::Fade_In(unsigned int time)
{
    Set_Opacity(1.0f);
    m_fadingMode = FADING_MODE_IN;
    m_timeToFade = time;
    m_curFadeFrame = 0;
}

void Drawable::Fade_Out(unsigned int time)
{
    Set_Opacity(0.0f);
    m_fadingMode = FADING_MODE_OUT;
    m_timeToFade = time;
    m_curFadeFrame = 0;
}

void Drawable::Flash_As_Selected(RGBColor const *color)
{
    if (m_selectionColorEnvelope == nullptr) {
        m_selectionColorEnvelope = new TintEnvelope();
    }

    if (color != nullptr) {
        m_selectionColorEnvelope->Play(color, 0, 4, 1);
    } else {
        Object *object = Get_Object();

        if (object != nullptr) {
            RGBColor new_color;

            if (g_theWriteableGlobalData->m_selectionFlashHouseColor) {
                new_color.Set_From_Int(object->Get_Indicator_Color());
            } else {
                new_color.Set_From_Int(0xFFFFFFFF);
            }

            Saturate_RGB(new_color, g_theWriteableGlobalData->m_selectionFlashSaturationFactor);
            m_selectionColorEnvelope->Play(&new_color, 0, 4, 1);
        }
    }
}

void Drawable::Friend_Bind_To_Object(Object *obj)
{
    m_object = obj;

    if (Get_Object() != nullptr) {
        int color;

        if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
            color = Get_Object()->Get_Night_Indicator_Color();
        } else {
            color = Get_Object()->Get_Indicator_Color();
        }

        Set_Indicator_Color(color);

        if (Get_Object()->Is_KindOf(KINDOF_FS_FAKE)) {
            Relationship relationship = g_thePlayerList->Get_Local_Player()->Get_Relationship(Get_Object()->Get_Team());

            if (relationship == ALLIES || relationship == NEUTRAL) {
                Set_Terrain_Decal(TERRAIN_DECAL_9);
            } else {
                Set_Terrain_Decal(TERRAIN_DECAL_8);
            }
        }
    }

    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        (*i)->On_Drawable_Bound_To_Object();
    }
}

void Drawable::Friend_Clear_Selected()
{
    if (Is_Selected()) {
        m_selected = false;
        On_Unselected();
    }
}

void Drawable::Friend_Set_Selected()
{
    if (!Is_Selected()) {
        m_selected = true;
        On_Selected();
    }
}

void Drawable::On_Selected()
{
    Flash_As_Selected(nullptr);

    Object *object = Get_Object();

    if (object != nullptr) {
        ContainModuleInterface *contain = object->Get_Contain();

        if (contain != nullptr) {
            contain->Client_Visible_Contained_Flash_As_Selected();
        }
    }
}

void Drawable::Saturate_RGB(RGBColor &color, float factor)
{
    color.red *= factor;
    color.green *= factor;
    color.blue *= factor;
    float mult = factor * 0.5f;
    color.red -= mult;
    color.green -= mult;
    color.blue -= mult;
}

int Drawable::Get_Barrel_Count(WeaponSlotType slot) const
{
    for (const DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        const ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        int count;

        if (draw != nullptr) {
            count = draw->Get_Barrel_Count(slot);
        } else {
            count = 0;
        }

        if (count) {
            return count;
        }
    }

    return 0;
}

Utf16String Drawable::Get_Caption_Text()
{
    if (m_captionText != nullptr) {
        return m_captionText->Get_Text();
    } else {
        return Utf16String::s_emptyString;
    }
}

bool Drawable::Get_Projectile_Launch_Offset(WeaponSlotType wslot,
    int ammo_index,
    Matrix3D *launch_pos,
    WhichTurretType tur,
    Coord3D *turret_rot_pos,
    Coord3D *turret_pitch_pos) const
{
    for (const DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        const ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr
            && draw->Get_Projectile_Launch_Offset(
                m_conditionState, wslot, ammo_index, launch_pos, tur, turret_rot_pos, turret_pitch_pos)) {
            return true;
        }
    }

    return false;
}

bool Drawable::Handle_Weapon_Fire_FX(WeaponSlotType wslot,
    int specific_barrel_to_use,
    FXList const *fxl,
    float weapon_speed,
    float recoil_amount,
    float recoil_angle,
    Coord3D const *victim_pos,
    float radius)
{
    if (recoil_amount != 0.0f) {
        if (Get_Object() != nullptr) {
            recoil_angle -= Get_Object()->Get_Orientation();
        }

        float f1 = recoil_angle + GAMEMATH_PI;

        if (m_drawableLocoInfo != nullptr) {
            m_drawableLocoInfo->m_accelerationPitchRate =
                GameMath::Cos(f1) * recoil_amount + m_drawableLocoInfo->m_accelerationPitchRate;
            m_drawableLocoInfo->m_accelerationRollRate =
                GameMath::Sin(f1) * recoil_amount + m_drawableLocoInfo->m_accelerationRollRate;
        }
    }

    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr
            && draw->Handle_Weapon_Fire_FX(wslot, specific_barrel_to_use, fxl, weapon_speed, victim_pos, radius)) {
            return true;
        }
    }

    return false;
}

bool Drawable::Is_Mass_Selectable() const
{
    if (Get_Object() != nullptr) {
        if (Get_Object()->Is_Mass_Selectable()) {
            return true;
        }
    }

    return false;
}

bool Drawable::Is_Selectable() const
{
    if (Get_Object() != nullptr) {
        if (Get_Object()->Is_Selectable()) {
            return true;
        }
    }

    return false;
}

void Drawable::Notify_Drawable_Dependency_Cleared()
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw != nullptr) {
            draw->Notify_Draw_Module_Dependency_Cleared();
        }
    }
}

void Drawable::On_Destroy()
{
    for (int i = 0; i < NUM_DRAWABLE_MODULE_TYPES; i++) {
        for (Module **j = m_modules[i]; j != nullptr && *j != nullptr; j++) {
            (*j)->On_Delete();
        }
    }
}

void Drawable::On_Level_Start()
{
    if (m_ambientSoundEnabled && m_ambientSoundFromScriptEnabled) {
        if (m_ambientSound == nullptr
            || (!m_ambientSound->m_event.Get_Event_Name().Is_Empty() && !m_ambientSound->m_event.Is_Currently_Playing())) {
            Start_Ambient_Sound(false);
        }
    }
}

void Drawable::Preload_Assets(TimeOfDayType time_of_day)
{
    for (int i = 0; i < NUM_DRAWABLE_MODULE_TYPES; i++) {
        for (Module **j = m_modules[i]; j != nullptr && *j != nullptr; j++) {
            (*j)->Preload_Assets(time_of_day);
        }
    }
}

void Drawable::Prepend_To_List(Drawable **list)
{
    m_prevDrawable = nullptr;
    m_nextDrawable = *list;

    if (*list != nullptr) {
        (*list)->m_prevDrawable = this;
    }

    *list = this;
}

void Drawable::Remove_From_List(Drawable **list)
{
    if (m_nextDrawable != nullptr) {
        m_nextDrawable->m_prevDrawable = m_prevDrawable;
    }

    if (m_prevDrawable != nullptr) {
        m_prevDrawable->m_nextDrawable = m_nextDrawable;
    } else {
        *list = m_nextDrawable;
    }
}

void Drawable::React_To_Body_Damage_State_Change(BodyDamageType damage)
{
    BitFlags<MODELCONDITION_COUNT> flags;

    static ModelConditionFlagType s_theDamageMap[] = {
        MODELCONDITION_INVALID, MODELCONDITION_DAMAGED, MODELCONDITION_REALLYDAMAGED, MODELCONDITION_RUBBLE
    };

    if (s_theDamageMap[damage] != MODELCONDITION_INVALID) {
        flags.Set(s_theDamageMap[damage], true);
    }

    Clear_And_Set_Model_Condition_Flags(BitFlags<MODELCONDITION_COUNT>(BitFlags<MODELCONDITION_COUNT>::kInit,
                                            MODELCONDITION_DAMAGED,
                                            MODELCONDITION_REALLYDAMAGED,
                                            MODELCONDITION_RUBBLE),
        flags);

    if (!g_theGameLogic->Get_Prepare_New_Game()) {
        Start_Ambient_Sound(damage, g_theWriteableGlobalData->m_timeOfDay, false);
    }
}

void Drawable::React_To_Geometry_Change()
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        (*i)->React_To_Geometry_Change();
    }
}

void Drawable::Set_Caption_Text(Utf16String const &caption)
{
    if (caption.Is_Empty()) {
        Clear_Caption_Text();
    } else {
        Utf16String str(caption);
        g_theLanguageFilter->Filter_Line(str);

        if (m_captionText == nullptr) {
            m_captionText = g_theDisplayStringManager->New_Display_String();
            m_captionText->Set_Font(g_theFontLibrary->Get_Font(g_theInGameUI->Get_Drawable_Caption_Font(),
                g_theGlobalLanguage->Adjust_Font_Size(g_theInGameUI->Get_Drawable_Caption_Size()),
                g_theInGameUI->Get_Drawable_Caption_Bold()));
            m_captionText->Set_Text(str);
        } else {
            if (m_captionText->Get_Text().Compare(str) != 0) {
                m_captionText->Set_Text(str);
            }
        }
    }
}

void Drawable::Set_Drawable_Hidden(bool hidden)
{
    if (hidden != m_hidden) {
        m_hidden = hidden;
        Update_Hidden_Status();
    }
}

void Drawable::Set_Effective_Opacity(float opacity1, float opacity2)
{
    if (opacity2 != -1.0f) {
        float f1;

        if (opacity2 < 0.0f) {
            f1 = 0.0;
        } else {
            f1 = opacity2;
        }

        float f2;

        if (f1 > 1.0f) {
            f2 = 1.0f;
        } else if (opacity2 < 0.0f) {
            f2 = 0.0f;
        } else {
            f2 = opacity2;
        }

        m_effectiveOpacity1 = f2;
    }

    float f3;

    if (opacity1 < 0.0f) {
        f3 = 0.0f;
    } else {
        f3 = opacity1;
    }

    float f4;

    if (f3 > 1.0f) {
        f4 = 1.0f;
    } else if (opacity1 < 0.0f) {
        f4 = 0.0f;
    } else {
        f4 = opacity1;
    }

    m_effectiveOpacity2 = (1.0f - m_effectiveOpacity1) * f4 + m_effectiveOpacity1;
}

void Drawable::Set_Selectable(bool selectable)
{
    if (!selectable) {
        g_theInGameUI->Deselect_Drawable(this);
    }

    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw) {
            draw->Set_Selectable(selectable);
        }
    }
}

void Drawable::Set_Time_Of_Day(TimeOfDayType tod)
{
    BodyDamageType damage = BODY_PRISTINE;

    if (Get_Object() != nullptr) {
        if (Get_Object()->Get_Body_Module() != nullptr) {
            damage = Get_Object()->Get_Body_Module()->Get_Damage_State();
        }
    }

    Start_Ambient_Sound(damage, tod, false);
    BitFlags<MODELCONDITION_COUNT> flags;
    flags = m_conditionState;
    flags.Set(MODELCONDITION_NIGHT, tod == TIME_OF_DAY_NIGHT);
    Replace_Model_Condition_Flags(flags, false);
}

void Drawable::Update_Drawable_Clip_Status(unsigned int show, unsigned int count, WeaponSlotType slot)
{
    for (DrawModule **i = Get_Draw_Modules_Non_Dirty(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw) {
            draw->Update_Projectile_Clip_Status(show, count, slot);
        }
    }
}

void Drawable::Update_Drawable_Supply_Status(int max, int current)
{
    for (DrawModule **i = Get_Draw_Modules(); *i != nullptr; i++) {
        ObjectDrawInterface *draw = (*i)->Get_Object_Draw_Interface();

        if (draw) {
            draw->Update_Draw_Module_Supply_Status(max, current);
        }
    }
}
