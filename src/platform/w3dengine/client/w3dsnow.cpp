/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3d Snow Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dsnow.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
W3DSnowManager *g_theSnowManager;
#endif

void W3DSnowManager::Release_Resources()
{
#ifdef GAME_DLL
    Call_Method<void, W3DSnowManager>(PICK_ADDRESS(0x0079EBB0, 0x0066BF29), this);
#endif
}

void W3DSnowManager::Re_Acquire_Resources()
{
#ifdef GAME_DLL
    Call_Method<void, W3DSnowManager>(PICK_ADDRESS(0x0079EC00, 0x0066BF98), this);
#endif
}
