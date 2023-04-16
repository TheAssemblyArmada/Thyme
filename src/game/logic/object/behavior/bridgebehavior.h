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
#pragma once
#include "always.h"
#include "terrainroads.h"

class Object;

class BridgeBehaviorInterface
{
public:
    virtual void Set_Tower(BridgeTowerType type, Object *obj) = 0;
    virtual ObjectID Get_Tower_ID(BridgeTowerType type) = 0;
    virtual void Create_Scaffolding() = 0;
    virtual void Remove_Scaffolding() = 0;
    virtual bool Is_Scaffold_In_Motion() = 0;
    virtual bool Is_Scaffold_Present() = 0;
};

class BridgeBehavior
{
public:
    static BridgeBehaviorInterface *Get_Bridge_Behavior_Interface_From_Object(Object *obj);
};
