/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief GameMain function which sets up and runs the game engine.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamemain.h"
#include "win32gameengine.h"

GameEngine *Create_Game_Engine()
{
#ifdef GAME_DLL
    return Call_Function<GameEngine *>(PICK_ADDRESS(0x00401CD0, 0));
#else
    GameEngine *tmp = new Win32GameEngine;
    tmp->Set_Is_Active(true);

    return tmp;
#endif
}

void Game_Main(int32_t argc, char *argv[])
{
    g_theGameEngine = Create_Game_Engine();
    g_theGameEngine->Init(argc, argv);
    g_theGameEngine->Execute();

    if (g_theGameEngine != nullptr) {
        delete g_theGameEngine;
    }

    g_theGameEngine = nullptr;
}
