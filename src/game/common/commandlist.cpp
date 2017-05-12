////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: COMMANDLIST.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Command message list handling.
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
#include "commandlist.h"
#include "gamemessage.h"

//CommandList *g_theCommandList = nullptr;

void CommandList::Destroy_All_Messages()
{
    GameMessage *next = nullptr;

    for ( GameMessage *msg = m_firstMessage; msg != nullptr; msg = next) {
        next = msg->Get_Next();
        Delete_Instance(msg);
    }

    m_firstMessage = nullptr;
    m_lastMessage = nullptr;
}

void CommandList::Append_Message_List(GameMessage *list)
{
    for ( GameMessage *msg = m_firstMessage; msg != nullptr; msg = msg->Get_Next() ) {
        Append_Message(msg);
    }
}

CommandList::~CommandList()
{
    Destroy_All_Messages();
}

void CommandList::Reset()
{
    Destroy_All_Messages();
}
