/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Bridge Behavior
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bridgebehavior.h"
#include "behaviormodule.h"
#include "object.h"

BridgeBehaviorInterface *BridgeBehavior::Get_Bridge_Behavior_Interface_From_Object(Object *obj)
{
    if (obj == nullptr) {
        return nullptr;
    }

    for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
        BridgeBehaviorInterface *bridge = (*module)->Get_Bridge_Behavior_Interface();

        if (bridge != nullptr) {
            return bridge;
        }
    }

    return nullptr;
}
