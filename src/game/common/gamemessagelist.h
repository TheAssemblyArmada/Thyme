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

    // Duplicates to test functionality of virtual members with hooking.
    void Append_Message_Nv(GameMessage *msg);
    void Insert_Message_Nv(GameMessage *msg, GameMessage *at);
    void Remove_Message_Nv(GameMessage *msg);
    bool Contains_Message_Of_Type_Nv(MessageType type);

    GameMessage *Get_First_Message() { return m_firstMessage; }

    static void Hook_Me();
protected:
    GameMessage *m_firstMessage;
    GameMessage *m_lastMessage;
};

inline void GameMessageList::Hook_Me()
{
    Hook_Method((Make_Method_Ptr<void, GameMessageList, GameMessage*>(0x0040D760)), &Append_Message_Nv);
    Hook_Method((Make_Method_Ptr<void, GameMessageList, GameMessage*, GameMessage*>(0x0040D7A0)), &Insert_Message_Nv);
    Hook_Method((Make_Method_Ptr<void, GameMessageList, GameMessage*>(0x0040D7D0)), &Remove_Message_Nv);
    Hook_Method((Make_Method_Ptr<bool, GameMessageList, MessageType>(0x0040D820)), &Contains_Message_Of_Type_Nv);
}

#endif
