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

#include "gameengine.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

class Win32GameEngine : public GameEngine
{
public:
    Win32GameEngine();
    virtual ~Win32GameEngine();

    // SubsystemInterface implementations, the original appears to do this to no benefit?
    virtual void Init() override { GameEngine::Init(); }
    virtual void Reset() override { GameEngine::Reset(); }
    virtual void Update() override { GameEngine::Update(); }

    // GameEngine interface
    virtual void Service_Windows_OS() override;
    virtual LocalFileSystem *Create_Local_File_System() override;
    virtual ArchiveFileSystem *Create_Archive_File_System() override;
    virtual GameLogic *Create_Game_Logic() override;
    virtual GameClient *Create_Game_Client() override;
    virtual ModuleFactory *Create_Module_Factory() override;
    virtual ThingFactory *Create_Thing_Factory() override;
    virtual FunctionLexicon *Create_Function_Lexicon() override;
    virtual Radar *Create_Radar() override;
    virtual WebBrowser *Create_Web_Browser() override;
    virtual ParticleSystemManager *Create_Particle_System_Manager() override;
    virtual AudioManager *Create_Audio_Manager() override;
    virtual Network *Create_Network() override;

#ifdef GAME_DLL
    LocalFileSystem *Create_Local_File_System_NV();
    ArchiveFileSystem *Create_Archive_File_System_NV();
    ModuleFactory *Hook_Create_Module_Factory();
    static void Hook_Me();
#endif
private:
    unsigned int m_previousErrorMode;
};

#ifdef GAME_DLL
inline void Win32GameEngine::Hook_Me()
{
    Hook_Method(0x007420F0, &Create_Local_File_System_NV);
    Hook_Method(0x00742150, &Create_Archive_File_System_NV);
    Hook_Method(0x00741FA0, &Hook_Create_Module_Factory);
}
#endif