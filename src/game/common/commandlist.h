/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Command message list handling.
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

#include "gamemessagelist.h"

class CommandList : public GameMessageList
{
    ALLOW_HOOKING
public:
    // SubsystemInterface implementations
    virtual ~CommandList();
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

    void Append_Message_List(GameMessage *list);

private:
    void Destroy_All_Messages();
};

#ifdef GAME_DLL
#include "hooker.h"
extern CommandList *&g_theCommandList;
#else
extern CommandList *g_theCommandList;
#endif