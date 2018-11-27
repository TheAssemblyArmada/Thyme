/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Typedefs to have generic type names for different underlying implementations.
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

#ifdef D3D8_BUILD
#include <d3d8.h>
#include <d3d8types.h>

typedef LPDIRECT3DTEXTURE8 w3dtexture_t;
typedef LPDIRECT3DBASETEXTURE8 w3dbasetexture_t;

#define W3D_TYPE_INVALID_TEXTURE nullptr
#else
// TODO for alternate 3D frameworks such as OpenGL or later D3D
typedef int w3dtexture_t;
typedef int w3dbasetexture_t;

#define W3D_TYPE_INVALID_TEXTURE 0
#endif
