////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: MAIN.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: User entry point.
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _MAIN_H_
#define _MAIN_H_

#include "always.h"

#define ApplicationHWnd (Make_Global<HWND>(0x00A27B08))

// This will eventually be replaced by a standard int main(int arc, char *argv[]) function
int __stdcall Main_Func(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

#endif