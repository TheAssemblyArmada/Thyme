/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for the game engine implementation.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "subsysteminterface.h"

class ArchiveFileSystem;
class AudioManager;
class FileSystem;
class FunctionLexicon;
class GameClient;
class GameLogic;
class LocalFileSystem;
class MessageStream;
class ModuleFactory;
class NetworkInterface;
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
    virtual void Init(int argc, char *argv[]);
    virtual void Execute();
    virtual void Set_FPS_Limit(int limit) { m_maxFPS = limit; }
    virtual int Get_FPS_Limit() { return m_maxFPS; }
    virtual void Set_Quitting(bool quitting) { m_isQuitting = quitting; }
    virtual bool Get_Quitting() { return m_isQuitting; }
    virtual bool Is_Multiplayer_Session();
    virtual void Service_Windows_OS() {}
    virtual bool Get_Is_Active() { return m_isActive; }
    virtual void Set_Is_Active(bool active) { m_isActive = active; }
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
    virtual NetworkInterface *Create_Network() = 0;

    void Real_Init(int argc, char *argv[]); // temporary thing to make hooking work since Hook_Any wont work because there
                                            // are 2 functions named Init and Hook_Method wont work because Init is virtual

protected:
    int m_maxFPS;
    bool m_isQuitting;
    bool m_isActive;
};

#ifdef GAME_DLL
extern GameEngine *&g_theGameEngine;
#else
extern GameEngine *g_theGameEngine;
#endif