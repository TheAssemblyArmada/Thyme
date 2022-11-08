/**
 * @file
 *
 * @author feliwir
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
#include "sdl2gameengine.h"
#include "audiomanager.h"
#include "main.h"
#include "messagestream.h"
#include "sdl2keybd.h"
#include "sdl2mouse.h"
#include "w3dfunctionlexicon.h"
#include "w3dmodulefactory.h"
#include "win32bigfilesystem.h"
#include "win32localfilesystem.h"

#ifdef BUILD_WITH_STDFS
#include "stdlocalfilesystem.h"
#endif

SDL_Window *g_applicationWindow = nullptr;

namespace Thyme
{
SDL2GameEngine::SDL2GameEngine() {}

SDL2GameEngine::~SDL2GameEngine() {}

void SDL2GameEngine::Handle_Window_Event(SDL_Event *ev)
{
    switch (ev->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            if (g_theMouse != nullptr) {
                g_theMouse->Set_Mouse_Limits();
            }
            if (g_theAudio != nullptr) {
                g_theAudio->Regain_Focus();
            }
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            if (g_theKeyboard != nullptr) {
                g_theKeyboard->Reset_Keys();
            }
            // TODO: handle mouse?
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            if (g_theKeyboard != nullptr) {
                g_theKeyboard->Reset_Keys();
            }
            // TODO: handle mouse?
            break;
    }
}

// Use this function to handle SDL2 events. Similar to win32gameengine & Wnd_Proc
void SDL2GameEngine::Service_Windows_OS()
{
    SDL_Event event;
    // Poll until all events are handled!
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                if (g_theGameEngine->Get_Quitting()) {
                    if (g_theKeyboard != nullptr) {
                        g_theKeyboard->Reset_Keys();
                    }
                    break;
                }
                // TODO: we can remove this once game messages are handled
                // g_theMessageStream->Append_Message(GameMessage::MessageType::MSG_META_DEMO_INSTANT_QUIT);
                g_theGameEngine->Set_Quitting(true);
                break;
            case SDL_WINDOWEVENT:
                Handle_Window_Event(&event);
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                if (g_theSDL2Mouse != nullptr) {
                    g_theSDL2Mouse->Add_SDL2_Event(&event);
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (g_theSDL2Keyboard != nullptr) {
                    g_theSDL2Keyboard->Add_SDL2_Event(&event);
                }
                break;
        }
    }
} // namespace Thyme

void SDL2GameEngine::Execute()
{
    while (!m_isQuitting) {
        Service_Windows_OS();
    }
}

LocalFileSystem *SDL2GameEngine::Create_Local_File_System()
{
#ifdef BUILD_WITH_STDFS
    return new StdLocalFileSystem;
#else
    return new Win32LocalFileSystem;
#endif
}

ArchiveFileSystem *SDL2GameEngine::Create_Archive_File_System()
{
    return new Win32BIGFileSystem;
}

GameLogic *SDL2GameEngine::Create_Game_Logic()
{
    return nullptr;
}

GameClient *SDL2GameEngine::Create_Game_Client()
{
    return nullptr;
}

ModuleFactory *SDL2GameEngine::Create_Module_Factory()
{
    return new W3DModuleFactory;
}

ThingFactory *SDL2GameEngine::Create_Thing_Factory()
{
    return nullptr;
}

FunctionLexicon *SDL2GameEngine::Create_Function_Lexicon()
{
    return new W3DFunctionLexicon;
}

Radar *SDL2GameEngine::Create_Radar()
{
    return nullptr;
}

WebBrowser *SDL2GameEngine::Create_Web_Browser()
{
    return nullptr;
}

ParticleSystemManager *SDL2GameEngine::Create_Particle_System_Manager()
{
    return nullptr;
}

AudioManager *SDL2GameEngine::Create_Audio_Manager()
{
    return nullptr;
}

NetworkInterface *SDL2GameEngine::Create_Network()
{
    return nullptr;
}
} // namespace Thyme
