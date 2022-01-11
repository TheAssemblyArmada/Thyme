/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Load Screen handling classes.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "loadscreen.h"
#include "display.h"
#include "fpusetting.h"
#include "gameengine.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

// zh: 0x00608620 wb: 0x009C3F11
LoadScreen::~LoadScreen()
{
#ifdef GAME_DLL
    Call_Method<void, LoadScreen>(PICK_ADDRESS(0x00608620, 0x009C3F11), this);
#else
    // TODO: Missing GameWindowManager
    if (m_loadScreen != nullptr) {
        // g_theWindowManager->Win_Destroy(m_loadScreen);
    }
    m_loadScreen = nullptr;
#endif
}

// zh: 0x00608690 wb: 0x009C3F53
void LoadScreen::Update(int32_t percent)
{

#ifdef GAME_DLL
    Call_Method<void, LoadScreen, int32_t>(PICK_ADDRESS(0x00608690, 0x009C3F53), this, percent);
#else
    // TODO: Missing GameWindowManager
    g_theGameEngine->Service_Windows_OS();
    if (!g_theGameEngine->Get_Quitting()) {
        // g_theWindowManager->Update();
        g_theDisplay->Update();
        g_theDisplay->Draw();
        Set_FP_Mode();
    }
#endif
}
