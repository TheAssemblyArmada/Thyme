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
#include "bodymodule.h"
#include "colorspace.h"
#include "displaystring.h"
#include "displaystringmanager.h"
#include "drawgroupinfo.h"
#include "drawmodule.h"
#include "gameclient.h"
#include "gamelogic.h"
#include "gametext.h"
#include "globaldata.h"
#include "ingameui.h"
#include "locomotor.h"
#include "object.h"
#include "opencontain.h"
#include "player.h"
#include "playerlist.h"
#include "scriptengine.h"
#include "stealthupdate.h"
#include "stickybombupdate.h"
#include "w3ddisplay.h"
#include "w3dview.h"

bool Drawable::s_staticImagesInited;
Image *Drawable::s_veterancyImage[4];
Image *Drawable::s_fullAmmo;
Image *Drawable::s_emptyAmmo;
Image *Drawable::s_fullContainer;
Image *Drawable::s_emptyContainer;
Anim2DTemplate **Drawable::s_animationTemplates;

static const char *s_theDrawableIconNames[] = { "DefaultHeal",
    "StructureHeal",
    "VehicleHeal"
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

const Matrix3D *Drawable::Get_Transform_Matrix() const
{
    const Object *object = Get_Object();

    if (object != nullptr) {
        return object->Get_Transform_Matrix();
    }

    return Thing::Get_Transform_Matrix();
}

bool Drawable::Is_Visible() const
{
    const DrawModule **draw_modules = Get_Draw_Modules();

    for (const DrawModule *draw_module = draw_modules[0]; draw_modules != nullptr; ++draw_module) {
        if (draw_module->Is_Visible()) {
            return true;
        }
    }
    return false;
}

void Drawable::Friend_Lock_Dirty_Stuff_For_Iteration()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x004794C0, 0x007CB858));
#endif
}

void Drawable::Friend_Unlock_Dirty_Stuff_For_Iteration()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00479550, 0x007CB8AF));
#endif
}

const Vector3 *Drawable::Get_Tint_Color() const
{
#ifdef GAME_DLL
    return Call_Method<Vector3 *, const Drawable>(PICK_ADDRESS(0x0046FA60, 0x007C16BF), this);
#else
    return nullptr;
#endif
}

const Vector3 *Drawable::Get_Selection_Color() const
{
#ifdef GAME_DLL
    return Call_Method<Vector3 *, const Drawable>(PICK_ADDRESS(0x0046FA80, 0x007C16F6), this);
#else
    return nullptr;
#endif
}

DrawModule **Drawable::Get_Draw_Modules()
{
#ifdef GAME_DLL
    return Call_Method<DrawModule **, Drawable>(PICK_ADDRESS(0x00475F40, 0x007C87E4), this);
#else
    return nullptr;
#endif
}

const DrawModule **Drawable::Get_Draw_Modules() const
{
#ifdef GAME_DLL
    return Call_Method<const DrawModule **, const Drawable>(PICK_ADDRESS(0x00475F40, 0x007C88A2), this);
#else
    return nullptr;
#endif
}

bool Drawable::Get_Current_Worldspace_Client_Bone_Positions(char const *none_name_prefix, Matrix3D &m) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Drawable, char const *, Matrix3D &>(
        PICK_ADDRESS(0x0046F190, 0x007C122A), this, none_name_prefix, m);
#else
    return false;
#endif
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
#ifdef GAME_DLL
    return Call_Method<ClientUpdateModule *, const Drawable, NameKeyType>(PICK_ADDRESS(0x00477A80, 0x007C9F40), this, key);
#else
    return nullptr;
#endif
}

int Drawable::Get_Pristine_Bone_Positions(
    char const *bone_name_prefix, int start_index, Coord3D *positions, Matrix3D *transforms, int max_bones) const
{
#ifdef GAME_DLL
    return Call_Method<int, const Drawable, char const *, int, Coord3D *, Matrix3D *, int>(
        PICK_ADDRESS(0x0046EFD0, 0x007C109A), this, bone_name_prefix, start_index, positions, transforms, max_bones);
#else
    return 0;
#endif
}

