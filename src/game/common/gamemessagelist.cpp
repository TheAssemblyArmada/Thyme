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
#include "gamemessagelist.h"
#include "gamemessage.h"

GameMessageList::~GameMessageList()
{
    for (GameMessage *data = m_firstMessage; data != nullptr;) {
        GameMessage *current = data;
        data = data->m_next;
        current->m_list = nullptr;
        Delete_Instance(current);
    }
}

void GameMessageList::Append_Message(GameMessage *msg)
{
    if (m_lastMessage != nullptr) {
        m_lastMessage->m_next = msg;
        msg->m_prev = m_lastMessage;
        m_lastMessage = msg;
    } else {
        m_firstMessage = msg;
        msg->m_prev = nullptr;
        m_lastMessage = msg;
    }

    msg->m_list = this;
}

void GameMessageList::Insert_Message(GameMessage *msg, GameMessage *at)
{
    msg->m_next = at->m_next;
    msg->m_prev = at;

    if (at->m_next) {
        at->m_next->m_prev = msg;
        at->m_next = msg;
    } else {
        m_lastMessage = msg;
        at->m_next = msg;
    }

    msg->m_list = this;
}

void GameMessageList::Remove_Message(GameMessage *msg)
{
    if (msg->m_next) {
        msg->m_next->m_prev = msg->m_prev;
    } else {
        m_lastMessage = msg->m_prev;
    }

    if (msg->m_prev) {
        msg->m_prev->m_next = msg->m_next;
    } else {
        m_firstMessage = msg->m_next;
    }

    msg->m_list = nullptr;
}

bool GameMessageList::Contains_Message_Of_Type(GameMessage::MessageType type)
{
    GameMessage *msg = m_firstMessage;

    while (msg != nullptr) {
        if (msg->m_type == type) {
            return true;
        }

        msg = msg->m_next;
    }

    return false;
}

#ifndef THYME_STANDALONE
void GameMessageList::Append_Message_Nv(GameMessage *msg)
{
    GameMessageList::Append_Message(msg);
}

void GameMessageList::Insert_Message_Nv(GameMessage *msg, GameMessage *at)
{
    GameMessageList::Insert_Message(msg, at);
}

void GameMessageList::Remove_Message_Nv(GameMessage *msg)
{
    GameMessageList::Remove_Message(msg);
}

bool GameMessageList::Contains_Message_Of_Type_Nv(GameMessage::MessageType type)
{
    return GameMessageList::Contains_Message_Of_Type(type);
}
#endif
