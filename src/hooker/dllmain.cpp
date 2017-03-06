////////////////////////////////////////////////////////////////////////////////
//                               --  THYME --                                 //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: DLLMAIN.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: CCHyper
//
//   Description:: Defines the entry point for the DLL application.
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


////////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////////
#include "hooker.h"
#include "archivefile.h"
#include "asciistring.h"
#include "copyprotect.h"
#include "force_nocd.h"
#include "gamememory.h"
#include "gamememoryinit.h"
#include "gamedebug.h"
#include "ini.h"
#include "main.h"
#include "win32gameengine.h"
#include "win32localfilesystem.h"
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

void Setup_Hooks()
{
    //
    // Hook WinMain
    //
    Hook_StdCall_Function((Make_StdCall_Ptr<int, HINSTANCE, HINSTANCE, LPSTR, int>(0x00401700)), Main_Func);

	//
	// Code that checks the launcher is running, launcher does CD check.
	//
    Hook_Function((Make_Function_Ptr<void, unsigned int, int>(0x00412420)), CopyProtect::checkForMessage);
    Hook_Function((Make_Function_Ptr<bool>(0x00412450)), CopyProtect::validate);

    //
    // Returns true for any CD checks
    //
    Hook_Function((Make_Function_Ptr<bool>(0x004469C0)), IsFirstCDPresent);
    
    //
    // Replace memory intialisation
    //
    Hook_Function((Make_Function_Ptr<void>(0x00414510)), Init_Memory_Manager);
    Hook_Function((Make_Function_Ptr<void>(0x004148C0)), Init_Memory_Manager_Pre_Main);


    //
    // Replace memory allocation operators
    //
    Hook_Function((Make_Function_Ptr<void *, size_t>(0x00414450)), New_New);    // operator new
    Hook_Function((Make_Function_Ptr<void *, size_t>(0x00414490)), New_New);    // operator new[]
    Hook_Function((Make_Function_Ptr<void, void *>(0x004144D0)), New_Delete);   // operator delete
    Hook_Function((Make_Function_Ptr<void, void *>(0x004144F0)), New_Delete);   // operator delete[]
    Hook_Function((Make_Function_Ptr<MemoryPool *, const char *, int>(0x00414B30)), Create_Named_Pool);
    
    //
    // Replace pool functions
    //
    Hook_Method((Make_Method_Ptr<void *, MemoryPool>(0x00413C10)), &MemoryPool::Allocate_Block);
    Hook_Method((Make_Method_Ptr<void, MemoryPool, void *>(0x00413C40)), &MemoryPool::Free_Block);

    //
    // Replace DMA functions
    //
    Hook_Method((Make_Method_Ptr<void *, DynamicMemoryAllocator, int>(0x00413EE0)), &DynamicMemoryAllocator::Allocate_Bytes_No_Zero);
    Hook_Method((Make_Method_Ptr<void *, DynamicMemoryAllocator, int>(0x00413FE0)), &DynamicMemoryAllocator::Allocate_Bytes);
    Hook_Method((Make_Method_Ptr<void, DynamicMemoryAllocator, void *>(0x00414010)), &DynamicMemoryAllocator::Free_Bytes);
    Hook_Method((Make_Method_Ptr<int, DynamicMemoryAllocator, int>(0x00414140)), &DynamicMemoryAllocator::Get_Actual_Allocation_Size);
    
    //
    // Replace MemoryPoolFactory functions
    //
    Hook_Method((Make_Method_Ptr<MemoryPool *, MemoryPoolFactory, char const *, int, int, int>(0x00414180)), static_cast<MemoryPool *(MemoryPoolFactory::*const)(char const*, int, int, int)>(&MemoryPoolFactory::Create_Memory_Pool));
    
    //
    // Replace File functions
    //
    Hook_Method((Make_Method_Ptr<LocalFileSystem*, Win32GameEngine>(0x007420F0)), &Win32GameEngine::Create_Local_File_System_NV);

    //
    // Replace AsciiString
    //
    Hook_Method((Make_Method_Ptr<void, AsciiString, char const *>(0x0040D640)), static_cast<void (AsciiString::*)(char const*)>(&AsciiString::Set));
    Hook_Method((Make_Method_Ptr<void, AsciiString, int, bool, char const *, char const *>(0x00415290)), &AsciiString::Ensure_Unique_Buffer_Of_Size);
    Hook_Method((Make_Method_Ptr<void, AsciiString, char const *>(0x0040FB40)), static_cast<void (AsciiString::*)(char const*)>(&AsciiString::Concat));

    //
    // Replace INI
    //
    Hook_Method((Make_Method_Ptr<char *, INI, char const*>(0x0041D6E0)), &INI::Get_Next_Token);
    Hook_Method((Make_Method_Ptr<char *, INI, char const*>(0x0041D720)), &INI::Get_Next_Token_Or_Null);
    Hook_Method((Make_Method_Ptr<char *, INI, char const*>(0x0041D950)), &INI::Get_Next_Sub_Token);
    Hook_Method((Make_Method_Ptr<void, INI, AsciiString, INILoadType>(0x0041A4B0)), &INI::Prep_File);
    Hook_Method((Make_Method_Ptr<void, INI, void *, MultiIniFieldParse const &>(0x0041D460)), &INI::Init_From_INI_Multi);
    Hook_Method((Make_Method_Ptr<void, INI, AsciiString, INILoadType, Xfer*>(0x0041A5C0)), &INI::Load);
    // Field parsing functions
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041ADA0)), &INI::Parse_Bool);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041A980)), &INI::Parse_Byte);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041AAB0)), &INI::Parse_Int);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041AB20)), &INI::Parse_Unsigned);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041AB90)), &INI::Parse_Real);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041AC00)), &INI::Parse_Positive_None_Zero_Real);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041BA50)), &INI::Parse_Percent_To_Real);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041ACA0)), &INI::Parse_Angle_Real);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041AD20)), &INI::Parse_Angular_Velocity_Real);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041AF20)), &INI::Parse_AsciiString);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041B1B0)), &INI::Parse_AsciiString_Vector_Append);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041BDD0)), &INI::Parse_RGB_Color);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041C100)), &INI::Parse_Color_Int);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041C530)), &INI::Parse_Coord2D);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041C2C0)), &INI::Parse_Coord3D);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041D140)), &INI::Parse_Index_List);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041DB50)), &INI::Parse_Duration_Real);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041DBD0)), &INI::Parse_Duration_Int);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041DD10)), &INI::Parse_Velocity_Real);
    Hook_Function((Make_Function_Ptr<void, INI*, void*, void*, void const*>(0x0041DD90)), &INI::Parse_Acceleration_Real);
}

// Use DLLMain to Set up our hooks when the DLL loads. The launcher should stall
// the main thread at the entry point so hooked code called after that should
// be our code.
BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch ( ul_reason_for_call ) {

        case DLL_PROCESS_ATTACH:
			StartHooking();
            Setup_Hooks();
            break;

        case DLL_PROCESS_DETACH:
            StopHooking();
            break;
            
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;

    }

    return TRUE;
}