bool Drawable::Get_Should_Animate(bool should) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Drawable, bool>(PICK_ADDRESS(0x0046EB90, 0x007C0D05), this, should);
#else
    return false;
#endif
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
        s_animationTemplates = new Anim2DTemplate *[14];
        s_animationTemplates[0] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[0]);
        s_animationTemplates[1] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[1]);
        s_animationTemplates[2] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[2]);
        s_animationTemplates[4] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[4]);
        s_animationTemplates[5] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[5]);
        s_animationTemplates[6] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[6]);
        s_animationTemplates[7] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[7]);
        s_animationTemplates[8] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[8]);
        s_animationTemplates[9] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[9]);
        s_animationTemplates[10] = nullptr;
        s_animationTemplates[11] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[11]);
        s_animationTemplates[12] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[12]);
        s_animationTemplates[13] = g_theAnim2DCollection->Find_Template(s_theDrawableIconNames[13]);
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
    return object->Get_Formation_ID() != FORMATION_UNK;
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
                            int infantry = Make_Color(0, 255, 0, 255);
                            int vehicle = Make_Color(0, 0, 255, 255);

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

        if (Get_Icon_Info()->anims[ICON_CARBOMB] != nullptr) {
            int fwidth = Get_Icon_Info()->anims[ICON_CARBOMB]->Get_Current_Frame_Width();
            int fheight = Get_Icon_Info()->anims[ICON_CARBOMB]->Get_Current_Frame_Height();
            int width = GameMath::Fast_To_Int_Truncate((region->hi.x - region->lo.x) * 0.5f);
            int height = GameMath::Fast_To_Int_Truncate((float)width / (float)(fwidth) * (float(fheight)));
            int x = GameMath::Fast_To_Int_Truncate(region->lo.x * (region->hi.x - region->lo.x) * 0.5f - width * 0.5f);
            int y = GameMath::Fast_To_Int_Truncate(region->lo.y * (region->hi.y - region->lo.y) * 0.5f) + 5;
            Get_Icon_Info()->anims[ICON_CARBOMB]->Draw(x, y, width, height);
            Get_Icon_Info()->timings[ICON_CARBOMB] = 0x3FFFFFFF;
        }
    } else {
        Kill_Icon(ICON_CARBOMB);
    }

    static const NameKeyType key_StickyBombUpdate = g_theNameKeyGenerator->Name_To_Key("StickyBombUpdate");
    StickyBombUpdate *update = (StickyBombUpdate *)object->Find_Update_Module(key_StickyBombUpdate);

    if (update && update->Get_Target_Object()) {
        if (update->Has_Die_Frame()) {
            if (Get_Icon_Info()->anims[ICON_BOMB_TIMED] == nullptr) {
                Get_Icon_Info()->anims[ICON_BOMB_TIMED] =
                    new Anim2D(s_animationTemplates[ICON_BOMB_REMOTE], g_theAnim2DCollection);

                Get_Icon_Info()->anims[ICON_BOMB_REMOTE] =
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
                int x = GameMath::Fast_To_Int_Truncate(region->lo.x * (region->hi.x - region->lo.x) * 0.5f - width * 0.5f);
                int y = GameMath::Fast_To_Int_Truncate(region->lo.y * (region->hi.y - region->lo.y) * 0.5f) + 5;
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
                int fwidth = Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Get_Current_Frame_Width();
                int fheight = Get_Icon_Info()->anims[ICON_BOMB_TIMED]->Get_Current_Frame_Height();
                int width = GameMath::Fast_To_Int_Truncate((region->hi.x - region->lo.x) * 0.65f);
                int height = GameMath::Fast_To_Int_Truncate((float)width / (float)fwidth * (float)fheight);
                int x = GameMath::Fast_To_Int_Truncate(region->lo.x * (region->hi.x - region->lo.x) * 0.5f - width * 0.5f);
                int y = GameMath::Fast_To_Int_Truncate(region->lo.y * (region->hi.y - region->lo.y) * 0.5f) + 5;
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
