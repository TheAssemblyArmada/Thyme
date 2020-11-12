/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Model Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "meshmdl.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

bool MeshModelClass::Needs_Vertex_Normals()
{
#ifdef GAME_DLL
    return Call_Method<bool, MeshModelClass>(PICK_ADDRESS(0x00829A90, 0x00526B20), this);
#else
    return false;
#endif
}
