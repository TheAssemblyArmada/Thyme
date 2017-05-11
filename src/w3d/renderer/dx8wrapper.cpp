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
#include "hooker.h"

d3dcreate_t DX8Wrapper::s_d3dCreateFunction = nullptr;
HINSTANCE DX8Wrapper::s_d3dLib = nullptr;

void DX8Wrapper::Init(void *hwnd, bool lite)
{
    Call_Function<void, void*, bool>(0x00800670, hwnd, lite);
}

void DX8Wrapper::Shutdown()
{
    Call_Function<void>(0x00800860);
}
