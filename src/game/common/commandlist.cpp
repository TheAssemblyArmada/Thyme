/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Command message list handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "commandlist.h"
#include "gamemessage.h"

#ifndef GAME_DLL
CommandList *g_theCommandList = nullptr;
#endif

CommandList::~CommandList()
{
    Destroy_All_Messages();
}

void CommandList::Init()
{
    GameMessageList::Init();
}

void CommandList::Reset()
{
    GameMessageList::Reset();
    Destroy_All_Messages();
}

void CommandList::Update()
{
    GameMessageList::Update();
}

void CommandList::Destroy_All_Messages()
{
    GameMessage *next = nullptr;

    for (GameMessage *msg = m_firstMessage; msg != nullptr; msg = next) {
        next = msg->Get_Next();
        msg->Delete_Instance();
    }

    m_firstMessage = nullptr;
    m_lastMessage = nullptr;
}

void CommandList::Append_Message_List(GameMessage *list)
{
    for (GameMessage *msg = list; msg != nullptr;) {
        GameMessage *next = msg->Get_Next();
        Append_Message(msg);
        msg = next;
    }
}
