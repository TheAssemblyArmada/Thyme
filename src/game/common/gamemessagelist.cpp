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
#include "gamemessagelist.h"

GameMessageList::~GameMessageList()
{
    for (GameMessage *data = m_firstMessage; data != nullptr;) {
        GameMessage *current = data;
        data = data->m_next;
        current->m_list = nullptr;
        current->Delete_Instance();
    }
}

void GameMessageList::Append_Message(GameMessage *msg)
{
    msg->m_next = nullptr;

    if (m_lastMessage != nullptr) {
        m_lastMessage->m_next = msg;
        msg->m_prev = m_lastMessage;
        m_lastMessage = msg;
    } else {
        m_firstMessage = msg;
        m_lastMessage = msg;
        msg->m_prev = nullptr;
    }

    msg->m_list = this;
}

void GameMessageList::Insert_Message(GameMessage *msg, GameMessage *at)
{
    msg->m_next = at->m_next;
    msg->m_prev = at;

    if (at->m_next) {
        at->m_next->m_prev = msg;
    } else {
        m_lastMessage = msg;
    }

    at->m_next = msg;
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
