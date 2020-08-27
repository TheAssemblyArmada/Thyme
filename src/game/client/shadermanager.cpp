/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shader Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "shadermanager.h"
#ifndef GAME_DLL
TextureClass *ShaderManager::s_textures[MAX_TEXTURE_STAGES];
#endif

int ShaderManager::Get_Shader_Passes(ShaderTypes shader)
{
#ifdef GAME_DLL
    return Call_Function<int, ShaderTypes>(PICK_ADDRESS(0x0074E140, 0x00613479), shader);
#else
    return 0;
#endif
}

int ShaderManager::Set_Shader(ShaderTypes shader, int pass)
{
#ifdef GAME_DLL
    return Call_Function<int, ShaderTypes, int>(PICK_ADDRESS(0x0074E150, 0x00613488), shader, pass);
#else
    return 0;
#endif
}

void ShaderManager::Reset_Shader(ShaderTypes shader)
{
#ifdef GAME_DLL
    Call_Function<void, ShaderTypes>(PICK_ADDRESS(0x0074E190, 0x006134E8), shader);
#endif
}
