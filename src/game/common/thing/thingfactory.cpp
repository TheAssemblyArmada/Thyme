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
#include "thingfactory.h"
#ifndef GAME_DLL
ThingFactory *g_theThingFactory = nullptr;
#endif

ThingTemplate *ThingFactory::Find_Template_Internal(Utf8String name)
{
#ifdef GAME_DLL
    return Call_Method<ThingTemplate *, ThingFactory, Utf8String>(PICK_ADDRESS(0x004B0B50, 0x00407DD0), this, name);
#else
    return nullptr;
#endif
}
