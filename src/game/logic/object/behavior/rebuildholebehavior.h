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
#pragma once
#include "always.h"
#include "object.h"

class RebuildHoleBehaviorInterface
{
public:
    virtual void Start_Rebuild_Process(const ThingTemplate *rebuild, ObjectID spawner) = 0;
    virtual ObjectID Get_Spawner_ID() = 0;
    virtual ObjectID Get_Reconstructed_Building_ID() = 0;
    virtual const ThingTemplate *Get_Rebuild_Template() const = 0;
};

class RebuildHoleBehavior
{
public:
    static RebuildHoleBehaviorInterface *Get_Rebuild_Hole_Behavior_Interface_From_Object(Object *obj);
};
