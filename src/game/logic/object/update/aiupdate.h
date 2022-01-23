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
#pragma once
#include "always.h"
#include "object.h"
#include "updatemodule.h"
class INI;

class AIUpdateModuleData
{
public:
    static void Parse_Locomotor_Set(INI *ini, void *formal, void *store, const void *user_data);
};

class AIUpdateInterface
{
public:
    bool Get_Turret_Rot_And_Pitch(WhichTurretType tur, float *turret_angle, float *turret_pitch);
};
