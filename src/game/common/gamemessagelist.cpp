////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEMESSAGELIST.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Message list handling.
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
#include "gamemessagelist.h"

void GameMessageList::Append_Message(GameMessage *msg)
{
    if ( LastMessage != nullptr ) {
        LastMessage->Next = msg;
        msg->Prev = LastMessage;
        LastMessage = msg;
    } else {
        FirstMessage = msg;
        msg->Prev = nullptr;
        LastMessage = msg;
    }

    msg->List = this;
}

void GameMessageList::Insert_Message(GameMessage *msg, GameMessage *at)
{
    msg->Next = at->Next;
    msg->Prev = at;

    if ( at->Next ) {
        at->Next->Prev = msg;
        at->Next = msg;
    } else {
        LastMessage = msg;
        at->Next = msg;
    }

    msg->List = this;
}

void GameMessageList::Remove_Message(GameMessage *msg)
{
    if ( msg->Next ) {
        msg->Next->Prev = msg->Prev;
    } else {
        LastMessage = msg->Prev;
    }

    if ( msg->Prev ) {
        msg->Prev->Next = msg->Next;
    } else {
        FirstMessage = msg->Next;
    }

    msg->List = nullptr;
}

bool GameMessageList::Contains_Message_Of_Type(MessageType type)
{
    GameMessage *msg = FirstMessage;

    while ( msg != nullptr ) {
        if ( msg->Type == type ) {
            return true;
        }

        msg = msg->Next;
    }

    return false;
}
