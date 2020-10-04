/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Vertex Materials
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "vertmaterial.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void VertexMaterialClass::Init()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008194F0, 0x004E6590));
#endif
}

void VertexMaterialClass::Shutdown()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008195B0, 0x004E6650));
#endif
}

void VertexMaterialClass::Apply() const
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x008193A0, 0x004E63A0));
#endif
}

void VertexMaterialClass::Apply_Null()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00819460, 0x004E64B0));
#endif
}
