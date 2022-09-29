/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling message streams.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "messagestream.h"
#include "commandlist.h"
#include "gamelogic.h"

#ifndef GAME_DLL
MessageStream *g_theMessageStream = nullptr;
#endif

bool Is_Invalid_Debug_Command(GameMessage::MessageType type)
{
#ifndef GAME_DEBUG_STRUCTS
    // todo later
#endif

    return false;
}

MessageStream::~MessageStream()
{
    for (TranslatorData *data = m_firstTranslator; data != nullptr;) {
        TranslatorData *current = data;
        data = data->m_next;

        if (current->m_translator != nullptr) {
            delete current->m_translator;
        }

        delete current;
    }
}

/**
 * @brief Add a message to the end of the stream.
 */
GameMessage *MessageStream::Append_Message(GameMessage::MessageType type)
{
    GameMessage *msg = NEW_POOL_OBJ(GameMessage, type);
    GameMessageList::Append_Message(msg);

    return msg;
}

/**
 * @brief Add a message to stream just before the message provided.
 */
GameMessage *MessageStream::Insert_Message(GameMessage::MessageType type, GameMessage *at)
{
    GameMessage *msg = NEW_POOL_OBJ(GameMessage, type);
    GameMessageList::Insert_Message(msg, at);

    return msg;
}

/**
 * @brief Register a message translator to handle messages.
 */
unsigned MessageStream::Attach_Translator(GameMessageTranslator *translator, unsigned priority)
{
    TranslatorData *data = new TranslatorData();
    data->m_translator = translator;
    data->m_priority = priority;
    data->m_id = m_nextTranslatorID++;

    if (m_firstTranslator == nullptr) {
        data->m_next = nullptr;
        data->m_prev = nullptr;
        m_firstTranslator = data;
        m_lastTranslator = data;
    } else {
        TranslatorData *list = m_firstTranslator;

        // Find the entry in the list
        while (list->m_priority <= data->m_priority) {
            list = list->m_next;

            if (list == nullptr) {
                m_lastTranslator->m_next = data;
                data->m_prev = m_lastTranslator;
                data->m_next = 0;
                m_lastTranslator = data;

                return data->m_id;
            }
        }

        if (list->m_prev) {
            list->m_prev->m_next = data;
            data->m_prev = list->m_prev;
            data->m_next = list;
            list->m_prev = data;
        } else {
            data->m_prev = nullptr;
            data->m_next = m_firstTranslator;
            m_firstTranslator->m_prev = data;
            m_firstTranslator = data;
        }
    }

    return data->m_id;
}

/**
 * @brief Find a particular message translator.
 */
GameMessageTranslator *MessageStream::Find_Translator(unsigned id)
{
    if (m_firstTranslator == nullptr) {
        return nullptr;
    }

    for (TranslatorData *data = m_firstTranslator; data != nullptr; data = data->m_next) {
        if (data->m_id == id) {
            return data->m_translator;
        }
    }

    return nullptr;
}

/**
 * @brief Unregister a particular message translator.
 */
void MessageStream::Remove_Translator(unsigned id)
{
    for (TranslatorData *data = m_firstTranslator; data != nullptr; data = data->m_next) {
        if (data->m_id == id) {
            // Unlink the data entry we are about to remove.
            if (data->m_prev != nullptr) {
                data->m_prev->m_next = data->m_next;
            } else {
                m_firstTranslator = data->m_next;
            }

            if (data->m_next != nullptr) {
                data->m_next->m_prev = data->m_prev;
            } else {
                m_lastTranslator = data->m_prev;
            }

            delete data;

            return;
        }
    }
}

/**
 * @brief Process any messages we have translators for and pass the list on to CommandList.
 */
void MessageStream::Propagate_Messages()
{
    // Go through the message list and look for any messages that we have translators for.
    // If the translator tells us to remove the message we do before passing the remaining list
    // further on.
    for (TranslatorData *tdata = m_firstTranslator; tdata != nullptr; tdata = tdata->m_next) {
        for (GameMessage *msg = m_firstMessage; msg != nullptr;) {
            GameMessageTranslator *translator = tdata->m_translator;
            GameMessage *msg_next = nullptr;

            if (translator != nullptr && !Is_Invalid_Debug_Command(msg->Get_Type())) {
                GameMessageDisposition disp = translator->Translate_Game_Message(msg);
                // Translate can change the message so we need to get the next message after calling it.
                msg_next = msg->Get_Next();
                if (disp == DESTROY_MESSAGE) {
                    msg->Delete_Instance();
                }
            } else {
                msg_next = msg->Get_Next();
            }

            msg = msg_next;
        }
    }

    g_theCommandList->Append_Message_List(m_firstMessage);
    m_firstMessage = nullptr;
    m_lastMessage = nullptr;
}
