/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief W3D UI function pointer manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dfunctionlexicon.h"

void W3DFunctionLexicon::Init()
{
#ifdef GAME_DLL
    Call_Method<void, FunctionLexicon>(PICK_ADDRESS(0x007774E0, 0), this);
#else
    FunctionLexicon::Init();
    // TODO requires several function pointer tables, do them as functions that call them require it?
#endif
}
