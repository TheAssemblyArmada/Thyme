/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base client object providing overall control of client IO and rendering.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */ 
#include "gameclient.h"

#ifndef THYME_STANDALONE
GameClient *&g_theGameClient = Make_Global<GameClient *>(0x00A2BBF8);
#else
GameClient *g_theGameClient;
#endif