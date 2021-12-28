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
#include "object.h"

const Matrix3D *Drawable::Get_Transform_Matrix() const
{
    const Object *object = Get_Object();

    if (object != nullptr) {
        return object->Get_Transform_Matrix();
    }

    return Thing::Get_Transform_Matrix();
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