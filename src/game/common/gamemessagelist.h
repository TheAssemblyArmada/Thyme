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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _GAMEMESSAGELIST_H_
#define _GAMEMESSAGELIST_H_

#include "subsysteminterface.h"
#include "gamemessage.h"

class GameMessageList : public SubsystemInterface
{
public:
    GameMessageList() : m_firstMessage(nullptr), m_lastMessage(nullptr) {}

    // SubsystemInterface implementations
    virtual ~GameMessageList() {}
    virtual void Init() {}
    virtual void Reset() {}
    virtual void Update() {}
    
    virtual void Append_Message(GameMessage *msg);
    virtual void Insert_Message(GameMessage *msg, GameMessage *at);
    virtual void Remove_Message(GameMessage *msg);
    virtual bool Contains_Message_Of_Type(MessageType type);

protected:
    GameMessage *m_firstMessage;
    GameMessage *m_lastMessage;
};

#endif
