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
#include "bitflags.h"
#include "disabledtypes.h"
#include "main.h"
#include "win32gameengine.h"

GameEngine *Create_Game_Engine()
{
    GameEngine *engine = new Win32GameEngine;
    engine->Set_Is_Active(g_gameActive);
    return engine;
}

void Game_Main(int argc, char *argv[])
{
    g_theGameEngine = Create_Game_Engine();
    g_theGameEngine->Init(argc, argv);
    g_theGameEngine->Execute();

    if (g_theGameEngine != nullptr) {
        delete g_theGameEngine;
    }

    g_theGameEngine = nullptr;
}
