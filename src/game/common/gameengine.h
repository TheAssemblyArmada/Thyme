////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEENGINE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Interface for the game engine implementation.
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
#pragma once

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "subsysteminterface.h"
#include "hooker.h"

class ArchiveFileSystem;
class AudioManager;
class FileSystem;
class FunctionLexicon;
class GameClient;
class GameLogic;
class LocalFileSystem;
class MessageStream;
class ModuleFactory;
class Network;
class ParticleSystemManager;
class Radar;
class ThingFactory;
class WebBrowser;

class GameEngine : public SubsystemInterface
{
public:
    GameEngine();
    virtual ~GameEngine();

    // SubsystemInterface implementations
    virtual void Init() {}
    virtual void Reset();
    virtual void Update();

    // GameEngine interface
    virtual void Init(int argc, char **argv);
    virtual void Execute();
    virtual void Set_FPS_Limit(int limit);
    virtual int Get_FPS_Limit();
    virtual void Set_Quitting(bool quitting);
    virtual void Get_Quitting();
    virtual bool Is_Multiplayer_Session();
    virtual void Service_Windows_OS();
    virtual bool Get_Is_Active();
    virtual void Set_Is_Active();
    virtual FileSystem *Create_File_System();
    virtual LocalFileSystem *Create_Local_File_System() = 0;
    virtual ArchiveFileSystem *Create_Archive_File_System() = 0;
    virtual GameLogic *Create_Game_Logic() = 0;
    virtual GameClient *Create_Game_Client() = 0;
    virtual MessageStream *Create_Message_Stream();
    virtual ModuleFactory *Create_Module_Factory() = 0;
    virtual ThingFactory *Create_Thing_Factory() = 0;
    virtual FunctionLexicon *Create_Function_Lexicon() = 0;
    virtual Radar *Create_Radar() = 0;
    virtual WebBrowser *Create_Web_Browser() = 0;
    virtual ParticleSystemManager *Create_Particle_System_Manager() = 0;
    virtual AudioManager *Create_Audio_Manager() = 0;
    virtual Network *Create_Network() = 0;

protected:
    int MaxFPS;
    bool Quitting;
    bool Active;
};

#define g_theGameEngine Make_Global<GameEngine*>(0x00A29B80)
//extern GameEngine *g_theGameEngine;

#endif // _GAMEENGINE_H
