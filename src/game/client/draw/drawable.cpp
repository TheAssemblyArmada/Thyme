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

#include "drawmodule.h"
#include "object.h"

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
