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
    ~Win32GameEngine() override;

    // SubsystemInterface implementations, the original appears to do this to no benefit?
    void Init() override { GameEngine::Init(); }
    void Reset() override { GameEngine::Reset(); }
    void Update() override;

    // GameEngine interface
    void Service_Windows_OS() override;
    LocalFileSystem *Create_Local_File_System() override;
    ArchiveFileSystem *Create_Archive_File_System() override;
    GameLogic *Create_Game_Logic() override;
    GameClient *Create_Game_Client() override;
    ModuleFactory *Create_Module_Factory() override;
    ThingFactory *Create_Thing_Factory() override;
    FunctionLexicon *Create_Function_Lexicon() override;
    Radar *Create_Radar() override;
    WebBrowser *Create_Web_Browser() override;
    ParticleSystemManager *Create_Particle_System_Manager() override;
    AudioManager *Create_Audio_Manager() override;
    NetworkInterface *Create_Network() override;

private:
    unsigned int m_previousErrorMode;
};
