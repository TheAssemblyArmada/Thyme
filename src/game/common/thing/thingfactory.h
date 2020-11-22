/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
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
#include "subsysteminterface.h"
#include "thingtemplate.h"

class ThingFactory : public SubsystemInterface
{
public:
    ThingTemplate *Find_Template_Internal(Utf8String name);
};

#ifdef GAME_DLL
#include "hooker.h"
extern ThingFactory *&g_theThingFactory;
#else
extern ThingFactory *g_theThingFactory;
#endif
