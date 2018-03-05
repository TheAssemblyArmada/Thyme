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
#include "dx8wrapper.h"

#ifndef THYME_STANDALONE
IDirect3D8 *(__stdcall *&DX8Wrapper::s_d3dCreateFunction)(unsigned) = Make_Global<IDirect3D8 *(__stdcall *)(unsigned)>(0x00A47F6C);
HMODULE &DX8Wrapper::s_d3dLib = Make_Global<HMODULE>(0x00A47F70);
#else
#ifdef PLATFORM_WINDOWS
IDirect3D8 *(__stdcall *DX8Wrapper::s_d3dCreateFunction)(unsigned) = nullptr;
HMODULE DX8Wrapper::s_d3dLib = nullptr;
#endif
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
