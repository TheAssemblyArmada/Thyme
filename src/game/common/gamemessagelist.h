/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Message list handling.
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

#include "gamemessage.h"
#include "subsysteminterface.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

class GameMessageList : public SubsystemInterface
{
public:
    GameMessageList() : m_firstMessage(nullptr), m_lastMessage(nullptr) {}

    // SubsystemInterface implementations
    virtual ~GameMessageList();
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    virtual void Append_Message(GameMessage *msg);
    virtual void Insert_Message(GameMessage *msg, GameMessage *at);
    virtual void Remove_Message(GameMessage *msg);
    virtual bool Contains_Message_Of_Type(GameMessage::MessageType type);

    GameMessage *Get_First_Message() { return m_firstMessage; }

#ifndef THYME_STANDALONE
    // Duplicates to test functionality of virtual members with hooking.
    void Append_Message_Nv(GameMessage *msg);
    void Insert_Message_Nv(GameMessage *msg, GameMessage *at);
    void Remove_Message_Nv(GameMessage *msg);
    bool Contains_Message_Of_Type_Nv(GameMessage::MessageType type);

    static void Hook_Me();
#endif
protected:
    GameMessage *m_firstMessage;
    GameMessage *m_lastMessage;
};

#ifndef THYME_STANDALONE
inline void GameMessageList::Hook_Me()
{
    Hook_Method(0x0040D760, &Append_Message_Nv);
    Hook_Method(0x0040D7A0, &Insert_Message_Nv);
    Hook_Method(0x0040D7D0, &Remove_Message_Nv);
    Hook_Method(0x0040D820, &Contains_Message_Of_Type_Nv);
}
#endif