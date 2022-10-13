/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aiupdate.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void AIUpdateModuleData::Parse_Locomotor_Set(INI *ini, void *formal, void *store, const void *user_data)
{
    // todo needs AIUpdateModuleData
#ifdef GAME_DLL
    Call_Function<void, INI *, void *, void *, const void *>(
        PICK_ADDRESS(0x005CFEF0, 0x007F35AD), ini, formal, store, user_data);
#endif
}

bool AIUpdateInterface::Get_Turret_Rot_And_Pitch(WhichTurretType tur, float *turret_angle, float *turret_pitch)
{
#ifdef GAME_DLL
    return Call_Method<bool, AIUpdateInterface, WhichTurretType, float *, float *>(
        PICK_ADDRESS(0x005D13F0, 0x007F4BCB), this, tur, turret_angle, turret_pitch);
#else
    return false;
#endif
}

float AIUpdateInterface::Get_Cur_Locomotor_Speed() const
{
#ifdef GAME_DLL
    return Call_Method<float, AIUpdateInterface const>(PICK_ADDRESS(0x005D1500, 0x007F4D62), this);
#else
    return 0.0f;
#endif
}

bool AIUpdateInterface::Has_Locomotor_For_Surface(LocomotorSurfaceType t)
{
    return m_locomotorSet.Find_Locomotor(t) != nullptr;
}

unsigned int AIUpdateInterface::Get_Mood_Matrix_Action_Adjustment(MoodMatrixAction action) const
{
#ifdef GAME_DLL
    return Call_Method<unsigned int, AIUpdateInterface const, MoodMatrixAction>(
        PICK_ADDRESS(0x005D5E80, 0x007FB7F1), this, action);
#else
    return 0;
#endif
}

Object *AIUpdateInterface::Get_Next_Mood_Target(bool called_by_ai, bool called_during_idle)
{
#ifdef GAME_DLL
    return Call_Method<Object *, AIUpdateInterface, bool, bool>(
        PICK_ADDRESS(0x005D60B0, 0x007FBB78), this, called_by_ai, called_during_idle);
#else
    return nullptr;
#endif
}

void AIUpdateInterface::Reset_Next_Mood_Check_Time()
{
#ifdef GAME_DLL
    Call_Method<void, AIUpdateInterface>(PICK_ADDRESS(0x005D6030, 0x007FBAB5), this);
#endif
}
