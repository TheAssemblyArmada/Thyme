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
#pragma once

#include "gameengine.h"

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
    LocalFileSystem *Hook_Create_Local_File_System() { return Win32GameEngine::Create_Local_File_System(); }
    ArchiveFileSystem *Hook_Create_Archive_File_System() { return Win32GameEngine::Create_Archive_File_System(); }
    ModuleFactory *Hook_Create_Module_Factory() { return Win32GameEngine::Create_Module_Factory(); }
    AudioManager *Hook_Create_Audio_Manager() { return Win32GameEngine::Create_Audio_Manager(); }
#endif
private:
    unsigned int m_previousErrorMode;
};
