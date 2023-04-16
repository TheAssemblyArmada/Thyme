/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Bridge Tower Behavior
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bridgetowerbehavior.h"
#include "behaviormodule.h"
#include "object.h"

BridgeTowerBehaviorInterface *BridgeTowerBehavior::Get_Bridge_Tower_Behavior_Interface_From_Object(Object *obj)
{
    if (obj == nullptr || !obj->Is_KindOf(KINDOF_BRIDGE_TOWER)) {
        return nullptr;
    }

    for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
        BridgeTowerBehaviorInterface *tower = (*module)->Get_Bridge_Tower_Behavior_Interface();

        if (tower != nullptr) {
            return tower;
        }
    }

    return nullptr;
}
