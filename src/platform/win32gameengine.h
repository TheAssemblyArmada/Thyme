/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Implementation of the GameEngine interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef WIN32GAMEENGINE_H
#define WIN32GAMEENGINE_H

#include "gameengine.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

class Win32GameEngine : public GameEngine
{
public:
    Win32GameEngine();
    virtual ~Win32GameEngine();

    // SubsystemInterface implementations
    virtual void Init() { GameEngine::Init(); }
    virtual void Reset() { GameEngine::Reset(); }
    virtual void Update() { GameEngine::Update(); }

    // GameEngine interface
    virtual void Service_Windows_OS();
    virtual LocalFileSystem *Create_Local_File_System();
    virtual ArchiveFileSystem *Create_Archive_File_System();
    virtual GameLogic *Create_Game_Logic();
    virtual GameClient *Create_Game_Client();
    virtual ModuleFactory *Create_Module_Factory();
    virtual ThingFactory *Create_Thing_Factory();
    virtual FunctionLexicon *Create_Function_Lexicon();
    virtual Radar *Create_Radar();
    virtual WebBrowser *Create_Web_Browser();
    virtual ParticleSystemManager *Create_Particle_System_Manager();
    virtual AudioManager *Create_Audio_Manager();
    virtual Network *Create_Network();

#ifndef THYME_STANDALONE
    LocalFileSystem *Create_Local_File_System_NV();
    ArchiveFileSystem *Create_Archive_File_System_NV();
    static void Hook_Me();
#endif
private:
    unsigned int m_previousErrorMode;
};

#ifndef THYME_STANDALONE
inline void Win32GameEngine::Hook_Me()
{
    Hook_Method(0x007420F0, &Create_Local_File_System_NV);
    Hook_Method(0x00742150, &Create_Archive_File_System_NV);
}
#endif

#endif // _WIN32GAMEENGINE_H