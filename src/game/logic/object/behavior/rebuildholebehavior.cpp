/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Rebuild Hole Behavior
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "rebuildholebehavior.h"
#include "behaviormodule.h"

RebuildHoleBehaviorInterface *RebuildHoleBehavior::Get_Rebuild_Hole_Behavior_Interface_From_Object(Object *obj)
{
    RebuildHoleBehaviorInterface *rebuild = nullptr;

    for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
        rebuild = (*module)->Get_Rebuild_Hole_Behavior_Interface();

        if (rebuild != nullptr) {
            break;
        }
    }

    return rebuild;
}
