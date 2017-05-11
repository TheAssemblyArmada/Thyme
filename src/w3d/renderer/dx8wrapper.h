////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: DX8WRAPPER.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Wrapper around platform 3D functions.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef DX8WRAPPER_H
#define DX8WRAPPER_H

#include "hooker.h"
#include "vector4.h"
#include <d3d8.h>

#define o_d3dInterface Make_Global<IDirect3D8 *>(0x00A47EEC);

typedef IDirect3D8 *(__stdcall *d3dcreate_t)(unsigned sdk_version);

const unsigned MAX_TEXTURE_STAGES = 8;

// This class is going to be very much a WIP until we have a better idea
// of the way it interacts with the rest of the program and what its structure
// is.

class DX8Wrapper
{
public:
    static void Init(void *hwnd, bool lite = false);
    static void Shutdown();
private:
    static d3dcreate_t s_d3dCreateFunction;
    static HINSTANCE s_d3dLib;
    //static IDirect3DBaseTexture8 *s_textures[MAX_TEXTURE_STAGES];
    //static unsigned s_renderStates[256];
    //static unsigned s_textureStageStates[MAX_TEXTURE_STAGES][32];
    //static Vector4 s_vertexShaderConstants[96];   // Not 100% sure this is a Vector4 array
    //static unsigned s_pixelShaderConstants[32];   // Not 100% on type, seems unused.
};

#endif
