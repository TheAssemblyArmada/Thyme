/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Action Manager
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
#include "gametype.h"
#include "subsysteminterface.h"

class Object;

enum CanEnterType
{
    CAN_ENTER_UNK,
};

class ActionManager : public SubsystemInterface
{
public:
    ActionManager() {}
    virtual ~ActionManager() override {}
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    bool Can_Enter_Object(const Object *obj, const Object *object_to_enter, CommandSourceType source, CanEnterType type);
};

#ifdef GAME_DLL
extern ActionManager *&g_theActionManager;
#else
extern ActionManager *g_theActionManager;
#endif
