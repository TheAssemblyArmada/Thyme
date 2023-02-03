/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Game Results Thread
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

class GameResultsRequest;
class GameResultsResponse;

class GameResultsInterface : public SubsystemInterface
{
public:
    virtual void Start_Threads() = 0;
    virtual void End_Threads() = 0;
    virtual bool Are_Threads_Running() = 0;
    virtual void Add_Request(GameResultsRequest const &request) = 0;
    virtual bool Get_Request(GameResultsRequest &request) = 0;
    virtual void Add_Response(GameResultsResponse const &response) = 0;
    virtual bool Get_Response(GameResultsResponse &response) = 0;
    virtual bool Are_Game_Results_Being_Sent() = 0;

    static GameResultsInterface *Create_New_Game_Results_Interface();
};

#ifdef GAME_DLL
extern GameResultsInterface *&g_theGameResultsQueue;
#else
extern GameResultsInterface *g_theGameResultsQueue;
#endif
