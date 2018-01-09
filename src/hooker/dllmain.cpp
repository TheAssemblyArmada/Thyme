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
#include "filesystem.h"
#include "archivefile.h"
#include "archivefilesystem.h"
#include "asciistring.h"
#include "chunkio.h"
#include "copyprotect.h"
#include "commandline.h"
#include "commandlist.h"
#include "compressionmanager.h"
#include "datachunk.h"
#include "dict.h"
#include "filetransfer.h"
#include "force_nocd.h"
#include "gamememory.h"
#include "gamememoryinit.h"
#include "gamedebug.h"
#include "gamestate.h"
#include "gametext.h"
#include "globaldata.h"
#include "ini.h"
#include "main.h"
#include "messagestream.h"
#include "namekeygenerator.h"
#include "randomvalue.h"
#include "script.h"
#include "scriptaction.h"
#include "scriptcondition.h"
#include "scriptgroup.h"
#include "scriptlist.h"
#include "targa.h"
#include "teamsinfo.h"
#include "thread.h"
#include "w3dfilesystem.h"
#include "weapon.h"
#include "win32gameengine.h"
#include "win32localfilesystem.h"
#include "wwstring.h"
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

void Setup_Hooks()
{
    // Hook WinMain
    Hook_Function(0x00401700, Main_Func);

	// Code that checks the launcher is running, launcher does CD check.
    Hook_Function(0x00412420, CopyProtect::checkForMessage);
    Hook_Function(0x00412450, CopyProtect::validate);

    // Returns true for any CD checks
    Hook_Function(0x005F1CB0, IsFirstCDPresent);
    
    // Replace memory intialisation
    Hook_Function(0x00414510, Init_Memory_Manager);
    Hook_Function(0x004148C0, Init_Memory_Manager_Pre_Main);

    // Replace memory allocation operators
    Hook_Function(0x00414450, New_New);    // operator new
    Hook_Function(0x00414490, New_New);    // operator new[]
    Hook_Function(0x004144D0, New_Delete);   // operator delete
    Hook_Function(0x004144F0, New_Delete);   // operator delete[]
    Hook_Function(0x00414B30, Create_Named_Pool);
    
    // Replace pool functions
    Hook_Method(0x00413C10, &MemoryPool::Allocate_Block);
    Hook_Method(0x00413C40, &MemoryPool::Free_Block);

    // Replace DMA functions
    Hook_Method(0x00413EE0, &DynamicMemoryAllocator::Allocate_Bytes_No_Zero);
    Hook_Method(0x00413FE0, &DynamicMemoryAllocator::Allocate_Bytes);
    Hook_Method(0x00414010, &DynamicMemoryAllocator::Free_Bytes);
    Hook_Method(0x00414140, &DynamicMemoryAllocator::Get_Actual_Allocation_Size);
    
    // Replace MemoryPoolFactory functions
    Hook_Method(0x00414180, static_cast<MemoryPool *(MemoryPoolFactory::*const)(char const*, int, int, int)>(&MemoryPoolFactory::Create_Memory_Pool));
    
    // Replace File functions
    FileSystem::Hook_Me();
    ArchiveFileSystem::Hook_Me();

    // Replace AsciiString
    Hook_Method(0x0040D640, static_cast<void (AsciiString::*)(char const*)>(&AsciiString::Set));
    Hook_Method(0x00415290, &AsciiString::Ensure_Unique_Buffer_Of_Size);
    Hook_Method(0x0040FB40, static_cast<void (AsciiString::*)(char const*)>(&AsciiString::Concat));

    Win32GameEngine::Hook_Me();
    INI::Hook_Me();
    NameKeyGenerator::Hook_Me();
    GameTextManager::Hook_Me();
    GameMessageList::Hook_Me();
    CommandList::Hook_Me();
    RandomValue::Hook_Me();
    CommandLine::Hook_Me();
    W3DFileSystem::Hook_Me();
    ChunkSaveClass::Hook_Me();
    ChunkLoadClass::Hook_Me();
    TargaImage::Hook_Me();
    FileTransfer::Hook_Me();
    GameState::Hook_Me();
    WeaponBonusSet::Hook_Me();
    GlobalData::Hook_Me();
    ThreadClass::Hook_Me();
    StringClass::Hook_Me();
    MessageStream::Hook_Me();
    Dict::Hook_Me();
    CompressionManager::Hook_Me();
    DataChunkInput::Hook_Me();
    TeamsInfoRec::Hook_Me();
    Condition::Hook_Me();
    OrCondition::Hook_Me();
    ScriptAction::Hook_Me();
    Script::Hook_Me();
    ScriptGroup::Hook_Me();
    ScriptList::Hook_Me();
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
