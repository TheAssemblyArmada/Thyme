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
#pragma once

#ifndef BUILD_WITH_SDL2
#error Only include sdl2gameengine.h when building with SDL2
#endif

#include "always.h"
#include "gameengine.h"

#include <SDL.h>

namespace Thyme
{
class SDL2GameEngine : public GameEngine
{
public:
    SDL2GameEngine();
    virtual ~SDL2GameEngine();

    // SubsystemInterface implementations, the original appears to do this to no benefit?
    virtual void Init() override { GameEngine::Init(); }
    virtual void Reset() override { GameEngine::Reset(); }
    virtual void Update() override { GameEngine::Update(); }

    // GameEngine interface
    virtual void Service_Windows_OS() override;
    virtual void Execute() override;
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
    virtual NetworkInterface *Create_Network() override;

private:
    void Handle_Window_Event(SDL_Event *ev);
};
} // namespace Thyme
