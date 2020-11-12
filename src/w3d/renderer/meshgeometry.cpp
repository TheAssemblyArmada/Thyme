/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Geometry Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "meshgeometry.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

const Vector3 *MeshGeometryClass::Get_Vertex_Normal_Array()
{
#ifdef GAME_DLL
    return Call_Method<const Vector3 *, MeshGeometryClass>(PICK_ADDRESS(0x0083B760, 0x005233D0), this);
#else
    return nullptr;
#endif
}
