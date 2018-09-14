/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper around platform 3D graphics library.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "dx8wrapper.h"

#ifndef THYME_STANDALONE
IDirect3D8 *(__stdcall *&DX8Wrapper::s_d3dCreateFunction)(unsigned) = Make_Global<IDirect3D8 *(__stdcall *)(unsigned)>(
    0x00A47F6C);
HMODULE &DX8Wrapper::s_d3dLib = Make_Global<HMODULE>(0x00A47F70);
IDirect3D8 *&DX8Wrapper::s_d3dInterface = Make_Global<IDirect3D8 *>(0x00A47EEC);
IDirect3DBaseTexture8 **DX8Wrapper::s_textures = Make_Pointer<IDirect3DBaseTexture8 *>(0x00A42784);
unsigned *DX8Wrapper::s_renderStates = Make_Pointer<unsigned>(0x00A46CC0);
unsigned *DX8Wrapper::s_textureStageStates = Make_Pointer<unsigned>(0x00A46CC0);
Vector4 *DX8Wrapper::s_vertexShaderConstants = Make_Pointer<Vector4>(0x00A47778);
unsigned *DX8Wrapper::s_pixelShaderConstants = Make_Pointer<unsigned>(0x00A427C0);
bool &DX8Wrapper::s_isInitialised = Make_Global<bool>(0x00A47EC8);
bool &DX8Wrapper::s_isWindowed = Make_Global<bool>(0x00A47EC9);
#else
#ifdef PLATFORM_WINDOWS
IDirect3D8 *(__stdcall *DX8Wrapper::s_d3dCreateFunction)(unsigned) = nullptr;
HMODULE DX8Wrapper::s_d3dLib = nullptr;
IDirect3D8 *DX8Wrapper::s_d3dInterface;
IDirect3DBaseTexture8 *DX8Wrapper::s_textures[MAX_TEXTURE_STAGES];
#endif
unsigned DX8Wrapper::s_renderStates[256];
unsigned DX8Wrapper::s_textureStageStates[MAX_TEXTURE_STAGES][32];
Vector4 DX8Wrapper::s_vertexShaderConstants[96]; // Not 100% sure this is a Vector4 array
unsigned DX8Wrapper::s_pixelShaderConstants[32]; // Not 100% on type, seems unused.
bool DX8Wrapper::s_isInitialised;
bool DX8Wrapper::s_isWindowed;
#endif

void DX8Wrapper::Init(void *hwnd, bool lite)
{
#ifndef THYME_STANDALONE
    Call_Function<void, void*, bool>(0x00800670, hwnd, lite);
#endif
}

void DX8Wrapper::Shutdown()
{
#ifndef THYME_STANDALONE
    Call_Function<void>(0x00800860);
#endif
}
