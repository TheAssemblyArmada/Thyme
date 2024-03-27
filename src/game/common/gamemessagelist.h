/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Message list handling.
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
#include "gamemessage.h"
#include "subsysteminterface.h"

class GameMessageList : public SubsystemInterface
{
public:
    GameMessageList() : m_firstMessage(nullptr), m_lastMessage(nullptr) {}

    // SubsystemInterface implementations
    virtual ~GameMessageList() override;
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    virtual void Append_Message(GameMessage *msg);
    virtual void Insert_Message(GameMessage *msg, GameMessage *at);
    virtual void Remove_Message(GameMessage *msg);
    virtual bool Contains_Message_Of_Type(GameMessage::MessageType type);

    GameMessage *Get_First_Message() { return m_firstMessage; }

protected:
    GameMessage *m_firstMessage;
    GameMessage *m_lastMessage;
};
