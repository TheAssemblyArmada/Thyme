/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "mesh.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void MeshClass::Render_Material_Pass(MaterialPassClass *pass, IndexBufferClass *ib)
{
#ifdef GAME_DLL
    Call_Method<void, MeshClass, MaterialPassClass *, IndexBufferClass *>(
        PICK_ADDRESS(0x0082C850, 0x004EAF20), this, pass, ib);
#endif
}

void MeshClass::Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm)
{
#ifdef GAME_DLL
    Call_Method<void, MeshClass, Vector3 *, Vector3 *>(PICK_ADDRESS(0x0082C150, 0x004EA5B0), this, dst_vert, dst_norm);
#endif
}
