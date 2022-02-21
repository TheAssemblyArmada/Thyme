/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Provides an interface for the Input method editor manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "imemanagerinterface.h"

#ifndef GAME_DLL
IMEManagerInterface *g_theIMEManager = nullptr;
#endif