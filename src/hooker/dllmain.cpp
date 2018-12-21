/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Defines the DLL entry point and performs initial hooking.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "archivefile.h"
#include "archivefilesystem.h"
#include "asciistring.h"
#include "binkvideoplayer.h"
#include "binkvideostream.h"
#include "cavesystem.h"
#include "chunkio.h"
#include "commandline.h"
#include "commandlist.h"
#include "compressionmanager.h"
#include "copyprotect.h"
#include "datachunk.h"
#include "ddsfile.h"
#include "dict.h"
#include "dx8caps.h"
#include "filesystem.h"
#include "filetransfer.h"
#include "force_nocd.h"
#include "gamedebug.h"
#include "gamemath.h"
#include "gamememory.h"
#include "gamememoryinit.h"
#include "gamemessage.h"
#include "gamestate.h"
#include "gametext.h"
#include "geometry.h"
#include "globaldata.h"
#include "hooker.h"
#include "ini.h"
#include "keyboard.h"
#include "main.h"
#include "messagestream.h"
#include "modulefactory.h"
#include "mouse.h"
#include "multilist.h"
#include "namekeygenerator.h"
#include "particle.h"
#include "particlesys.h"
#include "particlesysinfo.h"
#include "particlesysmanager.h"
#include "playertemplate.h"
#include "randomvalue.h"
#include "rankinfo.h"
#include "script.h"
#include "scriptaction.h"
#include "scriptcondition.h"
#include "scriptgroup.h"
#include "scriptlist.h"
#include "shader.h"
#include "sidesinfo.h"
#include "sideslist.h"
#include "targa.h"
#include "teamsinfo.h"
#include "texturebase.h"
#include "thread.h"
#include "w3dfilesystem.h"
#include "weapon.h"
#include "win32gameengine.h"
#include "win32localfilesystem.h"
#include "wwstring.h"
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>

struct hostent *__stdcall cnconline_hook(const char *name)
{
    if (strcmp(name, "gamestats.gamespy.com") == 0) {
        return gethostbyname("gamestats.server.cnc-online.net");
    }

    if (strcmp(name, "master.gamespy.com") == 0) {
        return gethostbyname("master.server.cnc-online.net");
    }

    if (strcmp(name, "peerchat.gamespy.com") == 0) {
        return gethostbyname("peerchat.server.cnc-online.net");
    }

    if (strcmp(name, "gpcm.gamespy.com") == 0) {
        return gethostbyname("gpcm.server.cnc-online.net");
    }

    if (strcmp(name, "master.gamespy.com") == 0 || strcmp(name, "ccgenerals.ms19.gamespy.com") == 0
        || strcmp(name, "ccgenzh.ms6.gamespy.com") == 0) {
        return gethostbyname("master.server.cnc-online.net");
    }

    if (strcmp(name, "servserv.generals.ea.com") == 0) {
        return gethostbyname("http.server.cnc-online.net");
    }

    if (strcmp(name, "www.gamespy.com") == 0 || strcmp(name, "ingamead.gamespy.com") == 0) {
        return gethostbyname("server.cnc-online.net");
    }

    return gethostbyname(name);
}

void Setup_Hooks()
{
    // Hook WinMain
    Hook_Function(0x00401700, Main_Func);

    Hook_Function(0x007F5B06, cnconline_hook);

    // Code that checks the launcher is running, launcher does CD check.
    Hook_Function(0x00412420, CopyProtect::checkForMessage);
    Hook_Function(0x00412450, CopyProtect::validate);

    // Returns true for any CD checks
    Hook_Function(0x005F1CB0, IsFirstCDPresent);

    // Replace memory init functions.
    GameMemory::Hook_Me();

    // Replace pool functions
    Hook_Method(0x00413C10, &MemoryPool::Allocate_Block);
    Hook_Method(0x00413C40, &MemoryPool::Free_Block);

    // Replace DMA functions
    Hook_Method(0x00413EE0, &DynamicMemoryAllocator::Allocate_Bytes_No_Zero);
    Hook_Method(0x00413FE0, &DynamicMemoryAllocator::Allocate_Bytes);
    Hook_Method(0x00414010, &DynamicMemoryAllocator::Free_Bytes);
    Hook_Method(0x00414140, &DynamicMemoryAllocator::Get_Actual_Allocation_Size);

    // Replace MemoryPoolFactory functions
    Hook_Method(0x00414180,
        static_cast<MemoryPool *(MemoryPoolFactory::*const)(char const *, int, int, int)>(
            &MemoryPoolFactory::Create_Memory_Pool));

    // Replace File functions
    FileSystem::Hook_Me();
    ArchiveFileSystem::Hook_Me();

    // Replace Utf8String
    Hook_Method(0x0040D640, static_cast<void (Utf8String::*)(char const *)>(&Utf8String::Set));
    Hook_Method(0x00415290, &Utf8String::Ensure_Unique_Buffer_Of_Size);
    Hook_Method(0x0040FB40, static_cast<void (Utf8String::*)(char const *)>(&Utf8String::Concat));

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
    CaveSystem::Hook_Me();
    TeamsInfoRec::Hook_Me();
    Condition::Hook_Me();
    OrCondition::Hook_Me();
    ScriptAction::Hook_Me();
    Script::Hook_Me();
    ScriptGroup::Hook_Me();
    ScriptList::Hook_Me();
    SidesInfo::Hook_Me();
    SidesList::Hook_Me();
    Keyboard::Hook_Me();
    Mouse::Hook_Me();
    GameMessage::Hook_Me();
    GeometryInfo::Hook_Me();
    ModuleFactory::Hook_Me();
    RankInfoStore::Hook_Me();
    PlayerTemplateStore::Hook_Me();
    ParticleSystemInfo::Hook_Me();
    Particle::Hook_Me();
    ParticleSystem::Hook_Me();
    ParticleSystemManager::Hook_Me();
    DDSFileClass::Hook_Me();
    ShaderClass::Hook_Me();
    GenericMultiListClass::Hook_Me();
    TextureBaseClass::Hook_Me();
    DX8Caps::Hook_Me();
    BinkVideoPlayer::Hook_Me();
    BinkVideoStream::Hook_Me();

    Hook_Function(0x00537580, &GameMath::Sin);
    Hook_Function(0x00537590, &GameMath::Cos);
    Hook_Function(0x005375B0, &GameMath::Asin);
    Hook_Function(0x005375A0, &GameMath::Acos);
}

// Use DLLMain to Set up our hooks when the DLL loads. The launcher should stall
// the main thread at the entry point so hooked code called after that should
// be our code.
BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
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
