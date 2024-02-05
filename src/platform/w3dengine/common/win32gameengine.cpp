/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implementation of the GameEngine interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "win32gameengine.h"
#include "audiomanager.h"
#include "lanapiinterface.h"
#include "main.h"
#include "thingfactory.h"
#include "w3dfunctionlexicon.h"
#include "w3dgameclient.h"
#include "w3dgamelogic.h"
#include "w3dmodulefactory.h"
#include "w3dparticlesys.h"
#include "win32bigfilesystem.h"
#include "win32localfilesystem.h"

#ifdef BUILD_WITH_OPENAL
#include "alaudiomanager.h"
#endif

#ifdef BUILD_WITH_MILES
#include "milesaudiomanager.h"
#endif

#ifdef PLATFORM_WINDOWS
#include <winuser.h>
#endif

#ifdef BUILD_WITH_STDFS
#include "stdlocalfilesystem.h"
#endif

Win32GameEngine::Win32GameEngine()
{
#ifdef PLATFORM_WINDOWS
    m_previousErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
}

Win32GameEngine::~Win32GameEngine()
{
#ifdef PLATFORM_WINDOWS
    SetErrorMode(m_previousErrorMode);
#endif
}

void Win32GameEngine::Update()
{
    GameEngine::Update();

    // this code exists in windows but not in mac
#ifdef PLATFORM_WINDOWS
    if (g_applicationHWnd != nullptr && IsIconic(g_applicationHWnd)) {
        for (HWND h = g_applicationHWnd; g_applicationHWnd != nullptr; h = g_applicationHWnd) {
            if (!IsIconic(h)) {
                break;
            }

            rts::Sleep_Ms(5);
            Service_Windows_OS();

            if (g_theLAN != nullptr) {
                g_theLAN->Set_Is_Active(Get_Is_Active());
                g_theLAN->Update();
            }

            if (Get_Quitting()) {
                break;
            }

            if (g_theGameLogic->Get_Game_Mode() == GAME_INTERNET || g_theGameLogic->Get_Game_Mode() == GAME_LAN) {
                break;
            }
        }

        g_theAudio->Set_Volume(g_theAudio->Get_Volume(AUDIOAFFECT_BASEVOL), AUDIOAFFECT_BASEVOL);
    }
#endif
    Service_Windows_OS();
}

void Win32GameEngine::Service_Windows_OS()
{
// Event loop.
#ifdef PLATFORM_WINDOWS
    BOOL result;
    MSG Msg;

    for (result = PeekMessageA(&Msg, 0, 0, 0, 0); result; result = PeekMessageA(&Msg, 0, 0, 0, 0)) {
        GetMessageA(&Msg, 0, 0, 0);
        g_theMessageTime = Msg.time;
        TranslateMessage(&Msg);
        DispatchMessageA(&Msg);
        g_theMessageTime = 0;
    }
#endif
}

LocalFileSystem *Win32GameEngine::Create_Local_File_System()
{
#ifdef BUILD_WITH_STDFS
    return new Thyme::StdLocalFileSystem;
#else
    return new Win32LocalFileSystem;
#endif
}

ArchiveFileSystem *Win32GameEngine::Create_Archive_File_System()
{
    return new Win32BIGFileSystem;
}

GameLogic *Win32GameEngine::Create_Game_Logic()
{
    return new W3DGameLogic;
}

GameClient *Win32GameEngine::Create_Game_Client()
{
    return new W3DGameClient();
}

ModuleFactory *Win32GameEngine::Create_Module_Factory()
{
    return new W3DModuleFactory;
}

ThingFactory *Win32GameEngine::Create_Thing_Factory()
{
    return new W3DThingFactory;
}

FunctionLexicon *Win32GameEngine::Create_Function_Lexicon()
{
    return new W3DFunctionLexicon;
}

Radar *Win32GameEngine::Create_Radar()
{
#ifdef GAME_DLL
    // only exists in game exe, not wb exe
    return Call_Method<Radar *, Win32GameEngine>(0x00742220, this);
#else
    return nullptr;
#endif
}

WebBrowser *Win32GameEngine::Create_Web_Browser()
{
    captainslog_relassert(false,
        0,
        "Web Browser class not implelented because it relies on old Internet Explorer code that isn't present in the "
        "versions of Windows we run on");
    return nullptr;
}

ParticleSystemManager *Win32GameEngine::Create_Particle_System_Manager()
{
    return new W3DParticleSystemManager();
}

AudioManager *Win32GameEngine::Create_Audio_Manager()
{
#ifdef BUILD_WITH_OPENAL
    return new Thyme::ALAudioManager;
#elif defined BUILD_WITH_MILES
    return new MilesAudioManager;
#else
    return nullptr;
#endif
}

NetworkInterface *Win32GameEngine::Create_Network()
{
#ifdef GAME_DLL
    // only exists in game exe, not wb exe
    return Call_Method<NetworkInterface *, Win32GameEngine>(0x00742210, this);
#else
    return nullptr;
#endif
}
