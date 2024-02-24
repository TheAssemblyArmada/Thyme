/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI guard
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "always.h"
#include "aiguard.h"
#include "ai.h"

float AIGuardMachine::Get_Std_Guard_Range(Object *obj)
{
    return AI::Get_Adjusted_Vision_Range_For_Object(obj, 7);
}
