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
#pragma once
#include "always.h"
#include "texture.h"
#include "dx8wrapper.h"
class ShaderManager
{
public:
    enum ShaderTypes
    {
        ST_INVALID,
        ST_TERRAIN_BASE,
        ST_TERRAIN_BASE_NOISE1,
        ST_TERRAIN_BASE_NOISE2,
        ST_TERRAIN_BASE_NOISE12,
        ST_SHROUD_TEXTURE,
        ST_ROAD_BASE,
        ST_ROAD_BASE_NOISE1,
        ST_ROAD_BASE_NOISE2,
        ST_ROAD_BASE_NOISE12,
        ST_MAX,
    };

    static void Set_Texture(int index, TextureClass *texture) { s_textures[index] = texture; }
    static int Get_Shader_Passes(ShaderTypes shader);
    static int Set_Shader(ShaderTypes shader, int pass);
    static void Reset_Shader(ShaderTypes shader);

private:
#ifdef GAME_DLL
    static ARRAY_DEC(TextureClass *, s_textures, MAX_TEXTURE_STAGES);
#else
    static TextureClass *s_textures[MAX_TEXTURE_STAGES];
#endif
};
