/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Provides an interface for networking implementations.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "lanapiinterface.h"

#ifndef GAME_DLL
LANAPIInterface *g_theLAN = nullptr;
#endif