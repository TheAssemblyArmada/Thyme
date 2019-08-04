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
#pragma once

#include "always.h"

void Game_Main(int argc, char *argv[]);

#ifdef GAME_DLL
#include "hooker.h"
#endif