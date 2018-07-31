/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for local file system implementations.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "localfilesystem.h"

#ifndef THYME_STANDALONE
LocalFileSystem *&g_theLocalFileSystem = Make_Global<LocalFileSystem *>(0x00A2BE60);
#else
LocalFileSystem *g_theLocalFileSystem = nullptr;
#endif
