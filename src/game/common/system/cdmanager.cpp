/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief CD Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "cdmanager.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
CDManagerInterface *g_theCDManager;
#endif

CDManagerInterface *Create_CD_Manager()
{
#ifdef GAME_DLL
    return Call_Function<CDManagerInterface *>(PICK_ADDRESS(0x007427B0, 0x00642E41));
#else
    return nullptr;
#endif
}
