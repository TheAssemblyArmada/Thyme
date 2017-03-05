////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WIN32GAMEENGINE.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Implementation of the GameEngine interface.
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
#include "win32gameengine.h"

Win32GameEngine::Win32GameEngine()
{
}

Win32GameEngine::~Win32GameEngine()
{
}

void Win32GameEngine::Service_Windows_OS()
{
}

LocalFileSystem *Win32GameEngine::Create_Local_File_System()
{
    return nullptr;
}

ArchiveFileSystem *Win32GameEngine::Create_Archive_File_System()
{
    return nullptr;
}

GameLogic *Win32GameEngine::Create_Game_Logic()
{
    return nullptr;
}

GameClient *Win32GameEngine::Create_Game_Client()
{
    return nullptr;
}

ModuleFactory *Win32GameEngine::Create_Module_Factory()
{
    return nullptr;
}

ThingFactory *Win32GameEngine::Create_Thing_Factory()
{
    return nullptr;
}

FunctionLexicon *Win32GameEngine::Create_Function_Lexicon()
{
    return nullptr;
}

Radar *Win32GameEngine::Create_Radar()
{
    return nullptr;
}

WebBrowser *Win32GameEngine::Create_Web_Browser()
{
    return nullptr;
}

ParticleSystemManager *Win32GameEngine::Create_Particle_System_Manager()
{
    return nullptr;
}

AudioManager *Win32GameEngine::Create_Audio_Manager()
{
    return nullptr;
}

Network *Win32GameEngine::Create_Network()
{
    return nullptr;
}
