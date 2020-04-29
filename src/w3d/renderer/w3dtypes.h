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

#ifdef BUILD_WITH_D3D8
#include <d3d8.h>
#include <d3d8types.h>

typedef LPDIRECT3DTEXTURE8 w3dtexture_t;
typedef LPDIRECT3DBASETEXTURE8 w3dbasetexture_t;
typedef LPDIRECT3DSURFACE8 w3dsurface_t;
typedef D3DCAPS8 w3dcaps_t;
typedef LPDIRECT3D8 w3dhandle_t;
typedef D3DADAPTER_IDENTIFIER8 w3dadapterid_t;
typedef LPDIRECT3DDEVICE8 w3ddevice_t;
typedef D3DPOOL w3dpool_t;
typedef D3DMATERIAL8 w3dmat_t;

#define W3D_TYPE_INVALID_TEXTURE nullptr
#define W3D_TYPE_INVALID_SURFACE nullptr
#else
// TODO for alternate 3D frameworks such as OpenGL or later D3D
typedef int w3dtexture_t;
typedef int w3dbasetexture_t;
typedef int w3dsurface_t;
typedef int w3dcaps_t;
typedef int w3dhandle_t;
typedef int w3dadapterid_t;
typedef int w3ddevice_t;
typedef int w3dpool_t;
typedef int w3dmat_t;

#define W3D_TYPE_INVALID_TEXTURE 0
#define W3D_TYPE_INVALID_SURFACE 0
#endif
