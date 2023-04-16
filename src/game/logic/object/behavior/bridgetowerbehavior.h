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
#pragma once
#include "always.h"
#include "terrainroads.h"

class Object;

class BridgeTowerBehaviorInterface
{
public:
    virtual void Set_Bridge(Object *obj) = 0;
    virtual ObjectID Get_Bridge_ID() = 0;
    virtual void Set_Tower_Type(BridgeTowerType type) = 0;
};

class BridgeTowerBehavior
{
public:
    static BridgeTowerBehaviorInterface *Get_Bridge_Tower_Behavior_Interface_From_Object(Object *obj);
};
